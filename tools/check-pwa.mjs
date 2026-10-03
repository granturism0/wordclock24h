// Bedient die PWA, die auf der Uhr laeuft, in einem echten Browser.
//
// Wird von tools/check-pwa.sh gestartet, das Chrome hochfaehrt und wieder abraeumt.
//
// WARUM DAS UEBER DAS LADEN HINAUSGEHT
//
// Die erste Fassung von check-pwa.sh hat die Seite nur GELADEN und den fertigen DOM
// angesehen. Der Nutzer hat am 03.10.2026 gefragt, warum nicht die echte PWA auf der
// Uhr geprueft wird -- und der Punkt war richtig, nur anders als die Frage klang:
// Geladen wurde sie schon vom Geraet. Bedient wurde sie nicht.
//
// "Laedt fehlerfrei" und "funktioniert" sind zwei verschiedene Aussagen. Ein Modul,
// das sich nicht oeffnen laesst, eine Schaltflaeche ohne Wirkung, ein Abruf, der beim
// Wechsel ins Modul scheitert -- nichts davon sieht man am DOM des Startbildschirms.
//
// Gesteuert wird ueber das Chrome DevTools Protocol, ohne Puppeteer oder Playwright:
// Node bringt seit v22 ein eingebautes WebSocket mit, und mehr braucht es nicht.
//
// AUSSCHLIESSLICH LESEND. Es werden Module geoeffnet und Werte abgelesen; es wird
// nichts gespeichert, nichts gesendet, keine Schaltflaeche mit Wirkung geklickt.

const PORT = process.env.CDP_PORT || 9222;
const STM_SOLL = process.env.STM_SOLL || "";

let ok = 0, fail = 0;
const pass = (w, d = "") => { console.log(`  OK    ${w.padEnd(38)} ${d}`); ok++; };
const bad  = (w, d = "") => { console.log(`  FEHLT ${w.padEnd(38)} ${d}`); fail++; };

// ---------------------------------------------------------------- CDP-Anbindung
const targets = await fetch(`http://127.0.0.1:${PORT}/json/list`).then(r => r.json());
const page = targets.find(t => t.type === "page");
if (!page) { console.log("  ABBRUCH: kein Browser-Target gefunden"); process.exit(2); }

const ws = new WebSocket(page.webSocketDebuggerUrl);
await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });

let seq = 0;
const konsole = [];          // alles, was die Seite selbst meldet
const netzfehler = [];       // fehlgeschlagene Abrufe
const urlVonId = new Map();  // requestId -> URL, damit der Fehler eine Adresse hat

ws.addEventListener("message", e => {
  const m = JSON.parse(e.data);
  if (m.method === "Runtime.exceptionThrown") {
    const d = m.params.exceptionDetails;
    konsole.push("EXCEPTION: " + (d.exception?.description || d.text));
  }
  if (m.method === "Runtime.consoleAPICalled" && ["error", "warning"].includes(m.params.type)) {
    konsole.push(m.params.type.toUpperCase() + ": " +
      m.params.args.map(a => a.value ?? a.description ?? "").join(" ").slice(0, 160));
  }
  if (m.method === "Network.requestWillBeSent") {
    urlVonId.set(m.params.requestId, m.params.request.url);
  }
  if (m.method === "Network.loadingFailed") {
    netzfehler.push({ fehler: m.params.errorText, url: urlVonId.get(m.params.requestId) || "?" });
  }
});

const send = (method, params = {}) => new Promise(res => {
  const id = ++seq;
  const h = e => { const m = JSON.parse(e.data); if (m.id === id) { ws.removeEventListener("message", h); res(m.result); } };
  ws.addEventListener("message", h);
  ws.send(JSON.stringify({ id, method, params }));
});

const js = async expr => (await send("Runtime.evaluate", { expression: expr, returnByValue: true, awaitPromise: true })).result?.value;
const schlaf = ms => new Promise(r => setTimeout(r, ms));

await send("Runtime.enable");
await send("Network.enable");
await schlaf(2500);          // der PWA Zeit lassen, ihre Abrufe zu fahren

// --------------------------------------------------------- 1. Grundzustand
const titel = await js("document.title");
titel ? pass("Seite geladen", `Titel "${titel}"`) : bad("Seite geladen", "kein Titel");

const module = await js(`[...document.querySelectorAll("[data-module]")].map(e=>e.dataset.module)`);
(module?.length >= 5)
  ? pass("Module vorhanden", `${module.length}: ${module.slice(0, 6).join(", ")}…`)
  : bad("Module vorhanden", `nur ${module?.length ?? 0}`);

// --------------------------------------------- 2. Echte Geraetewerte angekommen
// Gewartet wird, nicht einmal geschaut. Die PWA holt ihre Werte asynchron; eine
// einmalige Abfrage nach fester Zeit besteht mal und faellt mal durch -- genau das ist
// beim Bauen passiert, zwei Laeufe hintereinander mit verschiedenem Ergebnis. Ein
// flackernder Test ist schlimmer als keiner: Man gewoehnt sich an, ihn zu ignorieren.
if (STM_SOLL) {
  let drin = false;
  for (let v = 0; v < 20 && !drin; v++) {
    drin = await js(`document.body.innerText.includes(${JSON.stringify(STM_SOLL)})`);
    if (!drin) await schlaf(500);
  }
  drin ? pass("Geraetewerte angezeigt", `STM ${STM_SOLL} steht in der Seite`)
       : bad("Geraetewerte angezeigt", `STM ${STM_SOLL} fehlt nach 10 s — Daten nicht verarbeitet`);
}

// ------------------------------------------- 3. Jedes Modul tatsaechlich oeffnen
//
// Das ist der Teil, den das blosse Laden nicht leistet. Ein Modul kann im DOM
// stehen und sich trotzdem nicht oeffnen lassen, oder beim Oeffnen einen Abruf
// fahren, der scheitert.
const navi = await js(`[...document.querySelectorAll("[data-module-target],[data-nav-target],nav button,nav a")].length`);
let geoeffnet = 0, leer = [], ohneSchalter = [], unsichtbar = [], ausgeblendet = [];

for (const m of (module || [])) {
  // Ist der Umschalter ueberhaupt sichtbar? Die PWA blendet Module bewusst aus, wenn
  // die Hardware fehlt -- "DFPlayer is offline and hidden" steht so in ihrer eigenen
  // Textliste. Ein verstecktes Modul ist dann richtiges Verhalten, kein Fehlschlag.
  const schalterSichtbar = await js(`(() => {
    const sel = ['[data-module-target="${m}"]','[data-nav-target="${m}"]','[href="#${m}"]','button[data-target="${m}"]'];
    for (const s of sel) {
      const e = document.querySelector(s);
      if (e) { const r = e.getBoundingClientRect(); return r.height > 0 && getComputedStyle(e).display !== "none"; }
    }
    return false;
  })()`);
  if (!schalterSichtbar) { ausgeblendet.push(m); continue; }

  const auf = await js(`(() => {
    const sel = ['[data-module-target="${m}"]','[data-nav-target="${m}"]','[href="#${m}"]','button[data-target="${m}"]'];
    for (const s of sel) { const e = document.querySelector(s); if (e) { e.click(); return true; } }
    return false;
  })()`);
  if (!auf) { ohneSchalter.push(m); continue; }
  await schlaf(600);
  const sichtbar = await js(`(() => {
    const e = document.querySelector('[data-module="${m}"]');
    if (!e) return 0;
    const r = e.getBoundingClientRect();
    return (r.height > 40 && getComputedStyle(e).display !== "none") ? Math.round(e.innerText.trim().length) : 0;
  })()`);
  if (sichtbar > 0) { geoeffnet++; if (sichtbar < 40) leer.push(m); }
  else unsichtbar.push(m);   // Umschalter da, Modul bleibt verborgen -- der stillste Fehlerfall
}

if (geoeffnet === 0) {
  bad("Module bedienbar", `keines liess sich oeffnen (${navi} Navigationselemente gefunden)`);
} else if (leer.length) {
  bad("Module bedienbar", `${geoeffnet} geoeffnet, aber fast leer: ${leer.join(", ")}`);
} else {
  pass("Module bedienbar", `${geoeffnet} von ${module.length} geoeffnet, alle mit Inhalt`);
}
// Ein Modul, das weder gezaehlt noch gemeldet wird, waere der stillste Fehlerfall von
// allen -- die erste Fassung meldete "10 von 11" und verschwieg, welches das elfte war.
if (ohneSchalter.length) {
  bad("jedes Modul erreichbar", `ohne auffindbaren Umschalter: ${ohneSchalter.join(", ")}`);
} else if (unsichtbar.length) {
  bad("jedes Modul erreichbar", `oeffnet sich nicht sichtbar: ${unsichtbar.join(", ")}`);
} else {
  const txt = ausgeblendet.length
    ? `${geoeffnet} erreichbar, ${ausgeblendet.length} bewusst ausgeblendet (${ausgeblendet.join(", ")})`
    : `alle ${module.length}`;
  pass("jedes Modul erreichbar", txt);
}

// ----------------------------------------- 4. Meldungen der Seite ueber sich selbst
const echte = konsole.filter(z => !/favicon|DevTools|sourcemap/i.test(z));
echte.length === 0
  ? pass("keine Fehler in der Konsole")
  : bad("keine Fehler in der Konsole", `${echte.length}`);
echte.slice(0, 5).forEach(z => console.log("          " + z));

// ERR_ABORTED ist KEIN Fehlschlag: Es heisst "jemand hat abgebrochen", nicht "es ging
// schief". Beim Beenden des Browsers brechen alle laufenden Poller ab -- die PWA fragt
// settings_xml, display_power, ambilight_power und update_status zyklisch ab. Die erste
// Fassung dieser Pruefung meldete das als Fehler; die Zahl stieg von 8 auf 23, als der
// Lauf laenger dauerte, also proportional zur LAUFZEIT statt zu einem Problem. Ein
// Werkzeug mit Fehlalarmen liest irgendwann niemand mehr.
//
// Echte Fehlschlaege (Verbindung abgelehnt, Zeitueberschreitung, abgebrochene
// Uebertragung) bleiben gezaehlt.
const nf = netzfehler.filter(z => !/favicon/i.test(z.url) && z.fehler !== "net::ERR_ABORTED");
if (nf.length === 0) {
  pass("keine fehlgeschlagenen Abrufe");
} else {
  bad("keine fehlgeschlagenen Abrufe", `${nf.length}`);
  [...new Set(nf.map(z => `${z.fehler}  ${z.url.replace(/^https?:\/\/[^/]+/, "")}`))]
    .slice(0, 6).forEach(z => console.log("          " + z));
}

// ------------------------------------------------- 5. Offene Platzhalter zaehlen
const striche = await js(`(document.body.innerText.match(/(^|\\s)(—|--|…)(\\s|$)/g)||[]).length`);
(striche ?? 0) <= 25
  ? pass("Platzhalter gefuellt", `${striche} offen`)
  : bad("Platzhalter gefuellt", `${striche} leere Felder — Abrufe ohne Ergebnis?`);

ws.close();
console.log(`\n=== ${ok} bestanden, ${fail} fehlgeschlagen ===`);
process.exit(fail > 0 ? 1 : 0);
