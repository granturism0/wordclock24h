// Probe fuer Runde P4 (PWA-Anpassungen nach ESP 3.2.26, Runde F) -- gegen die Vorschau.
//
//     python3 tools/preview/server.py 8137 &
//     node tools/ui-mess/proben/runde-p4.mjs 8137
//
// B1 (C27): Kennung 8 "Datei leer" wie bisher als leer, bei .gz mit Warnung. B2 (Ent-4):
// Loeschen unterscheidet 6 und 7. B3 (Ent-5): leerer stm32_default -> Hinweis mit
// Rueckweg, Flash-Knopf gesperrt, auch bei gefuellter Liste (DIR-010). B4 (C38): SSID
// und Schluessel 31/63 Byte vor dem Senden, auch im Backup-Import (alles oder nichts).
// B5 (L341): Ganzzahlfelder weisen 8.5 und 1e1 ab. Die Probe stellt Antworten ueber eine
// fetch-Huelle selbst und laeuft deshalb auch gegen aeltere Vorschauen.
//
// Gegenprobe (DIR-014), 08.10.2026: gegen 1.4.93 26 OK / 41 FAIL (67 Faelle), gegen den
// Stand vor dem B3-Nachtrag 68/3. Neu 71/0. Geschrieben vom pwa-developer.
// Review P4 (H1 maxlength 32/64 statt 31/63, M1 badInput): Stand davor mit Markup 31/63
// 75/14 (89 Faelle), nachher 89/0.
import { starteBrowser } from "../wirt.mjs";
import { tmpdir } from "node:os";
import { readFileSync } from "node:fs";
import { setTimeout as sleep } from "node:timers/promises";

const port = process.argv[2];
const root = process.argv[3] || new URL("../../..", import.meta.url).pathname.replace(/\/$/, "");
const b = await starteBrowser({ port: 9471 + (Number(port) % 2), profil: tmpdir() + "/p4-probe-profil-" + port });
let ok = 0, fail = 0;
const pruef = (name, bed, info = "") => {
  if (bed) { ok++; console.log("  OK   " + name); }
  else { fail++; console.log("  FAIL " + name + (info ? "  -- " + info : "")); }
};

await b.cdp("Page.addScriptToEvaluateOnNewDocument", { source: `
  window.__pageErrors = [];
  addEventListener('error', e => window.__pageErrors.push(String(e.message)));
  window.confirm = () => true;
  window.__calls = [];
  window.__mock = {};
  const of = window.fetch.bind(window);
  const json = (o) => new Response(JSON.stringify(o), { status: 200, headers: { 'Content-Type': 'application/json' } });
  window.fetch = async (url, opts) => {
    const u = String(url && url.url ? url.url : url);
    window.__calls.push(u);
    for (const k of Object.keys(window.__mock)) {
      if (u.indexOf(k) >= 0) {
        const m = window.__mock[k];
        if (m.plain !== undefined) return new Response(m.plain, { status: 200, headers: { 'Content-Type': 'text/plain' } });
        return json(m);
      }
    }
    const us = localStorage.getItem('__p4UpdateStatus');
    if (us && u.indexOf('/api/update_status') >= 0) return json(JSON.parse(us));
    return of(url, opts);
  };
` }, b.sessionId);

const laden = async () => {
  await b.gehe(`http://127.0.0.1:${port}/app/`); await sleep(2200);
  await b.js(`(() => { window.__ann = []; const o = announceStatus; announceStatus = (m, t) => { window.__ann.push([m, t || ""]); return o(m, t); }; return true })()`);
};
const ann = () => b.js(`window.__ann.length ? window.__ann[window.__ann.length - 1] : null`);
const calls = (frag) => b.js(`window.__calls.filter(u => u.indexOf(${JSON.stringify(frag)}) >= 0)`);
const mock = (k, v) => b.js(`(window.__mock[${JSON.stringify(k)}] = ${JSON.stringify(v)}, window.__calls = [], window.__ann = [], true)`);
const unmock = () => b.js(`(window.__mock = {}, window.__calls = [], window.__ann = [], true)`);
const fsStatus = () => b.js(`[document.getElementById("fs-action-status").textContent, document.getElementById("fs-preview-content").textContent]`);

try {
  await laden();

  console.log("B1 / C27 fs_show: Kennung 8 = leere Datei");
  await mock("/api/fs_show", { ok: false, error: 8, detail: "file empty" });
  await b.js(`showFsFile("leer.txt", 0)`);
  let [st, pv] = await fsStatus(); let a = await ann();
  pruef("Kennung 8, .txt -> 'ist leer — 0 Byte ... gibt es'", /ist leer — 0 Byte\. Das ist kein Fehler, die Datei gibt es\./.test(st), st);
  pruef("Kennung 8, .txt -> Vorschau 'Diese Datei ist leer'", pv === "Diese Datei ist leer — 0 Byte.", pv);
  pruef("Kennung 8, .txt -> Ton ok", a && a[1] === "ok", JSON.stringify(a));
  await mock("/api/fs_show", { ok: false, error: 8, detail: "file empty" });
  await b.js(`showFsFile("app-app.js.gz", 0)`);
  [st, pv] = await fsStatus(); a = await ann();
  pruef("Kennung 8, .gz -> leer und Weissbildschirm-Warnung", /ist leer — 0 Byte/.test(st) && /weissen Bildschirm/.test(st), st);
  pruef("Kennung 8, .gz -> Ton warn", a && a[1] === "warn", JSON.stringify(a));
  await mock("/api/fs_show", { plain: "" });
  await b.js(`showFsFile("alt-leer.txt", 0)`);
  [st, pv] = await fsStatus(); a = await ann();
  pruef("alte Firmware: leerer Rumpf -> weiterhin leer", /ist leer — 0 Byte\. Das ist kein Fehler/.test(st) && a && a[1] === "ok", st + " " + JSON.stringify(a));
  await mock("/api/fs_show", { plain: "" });
  await b.js(`showFsFile("alt-leer.js.gz", 0)`);
  [st] = await fsStatus();
  pruef("alte Firmware: leere .gz -> Warnung", /weissen Bildschirm/.test(st), st);
  await mock("/api/fs_show", { plain: "Hallo Welt" });
  await b.js(`showFsFile("text.txt", 10)`);
  [st, pv] = await fsStatus();
  pruef("Inhalt wird angezeigt", pv === "Hallo Welt" && /wird angezeigt/.test(st), st + " | " + pv);
  await mock("/api/fs_show", { ok: false, error: 6, detail: "file not found" });
  await b.js(`showFsFile("fehlt.txt", 0)`);
  [st, pv] = await fsStatus(); a = await ann();
  pruef("Kennung 6 -> 'gibt es nicht', Fehler", /Diese Datei gibt es auf dem Gerät nicht\. \(file not found\)/.test(st) && a[1] === "error", st);
  await mock("/api/fs_show", { ok: false, error: 7, detail: "open failed" });
  await b.js(`showFsFile("kaputt.txt", 0)`);
  [st, pv] = await fsStatus(); a = await ann();
  pruef("Kennung 7 -> eigener Text, nicht 'gibt es nicht'", /nicht ausführen/.test(st) && !/gibt es auf dem Gerät nicht/.test(st) && /\(open failed\)/.test(st) && a[1] === "error", st);
  pruef("Kennung 7 -> Vorschau 'nicht verfügbar'", pv !== "" && !/Hallo Welt/.test(pv), pv);

  console.log("B2 / Ent-4 fs_remove: Kennung 6 und 7 unterscheidbar");
  await mock("/api/fs_remove", { ok: false, error: 6, detail: "file not found" });
  await b.js(`deleteFsFile("weg.txt")`);
  const a6 = await ann();
  pruef("fs_remove Kennung 6 -> 'gibt es nicht'", a6 && /gibt es auf dem Gerät nicht/.test(a6[0]) && a6[1] === "error", JSON.stringify(a6));
  await mock("/api/fs_remove", { ok: false, error: 7, detail: "remove failed" });
  await b.js(`deleteFsFile("fest.txt")`);
  const a7 = await ann();
  pruef("fs_remove Kennung 7 -> eigener Text mit (remove failed)", a7 && /nicht ausführen/.test(a7[0]) && /\(remove failed\)/.test(a7[0]) && a7[1] === "error", JSON.stringify(a7));
  pruef("fs_remove 6 und 7 verschieden", a6 && a7 && a6[0] !== a7[0]);
  pruef("fs_remove Fehler steht auch im Dateistatus", /\(remove failed\)/.test((await fsStatus())[0]), (await fsStatus())[0]);
  await mock("/api/fs_remove", { ok: false });
  await b.js(`deleteFsFile("x.txt")`);
  const a0 = await ann();
  pruef("fs_remove ohne Kennung -> Rueckfalltext", a0 && a0[0] === "Datei konnte nicht gelöscht werden", JSON.stringify(a0));
  await unmock();

  console.log("B3 / Ent-5 STM32-Liste");
  const stmOpt = () => b.js(`[...document.getElementById("update-stm32-select").options].map(o => o.textContent)`);
  const mitStatus = async (us) => {
    await b.js(`(localStorage.setItem('__p4UpdateStatus', ${JSON.stringify(JSON.stringify(us))}), true)`);
    await laden();
    await b.js(`(document.querySelector('.module-chip[data-module-target="maintenance"]').click(), true)`); await sleep(1500);
  };
  const basis = { ok: true, host: "update.wordclock.ch", path: "/firmware/", esp_version: "3.2.26" };
  await mitStatus({ ...basis, stm32_default: "", stm32_files: [] });
  let o = await stmOpt();
  pruef("stm32_default leer, Liste leer -> Hardware nicht erkannt", o.length === 1 && /Hardware nicht erkannt/.test(o[0]) && /zurück/.test(o[0]), JSON.stringify(o));
  pruef("... Flash-Knopf gesperrt", await b.js(`document.getElementById("update-stm32-button").disabled`));
  // Lead-Entscheid: Standard leer, Liste gefuellt -> ESP weist jeden Namen ab (DIR-010),
  // also ebenso gesperrt mit Hinweis, keine waehlbare Datei.
  await mitStatus({ ...basis, stm32_default: "", stm32_files: ["wc24h-stm32f411ce-25-sk6812-rgbw.hex", "wc24h-stm32f103.hex"] });
  o = await stmOpt();
  pruef("stm32_default leer, Liste gefuellt -> Hardware nicht erkannt", o.length === 1 && /Hardware nicht erkannt/.test(o[0]), JSON.stringify(o));
  pruef("... Flash-Knopf gesperrt", await b.js(`document.getElementById("update-stm32-button").disabled`));
  pruef("... keine Datei waehlbar", (await b.js(`document.getElementById("update-stm32-select").value`)) === "");
  await mitStatus({ ...basis, stm32_default: "wc24h-stm32f411ce-25-sk6812-rgbw.hex", stm32_files: [] });
  o = await stmOpt();
  pruef("stm32_default gesetzt, Liste leer -> keine STM32-Dateien", o.length === 1 && o[0] === "keine STM32-Dateien gefunden", JSON.stringify(o));
  await mitStatus({ ...basis });
  o = await stmOpt();
  pruef("stm32_default fehlt (alter ESP/kein Status) -> keine STM32-Dateien", o.length === 1 && o[0] === "keine STM32-Dateien gefunden", JSON.stringify(o));
  await mitStatus({ ...basis, stm32_default: "a.hex", stm32_files: ["a.hex", "b.hex"] });
  o = await stmOpt();
  pruef("Liste gefuellt -> Dateien", JSON.stringify(o) === '["a.hex","b.hex"]', JSON.stringify(o));
  pruef("... Flash-Knopf frei, Standard gewaehlt", (await b.js(`document.getElementById("update-stm32-button").disabled`)) === false && (await b.js(`document.getElementById("update-stm32-select").value`)) === "a.hex");
  await b.js(`(localStorage.removeItem('__p4UpdateStatus'), true)`);
  await laden();

  console.log("B4 / C38 WLAN-Byte-Grenzen");
  const setz = (id, v) => b.js(`(document.getElementById(${JSON.stringify(id)}).value = ${JSON.stringify(v)}, true)`);
  const client = async (ssid, key, sel) => {
    await b.js(`(window.__calls = [], window.__ann = [], true)`);
    if (sel !== undefined) {
      await b.js(`(() => { const s = document.getElementById("network-ssid-select"); s.innerHTML = ""; const op = document.createElement("option"); op.value = op.textContent = ${JSON.stringify(sel)}; s.appendChild(op); s.value = ${JSON.stringify(sel)}; return true })()`);
    }
    await setz("network-ssid-manual-input", ssid); await setz("network-key-input", key);
    await b.js(`saveNetworkClient()`); await sleep(200);
    return { n: (await calls("/api/network_client_set")).length, a: await ann() };
  };
  const ap = async (ssid, key) => {
    await b.js(`(window.__calls = [], window.__ann = [], true)`);
    await setz("network-ap-ssid-input", ssid); await setz("network-ap-key-input", key);
    await b.js(`saveNetworkAp()`); await sleep(200);
    return { n: (await calls("/api/network_ap_set")).length, a: await ann() };
  };
  const zuLang = (r) => r.n === 0 && r.a && /^Zu lang für/.test(r.a[0]) && r.a[1] === "error";
  let r = await client("S".repeat(32), "geheim1234");
  pruef("SSID 32 Byte -> abgewiesen, nichts gesendet", zuLang(r) && /32 Byte/.test(r.a[0]) && /höchstens 31/.test(r.a[0]), JSON.stringify(r));
  r = await client("S".repeat(31), "geheim1234");
  pruef("SSID 31 Byte -> gesendet", r.n === 1, JSON.stringify(r));
  r = await client("ä".repeat(16), "geheim1234");
  pruef("SSID 16 Umlaute = 32 Byte -> abgewiesen", zuLang(r), JSON.stringify(r));
  r = await client("Netz", "k".repeat(64));
  pruef("WLAN-Schluessel 64 Byte -> abgewiesen", zuLang(r) && /höchstens 63/.test(r.a[0]), JSON.stringify(r));
  r = await client("Netz", "k".repeat(63));
  pruef("WLAN-Schluessel 63 Byte -> gesendet", r.n === 1, JSON.stringify(r));
  r = await client("", "geheim1234", "L".repeat(32));
  pruef("SSID aus der Liste 32 Byte -> abgewiesen", zuLang(r), JSON.stringify(r));
  r = await ap("A".repeat(32), "wordclock24");
  pruef("AP-SSID 32 Byte -> abgewiesen", zuLang(r) && /höchstens 31/.test(r.a[0]), JSON.stringify(r));
  r = await ap("WordClock", "w".repeat(64));
  pruef("AP-Schluessel 64 Byte -> abgewiesen", zuLang(r) && /höchstens 63/.test(r.a[0]), JSON.stringify(r));
  r = await ap("A".repeat(31), "w".repeat(63));
  pruef("AP 31/63 Byte -> gesendet", r.n === 1, JSON.stringify(r));

  // H1 (Review P4): maxlength gleich der Grenze kuerzte beim Einfuegen still auf 31/63 --
  // die Pruefung sah dann einen passenden Wert und sendete ihn gekuerzt. Mit 32/64 bleibt
  // ein Byte zu viel im Feld stehen, und die Byte-Pruefung weist ab. Echtes Einfuegen per
  // Input.insertText, nicht .value: Nur so wirkt maxlength.
  console.log("H1 Einfuegen bis maxlength (32/64)");
  await sleep(2000);   // verzoegertes loadData aus den Sendefaellen oben abwarten, sonst steht dessen Meldung zuletzt
  await b.js(`(document.querySelector('.module-chip[data-module-target="network"]').click(), true)`); await sleep(600);
  const einfuegen = async (id, text) => {
    await b.js(`(() => { const e = document.getElementById(${JSON.stringify(id)}); e.value = ""; e.focus(); return true })()`);
    await b.cdp("Input.insertText", { text }, b.sessionId);
    return b.js(`document.getElementById(${JSON.stringify(id)}).value.length`);
  };
  const ml = await b.js(`["network-ssid-manual-input", "network-key-input", "network-ap-ssid-input", "network-ap-key-input"].map(i => document.getElementById(i).maxLength)`);
  pruef("H1: maxlength 32/64/32/64", JSON.stringify(ml) === "[32,64,32,64]", JSON.stringify(ml));
  // Die Meldung "Zu lang" suchen statt die letzte nehmen: Ein Neuladen nach einem Sendefall
  // kann "Aktualisiert ..." dahinter schreiben.
  const zuLangMeldung = () => b.js(`(window.__ann.find(x => /^Zu lang/.test(x[0])) || window.__ann[window.__ann.length - 1] || null)`);
  const clientEin = async (ssid, key) => {
    const ls = await einfuegen("network-ssid-manual-input", ssid);
    const lk = await einfuegen("network-key-input", key);
    await b.js(`(window.__calls = [], window.__ann = [], true)`);
    await b.js(`saveNetworkClient()`); await sleep(200);
    return { ls, lk, n: (await calls("/api/network_client_set")).length, a: await zuLangMeldung() };
  };
  const apEin = async (ssid, key) => {
    const ls = await einfuegen("network-ap-ssid-input", ssid);
    const lk = await einfuegen("network-ap-key-input", key);
    await b.js(`(window.__calls = [], window.__ann = [], true)`);
    await b.js(`saveNetworkAp()`); await sleep(200);
    return { ls, lk, n: (await calls("/api/network_ap_set")).length, a: await zuLangMeldung() };
  };
  const meldet = (r, bytes, max) => zuLang(r) && r.a[0].indexOf(bytes + " Byte") >= 0 && r.a[0].indexOf("höchstens " + max) >= 0;
  r = await clientEin("S".repeat(32), "geheim1234");
  pruef("H1: SSID 32 Zeichen eingefuegt -> 32 Byte, hoechstens 31, kein Abruf", r.ls === 32 && meldet(r, 32, 31), JSON.stringify(r));
  r = await clientEin("S".repeat(33), "geheim1234");
  pruef("H1: SSID 33 eingefuegt -> Browser kuerzt auf 32, trotzdem abgewiesen", r.ls === 32 && meldet(r, 32, 31), JSON.stringify(r));
  r = await clientEin("Netz", "k".repeat(64));
  pruef("H1: WLAN-Schluessel 64 eingefuegt -> 64 Byte, hoechstens 63, kein Abruf", r.lk === 64 && meldet(r, 64, 63), JSON.stringify(r));
  r = await clientEin("Netz", "k".repeat(65));
  pruef("H1: WLAN-Schluessel 65 eingefuegt -> gekuerzt auf 64, trotzdem abgewiesen", r.lk === 64 && meldet(r, 64, 63), JSON.stringify(r));
  r = await clientEin("S".repeat(31), "k".repeat(63));
  pruef("H1: Client 31/63 eingefuegt -> gesendet", r.ls === 31 && r.lk === 63 && r.n === 1, JSON.stringify(r));
  r = await apEin("A".repeat(32), "wordclock24");
  pruef("H1: AP-SSID 32 eingefuegt -> abgewiesen", r.ls === 32 && meldet(r, 32, 31), JSON.stringify(r));
  r = await apEin("A".repeat(33), "wordclock24");
  pruef("H1: AP-SSID 33 eingefuegt -> gekuerzt auf 32, abgewiesen", r.ls === 32 && meldet(r, 32, 31), JSON.stringify(r));
  r = await apEin("WordClock", "w".repeat(64));
  pruef("H1: AP-Schluessel 64 eingefuegt -> abgewiesen", r.lk === 64 && meldet(r, 64, 63), JSON.stringify(r));
  r = await apEin("WordClock", "w".repeat(65));
  pruef("H1: AP-Schluessel 65 eingefuegt -> gekuerzt auf 64, abgewiesen", r.lk === 64 && meldet(r, 64, 63), JSON.stringify(r));
  r = await apEin("A".repeat(31), "w".repeat(63));
  pruef("H1: AP 31/63 eingefuegt -> gesendet", r.ls === 31 && r.lk === 63 && r.n === 1, JSON.stringify(r));

  const imp = async (net) => {
    await b.js(`(window.__calls = [], resetTooLongImportFields(), true)`);
    await b.js(`importNetworkSettings(${JSON.stringify(net)})`);
    return { n: (await calls("/api/eeprom_settings_set")).length, s: await b.js(`getTooLongImportFieldsSummary()`) };
  };
  const netz = { timeserver: "", timezone_offset: 1, summertime: true, wifi_ssid: "Heim", wifi_key: "geheim1234", ap_ssid: "WordClock", ap_key: "wordclock24", boot_as_ap: false };
  let ri = await imp(netz);
  pruef("Import: passende WLAN-Werte -> geschrieben", ri.n === 1 && ri.s === "", JSON.stringify(ri));
  ri = await imp({ ...netz, wifi_key: "k".repeat(64), ap_ssid: "A".repeat(32) });
  pruef("Import: WLAN-Schluessel 64 und AP-SSID 32 -> nichts gesendet", ri.n === 0, JSON.stringify(ri));
  pruef("Import: beide Felder in der Liste", /WLAN-Passwort/.test(ri.s) && /Zugangspunkt/.test(ri.s), ri.s);
  ri = await imp({ ...netz, wifi_ssid: "S".repeat(32) });
  pruef("Import: SSID 32 -> nichts gesendet", ri.n === 0 && /SSID/.test(ri.s), JSON.stringify(ri));

  console.log("B5 / L341 Ganzzahlfelder");
  const zahl = async (v, min, max) => {
    await b.js(`(window.__ann = [], true)`);
    const n = await b.js(`readNumberInputOrReport({ value: ${JSON.stringify(v)} }, ${min}, ${max})`);
    return { n, a: await ann() };
  };
  const ganz = (r) => r.n === null && r.a && /keine ganze Zahl/.test(r.a[0]) && r.a[1] === "error";
  for (const [v, soll] of [["8", 8], ["0", 0], ["-3", -3], ["+5", 5], [" 7 ", 7], ["007", 7], ["255", 255]]) {
    r = await zahl(v, -20, 255);
    pruef(JSON.stringify(v) + " -> " + soll, r.n === soll, JSON.stringify(r));
  }
  for (const v of ["8.5", "1e1", "8.0", "0x10", "1E1", "-0.5", ".5", "5."]) {
    r = await zahl(v, -20, 255);
    pruef(JSON.stringify(v) + " -> ganze Zahl erwartet", ganz(r), JSON.stringify(r));
  }
  r = await zahl("", 0, 15);
  pruef("leer -> number_required wie bisher", r.n === null && /Bitte einen Wert zwischen 0 und 15/.test(r.a[0]), JSON.stringify(r));
  // M1: nicht leer, aber keine Zahl -> ganze Zahl erwartet (bis Review P4: number_required)
  r = await zahl("abc", 0, 15);
  pruef("abc -> ganze Zahl erwartet (M1)", ganz(r), JSON.stringify(r));
  r = await zahl("+", 0, 15);
  pruef("'+' -> ganze Zahl erwartet (M1)", ganz(r), JSON.stringify(r));
  r = await zahl("16", 0, 15);
  pruef("16 -> ausserhalb des Bereichs wie bisher", r.n === null && /ausserhalb des erlaubten Bereichs/.test(r.a[0]), JSON.stringify(r));
  r = await zahl("-21", -20, 20);
  pruef("-21 bei Korrektur -> ausserhalb", r.n === null && /ausserhalb/.test(r.a[0]), JSON.stringify(r));

  // M1 (Review P4): Ein type=number-Feld liefert value "" bei unlesbarer Eingabe
  // (validity.badInput). Das Feld ist nicht leer -- "ganze Zahl erwartet", nicht
  // "darf nicht leer sein". Echtes Tippen per Input.insertText.
  console.log("M1 badInput im type=number-Feld");
  await b.js(`(document.querySelector('.module-chip[data-module-target="display"]').click(), true)`); await sleep(600);
  const tippeZahl = async (text) => {
    await b.js(`(() => { const e = document.getElementById("ticker-deceleration-input"); e.value = ""; e.focus(); return true })()`);
    if (text) await b.cdp("Input.insertText", { text }, b.sessionId);
    const z = await b.js(`(() => { const e = document.getElementById("ticker-deceleration-input"); return { v: e.value, bad: e.validity.badInput } })()`);
    await b.js(`(window.__calls = [], window.__ann = [], true)`);
    await b.js(`saveTickerDeceleration()`); await sleep(200);
    return { ...z, n: (await calls("ticker_deceleration")).length, a: await ann() };
  };
  for (const t of ["5-", "1e", "--1"]) {
    r = await tippeZahl(t);
    pruef("M1: " + JSON.stringify(t) + " (badInput) -> ganze Zahl erwartet, kein Abruf", r.v === "" && r.bad === true && r.n === 0 && ganz({ n: null, a: r.a }), JSON.stringify(r));
  }
  // "8,5" haengt am Gebietsschema: Chrome (en-US, headless) liest es als 8.5, andere als
  // badInput. Beides muss "ganze Zahl erwartet" ergeben, keins darf senden.
  r = await tippeZahl("8,5");
  pruef("M1: \"8,5\" -> ganze Zahl erwartet, kein Abruf (v=" + JSON.stringify(r.v) + ", badInput=" + r.bad + ")", r.n === 0 && ganz({ n: null, a: r.a }), JSON.stringify(r));
  r = await tippeZahl("");
  pruef("M1: wirklich leer -> number_required, kein Abruf", r.v === "" && r.bad === false && r.n === 0 && /Bitte einen Wert zwischen 0 und 255/.test(r.a && r.a[0]), JSON.stringify(r));
  r = await tippeZahl("12");
  pruef("M1: 12 getippt -> gesendet", r.v === "12" && r.n === 1, JSON.stringify(r));
  // Ende zu Ende: ein echtes type=number-Feld, kein Abruf
  await b.js(`(window.__calls = [], window.__ann = [], document.getElementById("ticker-deceleration-input").value = "1e1", true)`);
  const roh = await b.js(`document.getElementById("ticker-deceleration-input").value`);
  await b.js(`saveTickerDeceleration()`); await sleep(200);
  r = { n: (await calls("ticker_deceleration")).length, a: await ann(), roh };
  pruef("Ticker-Verzoegerung '1e1' im type=number-Feld -> abgewiesen, kein Abruf", r.roh === "1e1" && r.n === 0 && /keine ganze Zahl/.test(r.a && r.a[0]), JSON.stringify(r));
  await b.js(`(window.__calls = [], document.getElementById("temperature-rtc-correction-input").value = "-3", true)`);
  await b.js(`saveTemperatureCorrection("temperature-rtc-correction-input", "temperature-rtc-correction-save-button", getTemperatureRtcCorrectionSetUrl(), "x", "y")`); await sleep(300);
  const tc = await calls("temperature_rtc_correction");
  pruef("Temperaturkorrektur -3 (halbe Grad als Ganzzahl) -> gesendet", tc.length === 1 && /value=-3(&|$)/.test(tc[0]), JSON.stringify(tc));
  await b.js(`(window.__calls = [], document.getElementById("temperature-rtc-correction-input").value = "1.5", true)`);
  await b.js(`saveTemperatureCorrection("temperature-rtc-correction-input", "temperature-rtc-correction-save-button", getTemperatureRtcCorrectionSetUrl(), "x", "y")`); await sleep(300);
  pruef("Temperaturkorrektur 1.5 -> abgewiesen (Feld zaehlt Schritte, step=1)", (await calls("temperature_rtc_correction")).length === 0);

  console.log("i18n: neue Schluessel in beiden Tabellen");
  const appjs = readFileSync(root + "/ESP8266/ESP-uclock/data/app/app.js", "utf8");
  const en = JSON.parse(readFileSync(root + "/ESP8266/ESP-uclock/data/app/i18n/en.json", "utf8"));
  for (const k of ["api.error.7", "api.error.8", "maintenance.fs_show_empty_gz", "maintenance.stm32_hardware_unknown", "input.integer_required",
                   "backup.field.wifi_ssid", "backup.field.wifi_key", "backup.field.ap_ssid", "backup.field.ap_key", "backup.network_skipped_too_long"]) {
    const de = new RegExp('^  "' + k.replace(/\./g, "\\.") + '": "', "m").test(appjs);
    pruef("Schluessel " + k + " DE+EN", de && typeof en[k] === "string" && en[k].length > 0, "de=" + de + " en=" + (k in en));
  }
  pruef("keine Seitenfehler", (await b.js(`(window.__pageErrors||[]).length`)) === 0, JSON.stringify(await b.js(`window.__pageErrors`)));
} catch (e) {
  fail++; console.log("  FAIL Abbruch: " + String(e && e.message || e).slice(0, 300));
} finally {
  console.log(`=== ${ok} OK, ${fail} FAIL (${ok + fail} Faelle)`);
  b.ende();
  process.exit(fail ? 1 : 0);
}
