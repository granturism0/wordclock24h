// Pruefstand c70p (Paket 2026-10-15; R2.3, AKC.6, Ent-3; nachgezogen in R2b.6, AKL.5, AKL.6, Ent-19 C, Ent-20 B).
//
//   node c70p.mjs <app.js> [bezeichnung]
//
// Schneidet die Abruffunktionen aus app.js aus (nicht abgeschrieben) und fuehrt sie in einem
// eigenen vm-Kontext gegen einen aufzeichnenden fetchWithTimeout und ein Attrappen-Dokument aus.
// Geprueft wird, was tatsaechlich an fetch geht, nicht der Quelltext:
//   A  loadLayoutPreviewRowsMap(): genau ein Abruf, URL /app/layout-previews.json OHNE Query,
//      cache "no-cache" (AKC.6, Ent-20 B), Frist 3500 ms; Ergebnis, Merker, Fehlerweg und !ok
//      liefern wie vorher {}. A7: auch wenn das Dokument (faelschlich) eine gueltige Marke fuer
//      layout-previews.json traegt, geht keine Query hinaus und das Meta-Element wird nicht gelesen
//   B  ensureLanguageTable(): Marke aus <meta name="wc-asset-tokens"> nur nach
//      ^[0-9]+-[0-9a-f]{16}$ (AKL.6); mit gueltiger Marke /app/i18n/<code>.json?v=<APP_VERSION>&h=<marke>,
//      sonst ?v=<APP_VERSION>; cache "default"; das Meta-Element wird einmal gelesen; jede Query
//      nur name=wert, jedes mit "=", Zeichen [0-9a-z.=&-], hoechstens 64 Zeichen (AKL.5)
//   C  statisch: Bestand der cache-Optionen; loadLayoutPreviewRowsMap() und LAYOUT_PREVIEW_ROWS_URL
//      zeichengleich wie in 1.4.101 (Fingerabdruck); genau eine Lesestelle fuer wc-asset-tokens,
//      genau ein Aufruf des Markenlesers, und der liegt in ensureLanguageTable().
//      Kommt eine cache-Stelle dazu oder faellt eine weg, BESTAND nachfuehren -- nicht nur die Zahl.
// Exit 0 OK, 1 FEHL, 2 Werkzeugfehler.
import { readFileSync } from "node:fs";
import { createHash } from "node:crypto";
import vm from "node:vm";

const [appPath, bez = "PWA"] = process.argv.slice(2);
if (!appPath) { console.error("Aufruf: node c70p.mjs <app.js> [bezeichnung]"); process.exit(2); }
const quelle = readFileSync(appPath, "utf8");
const fehler = [];
let faelle = 0;
const pruefe = (name, ok, info = "") => { faelle++; if (!ok) fehler.push(name + (info ? ": " + info : "")); };
const werkzeug = (text) => { console.error(`WERKZEUGFEHLER c70p (${bez}): ${text}`); process.exit(2); };

// Ausschneiden mit Klammerzaehlung, die Zeichenketten und Kommentare ueberspringt (wie regeln.mjs).
// Regex-Literale werden nicht erkannt; die ausgeschnittenen Funktionen enthalten keins mit Klammern.
function schneide(name, optional = false) {
  const m = new RegExp("^(async )?function " + name + "\\(", "m").exec(quelle);
  if (!m) { if (optional) return null; werkzeug(name + " nicht gefunden"); }
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

// Markenleser: von "const APP_ASSET_TOKEN_PATTERN" bis zum Ende von getAppAssetTokens(). Fehlt er
// (1.4.101 und frueher), ist das ein FEHL, kein Werkzeugfehler -- die Gegenprobe soll anschlagen.
const MARKEN_SELEKTOR = /^meta\[name=(["']?)wc-asset-tokens\1\]$/;
let markenBlock = "";
{
  const kopf = /^const APP_ASSET_TOKEN_PATTERN = /m.exec(quelle);
  const leser = schneide("getAppAssetTokens", true);
  if (kopf && leser) {
    if (leser.start < kopf.index || leser.start - kopf.index > 400) werkzeug("APP_ASSET_TOKEN_PATTERN steht nicht direkt vor getAppAssetTokens()");
    markenBlock = quelle.slice(kopf.index, leser.ende);
    if (/^(async )?function /m.test(quelle.slice(kopf.index, leser.start))) werkzeug("fremde Funktion zwischen APP_ASSET_TOKEN_PATTERN und getAppAssetTokens()");
  }
}
pruefe("B0 Markenleser vorhanden (APP_ASSET_TOKEN_PATTERN, getAppAssetTokens)", markenBlock !== "");

// Attrappen-Dokument. content undefined => kein Meta-Element. Zaehlt jede Lesestelle.
function dokument(content) {
  const z = { meta: 0, attribut: 0 };
  return {
    z,
    document: {
      querySelector(sel) {
        if (!MARKEN_SELEKTOR.test(String(sel))) return null;
        z.meta++;
        if (content === undefined) return null;
        return { getAttribute: (n) => { z.attribut++; return n === "content" ? content : null; }, get content() { z.attribut++; return content; } };
      },
      querySelectorAll: () => [],
    },
  };
}

// Aufzeichnender Abruf. antwort: Funktion (url) => Response-Attrappe oder Error zum Werfen.
function kontext(antwort, doc) {
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
  if (doc) ctx.document = doc.document;
  vm.createContext(ctx);
  return ctx;
}
const antwortJson = (wert, ok = true, status = 200) => ({
  ok, status, json: async () => wert, text: async () => JSON.stringify(wert), clone() { return this; },
});

const APP_VERSION = konstante("APP_VERSION");
const version = JSON.parse(APP_VERSION);
const M1 = "12345-0123456789abcdef";
const M2 = "678-fedcba9876543210";

// ---------- A loadLayoutPreviewRowsMap ----------
const layout = schneide("loadLayoutPreviewRowsMap").text;
const LAYOUT_URL = konstante("LAYOUT_PREVIEW_ROWS_URL");
const layoutCode = `const APP_VERSION = ${APP_VERSION};
const LAYOUT_PREVIEW_ROWS_URL = ${LAYOUT_URL};
let layoutPreviewRowsMap = null;
let layoutPreviewRowsMapPromise = null;
${markenBlock}
${layout}
this.lade = loadLayoutPreviewRowsMap;
this.zustand = () => ({ map: layoutPreviewRowsMap, promise: layoutPreviewRowsMapPromise });`;

async function teilA() {
  const payload = { "wc12h-tables-de.txt": ["ES IST"] };
  let ctx = kontext(() => antwortJson(payload));
  vm.runInContext(layoutCode, ctx);
  const r1 = await ctx.lade();
  pruefe("A1 genau ein Abruf", ctx.abrufe.length === 1, "gezaehlt " + ctx.abrufe.length);
  const a = ctx.abrufe[0] || {};
  pruefe("A1 URL /app/layout-previews.json ohne Query (Ent-20 B)", a.url === "/app/layout-previews.json", JSON.stringify(a.url));
  pruefe("A2 cache \"no-cache\" (AKC.6, Ent-20 B)", a.options && a.options.cache === "no-cache", "gesendet " + JSON.stringify(a.options));
  pruefe("A2 keine weiteren Optionen", a.options && Object.keys(a.options).join(",") === "cache", JSON.stringify(a.options));
  pruefe("A3 Frist 3500 ms", a.timeoutMs === 3500, String(a.timeoutMs));
  pruefe("A4 Ergebnis ist die geladene Tabelle", JSON.stringify(r1) === JSON.stringify(payload), JSON.stringify(r1));
  const r2 = await ctx.lade();
  pruefe("A4 zweiter Aufruf aus dem Merker, ohne Abruf", ctx.abrufe.length === 1 && r2 === r1, "Abrufe " + ctx.abrufe.length);
  pruefe("A4 Promise danach freigegeben", ctx.zustand().promise === null);

  ctx = kontext(() => antwortJson(payload));
  vm.runInContext(layoutCode, ctx);
  const [p, q] = await Promise.all([ctx.lade(), ctx.lade()]);
  pruefe("A5 zwei gleichzeitige Aufrufe, ein Abruf", ctx.abrufe.length === 1 && p === q, "Abrufe " + ctx.abrufe.length);

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

  // A7 Dokument nennt (faelschlich) eine gueltige Marke fuer layout-previews.json: weiter ohne Query.
  const doc = dokument(`layout-previews.json=${M1};i18n/en.json=${M2}`);
  ctx = kontext(() => antwortJson(payload), doc);
  vm.runInContext(layoutCode, ctx);
  await ctx.lade();
  const a7 = ctx.abrufe[0] || {};
  pruefe("A7 Marke im Dokument: URL weiter ohne Query", a7.url === "/app/layout-previews.json", JSON.stringify(a7.url));
  pruefe("A7 Marke im Dokument: weiter \"no-cache\"", a7.options && a7.options.cache === "no-cache", JSON.stringify(a7.options));
  pruefe("A7 loadLayoutPreviewRowsMap liest wc-asset-tokens nicht", doc.z.meta === 0, "gelesen " + doc.z.meta + "-mal");
}

// ---------- B ensureLanguageTable ----------
const sprache = schneide("ensureLanguageTable").text;
const sprachCode = `const APP_VERSION = ${APP_VERSION};
const LANGUAGE_FETCH_TIMEOUT_MS = ${konstante("LANGUAGE_FETCH_TIMEOUT_MS")};
const loadedLanguageTables = Object.create(null);
${markenBlock}
${sprache}
this.lade = ensureLanguageTable;`;
const TIMEOUT = Number(konstante("LANGUAGE_FETCH_TIMEOUT_MS"));
const basis = (code) => "/app/i18n/" + code + ".json?v=" + encodeURIComponent(version);
const alleAbrufe = [];

// Ein frischer Kontext je Fall (der Markenleser merkt sich das Ergebnis). Liefert URL und Zaehler.
async function sprachAbruf(content, codes = ["en"]) {
  const doc = dokument(content);
  const ctx = kontext(() => antwortJson({ "common.ok": "OK" }), doc);
  vm.runInContext(sprachCode, ctx);
  for (const c of codes) await ctx.lade(c);
  alleAbrufe.push(...ctx.abrufe);
  return { abrufe: ctx.abrufe, z: doc.z, url: (ctx.abrufe[0] || {}).url };
}

async function teilB() {
  let r = await sprachAbruf(undefined);
  pruefe("B1 ohne Meta-Element: ?v=<APP_VERSION>", r.url === basis("en"), JSON.stringify(r.url));
  r = await sprachAbruf("");
  pruefe("B2 Meta-Element leer (Quelle): ?v=<APP_VERSION>", r.url === basis("en"), JSON.stringify(r.url));
  r = await sprachAbruf(`i18n/en.json=${M1}`);
  pruefe("B3 gueltige Marke: ?v=<APP_VERSION>&h=<marke>", r.url === basis("en") + "&h=" + M1, JSON.stringify(r.url));
  r = await sprachAbruf(`i18n/de.json=${M2};i18n/en.json=${M1}`);
  pruefe("B4 mehrere Eintraege: Marke der eigenen Datei", r.url === basis("en") + "&h=" + M1, JSON.stringify(r.url));
  r = await sprachAbruf(`i18n/de.json=${M2};i18n/en.json=${M1}`, ["de"]);
  pruefe("B4 mehrere Eintraege: zweite Sprache mit ihrer Marke", r.url === basis("de") + "&h=" + M2, JSON.stringify(r.url));

  // B5 ungueltige Marken: nie uebernommen, Form ohne Marke.
  const ungueltig = {
    "Grossbuchstaben": "12345-0123456789ABCDEF",
    "15 Hexziffern": "12345-0123456789abcde",
    "17 Hexziffern": "12345-0123456789abcdef0",
    "ohne Bindestrich": "123450123456789abcdef",
    "ohne Groesse": "-0123456789abcdef",
    "Groesse nicht dezimal": "1e3-0123456789abcdef",
    "kein Hex": "12345-0123456789abcdeg",
    "mit Anfuehrungszeichen (ETag-Form)": `"${M1}"`,
    "angehaengter Parameter": M1 + "&x=1",
    "Prozentkodierung": "12345-0123456789abcd%20",
    "Pluszeichen": "12345-0123456789abcde+",
    "vorgestellte Marke": "9-" + M1,
    "leer": "",
  };
  for (const [name, marke] of Object.entries(ungueltig)) {
    r = await sprachAbruf(`i18n/en.json=${marke}`);
    pruefe("B5 ungueltige Marke (" + name + ") nicht uebernommen", r.url === basis("en"), JSON.stringify(r.url));
  }
  // B6 Marke einer anderen Datei bzw. falscher Pfad: keine Marke.
  r = await sprachAbruf(`layout-previews.json=${M1}`);
  pruefe("B6 Marke nur fuer layout-previews.json: Sprachdatei ohne h", r.url === basis("en"), JSON.stringify(r.url));
  r = await sprachAbruf(`i18n/de.json=${M1}`);
  pruefe("B6 Marke nur fuer eine andere Sprache: ohne h", r.url === basis("en"), JSON.stringify(r.url));
  r = await sprachAbruf(`en.json=${M1}`);
  pruefe("B6 Pfad ohne i18n/: ohne h", r.url === basis("en"), JSON.stringify(r.url));
  r = await sprachAbruf(`/app/i18n/en.json=${M1}`);
  pruefe("B6 Pfad mit /app/: ohne h (Pfad relativ zu /app/)", r.url === basis("en"), JSON.stringify(r.url));
  // B7 Pfad doppelt (Zusatz pwa-developer, sichere Richtung): keine Marke.
  r = await sprachAbruf(`i18n/en.json=${M1};i18n/en.json=${M2}`);
  pruefe("B7 Pfad doppelt mit zwei Marken: ohne h", r.url === basis("en"), JSON.stringify(r.url));
  // B8 einmal lesen: drei Aufrufe, zwei Abrufe, ein Lesen.
  r = await sprachAbruf(`i18n/en.json=${M1}`, ["en", "fr", "en"]);
  pruefe("B8 Meta-Element einmal gelesen", r.z.meta === 1, "gelesen " + r.z.meta + "-mal");
  pruefe("B8 zweite Sprache ohne eigene Marke: ?v=", (r.abrufe[1] || {}).url === basis("fr"), JSON.stringify((r.abrufe[1] || {}).url));
  pruefe("B8 dritter Aufruf aus loadedLanguageTables, ohne Abruf", r.abrufe.length === 2, "Abrufe " + r.abrufe.length);
  r = await sprachAbruf(undefined, ["en", "fr"]);
  pruefe("B8 ohne Meta-Element ebenfalls einmal gesucht", r.z.meta === 1, "gesucht " + r.z.meta + "-mal");

  // B9 fuer JEDEN aufgezeichneten Abruf: cache "default", Frist, Form der Query (AKL.5).
  pruefe("B9 Abrufe aufgezeichnet", alleAbrufe.length >= 25, "gezaehlt " + alleAbrufe.length);
  let nr = 0;
  for (const a of alleAbrufe) {
    nr++;
    const u = String(a.url || "");
    const query = u.includes("?") ? u.slice(u.indexOf("?") + 1) : "";
    const paare = query.split("&");
    pruefe(`B9.${nr} cache "default"`, a.options && a.options.cache === "default" && Object.keys(a.options).join(",") === "cache", u + " " + JSON.stringify(a.options));
    pruefe(`B9.${nr} Frist LANGUAGE_FETCH_TIMEOUT_MS`, a.timeoutMs === TIMEOUT, String(a.timeoutMs));
    pruefe(`B9.${nr} genau ein "?"`, u.split("?").length === 2, u);
    pruefe(`B9.${nr} Query nur [0-9a-z.=&-], hoechstens 64 Zeichen`, /^[0-9a-z.=&-]{1,64}$/.test(query), JSON.stringify(query) + " Laenge " + query.length);
    pruefe(`B9.${nr} jeder Parameter name=wert mit "="`, paare.every((t) => /^[a-z]+=[0-9a-z.-]+$/.test(t)), JSON.stringify(query));
    pruefe(`B9.${nr} v zuerst, h hoechstens einmal`, paare[0] === "v=" + version && paare.length <= 2 && (paare.length === 1 || paare[1].startsWith("h=")), JSON.stringify(query));
  }
}

// ---------- C statischer Bestand ----------
function lage(index) {
  const vorher = quelle.slice(0, index);
  const re = /^(?:async )?function (\w+)\(/gm;
  let m, name = "?";
  while ((m = re.exec(vorher))) name = m[1];
  return name;
}
// Stand nach R2.3 (16.10.2026), unveraendert in R2b.6: 11 Stellen. Alle zielen auf /api/*, auf fremde
// Dienste (nominatim, ipapi) oder auf die Installationsaktion /app/?action=install -- keine auf ein Asset.
const BESTAND = {
  apiFetch: 1,
  searchWeatherLocation: 1,
  useApproximateWeatherLocation: 1,
  reverseLookupWeatherLocation: 1,
  downloadUpdateAppFiles: 1,
  waitForStm32ResetAndReload: 1,
  waitForDeviceReady: 2,
  settleFetchText: 1,
  settleFetchJson: 1,
  loadWordclockLayoutPreview: 1,
};
// Fingerabdruck von "const LAYOUT_PREVIEW_ROWS_URL = ...;" + "\n" + loadLayoutPreviewRowsMap(), gleich in
// 1.4.101 (release/3.2.34-3.2.40-1.4.101) und R2b.6. Ent-20 B: Diese Funktion bleibt unveraendert.
const LAYOUT_FINGERABDRUCK = "825f7b93506ac1aeafefe9d79e1ba1bb6cc34b492204e15e06cb5ab37fd6dd30";
function teilC() {
  const stellen = [...quelle.matchAll(/cache:\s*"([\w-]+)"/g)].map((m) => ({ art: m[1], wo: lage(m.index), index: m.index }));
  const noStore = stellen.filter((s) => s.art === "no-store");
  const layoutStelle = schneide("loadLayoutPreviewRowsMap");
  pruefe("C1 kein no-store in loadLayoutPreviewRowsMap",
    !noStore.some((s) => s.index >= layoutStelle.start && s.index < layoutStelle.ende));
  pruefe("C2 kein cache \"reload\" in app.js", !stellen.some((s) => s.art === "reload"),
    stellen.filter((s) => s.art === "reload").map((s) => s.wo).join(", "));
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

  const konstZeile = (/^const LAYOUT_PREVIEW_ROWS_URL = [^\n]+$/m.exec(quelle) || [""])[0];
  const abdruck = createHash("sha256").update(konstZeile + "\n" + layoutStelle.text).digest("hex");
  pruefe("C5 loadLayoutPreviewRowsMap und LAYOUT_PREVIEW_ROWS_URL unveraendert (Ent-20 B)", abdruck === LAYOUT_FINGERABDRUCK, abdruck);

  const lesestellen = [...quelle.matchAll(/(?:querySelector(?:All)?|getElementsByName|getElementsByTagName|closest|matches)\(\s*(["'`])((?:(?!\1)[^\n])*)\1/g)]
    .filter((m) => m[2].includes("wc-asset-tokens"));
  pruefe("C6 genau eine Lesestelle fuer wc-asset-tokens", lesestellen.length === 1, "gezaehlt " + lesestellen.length);
  pruefe("C6 Lesestelle in getAppAssetTokens", lesestellen.every((m) => lage(m.index) === "getAppAssetTokens"), lesestellen.map((m) => lage(m.index)).join(", "));
  const aufrufe = [...quelle.matchAll(/\bgetAppAssetTokens\(/g)].filter((m) => !/function $/.test(quelle.slice(Math.max(0, m.index - 9), m.index)));
  pruefe("C7 genau ein Aufruf von getAppAssetTokens()", aufrufe.length === 1, "gezaehlt " + aufrufe.length);
  pruefe("C7 Aufruf nur in ensureLanguageTable", aufrufe.every((m) => lage(m.index) === "ensureLanguageTable"), aufrufe.map((m) => lage(m.index)).join(", "));
  console.log(`  ${bez}: cache-Optionen ${stellen.length} (no-store ${noStore.length}, ` +
    [...new Set(stellen.map((s) => s.art))].filter((x) => x !== "no-store").map((x) => x + " " + stellen.filter((s) => s.art === x).length).join(", ") +
    `), Sprachabrufe ${alleAbrufe.length}`);
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
  for (const f of fehler.slice(0, 25)) console.log("  - " + f);
  if (fehler.length > 25) console.log(`  ... und ${fehler.length - 25} weitere`);
  process.exit(1);
}
console.log(`OK   c70p ${bez}: ${faelle} Faelle, layout-previews.json ohne Query mit no-cache, Sprachdatei mit ?v= bzw. ?v=&h= nach Muster`);
process.exit(0);
