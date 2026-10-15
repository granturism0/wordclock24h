// Ladeprobe (Paket 2026-10-15, design.md §5.1): Laedt die PWA einmal kalt und fuenfmal warm
// in einem Chrome mit DAUERHAFTEM Profil und misst je Laden, was ueber die Leitung geht und
// wie tief der Heap des ESP dabei faellt.
//
// Wird von tools/ladeprobe.sh gestartet, das Chrome hochfaehrt und wieder abraeumt.
//
// WARUM ES DAS GIBT
//
// check-pwa.sh laedt mit frischem Profil -- also immer kalt. Ob der Browser beim ZWEITEN
// Oeffnen noch alle ~200 KB holt (C70), sieht man dort nie. Am 14.10.2026 fiel der groesste
// freie Block beim Neuladen der PWA auf 752 Byte (L466); gemessen hat das niemand gezielt.
//
// Je Laden: /app/-Anfragen nach Status (200, 304, aus dem Browser-Cache ohne Anfrage),
// Summe der uebertragenen Byte an /app/, und aus /api/device_ready den Tiefstwert seit dem
// Zuruecksetzen (heap_low_block, heap_low_path) samt der Zaehler, die das Geraet meldet.
//
// --erwarte-304: Ein WARMES Laden, das ein App-Asset mit 200 holt, ist ein Fehlschlag
// (Exit 1). Ausnahme: LADEPROBE_AUSNAHME (Pfad), solange ein Asset bewusst no-store bleibt.
//
// SCHREIBT AM GERAET NUR den Ruecksetzer der Tiefstwert-Messung (device_ready?heap_low_reset=1).

import { appendFileSync } from "node:fs";

const PORT = process.env.CDP_PORT;
const SOLL = process.env.GERAET_URL;          // http://<host>/app/
const MARKE = process.env.START_MARKE;
const ERWARTE_304 = process.env.ERWARTE_304 === "1";
const ENGLISCH = process.env.ENGLISCH === "1";
const MITSCHRIFT = process.env.LADEPROBE_MITSCHRIFT || "";
const AUSNAHME = process.env.LADEPROBE_AUSNAHME || "";
const WARM = Number(process.env.LADEPROBE_WARM || 5);
const RUHE_MS = 1500, MAX_MS = 30000;

const host = new URL(SOLL).host;
const basis = `http://${host}`;
const zeit = () => new Date().toTimeString().slice(0, 8);
const aus = z => { console.log(z); if (MITSCHRIFT) appendFileSync(MITSCHRIFT, z + "\n"); };

let fehl = 0;
const listeHolen = () => fetch(`http://127.0.0.1:${PORT}/json/list`).then(r => r.json());
let ziele = await listeHolen(), seite;
for (let v = 0; v < 25 && !(seite = ziele.find(t => t.type === "page" && t.url === MARKE)); v++) {
  await new Promise(r => setTimeout(r, 200));
  ziele = await listeHolen();
}
if (!seite) { aus(`ABBRUCH: kein Seitenziel mit der eigenen Startmarke`); process.exit(2); }

const ws = new WebSocket(seite.webSocketDebuggerUrl);
await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
let seq = 0;
const wartend = new Map();
const send = (method, params = {}) => new Promise(res => {
  const id = ++seq; wartend.set(id, res); ws.send(JSON.stringify({ id, method, params }));
});

// ----------------------------------------------------------------- Buchhaltung je Laden
let lauf = null;   // { anfragen: Map(id -> {pfad,status,byte,cache}), letzte: ms, ladeEnde: bool }
const istApp = url => { try { const u = new URL(url); return u.host === host && u.pathname.startsWith("/app/"); } catch { return false; } };
const pfadVon = url => new URL(url).pathname;

ws.addEventListener("message", e => {
  const m = JSON.parse(e.data);
  if (m.id && wartend.has(m.id)) { wartend.get(m.id)(m.result || m.error); wartend.delete(m.id); return; }
  if (!lauf) return;
  const p = m.params || {};
  if (m.method === "Page.loadEventFired") lauf.ladeEnde = true;
  if (m.method === "Network.requestWillBeSent" && istApp(p.request.url)) {
    if (!lauf.anfragen.has(p.requestId)) lauf.anfragen.set(p.requestId, { pfad: pfadVon(p.request.url), status: "?", byte: 0, cache: false });
    lauf.letzte = Date.now();
  }
  // Den Status der LEITUNG liefert nur responseReceivedExtraInfo: Bei einer Nachpruefung mit 304
  // meldet responseReceived den Status der Cache-Kopie (200). Gegenprobe 15.10.2026: Pruefserver
  // sendete 304, responseReceived zeigte 200 bei 290 Byte.
  if (m.method === "Network.responseReceivedExtraInfo") {
    lauf.extra.set(p.requestId, p.statusCode);
  }
  if (m.method === "Network.requestServedFromCache" && lauf.anfragen.has(p.requestId)) lauf.anfragen.get(p.requestId).cache = true;
  if (m.method === "Network.responseReceived" && lauf.anfragen.has(p.requestId)) {
    const a = lauf.anfragen.get(p.requestId);
    a.status = p.response.status;
    if (p.response.fromDiskCache || p.response.fromMemoryCache) a.cache = true;
    lauf.letzte = Date.now();
  }
  if ((m.method === "Network.loadingFinished" || m.method === "Network.loadingFailed") && lauf.anfragen.has(p.requestId)) {
    const a = lauf.anfragen.get(p.requestId);
    if (m.method === "Network.loadingFinished") a.byte = p.encodedDataLength || 0;
    else a.status = "FAIL " + p.errorText;
    a.fertig = true;
    lauf.letzte = Date.now();
  }
});

async function geraet(pfad) {
  const r = await fetch(basis + pfad, { signal: AbortSignal.timeout(15000) });
  return r.json();
}

// Warten: Ladeereignis, dann keine /app/-Anfrage offen und RUHE_MS still. Die /api/-Abrufe
// laufen im Takt weiter und zaehlen hier nicht -- gemessen wird die Asset-Last.
async function warteRuhe() {
  const t0 = Date.now();
  for (;;) {
    await new Promise(r => setTimeout(r, 200));
    const offen = [...lauf.anfragen.values()].filter(a => !a.fertig && !a.cache).length;
    if (lauf.ladeEnde && offen === 0 && Date.now() - lauf.letzte >= RUHE_MS) return true;
    if (Date.now() - t0 > MAX_MS) return false;
  }
}

async function laden(nr, art, aktion) {
  try { await geraet("/api/device_ready?heap_low_reset=1"); }
  catch (e) { aus(`${zeit()}  #${nr} ${art}: device_ready nicht erreichbar (${e.message})`); fehl++; return; }
  lauf = { anfragen: new Map(), extra: new Map(), letzte: Date.now(), ladeEnde: false };
  const t0 = Date.now();
  await aktion();
  const ruhig = await warteRuhe();
  const ms = Date.now() - t0;
  for (const [id, a] of lauf.anfragen) if (lauf.extra.has(id) && typeof a.status === "number") a.status = lauf.extra.get(id);
  const liste = [...lauf.anfragen.values()];
  lauf = null;
  let dr = {};
  try { dr = await geraet("/api/device_ready"); } catch (e) { aus(`  device_ready danach nicht lesbar: ${e.message}`); fehl++; }
  const n200 = liste.filter(a => a.status === 200 && !a.cache);
  const n304 = liste.filter(a => a.status === 304);
  const nCache = liste.filter(a => a.cache);
  const nFail = liste.filter(a => String(a.status).startsWith("FAIL") || a.status === "?");
  const byte = liste.reduce((s, a) => s + (a.cache ? 0 : a.byte), 0);
  const felder = ["heap_low_block", "heap_low_path", "heap_low_req_path", "free_heap", "max_free_block",
                  "app_not_modified", "app_full", "http_backlog"]
    .filter(k => dr[k] !== undefined).map(k => `${k}=${dr[k]}`).join(" ");
  aus(`${zeit()}  #${nr} ${art.padEnd(9)} ${ms} ms${ruhig ? "" : " (KEINE RUHE)"}  /app/: ${liste.length} Anfr., 200=${n200.length} 304=${n304.length} cache=${nCache.length} fehl=${nFail.length}  ${byte} Byte  ${felder}`);
  if (nFail.length) { aus(`    fehlgeschlagen: ${nFail.map(a => a.pfad + " " + a.status).join(", ")}`); fehl++; }
  if (!ruhig) fehl++;
  if (ERWARTE_304 && art !== "kalt") {
    const falsch = n200.filter(a => a.pfad !== AUSNAHME);
    if (falsch.length) { aus(`    FEHL --erwarte-304: warm mit 200 geholt: ${falsch.map(a => a.pfad).join(", ")}`); fehl++; }
  }
}

await send("Network.enable");
await send("Page.enable");
// Startmarke ist eine data:-Seite; localStorage gilt je Herkunft -- die Sprache wird nach dem
// ersten Laden auf der Uhr gesetzt und gilt ab dem naechsten Laden (deshalb: kalt laedt immer
// deutsch, die Sprache wirkt auf die warmen Ladungen).
aus(`=== Ladeprobe ${basis}/app/ — ${new Date().toISOString().slice(0, 16)} — warm=${WARM}${ENGLISCH ? " englisch" : ""}${ERWARTE_304 ? " --erwarte-304" : ""} ===`);
await laden(0, "kalt", () => send("Page.navigate", { url: SOLL }));
if (ENGLISCH) {
  await send("Runtime.evaluate", { expression: `localStorage.setItem("wordclock-language","en")` });
  aus(`    Sprache auf Englisch gesetzt (wirkt ab dem naechsten Laden)`);
}
for (let i = 1; i <= WARM; i++) {
  // Abwechselnd Neuladen (Browser validiert) und erneutes Oeffnen (das "zweite Oeffnen").
  if (i % 2 === 1) await laden(i, "neuladen", () => send("Page.reload", { ignoreCache: false }));
  else await laden(i, "oeffnen", async () => { await send("Page.navigate", { url: "about:blank" }); await new Promise(r => setTimeout(r, 300)); lauf.ladeEnde = false; await send("Page.navigate", { url: SOLL }); });
}
ws.close();
aus(fehl ? `ERGEBNIS: ${fehl} Fehlschlag/-schlaege` : `ERGEBNIS: ohne Fehlschlag`);
process.exit(fehl ? 1 : 0);
