// Ladeprobe (Paket 2026-10-15, design.md §5.1): Laedt die PWA einmal kalt und mehrfach warm
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
// Je Laden: jede /app/-Anfrage mit Leitungsstatus und ob der Browser If-None-Match schickte,
// Summe der uebertragenen Byte an /app/, und aus /api/device_ready der Tiefstwert seit dem
// Zuruecksetzen samt I-4-Momentaufnahme.
//
// ZWEI FALLEN, DIE HIER ABGEFANGEN SIND
//
// - Den Status der LEITUNG liefert nur responseReceivedExtraInfo. Bei einer Nachpruefung mit
//   304 meldet responseReceived den Status der Cache-Kopie (200). Gegenprobe 15.10.2026.
// - /app/-Anfragen, die erst nach einer /api/-Antwort kommen (layout-previews.json nach den
//   Einstellungen), koennen nach der Ruheerkennung eintreffen. Sie werden als "spaet" gemeldet
//   und als Fehlschlag gezaehlt, nicht still verworfen (Review 0.5, M3).
//
// --erwarte-304: Ein WARMES Laden, das ein App-Asset mit 200 holt, ist ein Fehlschlag.
// Ausnahme: LADEPROBE_AUSNAHME (Pfad).
//
// Exit 0 ohne Fehlschlag, 1 Fehlschlag, 2 Werkzeugfehler.
// SCHREIBT AM GERAET NUR den Ruecksetzer der Tiefstwert-Messung (device_ready?heap_low_reset=1).

import { appendFileSync } from "node:fs";
import { execFileSync } from "node:child_process";

const PORT = process.env.CDP_PORT;
const SOLL = process.env.GERAET_URL;          // http://<host>/app/
const MARKE = process.env.START_MARKE;
const ERWARTE_304 = process.env.ERWARTE_304 === "1";
// R2b (C70 Variante c, AKL.8 iii): unversionierte Pfade warm auf der Leitung mit 304, alle anderen aus dem Cache.
const ERWARTE_CACHE = process.env.ERWARTE_CACHE === "1";
const UNVERSIONIERT = new Set(["/app/", "/app/index.html", "/app/layout-previews.json"]);
const ENGLISCH = process.env.ENGLISCH === "1";
const MITSCHRIFT = process.env.LADEPROBE_MITSCHRIFT || "";
const AUSNAHME = process.env.LADEPROBE_AUSNAHME || "";
const WARM = Number(process.env.LADEPROBE_WARM ?? 5);
// G10 (Probe U, AKW.3): LADEPROBE_HAKEN wird vor jedem warmen Laden mit dessen Nummer aufgerufen (Exit != 0 = Werkzeug-
// fehler); LADEPROBE_ETAG_PFAD nennt einen Pfad, dessen ETag und Leitungsstatus je Laden ausgegeben werden.
const HAKEN = process.env.LADEPROBE_HAKEN || "";
const ETAG_PFAD = process.env.LADEPROBE_ETAG_PFAD || "";
// Pfade, die dieses Profil schon einmal geladen hat. Ein 200 auf einen NEUEN Pfad ist kein Verstoss gegen --erwarte-304:
// Der Browser kann nicht nachfragen, was er nie hatte (G10.1, 16.10.2026: i18n/en.json erst nach dem Sprachwechsel).
const gesehen = new Set();
const vorZ = {};   // Geraetezaehler der vorigen Lesung, fuer die Δ je Laden
const RUHE_MS = 1500, MAX_MS = 30000, NACHLAUF_MS = 1500;
const I4 = ["heap_low_block", "heap_low_free", "heap_low_path", "heap_low_req_path", "heap_low_ms",
            "heap_low_accepts_pending", "heap_low_backlog", "heap_low_pcb_states", "heap_low_tw",
            "heap_low_unacked", "free_heap", "max_free_block", "http_backlog", "app_not_modified", "app_full"];

const zeit = () => new Date().toTimeString().slice(0, 8);
const aus = z => { console.log(z); if (MITSCHRIFT) appendFileSync(MITSCHRIFT, z + "\n"); };
let fertig = false;
const werkzeugfehler = t => { fertig = true; aus(`WERKZEUGFEHLER: ${t}`); process.exit(2); };
process.on("uncaughtException", e => werkzeugfehler(e.stack || e.message));
process.on("unhandledRejection", e => werkzeugfehler(String((e && e.stack) || e)));

if (!PORT || !SOLL || !MARKE) werkzeugfehler("CDP_PORT, GERAET_URL oder START_MARKE fehlt (Start ueber ladeprobe.sh)");
if (!Number.isInteger(WARM) || WARM < 1) werkzeugfehler(`LADEPROBE_WARM ungueltig: ${process.env.LADEPROBE_WARM}`);

const host = new URL(SOLL).host;
const basis = `http://${host}`;
let fehl = 0;

const listeHolen = () => fetch(`http://127.0.0.1:${PORT}/json/list`).then(r => r.json());
let ziele = await listeHolen(), seite;
for (let v = 0; v < 25 && !(seite = ziele.find(t => t.type === "page" && t.url === MARKE)); v++) {
  await new Promise(r => setTimeout(r, 200));
  ziele = await listeHolen();
}
if (!seite) werkzeugfehler("kein Seitenziel mit der eigenen Startmarke");

const ws = new WebSocket(seite.webSocketDebuggerUrl);
await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
ws.onclose = () => { if (!fertig) werkzeugfehler("Verbindung zu Chrome abgerissen (Chrome beendet?)"); };
let seq = 0;
const wartend = new Map();
// CDP-Fehler sind KEIN gewoehnliches Ergebnis (Review S7): send() wirft.
const send = (method, params = {}) => new Promise((res, rej) => {
  const id = ++seq; wartend.set(id, { res, rej, method }); ws.send(JSON.stringify({ id, method, params }));
});

// ----------------------------------------------------------------- Buchhaltung je Laden
let lauf = null;
let spaet = [];   // /app/-Anfragen ausserhalb eines Fensters
const istApp = url => { try { const u = new URL(url); return u.host === host && u.pathname.startsWith("/app/"); } catch { return false; } };
const pfadVon = url => new URL(url).pathname;

ws.addEventListener("message", e => {
  const m = JSON.parse(e.data);
  if (m.id && wartend.has(m.id)) {
    const w = wartend.get(m.id); wartend.delete(m.id);
    if (m.error) w.rej(new Error(`${w.method}: ${m.error.message}`)); else w.res(m.result);
    return;
  }
  const p = m.params || {};
  if (m.method === "Network.requestWillBeSent" && istApp(p.request.url) && !lauf) {
    spaet.push(pfadVon(p.request.url));
    return;
  }
  if (!lauf) return;
  if (m.method === "Page.loadEventFired") lauf.ladeEnde = true;
  if (m.method === "Network.requestWillBeSent" && istApp(p.request.url)) {
    if (!lauf.anfragen.has(p.requestId)) lauf.anfragen.set(p.requestId, { pfad: pfadVon(p.request.url), status: "?", byte: 0, cache: false, inm: false });
    lauf.letzte = Date.now();
  }
  // If-None-Match steht nur in den tatsaechlich gesendeten Kopfzeilen (Review M4): Ohne sie
  // laesst sich eine 200 nicht zuordnen -- hat der Browser nicht gefragt (HTTP/1.0, L468),
  // oder hat der Server die Frage uebergangen?
  if (m.method === "Network.requestWillBeSentExtraInfo") lauf.inm.set(p.requestId, Object.keys(p.headers || {}).some(k => k.toLowerCase() === "if-none-match"));
  if (m.method === "Network.responseReceivedExtraInfo") {
    lauf.extra.set(p.requestId, p.statusCode);
    const h = p.headers || {}; const k = Object.keys(h).find(x => x.toLowerCase() === "etag");
    if (k) lauf.etag.set(p.requestId, h[k]);
  }
  if (m.method === "Network.requestServedFromCache" && lauf.anfragen.has(p.requestId)) { const a = lauf.anfragen.get(p.requestId); a.cache = true; a.fertig = true; }
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
  if (!r.ok) throw new Error(`${pfad}: HTTP ${r.status}`);
  return r.json();
}

// Warten: Ladeereignis, dann keine /app/-Anfrage offen und RUHE_MS still, danach NACHLAUF_MS
// fest -- spaet nachgeladene Assets (nach /api/-Antworten) sollen noch ins Fenster fallen.
async function warteRuhe() {
  const t0 = Date.now();
  for (;;) {
    await new Promise(r => setTimeout(r, 200));
    const offen = [...lauf.anfragen.values()].filter(a => !a.fertig).length;
    if (lauf.ladeEnde && offen === 0 && Date.now() - lauf.letzte >= RUHE_MS) {
      await new Promise(r => setTimeout(r, NACHLAUF_MS));
      if ([...lauf.anfragen.values()].every(a => a.fertig)) return true;
    }
    if (Date.now() - t0 > MAX_MS) return false;
  }
}

async function laden(nr, art, aktion) {
  if (spaet.length) { aus(`    SPAET (ausserhalb eines Fensters, nicht gezaehlt): ${spaet.join(", ")}`); fehl++; spaet = []; }
  try { await geraet("/api/device_ready?heap_low_reset=1"); }
  catch (e) { aus(`${zeit()}  #${nr} ${art}: device_ready nicht erreichbar (${e.message})`); fehl++; return; }
  lauf = { anfragen: new Map(), extra: new Map(), inm: new Map(), etag: new Map(), letzte: Date.now(), ladeEnde: false };
  const t0 = Date.now();
  await aktion();
  const ruhig = await warteRuhe();
  const ms = Date.now() - t0;
  let ohneLeitung = 0;
  for (const [id, a] of lauf.anfragen) {
    if (lauf.extra.has(id) && typeof a.status === "number") a.status = lauf.extra.get(id);
    else if (!a.cache && typeof a.status === "number") ohneLeitung++;
    a.inm = lauf.inm.get(id) === true;
    a.etag = lauf.etag.get(id) || "";
    a.neu = !gesehen.has(a.pfad);
  }
  const liste = [...lauf.anfragen.values()];
  lauf = null;
  for (const a of liste) gesehen.add(a.pfad);
  let dr = {};
  try { dr = await geraet("/api/device_ready"); } catch (e) { aus(`  device_ready danach nicht lesbar: ${e.message}`); fehl++; }
  const n200 = liste.filter(a => a.status === 200 && !a.cache);
  const n304 = liste.filter(a => a.status === 304);
  const nCache = liste.filter(a => a.cache);
  const nFail = liste.filter(a => String(a.status).startsWith("FAIL") || a.status === "?");
  const byte = liste.reduce((s, a) => s + (a.cache ? 0 : a.byte), 0);
  const nInm = liste.filter(a => a.inm).length;
  aus(`${zeit()}  #${nr} ${art.padEnd(9)} ${ms} ms${ruhig ? "" : " (KEINE RUHE)"}  /app/: ${liste.length} Anfr., 200=${n200.length} 304=${n304.length} cache=${nCache.length} fehl=${nFail.length} INM=${nInm}${ohneLeitung ? ` ohne-Leitungsstatus=${ohneLeitung}` : ""}  ${byte} Byte`);
  aus(`    Pfade: ${liste.map(a => `${a.pfad.replace("/app/", "") || "/"}=${a.cache ? "cache" : a.status}${a.inm ? "+INM" : ""}`).join(" ")}`);
  aus(`    Geraet: ${I4.filter(k => dr[k] !== undefined).map(k => `${k}=${JSON.stringify(dr[k])}`).join(" ")}`);
  // Die eigene Lesung kann den Tiefstwert selbst verursachen (Review S6) -- kennzeichnen.
  if (dr.heap_low_req_path === "/api/device_ready") aus(`    HINWEIS: Tiefstwert in der eigenen Lesung (device_ready) -- nicht der Ladung zurechnen`);
  if (ENGLISCH && art !== "kalt" && !liste.some(a => a.pfad === "/app/i18n/en.json"))
    aus(`    HINWEIS: kein /app/i18n/en.json in diesem Laden -- englisch nicht nachgewiesen`);
  if (nFail.length) { aus(`    fehlgeschlagen: ${nFail.map(a => a.pfad + " " + a.status).join(", ")}`); fehl++; }
  if (!ruhig) fehl++;
  // Unabhaengige Nachzaehlung ab R2 (L235): die Zaehler des ESP neben die eigene Zahl.
  if (dr.app_full !== undefined && dr.app_not_modified !== undefined) {
    const z = ["app_full", "app_not_modified", "app_immutable", "app_token_miss"].filter(k => dr[k] !== undefined);
    const d = z.map(k => `${k}=${dr[k]}${vorZ[k] !== undefined ? ` (Δ${dr[k] - vorZ[k] >= 0 ? "+" : ""}${dr[k] - vorZ[k]})` : ""}`);
    aus(`    Geraetezaehler (seit Start): ${d.join(" ")}`);
    // Zwei unabhaengige Instrumente (L235, design.md 18.8): Leitungsanfragen an /app/ = Δapp_full + Δapp_not_modified.
    // Die eigenen device_ready-Lesungen gehen an /api/ und zaehlen dort nicht.
    if (vorZ.app_full !== undefined) {
      const leitung = liste.filter(a => !a.cache).length, dz = (dr.app_full - vorZ.app_full) + (dr.app_not_modified - vorZ.app_not_modified);
      if (leitung !== dz) aus(`    HINWEIS: Leitung ${leitung} Anfragen, Geraet zaehlt ${dz} -- Instrumente uneinig`);
    }
    for (const k of z) vorZ[k] = dr[k];
  }
  if (ERWARTE_CACHE && art !== "kalt") {
    const leitung304 = liste.filter(a => !a.cache && a.status === 304 && !UNVERSIONIERT.has(a.pfad) && !a.neu);
    const leitung200 = liste.filter(a => !a.cache && a.status === 200 && !UNVERSIONIERT.has(a.pfad) && !a.neu);
    const unvNicht = liste.filter(a => UNVERSIONIERT.has(a.pfad) && (a.cache || a.status !== 304));
    if (leitung304.length) { aus(`    FEHL --erwarte-cache: warm: Cache erwartet, 304 erhalten: ${leitung304.map(a => a.pfad).join(", ")}`); fehl++; }
    if (leitung200.length) { aus(`    FEHL --erwarte-cache: warm: Cache erwartet, 200 erhalten: ${leitung200.map(a => a.pfad).join(", ")}`); fehl++; }
    if (unvNicht.length) { aus(`    FEHL --erwarte-cache: warm: unversioniert auf der Leitung mit 304 erwartet: ${unvNicht.map(a => a.pfad + "=" + (a.cache ? "cache" : a.status)).join(", ")}`); fehl++; }
  }
  if (ETAG_PFAD) for (const a of liste.filter(x => x.pfad === ETAG_PFAD)) aus(`    ETag ${a.pfad}: ${a.etag || "-"} (${a.cache ? "cache" : a.status})`);
  if (ERWARTE_304 && art !== "kalt") {
    const erstmals = n200.filter(a => a.neu);
    if (erstmals.length) aus(`    erstmals in diesem Profil, 200 ohne Vorgaenger: ${erstmals.map(a => a.pfad).join(", ")} (kein Verstoss)`);
    const falsch = n200.filter(a => a.pfad !== AUSNAHME && !a.neu);
    if (falsch.length) { aus(`    FEHL --erwarte-304: warm: 304 erwartet, 200 erhalten: ${falsch.map(a => a.pfad + (a.inm ? " (mit INM)" : " (ohne INM)")).join(", ")}`); fehl++; }
  }
}

await send("Network.enable");
await send("Page.enable");
aus(`=== Ladeprobe ${basis}/app/ — ${new Date().toISOString().slice(0, 16)} — warm=${WARM}${ENGLISCH ? " englisch" : ""}${ERWARTE_304 ? " --erwarte-304" : ""} ===`);
let vorher = {};
try { vorher = await geraet("/api/device_ready"); } catch (e) { werkzeugfehler(`device_ready vor dem Lauf: ${e.message}`); }
aus(`    vorher: free_heap=${vorher.free_heap} max_free_block=${vorher.max_free_block}`);
await laden(0, "kalt", () => send("Page.navigate", { url: SOLL }));
if (ENGLISCH) {
  const r = await send("Runtime.evaluate", { expression: `localStorage.setItem("wordclock-language","en"); localStorage.getItem("wordclock-language")`, returnByValue: true });
  if (r.exceptionDetails || r.result?.value !== "en") werkzeugfehler(`Sprache nicht gesetzt: ${JSON.stringify(r.exceptionDetails || r.result)}`);
  aus(`    Sprache auf Englisch gesetzt (wirkt ab dem naechsten Laden)`);
}
for (let i = 1; i <= WARM; i++) {
  if (HAKEN) {
    try { execFileSync(HAKEN, [String(i)], { stdio: ["ignore", "inherit", "inherit"], timeout: 120000 }); }
    catch (e) { werkzeugfehler(`Haken vor warm #${i}: ${e.message}`); }
    aus(`    Haken vor warm #${i} ausgefuehrt`);
  }
  // Abwechselnd Neuladen (Browser validiert) und erneutes Oeffnen (das "zweite Oeffnen").
  if (i % 2 === 1) await laden(i, "neuladen", () => send("Page.reload", { ignoreCache: false }));
  else await laden(i, "oeffnen", async () => {
    await send("Page.navigate", { url: "about:blank" });
    await new Promise(r => setTimeout(r, 300));
    lauf.ladeEnde = false;
    await send("Page.navigate", { url: SOLL });
  });
}
await new Promise(r => setTimeout(r, NACHLAUF_MS));
if (spaet.length) { aus(`    SPAET (nach dem letzten Fenster, nicht gezaehlt): ${spaet.join(", ")}`); fehl++; }
fertig = true;
ws.close();
aus(fehl ? `ERGEBNIS: ${fehl} Fehlschlag/-schlaege` : `ERGEBNIS: ohne Fehlschlag`);
process.exit(fehl ? 1 : 0);
