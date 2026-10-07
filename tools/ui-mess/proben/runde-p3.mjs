// Probe fuer Runde P3 (Paket 2026-10-06, P3.2) -- A5 in der PWA und B41/L326, gegen die Vorschau.
//
//     python3 tools/preview/server.py 8137 &
//     node tools/ui-mess/proben/runde-p3.mjs 8137
//
// A5: RTC-Temperatur aus Index 49 (halbe Grad, Zweierkomplement, im XML vorzeichenlos),
// Rueckfall auf Index 21 bei fehlendem Index 49 (alter ESP) und bei 0x8000 (alter STM,
// RTC-Lesefehler). Die Probe schreibt die settings_xml-Antwort per fetch-Huelle um.
// Minusgrade sind am Geraet nicht herstellbar -- diese Probe ist der tragende Nachweis.
// B41: Eine WLAN-Auswahl ohne Ausgangswert uebersteht ein Speichern in derselben Sektion.
//
// Gegenprobe (DIR-014), 08.10.2026, gegen PWA 1.4.92: 16 OK, 10 FAIL -- darunter die vier
// Werte -20, -1, 0, +49 und der Rundungsfehler von formatHalfDegreeValue (-3 ergab -2.5).
// Die beiden Rueckfallfaelle bestehen dort zwangslaeufig, weil 1.4.92 immer Index 21 liest.
// Geschrieben vom pwa-developer, uebernommen vom Lead.
import { starteBrowser } from "../wirt.mjs";
import { tmpdir } from "node:os";
import { setTimeout as sleep } from "node:timers/promises";

const port = process.argv[2];
const b = await starteBrowser({ port: 9381, profil: tmpdir() + "/p3-probe-profil" });
let ok = 0, fail = 0;
const pruef = (name, bed, info = "") => {
  if (bed) { ok++; console.log("  OK   " + name); }
  else { fail++; console.log("  FAIL " + name + (info ? "  -- " + info : "")); }
};

// Instrumentierung: settings_xml umschreiben (Index 21 und 49 aus localStorage),
// network_client_set auf Wunsch scheitern lassen.
await b.cdp("Page.addScriptToEvaluateOnNewDocument", { source: `
  window.__pageErrors = [];
  addEventListener('error', e => window.__pageErrors.push(String(e.message)));
  window.confirm = () => true;
  const of = window.fetch.bind(window);
  window.fetch = async (url, opts) => {
    const u = String(url && url.url ? url.url : url);
    if (u.indexOf('/api/network_client_set') >= 0 && localStorage.getItem('__p3ClientFail') === '1') {
      return new Response('fail', { status: 500 });
    }
    const r = await of(url, opts);
    const fall = localStorage.getItem('__p3case');
    if (u.indexOf('/api/settings_xml') < 0 || !fall) return r;
    const c = JSON.parse(fall);
    let t = await r.text();
    t = t.replace(/<numvar idx="21" value="[^"]*" \\/>/, '<numvar idx="21" value="' + c.n21 + '" />');
    t = t.replace(/<numvar idx="49" value="[^"]*" \\/>/, c.n49 === null ? '' : '<numvar idx="49" value="' + c.n49 + '" />');
    return new Response(t, { status: 200, headers: { 'Content-Type': 'text/xml; charset=utf-8' } });
  };
` }, b.sessionId);

const rtcText = () => b.js(`(() => { const it=[...document.querySelectorAll('#temperature-list .info-item')].find(e=>e.querySelector('.label').textContent==='RTC'); return it ? it.querySelector('strong').textContent : null })()`);
const dsText = () => b.js(`(() => { const it=[...document.querySelectorAll('#temperature-list .info-item')].find(e=>e.querySelector('.label').textContent==='DS18xx'); return it ? it.querySelector('strong').textContent : null })()`);

try {
  await b.gehe(`http://127.0.0.1:${port}/app/`); await sleep(1500);
  console.log("P3.1 / A5 RTC-Temperatur (Index 21 = 44, also 22.0 °C im Rueckfall)");
  // n49: Wert wie im XML (vorzeichenlos); null = Index fehlt
  const FAELLE = [
    ["Index 49 = -20 (65516)", 44, 65516, "-10.0 °C"],
    ["Index 49 = -1 (65535)", 44, 65535, "-0.5 °C"],
    ["Index 49 = -3 (65533)", 44, 65533, "-1.5 °C"],
    ["Index 49 = 0", 44, 0, "0.0 °C"],
    ["Index 49 = +49", 44, 49, "24.5 °C"],
    ["Index 49 = 0x8000 -> Rueckfall Index 21", 44, 32768, "22.0 °C"],
    ["Index 49 fehlt -> Rueckfall Index 21", 44, null, "22.0 °C"],
    ["Index 49 = 0x8000, Index 21 = 0 (RTC-Lesefehler, H6)", 0, 32768, "0.0 °C"],
    ["Index 49 fehlt, Index 21 = 255 (wie heute)", 255, null, "ungültig"],
  ];
  for (const [name, n21, n49, soll] of FAELLE) {
    await b.js(`(localStorage.setItem('__p3case', ${JSON.stringify(JSON.stringify({ n21, n49 }))}), true)`);
    await b.gehe(`http://127.0.0.1:${port}/app/`); await sleep(2000);
    await b.js(`(document.querySelector('.module-chip[data-module-target="climate"]').click(), true)`); await sleep(800);
    const ist = await rtcText();
    pruef(name + " => " + soll, ist === soll, "ist " + JSON.stringify(ist));
    if (n49 === 65516) pruef("DS18xx unveraendert (Index 23 = 45 => 22.5 °C)", (await dsText()) === "22.5 °C", "ist " + JSON.stringify(await dsText()));
  }
  await b.js(`(localStorage.removeItem('__p3case'), true)`);
  // Formatierfunktion direkt: faengt den Rundungsfehler auch ohne Index 49 (Gegenprobe HEAD)
  for (const [w, soll] of [[-3, "-1.5 °C"], [-1, "-0.5 °C"], [-20, "-10.0 °C"], [49, "24.5 °C"], [0, "0.0 °C"]]) {
    const ist = await b.js(`formatHalfDegreeValue(${w})`);
    pruef("formatHalfDegreeValue(" + w + ") => " + soll, ist === soll, "ist " + JSON.stringify(ist));
  }

  console.log("B41 / L326 WLAN-Auswahl ohne Ausgangswert");
  const verlasse = () => b.js(`(document.activeElement && document.activeElement.blur(), true)`);
  const waehle = async (id, k) => {
    await b.js(`(document.getElementById(${JSON.stringify(id)}).focus(), true)`);
    for (const t of ["keyDown", "keyUp"]) await b.cdp("Input.dispatchKeyEvent", { type: t, key: k, code: "Key" + k.toUpperCase(), text: t === "keyDown" ? k : undefined, windowsVirtualKeyCode: k.toUpperCase().charCodeAt(0) }, b.sessionId);
  };
  const tippe = async (id, text) => {
    await b.js(`(() => { const e=document.getElementById(${JSON.stringify(id)}); e.focus(); e.select(); return true })()`);
    await b.cdp("Input.insertText", { text }, b.sessionId);
  };
  const scanN = (nets, cur) => b.js(`(() => { const m = getSettingsControlUiMeta(getCurrentSettingsSnapshot(), getCurrentNetworkInfo()).network; m.networks=${JSON.stringify(nets)}; m.currentSsid=${JSON.stringify(cur)}; updateNetworkControlsFromMeta(m); return true })()`);
  const selN = () => b.js(`(() => { const s=document.getElementById("network-ssid-select"); return { v: s.value, base: s.dataset.devicePrefill, uc: s.dataset.userChoice, opts: [...s.options].map(o=>o.value) } })()`);
  const netzStart = async (clientFail = false) => {
    await b.js(`(localStorage.setItem('__p3ClientFail', ${JSON.stringify(clientFail ? "1" : "0")}), true)`);
    await b.gehe(`http://127.0.0.1:${port}/app/`); await sleep(2000);
    await b.js(`(document.querySelector('.module-chip[data-module-target="network"]').click(), true)`); await sleep(2000);
  };
  const speichereZeitserver = async () => {
    await tippe("network-timeserver-input", "ntp.example.ch"); await verlasse();
    await b.js(`(document.getElementById("network-timeserver-save-button").click(), true)`); await sleep(1500);
  };

  // 1. Kern von L326: Wahl ohne Ausgangswert, dann Zeitserver gespeichert, dann Scan
  await netzStart();
  await scanN(["Bravo", "Charlie"], "Alpha");
  await waehle("network-ssid-select", "c"); await verlasse();
  let s = await selN();
  pruef("B41: Wahl Charlie ohne Ausgangswert", s.v === "Charlie" && s.base === undefined, JSON.stringify(s));
  await speichereZeitserver();
  pruef("B41: Zeitserver-Speichern hat markEditsPersisted ausgeloest", (await b.js(`hasUnsavedEdits`)) === false);
  await scanN(["Bravo", "Charlie", "Delta"], "Alpha");
  s = await selN();
  pruef("B41: Wahl uebersteht anderes Speichern in derselben Sektion", s.v === "Charlie", JSON.stringify(s));
  await scanN(["Bravo", "Delta"], "Alpha");
  s = await selN();
  pruef("B41: ... auch wenn Charlie danach aus dem Scan faellt", s.v === "Charlie" && s.opts.includes("Charlie"), JSON.stringify(s));
  // 2. Erfolgreiches WLAN-Speichern loescht das Merkmal
  await b.js(`(document.getElementById("network-client-save-button").click(), true)`); await sleep(600);
  s = await selN();
  pruef("B41: nach erfolgreichem WLAN-Speichern kein Merkmal mehr", s.uc === undefined, JSON.stringify(s));
  await sleep(2000);
  await scanN(["Bravo", "Delta"], "Alpha");
  s = await selN();
  pruef("B41: danach ist der angezeigte erste Eintrag wieder blosse Anzeige", s.v === "Bravo" && !s.opts.includes("Charlie"), JSON.stringify(s));
  // 3. Gescheitertes WLAN-Speichern laesst das Merkmal stehen
  await netzStart(true);
  await scanN(["Bravo", "Charlie"], "Alpha");
  await waehle("network-ssid-select", "c"); await verlasse();
  await b.js(`(document.getElementById("network-client-save-button").click(), true)`); await sleep(1500);
  s = await selN();
  pruef("B41: gescheitertes WLAN-Speichern -> Merkmal bleibt", s.uc === "1", JSON.stringify(s));
  await scanN(["Bravo", "Charlie", "Delta"], "Alpha");
  pruef("B41: ... und die Wahl bleibt", (await selN()).v === "Charlie", JSON.stringify(await selN()));
  await b.js(`(localStorage.setItem('__p3ClientFail', '0'), true)`);
  // 4. Unberuehrt: kein Merkmal, erster Eintrag folgt dem Scan auch nach anderem Speichern
  await netzStart();
  await scanN(["Bravo", "Charlie"], "Alpha");
  await speichereZeitserver();
  await scanN(["Charlie", "Bravo"], "Alpha");
  s = await selN();
  pruef("B41: unberuehrt -> kein Merkmal, folgt der Liste", s.uc === undefined && s.v === "Charlie", JSON.stringify(s));
  // 5. Gegenrichtung: Mit Ausgangswert entscheidet weiter der Ausgangswert
  await netzStart();
  await scanN(["Alpha", "Bravo"], "Alpha");
  await waehle("network-ssid-select", "b"); await verlasse();
  await speichereZeitserver();
  await scanN(["Alpha", "Bravo", "Charlie"], "Alpha");
  pruef("B41: mit Ausgangswert bleibt die Wahl ebenfalls", (await selN()).v === "Bravo", JSON.stringify(await selN()));

  pruef("keine Seitenfehler", (await b.js(`(window.__pageErrors||[]).length`)) === 0, JSON.stringify(await b.js(`window.__pageErrors`)));
} finally {
  console.log(`=== ${ok} OK, ${fail} FAIL (${ok + fail} Faelle)`);
  b.ende();
  process.exit(fail ? 1 : 0);
}
