// Probe fuer Runde P2 (Paket 2026-10-06) -- laeuft gegen die Vorschau, nicht gegen das Geraet.
//
//     python3 tools/preview/server.py 8137 &
//     node tools/ui-mess/proben/runde-p2.mjs 8137
//
// Prueft AKP.14 (B40: Zeitserver und Zeitzone ueber prefillDeviceValue), AKP.15
// (Massnahme 4: zurueckgetippter Ausgangswert ist keine Aenderung), P2.1f (SSID-Auswahl)
// und P2.1 (Overlay-Text 32 Byte, Editor und Import). Den spaeten Scan stellt diese Probe
// durch direkten Aufruf von updateNetworkControlsFromMeta() nach; den echten Weg ueber
// eine zurueckgehaltene network_scan-Antwort geht b40-spaeter-scan.mjs.
//
// Gegenprobe (DIR-014), 07.10.2026: gegen 1.4.91 17 von 31 fehlgeschlagen. Die drei
// SSID-Gegenfaelle (unberuehrt, kein verwaister Eintrag, zurueckgewaehlt) bestehen auch
// dort -- sie schuetzen vor Ueberkorrektur, belegen den Fix nicht. "echte Aenderung =>
// Dialog" haengt am Zaehler des vorigen Falls und ist allein nicht aussagekraeftig.
// Geschrieben vom pwa-developer, uebernommen vom Lead.
import { starteBrowser } from "../wirt.mjs";
import { tmpdir } from "node:os";
import { setTimeout as sleep } from "node:timers/promises";
const port = process.argv[2];
const b = await starteBrowser({ port: 9361, profil: tmpdir() + "/runde-p2-profil" });
let ok = 0, fail = 0;
const pruef = (n, c, i = "") => { if (c) { ok++; console.log("  OK   " + n); } else { fail++; console.log("  FAIL " + n + (i ? "  -- " + i : "")); } };
const tippe = async (id, text) => {
  await b.js(`(() => { const e = document.getElementById(${JSON.stringify(id)}); e.focus(); e.select && e.select(); return true; })()`);
  await b.cdp("Input.insertText", { text }, b.sessionId);
};
const verlasse = () => b.js(`(document.activeElement && document.activeElement.blur(), true)`);
const instrumentiere = async () => {
  await b.js(`(() => { window.__status=[]; const o=window.announceStatus; window.announceStatus=(m,t)=>{window.__status.push([t,m]);return o(m,t)};
    window.__calls=[]; const of=window.fetch.bind(window); window.fetch=(u,o)=>{window.__calls.push(String(u&&u.url?u.url:u)); return of(u,o)};
    window.__confirms=0; window.confirm=()=>{window.__confirms++; return true}; window.sleep=()=>Promise.resolve(); return true })()`);
};
try {
  await b.gehe(`http://127.0.0.1:${port}/app/`); await sleep(2500);
  pruef("app.js geladen", await b.js(`typeof updateNetworkControlsFromMeta === "function"`));
  await instrumentiere();
  await b.js(`(document.querySelector('.module-chip[data-module-target="network"]').click(), true)`); await sleep(2500);

  console.log("AKP.14 / B40");
  const meta = (ts, tz) => `(() => { const m = getSettingsControlUiMeta(getCurrentSettingsSnapshot(), getCurrentNetworkInfo()).network; m.timeserver=${JSON.stringify(ts)}; m.timezoneOffset=${tz}; updateNetworkControlsFromMeta(m); return true })()`;
  const val = (id) => b.js(`document.getElementById("${id}").value`);
  const ts0 = await val("network-timeserver-input"), tz0 = await val("network-timezone-input");
  pruef("Ausgangswert gesetzt", ts0 !== "" && tz0 !== "", ts0 + "/" + tz0);
  for (const [id, neu] of [["network-timeserver-input", "neu.example"], ["network-timezone-input", "5"]]) {
    // fokussiert
    await tippe(id, neu); await b.js(meta("spaet.example", 7));
    pruef(id + ": fokussiert, später Scan überschreibt nicht", await val(id) === neu, await val(id));
    // geändert und verlassen
    await verlasse(); await b.js(meta("spaet2.example", 8));
    pruef(id + ": geändert+verlassen, später Scan überschreibt nicht", await val(id) === neu, await val(id));
  }
  // unberührt: Seite neu, nichts tippen
  await b.gehe(`http://127.0.0.1:${port}/app/`); await sleep(2500);
  await b.js(`(window.confirm=()=>true, document.querySelector('.module-chip[data-module-target="network"]').click(), true)`); await sleep(2500);
  await b.js(meta("unberuehrt.example", 9));
  pruef("Zeitserver unberührt übernimmt Gerätewert", await val("network-timeserver-input") === "unberuehrt.example");
  pruef("Zeitzone unberührt übernimmt Gerätewert", await val("network-timezone-input") === "9");
  pruef("Zeitzone 0 wird als 0 übernommen", (await b.js(meta("unberuehrt.example", 0)), await val("network-timezone-input")) === "0");

  console.log("AKP.15 / Massnahme 4");
  await b.gehe(`http://127.0.0.1:${port}/app/`); await sleep(2500);
  await b.js(`(window.__confirms=0, window.confirm=()=>{window.__confirms++; return true}, document.querySelector('.module-chip[data-module-target="network"]').click(), true)`); await sleep(2500);
  const ts1 = await val("network-timeserver-input");
  await tippe("network-timeserver-input", ts1 + "x");
  pruef("echte Eingabe setzt hasUnsavedEdits", await b.js(`hasUnsavedEdits`) === true);
  await tippe("network-timeserver-input", ts1);
  pruef("Ausgangswert wiederhergestellt -> nicht mehr geändert", await b.js(`hasUnsavedEdits`) === false);
  await b.js(`(document.querySelector('.module-chip[data-module-target="system"]').click(), true)`); await sleep(300);
  pruef("Modulwechsel nach Wiederherstellen: kein Dialog", await b.js(`window.__confirms`) === 0);
  await b.js(`(document.querySelector('.module-chip[data-module-target="network"]').click(), true)`); await sleep(1500);
  await tippe("network-timezone-input", "11");
  await b.js(`(document.querySelector('.module-chip[data-module-target="system"]').click(), true)`); await sleep(300);
  pruef("echte Änderung: Dialog weiterhin", await b.js(`window.__confirms`) === 1);
  // gemischt: Feld ohne Ausgangswert bleibt geändert, auch wenn Zeitserver zurückgetippt wird
  await b.js(`(window.__confirms=0, document.querySelector('.module-chip[data-module-target="network"]').click(), true)`); await sleep(1500);
  await tippe("network-ap-ssid-input", "Fremd"); await tippe("network-timeserver-input", (await val("network-timeserver-input")) + "y");
  await tippe("network-timeserver-input", ts1);
  pruef("anderes geändertes Feld hält das Flag", await b.js(`hasUnsavedEdits`) === true);
  // nach Speichern: Gerät meldet den eingegebenen Wert -> neuer Ausgangswert
  await b.js(`(markEditsPersisted(), true)`); await verlasse();
  await b.js(`(() => { const m = getSettingsControlUiMeta(getCurrentSettingsSnapshot(), getCurrentNetworkInfo()).network; m.apSsid="Fremd"; updateNetworkControlsFromMeta(m); return true })()`);
  pruef("gespeicherter Wert wird neuer Ausgangswert", await b.js(`document.getElementById("network-ap-ssid-input").dataset.devicePrefill`) === "Fremd");


  console.log("P2.1f / SSID-Auswahl");
  await b.gehe(`http://127.0.0.1:${port}/app/`); await sleep(2500); await instrumentiere();
  await b.js(`(document.querySelector('.module-chip[data-module-target="network"]').click(), true)`); await sleep(2500);
  const scan = (nets, cur) => b.js(`(() => { const m = getSettingsControlUiMeta(getCurrentSettingsSnapshot(), getCurrentNetworkInfo()).network; m.networks=${JSON.stringify(nets)}; m.currentSsid=${JSON.stringify(cur)}; updateNetworkControlsFromMeta(m); return true })()`);
  const sel = () => b.js(`(() => { const s=document.getElementById("network-ssid-select"); return { v: s.value, opts: [...s.options].map(o=>o.value) } })()`);
  await scan(["Heim", "Nachbar", "Gast"], "Heim");
  pruef("Ausgang: verbundenes Netz gewählt", (await sel()).v === "Heim", JSON.stringify(await sel()));
  // fokussiert, andere Wahl
  await b.js(`(() => { const s=document.getElementById("network-ssid-select"); s.focus(); s.value="Gast"; return true })()`);
  await scan(["Heim", "Nachbar", "Gast", "Neu1"], "Heim");
  let s1 = await sel();
  pruef("SSID fokussiert: Auswahl bleibt, neues Netz erscheint", s1.v === "Gast" && s1.opts.includes("Neu1"), JSON.stringify(s1));
  // geändert und verlassen
  await verlasse();
  await scan(["Heim", "Nachbar", "Gast"], "Heim");
  s1 = await sel();
  pruef("SSID geändert+verlassen: Auswahl bleibt", s1.v === "Gast", JSON.stringify(s1));
  // Netz verschwindet aus dem Scan
  await scan(["Heim", "Nachbar"], "Heim");
  s1 = await sel();
  pruef("SSID verschwunden: bleibt als Eintrag und gewählt", s1.v === "Gast" && s1.opts.includes("Gast"), JSON.stringify(s1));
  // leere Liste
  await scan([], "Heim");
  s1 = await sel();
  pruef("SSID bei leerem Scan: Auswahl bleibt", s1.v === "Gast", JSON.stringify(s1));
  // unberührt
  await b.gehe(`http://127.0.0.1:${port}/app/`); await sleep(2500); await instrumentiere();
  await b.js(`(document.querySelector('.module-chip[data-module-target="network"]').click(), true)`); await sleep(2500);
  await scan(["Heim", "Nachbar"], "Heim");
  await scan(["Heim", "Nachbar", "Neu2"], "Neu2");
  s1 = await sel();
  pruef("SSID unberührt: folgt dem Gerät", s1.v === "Neu2", JSON.stringify(s1));
  await scan(["Nachbar", "Neu3"], "Neu3");
  s1 = await sel();
  pruef("SSID unberührt, altes Netz weg: kein verwaister Eintrag", s1.v === "Neu3" && !s1.opts.includes("Neu2"), JSON.stringify(s1));
  // zurückgewählt auf Ausgangswert gilt nicht als offen
  await b.js(`(() => { const s=document.getElementById("network-ssid-select"); s.value="Nachbar"; s.value="Neu3"; return true })()`);
  await scan(["Nachbar", "Neu3", "Neu4"], "Neu4");
  s1 = await sel();
  pruef("SSID auf Ausgangswert zurück: folgt dem Gerät", s1.v === "Neu4", JSON.stringify(s1));
  await b.gehe(`http://127.0.0.1:${port}/app/`); await sleep(2500); await instrumentiere();
  console.log("P2.1 / AKP.1 + AKP.10 Overlay-Text");
  await b.js(`(document.querySelector('.module-chip[data-module-target="overlays"]').click(), true)`); await sleep(2500);
  const hatRow = await b.js(`!!document.getElementById("ov-type-0")`);
  pruef("Overlay-Zeile 0 vorhanden", hatRow);
  for (const [n, sollRaus] of [[17, false], [16, true]]) {
    await b.js(`(() => { window.__calls.length=0; window.__status.length=0; const t=document.getElementById("ov-type-0"); t.value="6"; updateOverlayRowVisibility(0); document.getElementById("ov-value-0").value="${"ä".repeat(n)}"; return true })()`);
    await b.js(`saveOverlay(0).then(()=>true)`);
    const raus = await b.js(`window.__calls.some(u=>u.indexOf("overlay_set")>=0)`);
    const st = await b.js(`window.__status`);
    pruef(n + " Umlaute (" + 2*n + " Byte): Anfrage " + (sollRaus ? "geht hinaus" : "unterbleibt"), raus === sollRaus, JSON.stringify(st));
    if (!sollRaus) pruef("Meldung nennt 34 und 32 Byte", st.some(([t, m]) => t === "error" && m.includes("34") && m.includes("32") && m.includes("Overlay-Text")), JSON.stringify(st));
  }
  await b.js(`(() => { window.__calls.length=0; resetTooLongImportFields(); return true })()`);
  await b.js(`importOverlaySettings({ items: [ {idx:0,type:6,value:"${"a".repeat(33)}"}, {idx:1,type:6,value:"kurz"}, {idx:2,type:6,value:"${"ä".repeat(17)}"}, {idx:3,type:1,value:"herz"} ] }).then(()=>true)`);
  const sets = await b.js(`window.__calls.filter(u=>u.indexOf("overlay_set")>=0)`);
  pruef("Import: 2 von 4 Overlays geschrieben", sets.length === 2, JSON.stringify(sets));
  pruef("Import: lückenlose Indizes 0,1", sets.length === 2 && /idx=0\b/.test(sets[0]) && /idx=1\b/.test(sets[1]), JSON.stringify(sets));
  const summ = await b.js(`getImportNoticeSummary()`);
  pruef("Import: beide genannt (Overlay 1 und 3, 33/34 Byte)", summ.includes("Overlay 1 (33") && summ.includes("Overlay 3 (34"), summ);
  pruef("keine Seitenfehler", (await b.js(`(window.__pageErrors||[]).length`)) === 0);
} finally { console.log(`\n${ok} bestanden, ${fail} fehlgeschlagen, ${ok + fail} Fälle`); b.ende(); }
process.exit(fail ? 1 : 0);
