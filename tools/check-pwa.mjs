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

import { appendFileSync } from "node:fs";

const PORT = process.env.CDP_PORT || 9222;
const STM_SOLL = process.env.STM_SOLL || "";

let ok = 0, fail = 0;
const pass = (w, d = "") => { console.log(`  OK    ${w.padEnd(38)} ${d}`); ok++; };
const bad  = (w, d = "") => { console.log(`  FEHLT ${w.padEnd(38)} ${d}`); fail++; };
const info = (w, d = "") => { console.log(`  INFO  ${w.padEnd(38)} ${d}`); };

// ---------------------------------------------------------------- CDP-Anbindung
const listeHolen = () => fetch(`http://127.0.0.1:${PORT}/json/list`).then(r => r.json());
let targets = await listeHolen();
// Das Ziel an einer EIGENEN Marke festmachen, nicht das erste Seitenziel nehmen (L301).
// Mit dem ersten Ziel prueft ein Lauf an einem fremden Browser die falsche Seite --
// und meldet womoeglich gruen.
//
// Seit E4.9 startet Chrome auf about:blank#<Marke> und die PWA wird erst geladen, wenn
// Network und Runtime eingeschaltet sind. Vorher lud Chrome die Seite schon beim Start,
// und alles, was vor dem Anschluss geschah, war unsichtbar: die ersten Abrufe (also
// genau die Startphase, die gemessen werden soll) und Ausnahmen beim ersten Laden.
const SOLL = process.env.GERAET_URL || "";
const MARKE = process.env.START_MARKE || "";
// Chrome oeffnet den Debug-Port, bevor das Startziel seine Adresse traegt -- kurz nachfragen.
let page;
for (let v = 0; v < 25 && !(page = targets.find(t => t.type === "page" && MARKE && t.url === MARKE)); v++) {
  await new Promise(r => setTimeout(r, 200));
  targets = await listeHolen();
}
if (!page || !SOLL) {
  console.log(`  ABBRUCH: kein Seitenziel mit der eigenen Startmarke (${targets.filter(t => t.type === "page").length} fremde Seitenziele)`);
  process.exit(2);
}
const ZIEL_HOST = new URL(SOLL).host;

const ws = new WebSocket(page.webSocketDebuggerUrl);
await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });

let seq = 0;
const konsole = [];          // alles, was die Seite selbst meldet
const netzfehler = [];       // fehlgeschlagene Abrufe
const urlVonId = new Map();  // requestId -> URL, damit der Fehler eine Adresse hat

// ------------------------------------------------------- Netzbuchhaltung (E4.9)
//
// Feste Wartezeiten (2,5 s nach dem Laden, 0,6 s nach dem Oeffnen, 4 s nach jedem
// Neuladen) reichen nicht mehr, seit die PWA ihre /api/-Abrufe durch eine Warteschlange
// schickt -- in der Startphase einer nach dem anderen -- und der ESP mit kleinem Backlog
// einen Verbindungsaufbau um ~1 s verzoegern kann (design-e4.md §19.10). Gewartet wird
// deshalb auf NETZRUHE: kein Abruf an die Uhr unterwegs seit NETZRUHE_MS.
//
// Gezaehlt wird nur, was an den Zielhost geht. Ein Abruf an einen fremden Host (Wetter-
// Icons, Schriften) ist nicht die Last der Uhr.
const NETZRUHE_MS = 1500;
const NETZRUHE_MAX_MS = 20000;
const ZAEHLEN = ["network_scan", "display_power", "ambilight_power", "power_status"];

const offen = new Map();          // requestId -> { pfad (ohne Query), loader (Dokument) }
const bekannteHaenger = new Set(); // Pfade, die schon als "keine Netzruhe" gemeldet sind
const letzteAktivitaet = new Map(); // Pfad -> Zeit des letzten Sendens/Abschlusses
const abschluesse = [];             // { pfad, t } je erfolgreich beendetem Abruf
const taktPfade = new Set();        // Pfade, die irgendwann als Takt galten (Ausgabe)
let apiOffen = 0;
let startphase = true;
const apiMax = { start: 0, danach: 0 };
const anzahl = Object.fromEntries(ZAEHLEN.map(n => [n, 0]));
let verbindungen = 0, langsameVerbindungen = 0, langsamsteMs = 0;
let ladeEreignisse = 0;

// CHECK_PWA_NETZLOG=<datei>: jedes Ereignis an den Zielhost mit Zeit, fuer die Diagnose,
// wenn "keine Netzruhe" kommt und unklar ist, wer das Netz belegt.
const NETZLOG = process.env.CHECK_PWA_NETZLOG || "";
const T0 = Date.now();
const netzlog = z => { if (NETZLOG) appendFileSync(NETZLOG, `${String(Date.now() - T0).padStart(7)} ${z}\n`); };

const pfadVon = url => { try { return new URL(url).pathname; } catch { return url.split("?")[0]; } };
const gehtAnZiel = url => { try { return new URL(url).host === ZIEL_HOST; } catch { return false; } };

ws.addEventListener("message", e => {
  const m = JSON.parse(e.data);
  if (m.method === "Page.loadEventFired") ladeEreignisse++;
  if (m.method === "Network.requestWillBeSent" && gehtAnZiel(m.params.request.url)) {
    const id = m.params.requestId;
    const pfad = pfadVon(m.params.request.url);
    // Eine Weiterleitung traegt dieselbe requestId -- nicht doppelt zaehlen.
    if (!offen.has(id)) {
      offen.set(id, { pfad, loader: m.params.loaderId });
      letzteAktivitaet.set(pfad, Date.now());
      netzlog(`SEND ${id} ${pfad} offen=${offen.size} api=${apiOffen + (pfad.startsWith("/api/") ? 1 : 0)} ${startphase ? "start" : ""}`);
      if (pfad.startsWith("/api/")) {
        apiOffen++;
        const k = startphase ? "start" : "danach";
        if (apiOffen > apiMax[k]) apiMax[k] = apiOffen;
        const name = pfad.slice(5);
        if (name in anzahl) anzahl[name]++;
      }
    }
  }
  if (m.method === "Network.responseReceived" && gehtAnZiel(m.params.response.url)) {
    // connectStart/connectEnd sind -1, wenn eine bestehende Verbindung wiederverwendet
    // wurde. Ein Aufbau ab 900 ms ist die Spur eines verworfenen SYN: Der erste
    // Versuch ging verloren, der zweite kam nach der Wiederholfrist durch.
    const t = m.params.response.timing;
    if (t && t.connectStart >= 0 && t.connectEnd >= 0) {
      verbindungen++;
      const dauer = t.connectEnd - t.connectStart;
      if (dauer >= 900) langsameVerbindungen++;
      if (dauer > langsamsteMs) langsamsteMs = dauer;
    }
  }
  if ((m.method === "Network.loadingFinished" || m.method === "Network.loadingFailed") && offen.has(m.params.requestId)) {
    const r = offen.get(m.params.requestId);
    if (r.pfad.startsWith("/api/")) apiOffen--;
    netzlog(`${m.method === "Network.loadingFailed" ? "FAIL " + m.params.errorText : "DONE"} ${m.params.requestId} ${r.pfad}`);
    offen.delete(m.params.requestId);
    letzteAktivitaet.set(r.pfad, Date.now());
    if (m.method === "Network.loadingFinished") {
      abschluesse.push({ pfad: r.pfad, t: Date.now() });
      if (abschluesse.length > 500) abschluesse.splice(0, 250);
    }
  }
  // Wechselt das Hauptdokument (Neuladen, oder die PWA laedt sich nach dem Wechsel des
  // Service Workers selbst neu), meldet Chrome fuer die Abrufe des ALTEN Dokuments oft
  // weder loadingFinished noch loadingFailed. Sie blieben sonst fuer immer "offen" -- in
  // der Vorschau mit 1'200 ms Verzoegerung so beobachtet: ein update_status-Geist, der
  // jede Netzruhe verhinderte und die Gleichzeitigkeit auf 5 hochzaehlte.
  if (m.method === "Page.frameNavigated" && !m.params.frame.parentId) {
    startphase = true;
    for (const [id, r] of offen) {
      if (r.loader && r.loader !== m.params.frame.loaderId) {
        if (r.pfad.startsWith("/api/")) apiOffen--;
        netzlog(`GONE ${id} ${r.pfad} (altes Dokument)`);
        offen.delete(id);
      }
    }
  }
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

// Wartet, bis seit NETZRUHE_MS kein Abruf an die Uhr mehr unterwegs ist. Nach
// NETZRUHE_MAX_MS ist das ein Fehlschlag, mit der Liste der offenen Abrufe.
// Ein schon gemeldeter haengender Pfad wird danach nicht mehr abgewartet: Er ist
// einmal FEHLT, und der Rest des Laufs soll trotzdem noch etwas messen koennen,
// statt an jeder Stelle erneut 20 s zu stehen.
//
// TAKTE HALTEN DIE NETZRUHE NICHT AUF. Ein Pfad, der waehrend DIESES Wartens schon
// zweimal erfolgreich zu Ende lief, ist ein Poller und kein Haenger -- im Modul "system"
// fragt die PWA stm32_log alle 2,5 s ab. Bei 1'200 ms Antwortzeit bleiben zwischen zwei
// Abrufen 1,3 s, die geforderten 1,5 s Ruhe kommen dort NIE (Vorschau, 14.10.2026, ohne
// diese Ausnahme: "keine Netzruhe nach 20 s (Neuladen in system) -- offen: stm32_log").
// Ein Abruf, der nie endet, kann nicht zweimal enden; der Haenger bleibt erkannt.
async function warteNetzruhe(wo) {
  const t0 = Date.now();
  for (;;) {
    const fertig = new Map();
    for (const a of abschluesse) if (a.t >= t0) fertig.set(a.pfad, (fertig.get(a.pfad) || 0) + 1);
    const takt = new Set([...fertig].filter(([, n]) => n >= 2).map(([p]) => p));
    const zaehlt = p => !bekannteHaenger.has(p) && !takt.has(p);
    const relevant = [...offen.values()].map(r => r.pfad).filter(zaehlt);
    const zuletzt = Math.max(0, ...[...letzteAktivitaet].filter(([p]) => zaehlt(p)).map(([, t]) => t));
    if (relevant.length === 0 && Date.now() - zuletzt >= NETZRUHE_MS) {
      takt.forEach(p => taktPfade.add(p));
      return true;
    }
    if (Date.now() - t0 >= NETZRUHE_MAX_MS) {
      const liste = [...new Set(relevant)];
      bad("Netzruhe", `keine Netzruhe nach ${NETZRUHE_MAX_MS / 1000} s (${wo}) — offen: ${liste.join(", ") || "(keiner, aber staendig neue Abrufe)"}`);
      liste.forEach(p => bekannteHaenger.add(p));
      return false;
    }
    await schlaf(100);
  }
}

// Laden heisst: Ladeereignis abwarten (hoechstens NETZRUHE_MAX_MS), dann Netzruhe. Bis
// dahin gilt jeder /api/-Abruf als Startphase -- auch nach jedem Neuladen, denn die PWA
// beginnt dann von vorn.
async function ladeUndWarte(wo, ausloesen) {
  // Startphase erst ab dem neuen Dokument (Page.frameNavigated, unten) -- vorher gesetzt, zaehlte ein
  // Takt-Abruf des ALTEN Dokuments zwischen Ausloesen und Navigation als Startphase (Review E4, N2).
  const vorher = ladeEreignisse;
  await ausloesen();
  const t0 = Date.now();
  while (ladeEreignisse === vorher && Date.now() - t0 < NETZRUHE_MAX_MS) await schlaf(50);
  if (ladeEreignisse === vorher) bad("Seite geladen", `kein Ladeereignis nach ${NETZRUHE_MAX_MS / 1000} s (${wo})`);
  const r = await warteNetzruhe(wo);
  startphase = false;
  return r;
}

const laufStart = Date.now();
await send("Runtime.enable");
await send("Network.enable");
await send("Page.enable");
await ladeUndWarte("nach dem Laden", () => send("Page.navigate", { url: SOLL }));
info("Netzruhe nach dem Laden", `${((Date.now() - laufStart) / 1000).toFixed(1)} s`);

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
  // Warten, bis das Modul sichtbar ist, hoechstens 3 s -- statt fest 600 ms.
  let sichtbar = 0;
  for (const t0 = Date.now(); Date.now() - t0 < 3000; ) {
    sichtbar = await js(`(() => {
      const e = document.querySelector('[data-module="${m}"]');
      if (!e) return 0;
      const r = e.getBoundingClientRect();
      return (r.height > 40 && getComputedStyle(e).display !== "none") ? Math.round(e.innerText.trim().length) : 0;
    })()`);
    if (sichtbar > 0) break;
    await schlaf(100);
  }
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

// ------------------------------------- 6. Start in jedem Modul (L411, B54)
// Die PWA stellt das zuletzt genutzte Modul aus localStorage wieder her. Mit frischem
// Profil startet dieser Lauf immer in "main" -- ein Initialisierungsfehler, der nur beim
// Start in einem anderen Modul auftritt, blieb deshalb unsichtbar (L411: ReferenceError
// beim Start in "system", in 1.4.94 bis 1.4.96, gefunden vom Testagenten, nicht hier).
// Je Modul: Schluessel setzen, neu laden, und jede Ausnahme oder Warnung mit
// ReferenceError/TypeError aus der Startphase zaehlen.
const START_KEY = "wordclock-app-active-module";
const startModule = await js(`[...new Set([...document.querySelectorAll(".module-section[data-module]")].map(e=>e.dataset.module))]`) || [];
const startFehler = [];
let gestartet = 0;
for (const m of startModule) {
  await js(`localStorage.setItem(${JSON.stringify(START_KEY)}, ${JSON.stringify(m)})`);
  const vorher = konsole.length;
  await ladeUndWarte(`Neuladen in ${m}`, () => send("Page.reload", { ignoreCache: false }));
  const aktiv = await js(`(document.querySelector(".module-section.is-active")||{}).dataset?.module || ""`);
  gestartet++;
  konsole.slice(vorher)
    .filter(z => /ReferenceError|TypeError|before initialization|is not defined/.test(z))
    .forEach(z => startFehler.push(`${m}${aktiv && aktiv !== m ? " (aktiv: " + aktiv + ")" : ""}: ${z}`));
}
await js(`localStorage.setItem(${JSON.stringify(START_KEY)}, "main")`);
if (startModule.length < 5) {
  bad("Start in jedem Modul", `nur ${startModule.length} Module gefunden — Pruefung sieht ihren Gegenstand nicht`);
} else if (startFehler.length === 0) {
  pass("Start in jedem Modul", `${gestartet} von ${startModule.length} Modulen ohne Ladefehler`);
} else {
  bad("Start in jedem Modul", `${startFehler.length} Ladefehler`);
  startFehler.slice(0, 6).forEach(z => console.log("          " + z));
}

// ----------------------------------------- 7. Netzmessung (E4.9, Instrument fuer AKV.19)
// Nur Ausgabe, kein Fehlschlag: Das Soll (hoechstens 2 gleichzeitig, in der Startphase
// einer; kein Scan im Automatikbetrieb ...) prueft der Pruefstand c65q in der Vorschau.
// Hier steht, was am Geraet tatsaechlich geschah.
info("/api/ gleichzeitig, Startphase", `hoechstens ${apiMax.start}`);
info("/api/ gleichzeitig, danach", `hoechstens ${apiMax.danach}`);
info("Abrufe je Endpunkt", ZAEHLEN.map(n => `${n} ${anzahl[n]}`).join(", "));
info("als Takt nicht abgewartet", taktPfade.size ? [...taktPfade].join(", ") : "keiner");
info("Verbindungsaufbau >= 900 ms", `${langsameVerbindungen} von ${verbindungen} neuen Verbindungen (laengster ${Math.round(langsamsteMs)} ms)`);
info("Laufzeit", `${((Date.now() - laufStart) / 1000).toFixed(1)} s`);

ws.close();
console.log(`\n=== ${ok} bestanden, ${fail} fehlgeschlagen ===`);
process.exit(fail > 0 ? 1 : 0);
