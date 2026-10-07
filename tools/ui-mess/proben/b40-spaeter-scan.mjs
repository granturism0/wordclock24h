// Probe fuer B40 / L321 (Paket 2026-10-06, P2.1c) -- laeuft gegen die Vorschau, nicht gegen das Geraet.
//
//     python3 tools/preview/server.py 8137 &
//     node tools/ui-mess/proben/b40-spaeter-scan.mjs 8137
//
// Der Fehler ist ein Wettlauf: Kommt die network_scan-Antwort erst, nachdem der Nutzer
// in Zeitserver oder Zeitzone getippt hat, schrieb updateNetworkControlsFromMeta() den
// Geraetewert darueber, und "Speichern" schickte still den alten Wert. Am Geraet trifft
// ein Lauf das nur zufaellig (am 05.10.2026 einmal getroffen, 23:58:46) -- und meldet
// gruen, wenn er es verfehlt. Hier wird die Scan-Antwort ABSICHTLICH zurueckgehalten,
// waehrend getippt wird, und erst danach ausgeliefert. Der Weg ist der echte: Modul
// oeffnen, fetch auf /api/network_scan, settleFetchJson, refreshNetworkUi.
//
// Haltezeit unter CONNECTION_STABILITY.networkScanTimeoutMs (3800 ms), sonst greift der
// Rueckfallwert und die Antwort kommt gar nicht mehr an -- dann bewiese die Probe nichts.
// Deshalb prueft sie zuerst, dass die zurueckgehaltene Antwort tatsaechlich angekommen ist.
//
// Gegenprobe (DIR-014), 07.10.2026: gegen 1.4.90 schlaegt sie an, gegen P2 nicht.
import { starteBrowser } from "../wirt.mjs";
import { tmpdir } from "node:os";
import { setTimeout as sleep } from "node:timers/promises";

const port = process.argv[2];
const b = await starteBrowser({ port: 9371, profil: tmpdir() + "/b40-profil" });

let ok = 0, fail = 0;
const pruef = (name, bed, info = "") => {
  if (bed) { ok++; console.log("  OK   " + name); }
  else { fail++; console.log("  FAIL " + name + (info ? "  -- " + info : "")); }
};

const SCAN = { networks: [{ ssid: "Heim", rssi: -50 }, { ssid: "Nachbar", rssi: -70 }, { ssid: "Gast", rssi: -80 }], ssid: "Heim", ip: "192.0.2.10", mode: "STA" };

await b.cdp("Page.addScriptToEvaluateOnNewDocument", { source: `
  window.__pageErrors = [];
  addEventListener('error', e => window.__pageErrors.push(String(e.message)));
  window.confirm = () => true;
  window.__scanHeld = 0; window.__scanDelivered = 0; window.__scanWaiters = [];
  const of = window.fetch.bind(window);
  window.fetch = (url, opts) => {
    const u = String(url && url.url ? url.url : url);
    if (u.indexOf('/api/network_scan') >= 0) {
      window.__scanHeld++;
      return new Promise(res => window.__scanWaiters.push(() => { window.__scanDelivered++;
        res(new Response(${JSON.stringify(JSON.stringify(SCAN))}, { status: 200, headers: { 'Content-Type': 'application/json' } })); }));
    }
    return of(url, opts);
  };
  window.__releaseScans = () => { const w = window.__scanWaiters.splice(0); w.forEach(f => f()); return w.length; };
` }, b.sessionId);

const tippe = async (id, text) => {
  await b.js(`(() => { const e = document.getElementById(${JSON.stringify(id)}); e.focus(); e.select && e.select(); return true; })()`);
  await b.cdp("Input.insertText", { text }, b.sessionId);
};
const val = (id) => b.js(`document.getElementById(${JSON.stringify(id)}).value`);

// Seite frisch laden, Netzwerkmodul oeffnen, warten bis der Scan haengt.
async function oeffneMitHaengendemScan() {
  await b.gehe(`http://127.0.0.1:${port}/app/`);
  await sleep(2500);
  await b.js(`(window.__releaseScans(), window.__scanHeld = 0, window.__scanDelivered = 0, true)`);
  await b.js(`(document.querySelector('.module-chip[data-module-target="network"]').click(), true)`);
  for (let i = 0; i < 30 && !(await b.js(`window.__scanWaiters.length`)); i++) await sleep(100);
  return b.js(`window.__scanWaiters.length > 0`);
}
async function liefereAus() {
  const n = await b.js(`window.__releaseScans()`);
  await sleep(800);
  return n;
}

try {
  const FELDER = [["network-timeserver-input", "ntp.zürich-ä.ch"], ["network-timezone-input", "5"]];
  for (const [id, neu] of FELDER) {
    console.log(id);

    pruef(id + ": Scan haengt beim Oeffnen", await oeffneMitHaengendemScan());
    await tippe(id, neu);
    await sleep(1200);                       // Haltezeit, deutlich unter 3800 ms
    await liefereAus();
    pruef(id + ": zurueckgehaltene Antwort kam an", await b.js(`window.__scanDelivered > 0`));
    pruef(id + ": fokussiert, spaeter Scan ueberschreibt nicht", await val(id) === neu, await val(id));

    await oeffneMitHaengendemScan();
    await tippe(id, neu);
    await b.js(`(document.activeElement && document.activeElement.blur(), true)`);
    await sleep(1200);
    await liefereAus();
    pruef(id + ": geaendert und verlassen, spaeter Scan ueberschreibt nicht", await val(id) === neu, await val(id));

    await oeffneMitHaengendemScan();
    const vorher = await val(id);
    await liefereAus();
    const nachher = await val(id);
    pruef(id + ": unberuehrt, uebernimmt den Geraetewert", nachher !== "" && nachher !== neu, JSON.stringify([vorher, nachher]));
  }

  console.log("network-ssid-select (P2.1f)");
  await oeffneMitHaengendemScan();
  await liefereAus();                        // erste Liste aufbauen
  await b.js(`(window.__releaseScans(), document.querySelector('.module-chip[data-module-target="system"]').click(), true)`);
  await sleep(1500);
  await b.js(`(document.querySelector('.module-chip[data-module-target="network"]').click(), true)`);
  for (let i = 0; i < 30 && !(await b.js(`window.__scanWaiters.length`)); i++) await sleep(100);
  const hatGast = await b.js(`[...document.getElementById("network-ssid-select").options].some(o => o.value === "Gast")`);
  if (hatGast) {
    await b.js(`(() => { const s = document.getElementById("network-ssid-select"); s.focus(); s.value = "Gast"; s.dispatchEvent(new Event("change", { bubbles: true })); return true; })()`);
    await sleep(1200);
    await liefereAus();
    pruef("SSID: ungespeicherte Auswahl bleibt nach spaetem Scan", await val("network-ssid-select") === "Gast", await val("network-ssid-select"));
  } else {
    pruef("SSID: Auswahlliste aus erstem Scan aufgebaut", false, "kein Eintrag Gast");
  }

  pruef("keine Seitenfehler", (await b.js(`window.__pageErrors.length`)) === 0, JSON.stringify(await b.js(`window.__pageErrors`)));
} finally {
  console.log(`\n=== ${ok} OK, ${fail} FAIL (${ok + fail} Faelle)`);
  b.ende();
}
process.exit(fail ? 1 : 0);
