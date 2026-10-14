// Pruefstand ep (Paket 2026-10-13, EP.7): C6b (AKS.3), C46 (AKQ.3), B55 (AKL.3) in der PWA.
//
//   node ep.mjs <app.js> <index.html> <en.json> [bezeichnung]
//
// Laedt app.js VOLLSTAENDIG in eine vm (DOM-Attrappen aus tdz) und ruft die Originalfunktionen
// mit einem gesteuerten fetch auf. Gezaehlt wird jeder fetch mit Adresse. Nichts ist
// abgeschrieben; die Entscheidungen laufen im Originalcode.
// Exit 0 alle Faelle OK, 1 Abweichung, 2 Werkzeugfehler (auch: Funktion fehlt).
import fs from "node:fs";
import vm from "node:vm";
import { makeElement } from "./dom.inc.mjs";

const [appPath, htmlPath, enPath, bez = "PWA"] = process.argv.slice(2);
const code = fs.readFileSync(appPath, "utf8");
const html = fs.readFileSync(htmlPath, "utf8");
const EN = JSON.parse(fs.readFileSync(enPath, "utf8"));
const ids = new Set([...html.matchAll(/\bid="([^"]+)"/g)].map((m) => m[1]));
const els = new Map([...ids].map((id) => [id, makeElement("div", { id })]));

let route = () => ({ status: 200, body: { ok: true } });
const calls = [];
let live = false;
const sandbox = {
  console: { log() {}, info() {}, debug() {}, warn() {}, error() {} },
  document: {
    hidden: false, visibilityState: "visible", readyState: "complete", title: "",
    documentElement: makeElement("html"), body: makeElement("body"), head: makeElement("head"),
    getElementById: (id) => els.get(id) || null, querySelector: (s) => (/^#([\w-]+)$/.test(s) ? els.get(s.slice(1)) || null : null),
    querySelectorAll: () => [], createElement: (t) => makeElement(t), createTextNode: (t) => ({ textContent: t }),
    createDocumentFragment: () => makeElement("fragment"), addEventListener() {}, removeEventListener() {}
  },
  localStorage: { getItem: () => null, setItem() {}, removeItem() {}, clear() {}, key: () => null, length: 0 },
  sessionStorage: { getItem: () => null, setItem() {}, removeItem() {}, clear() {}, key: () => null, length: 0 },
  navigator: { userAgent: "node-ep", language: "de-CH", languages: ["de-CH"], onLine: true },
  location: { href: "http://uhr.invalid/app/", origin: "http://uhr.invalid", protocol: "http:", host: "uhr.invalid", hostname: "uhr.invalid", pathname: "/app/", search: "", hash: "", reload() {}, replace() {}, assign() {} },
  history: { replaceState() {}, pushState() {}, state: null },
  isSecureContext: true, innerWidth: 1280, innerHeight: 800, devicePixelRatio: 1,
  // Beim Laden bleiben Timer liegen (die Startschleifen sollen nicht laufen). In den Faellen
  // laufen kurze Timer (sleep im Import, <= 1 s) beschleunigt; lange (Banner, Fristen) nie.
  setTimeout: (fn, ms) => (live && (Number(ms) || 0) <= 1000 ? setTimeout(fn, 5) : 0),
  clearTimeout: (id) => { if (id) clearTimeout(id); },
  setInterval: () => 0, clearInterval() {}, requestAnimationFrame: () => 0, cancelAnimationFrame() {},
  matchMedia: () => ({ matches: false, addEventListener() {}, removeEventListener() {}, addListener() {}, removeListener() {} }),
  getComputedStyle: () => ({ getPropertyValue: () => "" }),
  addEventListener() {}, removeEventListener() {}, scrollTo() {}, alert() {}, confirm: () => false, prompt: () => null, open: () => null,
  fetch: (url) => {
    calls.push(String(url));
    if (!live) return new Promise(() => {});
    const r = route(String(url));
    if (r.reject) return Promise.reject(new TypeError("Failed to fetch"));
    const text = typeof r.body === "string" ? r.body : JSON.stringify(r.body);
    return Promise.resolve(new Response(text, { status: r.status || 200, headers: { "content-type": r.ctype || (typeof r.body === "string" ? "text/html" : "application/json") } }));
  },
  URL, URLSearchParams, AbortController, AbortSignal, TextDecoder, TextEncoder, Blob, FormData, Headers, Response, Request, Event, EventTarget, performance, crypto, structuredClone, queueMicrotask,
  CustomEvent: class { constructor(t, o) { this.type = t; this.detail = o && o.detail; } },
  MutationObserver: class { observe() {} disconnect() {} }, ResizeObserver: class { observe() {} unobserve() {} disconnect() {} },
  // parseSettings("") baut das leere Geruest (der normale Startfall); dafuer reicht ein DOMParser ohne Inhalt.
  DOMParser: class { parseFromString() { return { querySelector: () => null, querySelectorAll: () => [], getElementsByTagName: () => [] }; } },
  IntersectionObserver: class { observe() {} unobserve() {} disconnect() {} }, Image: class {}, HTMLElement: class {}, Element: class {}, Node: class {}
};
sandbox.window = sandbox; sandbox.self = sandbox; sandbox.globalThis = sandbox;
vm.createContext(sandbox);
try { vm.runInContext(code, sandbox, { filename: appPath }); } catch (e) { console.log("KAPUTT ep " + bez + ": app.js laedt nicht: " + e.message); process.exit(2); }
await new Promise((r) => setImmediate(r));
const run = (expr) => vm.runInContext(expr, sandbox);
for (const f of ["apiFetch", "describeApiError", "importNetworkSettings", "setLdrMinValue", "setLdrMaxValue", "exportSettingsBackup", "ensureBackupExportState", "getNetworkBackupMeta"]) {
  if (run("typeof " + f) !== "function") { console.log("FEHL  ep " + bez + ": Funktion " + f + " fehlt"); process.exit(1); }
}
live = true;
const fehler = [];
let faelle = 0;
const pruefe = (name, ok, info = "") => { faelle++; if (!ok) fehler.push(name + (info ? ": " + info : "")); };
const lang = (l) => run(l === "en" ? "loadedLanguageTables.en = " + JSON.stringify(EN) + "; currentLanguage = 'en';" : "currentLanguage = 'de';");
// Jeder Abschnitt laeuft gekapselt: Wirft der Originalcode (alter Stand, Funktion fehlt),
// ist das ein FEHL dieses Abschnitts -- kein Absturz des Pruefstands mit Exit 0 dahinter.
const fall = async (name, fn) => { try { await fn(); } catch (e) { pruefe(name + " lief durch", false, String(e && e.message || e)); } };
const T = (key, l) => (l === "en" ? EN[key] : run("I18N_DE[" + JSON.stringify(key) + "]"));

// ---------- AKQ.3: Grenze in apiFetch ----------
await fall("AKQ.3 Grenze", async () => {
for (const [n, soll] of [[255, true], [256, false]]) {
  calls.length = 0;
  route = () => ({ body: { ok: true } });
  sandbox.__u = "/api/device_ready?" + "x".repeat(n - 0).slice(0, n);
  let err = null;
  try { await run("apiFetch(__u)"); } catch (e) { err = e; }
  if (soll) pruefe("AKQ.3 Query " + n + " geht hinaus", !err && calls.length === 1, err && err.message);
  else {
    pruefe("AKQ.3 Query " + n + " kein Request", calls.length === 0, calls.length + " Requests");
    pruefe("AKQ.3 Query " + n + " Kennung 9", err && err.apiErrorCode === 9 && err.queryLength === n);
    for (const l of ["de", "en"]) {
      lang(l); sandbox.__e = err;
      const m = run("describeApiError(__e, 'x')");
      pruefe("AKQ.3 Meldung " + l + " nennt Laenge und Grenze", m.includes(String(n)) && m.includes("255") && !m.includes("{"), m);
    }
  }
}
});
// Kennung 9 vom Geraet => dieselbe Meldung
await fall("AKQ.3 Kennung 9 vom Geraet", async () => {
  route = () => ({ body: { ok: false, error: 9, detail: "query too long" } });
  sandbox.__u = "/api/device_ready?" + "y".repeat(200);
  let err = null;
  try { await run("apiFetch(__u)"); } catch (e) { err = e; }
  lang("de"); sandbox.__e = err;
  const vomGeraet = err ? run("describeApiError(__e, 'x')") : "";
  sandbox.__e = run("markApiQueryTooLong(new Error('api-9'), 200)");
  const vorher = run("describeApiError(__e, 'x')");
  pruefe("AKQ.3 Kennung 9 vom Geraet = Meldung vor dem Senden", err && vomGeraet === vorher, vomGeraet + " | " + vorher);
});
// Import je Feld
await fall("AKQ.3 Import", async () => {
  // Grenzlaengen in Byte, nur Zeichen, die URLSearchParams als %XX kodiert (schlimmster Fall).
  const sonder = "&=%#/?\"'<>{}[]@!$";
  const fuell = (n) => sonder.repeat(10).slice(0, n);
  const net = { timeserver: "", wifi_ssid: fuell(31), wifi_key: fuell(63), ap_ssid: fuell(31), ap_key: fuell(63), boot_as_ap: true };
  calls.length = 0; route = () => ({ body: { ok: true } });
  sandbox.__n = net;
  run("importNetworkTimeSettings = async () => {};");
  let err = null;
  try { await run("importNetworkSettings(__n)"); } catch (e) { err = e; }
  const sets = calls.filter((u) => u.includes("eeprom_settings_set"));
  const q = sets.map((u) => new URLSearchParams(u.slice(u.indexOf("?") + 1)));
  pruefe("AKQ.3 Import ohne Fehler", !err, err && err.message);
  pruefe("AKQ.3 Import vier Aufrufe", sets.length === 4, sets.length + "");
  pruefe("AKQ.3 Reihenfolge ap_ssid, ap_key, key, ssid", q.map((p) => [...p.keys()].filter((k) => k !== "boot_as_ap").join()).join("|") === "ap_ssid|ap_key|key|ssid", q.map((p) => [...p.keys()].join()).join("|"));
  pruefe("AKQ.3 jeder mit boot_as_ap=on", q.every((p) => p.get("boot_as_ap") === "on"));
  pruefe("AKQ.3 Werte unverfaelscht", q[0] && q[0].get("ap_ssid") === net.ap_ssid && q[1].get("ap_key") === net.ap_key && q[2].get("key") === net.wifi_key && q[3].get("ssid") === net.wifi_ssid);
  const max = Math.max(...sets.map((u) => u.length - u.indexOf("?") - 1));
  pruefe("AKQ.3 kein Aufruf ueber 255", max <= 255, "max " + max);
  console.log("      Import: laengste Query " + max + " Zeichen");
  calls.length = 0; sandbox.__n = { ...net, ap_key: "", boot_as_ap: false };
  await run("importNetworkSettings(__n)");
  const s2 = calls.filter((u) => u.includes("eeprom_settings_set"));
  // Nutzerentscheid 14.10.2026 (Review EP M1): alle Aufrufe ausser dem letzten tragen
  // boot_as_ap=on, nur der letzte (immer ssid) den Wert aus dem Backup.
  pruefe("AKQ.3 leeres Feld kein Aufruf", s2.length === 3 && !s2.some((u) => u.includes("ap_key=")), s2.join(" "));
  pruefe("AKQ.3 M1 vorne on, zuletzt ssid mit off", s2.length === 3 && s2.slice(0, -1).every((u) => u.endsWith("boot_as_ap=on")) && /[?&]ssid=/.test(s2[2]) && s2[2].endsWith("boot_as_ap=off"), s2.join(" "));
  // Abbruch beim key-Aufruf: alles bis dahin Gesendete traegt on, ssid wird nicht mehr gesendet.
  calls.length = 0; sandbox.__n = { ...net, boot_as_ap: false };
  route = (u) => (u.includes("eeprom_settings_set") && /[?&]key=/.test(u) ? { body: { ok: false, error: 3, detail: "x" } } : { body: { ok: true } });
  let abbruch = null;
  try { await run("importNetworkSettings(__n)"); } catch (e) { abbruch = e; }
  const s3 = calls.filter((u) => u.includes("eeprom_settings_set"));
  pruefe("AKQ.3 M1 Abbruch bei key: bis dahin alles on, kein ssid", s3.length === 3 && s3.every((u) => u.endsWith("boot_as_ap=on")) && !s3.some((u) => /[?&]ssid=/.test(u)), s3.join(" ") + " / " + (abbruch && abbruch.message));
  route = () => ({ body: { ok: true } });
  // M1-Nachtrag: Ist das WLAN-Schreiben abgebrochen, laedt die Seite NICHT automatisch neu --
  // sonst verschwaende der Hinweis "nicht neu starten" nach 6 s. Gezaehlt wird jedes
  // setTimeout(reloadAppPage) am Ende von finalizeImportRestartAndReload().
  pruefe("AKQ.3 M1 Abbruch setzt den Merker", run("networkImportWriteInterrupted") === true);
  pruefe("AKQ.3 M1 Hinweis DE vorhanden", /nicht neu/.test(run("translate('backup.network_write_interrupted')")), run("translate('backup.network_write_interrupted')"));
  run("sleep = async () => {}; loadData = async () => {}; globalThis.__reload = 0; globalThis.__st = setTimeout; setTimeout = (fn, ms) => { if (fn === reloadAppPage) globalThis.__reload++; return 0; };");
  for (const [merker, soll] of [[true, 0], [false, 1]]) {
    run("networkImportWriteInterrupted = " + merker + "; globalThis.__reload = 0;");
    await run("finalizeImportRestartAndReload({})");
    pruefe("AKQ.3 M1 Neuladen bei Merker " + merker + ": " + soll, sandbox.__reload === soll, "gezaehlt " + sandbox.__reload);
  }
  run("setTimeout = globalThis.__st; networkImportWriteInterrupted = false;");
});

// ---------- AKL.3: LDR-Meldungen ----------
const banner = els.get("status-banner");
await fall("AKL.3", async () => {
for (const l of ["de", "en"]) {
  lang(l);
  for (const [fn, knopf, rej, saved] of [["setLdrMinValue", "ldr-min-button", "climate.ldr_min_rejected", "climate.ldr_min_saved"], ["setLdrMaxValue", "ldr-max-button", "climate.ldr_max_rejected", "climate.ldr_max_saved"]]) {
    const soll = { 10: T(rej, l), 7: T("climate.ldr_measure_failed", l), 11: T("api.error.11", l) };
    for (const k of [10, 7, 11, "ok"]) {
      route = () => ({ body: k === "ok" ? { ok: true, value: 42 } : { ok: false, error: k, detail: "x" } });
      banner.textContent = "";
      const vorher = calls.length;
      await run(fn + "()");
      const gesendet = calls.slice(vorher).some((u) => /ldr_m(in|ax)_set/.test(u));
      const s = k === "ok" ? T(saved, l) : soll[k];
      pruefe("AKL.3 " + l + " " + fn + " " + k, gesendet && !!s && banner.textContent.includes(s), "Banner '" + banner.textContent + "', erwartet '" + s + "'");
    }
  }
  // Gegenprobe: ein anderer Aufrufer behaelt den allgemeinen Text fuer 7
  route = () => ({ body: { ok: false, error: 7, detail: "" } });
  banner.textContent = "";
  if (run("typeof displayTemperatureNow") === "function") {
    await run("displayTemperatureNow()");
    pruefe("AKL.3 " + l + " Gegenprobe allgemeine Kennung 7", banner.textContent.includes(T("api.error.7", l)) && !banner.textContent.includes(T("climate.ldr_measure_failed", l)), banner.textContent);
  }
}
});
lang("de");
pruefe("AKL.3 Texte DE wie AKL.3", T("climate.ldr_min_rejected", "de") === "LDR-Minimum nicht übernommen: Der Messwert liegt nicht unter dem Maximum." && T("api.error.11", "de") === "Keine Rückmeldung der Uhr — ob der Wert übernommen wurde, ist offen.", T("climate.ldr_min_rejected", "de") + " | " + T("api.error.11", "de"));

// ---------- AKS.3: Schluessel ueber network_secrets ----------
const EEP = { ok: true, ssid: "Heim", key: "", key_set: true, ap_ssid: "WordClock", ap_key: "", ap_key_set: true, flags: 0, boot_as_ap: false };
const varianten = [
  ["ok", () => ({ body: { ok: true, key: "geheim1234", ap_key: "wordclock24" } }), true],
  ["404", () => ({ status: 404, body: "Not found" }), false],
  ["ok:false", () => ({ body: { ok: false, error: 3 } }), false],
  ["HTML", () => ({ body: "<html>x</html>", ctype: "text/html" }), false],
  ["Netzfehler", () => ({ reject: true }), false],
  ["key_set ohne key", () => ({ body: { ok: true, key: "", ap_key: "wordclock24" } }), false]
];
await fall("AKS.3", async () => {
run("getCurrentSettingsSnapshot = () => parseSettings(''); loadData = async () => {}; getUpdateTableCurrentFile = () => 'x';");
for (const [name, antwort, soll] of varianten) {
  calls.length = 0;
  route = (u) => (u.includes("network_secrets") ? antwort() : u.includes("eeprom_settings") ? { body: EEP } : { body: { ok: true } });
  sandbox.__eep = EEP;
  run("setCurrentEepromSettings(__eep)");
  let err = null;
  let r = null;
  try { r = await run("ensureBackupExportState()"); } catch (e) { err = e; }
  const ns = calls.filter((u) => u.includes("network_secrets")).length;
  if (soll) {
    pruefe("AKS.3 " + name + ": ein Abruf, kein Fehler", ns === 1 && !err, ns + " / " + (err && err.message));
    // Seit Review EP N3: die Schluessel sind der Rueckgabewert, nicht modulglobal.
    pruefe("AKS.3 " + name + ": Schluessel aus network_secrets", r && r.key === "geheim1234" && r.ap_key === "wordclock24", JSON.stringify(r));
    const meta = run("getNetworkBackupMeta(getCurrentSettingsSnapshot(), getCurrentEepromSettings())");
    pruefe("AKS.3 " + name + ": Meta ausserhalb des Exports ohne Schluessel", meta.wifiKey === "" && meta.apKey === "", JSON.stringify(meta));
  } else {
    pruefe("AKS.3 " + name + ": Abbruch mit eigener Kennung", err && err.backupReason === "network-secrets-unavailable", err ? err.backupReason || err.message : "kein Fehler");
  }
}
// Durchgang ueber exportSettingsBackup: Meldung bei 404, kein Download
for (const l of ["de", "en"]) {
  lang(l);
  route = (u) => (u.includes("network_secrets") ? { status: 404, body: "Not found" } : u.includes("eeprom_settings") ? { body: EEP } : { body: { ok: true } });
  run("triggerJsonDownload = () => { globalThis.__dl = (globalThis.__dl || 0) + 1; };");
  const note = els.get("settings-backup-note"); note.textContent = "";
  await run("exportSettingsBackup()");
  pruefe("AKS.3 " + l + " Export bei 404: Meldung, kein Download", note.textContent === T("backup.export_secrets_unavailable", l) && !sandbox.__dl, "'" + note.textContent + "' dl=" + sandbox.__dl);
}
// Erfolgsfall ueber prepareSettingsBackupExport(): Die Schluessel stehen im Dokument und sind
// danach aus dem Speicher der Seite verschwunden (das finally). Nach einem ERFOLG pruefen --
// (Review EP N2: frueher pruefte er nur nach dem 404 und sah das Verwerfen gar nicht; seit N3
// gibt es keine modulglobale Ablage mehr -- geprueft wird, dass keine existiert).
lang("de");
route = (u) => (u.includes("network_secrets") ? { body: { ok: true, key: "geheim1234", ap_key: "wordclock24" } } : u.includes("eeprom_settings") ? { body: EEP } : { body: { ok: true } });
run("collectIrBackupSection = async () => ({ section: null });");
const ergebnis = await run("prepareSettingsBackupExport()");
const netz = ergebnis && ergebnis.backup && ergebnis.backup.settings && ergebnis.backup.settings.network || {};
pruefe("AKS.3 Erfolg: Dokument traegt die Schluessel", netz.wifi_key === "geheim1234" && netz.ap_key === "wordclock24", JSON.stringify(netz));
const rest = run("[typeof getBackupNetworkSecrets, typeof backupNetworkSecrets]").join();
const metaDanach = run("getNetworkBackupMeta(getCurrentSettingsSnapshot(), getCurrentEepromSettings())");
pruefe("AKS.3 nach erfolgreichem Export kein Schluessel im Speicher der Seite", rest === "undefined,undefined" && metaDanach.wifiKey === "" && metaDanach.apKey === "", rest + " " + JSON.stringify(metaDanach));
});

for (const f of fehler) console.log("      FEHLER: " + f);
console.log((fehler.length ? "FEHL" : "OK  ") + "  ep " + bez + ": " + fehler.length + " Fehler in " + faelle + " Faellen (AKQ.3, AKL.3 DE/EN, AKS.3 " + varianten.length + " Antwortformen)");
process.exit(fehler.length ? 1 : 0);
