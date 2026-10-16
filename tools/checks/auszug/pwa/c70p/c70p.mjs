// Pruefstand c70p (Paket 2026-10-15, R2.3, AKC.6, Ent-3).
//
//   node c70p.mjs <app.js> [bezeichnung]
//
// Schneidet die Abruffunktionen aus app.js aus (nicht abgeschrieben) und fuehrt sie in einem
// eigenen vm-Kontext gegen einen aufzeichnenden fetchWithTimeout aus. Geprueft wird, was
// tatsaechlich an fetch geht, nicht der Quelltext:
//   A  loadLayoutPreviewRowsMap(): genau ein Abruf, URL /app/layout-previews.json,
//      cache "no-cache" (AKC.6), Frist 3500 ms; Ergebnis, Merker (zweiter Aufruf ohne
//      Abruf), Fehlerweg und !ok liefern wie vorher {} -- "sonst keine Aenderung"
//   B  ensureLanguageTable(): URL /app/i18n/<code>.json?v=<APP_VERSION> -- mit "=", denn ein
//      Parameter ohne "=" laesst den ESP abstuerzen (CLAUDE.md, R5) --, cache "default"
//   C  statisch: Bestand der cache-Optionen in app.js. Kein "no-store" mehr in
//      loadLayoutPreviewRowsMap, kein "no-store"/"reload" auf einen /app/-Literalpfad ausser
//      der Installationsaktion; Zahl und Lage der uebrigen "no-store" wie gezaehlt. Kommt eine
//      Stelle dazu oder faellt eine weg, BESTAND nachfuehren -- nicht nur die Zahl.
// Exit 0 OK, 1 FEHL, 2 Werkzeugfehler.
import { readFileSync } from "node:fs";
import vm from "node:vm";

const [appPath, bez = "PWA"] = process.argv.slice(2);
if (!appPath) { console.error("Aufruf: node c70p.mjs <app.js> [bezeichnung]"); process.exit(2); }
const quelle = readFileSync(appPath, "utf8");
const fehler = [];
let faelle = 0;
const pruefe = (name, ok, info = "") => { faelle++; if (!ok) fehler.push(name + (info ? ": " + info : "")); };
const werkzeug = (text) => { console.error(`WERKZEUGFEHLER c70p (${bez}): ${text}`); process.exit(2); };

// Ausschneiden mit Klammerzaehlung, die Zeichenketten und Kommentare ueberspringt (wie regeln.mjs).
function schneide(name) {
  const m = new RegExp("^(async )?function " + name + "\\(", "m").exec(quelle);
  if (!m) werkzeug(name + " nicht gefunden");
  let i = quelle.indexOf("{", m.index), tiefe = 0;
  for (; i < quelle.length; i++) {
    const c = quelle[i], n = quelle[i + 1];
    if (c === "/" && n === "/") { i = quelle.indexOf("\n", i); continue; }
    if (c === "/" && n === "*") { i = quelle.indexOf("*/", i) + 1; continue; }
    if (c === '"' || c === "'" || c === "`") { for (i++; i < quelle.length && quelle[i] !== c; i++) if (quelle[i] === "\\") i++; continue; }
    if (c === "{") tiefe++;
    if (c === "}" && --tiefe === 0) return { text: quelle.slice(m.index, i + 1), start: m.index, ende: i + 1 };
  }
  werkzeug(name + ": Klammern nicht geschlossen");
}
const konstante = (name) => {
  const m = new RegExp("^const " + name + " = ([^;\\n]+);", "m").exec(quelle);
  if (!m) werkzeug("Konstante " + name + " nicht gefunden");
  return m[1];
};

// Aufzeichnender Abruf. antwort: Funktion (url) => Response-Attrappe oder Error zum Werfen.
function kontext(antwort) {
  const abrufe = [];
  const ctx = {
    abrufe,
    fetchWithTimeout: async (url, options, timeoutMs) => {
      abrufe.push({ url, options: options ? { ...options } : options, timeoutMs });
      const r = antwort(url);
      if (r instanceof Error) throw r;
      return r;
    },
    sanitizeLanguageTable: (t) => t,
  };
  vm.createContext(ctx);
  return ctx;
}
const antwortJson = (wert, ok = true, status = 200) => ({
  ok, status, json: async () => wert, text: async () => JSON.stringify(wert), clone() { return this; },
});

// ---------- A loadLayoutPreviewRowsMap ----------
const layout = schneide("loadLayoutPreviewRowsMap").text;
const LAYOUT_URL = konstante("LAYOUT_PREVIEW_ROWS_URL");
const layoutCode = `const LAYOUT_PREVIEW_ROWS_URL = ${LAYOUT_URL};
let layoutPreviewRowsMap = null;
let layoutPreviewRowsMapPromise = null;
${layout}
this.lade = loadLayoutPreviewRowsMap;
this.zustand = () => ({ map: layoutPreviewRowsMap, promise: layoutPreviewRowsMapPromise });`;

async function teilA() {
  // A1-A4 Gutweg
  const payload = { "wc12h-tables-de.txt": ["ES IST"] };
  let ctx = kontext(() => antwortJson(payload));
  vm.runInContext(layoutCode, ctx);
  const r1 = await ctx.lade();
  pruefe("A1 genau ein Abruf", ctx.abrufe.length === 1, "gezaehlt " + ctx.abrufe.length);
  const a = ctx.abrufe[0] || {};
  pruefe("A1 URL /app/layout-previews.json", a.url === "/app/layout-previews.json", JSON.stringify(a.url));
  pruefe("A2 cache \"no-cache\" (AKC.6)", a.options && a.options.cache === "no-cache", "gesendet " + JSON.stringify(a.options));
  pruefe("A2 keine weiteren Optionen", a.options && Object.keys(a.options).join(",") === "cache", JSON.stringify(a.options));
  pruefe("A3 Frist 3500 ms", a.timeoutMs === 3500, String(a.timeoutMs));
  pruefe("A4 Ergebnis ist die geladene Tabelle", JSON.stringify(r1) === JSON.stringify(payload), JSON.stringify(r1));
  const r2 = await ctx.lade();
  pruefe("A4 zweiter Aufruf aus dem Merker, ohne Abruf", ctx.abrufe.length === 1 && r2 === r1, "Abrufe " + ctx.abrufe.length);
  pruefe("A4 Promise danach freigegeben", ctx.zustand().promise === null);

  // A5 gleichzeitige Aufrufe teilen einen Abruf
  ctx = kontext(() => antwortJson(payload));
  vm.runInContext(layoutCode, ctx);
  const [p, q] = await Promise.all([ctx.lade(), ctx.lade()]);
  pruefe("A5 zwei gleichzeitige Aufrufe, ein Abruf", ctx.abrufe.length === 1 && p === q, "Abrufe " + ctx.abrufe.length);

  // A6 Fehlerweg und !ok
  ctx = kontext(() => new Error("Netz weg"));
  vm.runInContext(layoutCode, ctx);
  const r3 = await ctx.lade();
  pruefe("A6 Abruf scheitert => {}", r3 && typeof r3 === "object" && Object.keys(r3).length === 0, JSON.stringify(r3));
  ctx = kontext(() => antwortJson({ x: 1 }, false, 404));
  vm.runInContext(layoutCode, ctx);
  const r4 = await ctx.lade();
  pruefe("A6 404 => {}", r4 && Object.keys(r4).length === 0, JSON.stringify(r4));
  ctx = kontext(() => antwortJson(null));
  vm.runInContext(layoutCode, ctx);
  const r5 = await ctx.lade();
  pruefe("A6 JSON null => {}", r5 && typeof r5 === "object" && Object.keys(r5).length === 0, JSON.stringify(r5));
}

// ---------- B ensureLanguageTable ----------
const sprache = schneide("ensureLanguageTable").text;
const APP_VERSION = konstante("APP_VERSION");
const sprachCode = `const APP_VERSION = ${APP_VERSION};
const LANGUAGE_FETCH_TIMEOUT_MS = ${konstante("LANGUAGE_FETCH_TIMEOUT_MS")};
const loadedLanguageTables = Object.create(null);
${sprache}
this.lade = ensureLanguageTable;`;

async function teilB() {
  const version = JSON.parse(APP_VERSION);
  const ctx = kontext(() => antwortJson({ "common.ok": "OK" }));
  vm.runInContext(sprachCode, ctx);
  await ctx.lade("en");
  const a = ctx.abrufe[0] || {};
  pruefe("B1 ein Abruf", ctx.abrufe.length === 1, "gezaehlt " + ctx.abrufe.length);
  pruefe("B1 URL /app/i18n/en.json?v=<APP_VERSION>", a.url === "/app/i18n/en.json?v=" + encodeURIComponent(version), JSON.stringify(a.url));
  const query = String(a.url || "").split("?")[1] || "";
  pruefe("B2 jeder Parameter mit \"=\" (R5)", query !== "" && query.split("&").every((t) => t.indexOf("=") > 0), JSON.stringify(query));
  pruefe("B3 cache \"default\" wie vorher", a.options && a.options.cache === "default", JSON.stringify(a.options));
}

// ---------- C statischer Bestand ----------
// Lage = Name der umschliessenden Funktion (letzte "function X(" auf Zeilenanfang davor).
function lage(index) {
  const vorher = quelle.slice(0, index);
  const re = /^(?:async )?function (\w+)\(/gm;
  let m, name = "?";
  while ((m = re.exec(vorher))) name = m[1];
  return name;
}
// Stand nach R2.3 (16.10.2026): 11 Stellen (vorher 12, die zwoelfte war layout-previews.json). Alle zielen auf /api/*, auf fremde Dienste
// (nominatim, ipapi) oder auf die Installationsaktion /app/?action=install -- keine auf ein Asset.
const BESTAND = {
  apiFetch: 1,                          // /api/* (alle schreibenden und lesenden Endpunkte)
  searchWeatherLocation: 1,             // nominatim search, extern
  useApproximateWeatherLocation: 1,     // ipapi.co, extern
  reverseLookupWeatherLocation: 1,      // nominatim reverse, extern
  downloadUpdateAppFiles: 1,            // /app/?action=install -- Aktion, kein Asset
  waitForStm32ResetAndReload: 1,        // /api/settings_xml?_ts=
  waitForDeviceReady: 2,                // /api/update_progress?_ts=, Proben (reconnect_probe, device_ready, settings_xml)
  settleFetchText: 1,                   // /api/*
  settleFetchJson: 1,                   // /api/*
  loadWordclockLayoutPreview: 1,        // /api/fs_show?filename=
};
function teilC() {
  const stellen = [...quelle.matchAll(/cache:\s*"([\w-]+)"/g)].map((m) => ({ art: m[1], wo: lage(m.index), index: m.index }));
  const noStore = stellen.filter((s) => s.art === "no-store");
  const layoutStelle = schneide("loadLayoutPreviewRowsMap");
  pruefe("C1 kein no-store in loadLayoutPreviewRowsMap",
    !noStore.some((s) => s.index >= layoutStelle.start && s.index < layoutStelle.ende));
  pruefe("C2 kein cache \"reload\" in app.js", !stellen.some((s) => s.art === "reload"),
    stellen.filter((s) => s.art === "reload").map((s) => s.wo).join(", "));
  // C3: Literalpfad /app/... in derselben Anweisung wie ein no-store (Fenster bis zum Semikolon davor).
  const assetTreffer = noStore.filter((s) => {
    const anfang = Math.max(quelle.lastIndexOf(";", s.index), quelle.lastIndexOf("{\n", s.index));
    const stueck = quelle.slice(anfang, s.index);
    return /"\/app\/(?!\?action=install")/.test(stueck) || /LAYOUT_PREVIEW_ROWS_URL/.test(stueck);
  });
  pruefe("C3 kein no-store auf ein App-Asset", assetTreffer.length === 0, assetTreffer.map((s) => s.wo).join(", "));
  const gezaehlt = {};
  for (const s of noStore) gezaehlt[s.wo] = (gezaehlt[s.wo] || 0) + 1;
  const namen = new Set([...Object.keys(BESTAND), ...Object.keys(gezaehlt)]);
  for (const n of namen) {
    pruefe("C4 no-store in " + n + " = " + (BESTAND[n] || 0), (gezaehlt[n] || 0) === (BESTAND[n] || 0), "gezaehlt " + (gezaehlt[n] || 0));
  }
  const soll = Object.values(BESTAND).reduce((x, y) => x + y, 0);
  pruefe("C4 no-store gesamt = " + soll, noStore.length === soll, "gezaehlt " + noStore.length);
  console.log(`  ${bez}: cache-Optionen ${stellen.length} (no-store ${noStore.length}, ` +
    [...new Set(stellen.map((s) => s.art))].filter((x) => x !== "no-store").map((x) => x + " " + stellen.filter((s) => s.art === x).length).join(", ") + ")");
}

try {
  await teilA();
  await teilB();
  teilC();
} catch (error) {
  werkzeug(error && error.stack ? error.stack : String(error));
}
if (fehler.length) {
  console.log(`FEHL c70p ${bez}: ${fehler.length} von ${faelle} Faellen abweichend`);
  for (const f of fehler) console.log("  - " + f);
  process.exit(1);
}
console.log(`OK   c70p ${bez}: ${faelle} Faelle, layout-previews.json mit no-cache, Sprachdatei mit ?v=`);
process.exit(0);
