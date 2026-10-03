// Prüft die i18n-Tabellen: benutzte-aber-undefinierte Keys, Parität zwischen den
// Sprachen und doppelte Schlüssel.
// Hätte common.uploaded / common.setting / common.set aus REVIEW.md gefunden.
//
// SEIT DER AUSLAGERUNG (B15) LIEGEN DIE SPRACHEN IN ZWEI WELTEN
//
// Deutsch steht weiterhin als `de: { … }` in app.js — es ist die eingebaute Sprache
// und muss ohne Nachladen verfügbar sein. Jede weitere Sprache ist eine eigene Datei
// unter data/app/i18n/<code>.json. Diese Prüfung liest beides und vergleicht jede
// gefundene Sprache gegen Deutsch.
//
// WARUM ZUSÄTZLICH ZEILEN GEGEN SCHLÜSSEL GEZÄHLT WIRD (L130)
//
// Am 03.10.2026 stand `system.debug_eyebrow` in BEIDEN Tabellen doppelt — 929 Zeilen,
// 928 verschiedene Schlüssel. Diese Prüfung meldete nichts, und zwar nicht aus
// Nachlässigkeit, sondern strukturell: collect() sammelt in ein Set, das Duplikat
// fällt lautlos zusammen, und weil es auf beiden Seiten stand, blieben die Mengen
// gleich gross. Die Stufe konnte den Befund gar nicht finden.
//
// Folgenlos war es nur, weil beide Werte identisch waren ("Debug"). Bei
// unterschiedlichen Werten gewinnt in JavaScript der zweite, und die Oberfläche zeigt
// still den falschen Text — ohne dass irgendetwas anschlägt.
//
// Deshalb die Einstufung: HOCH bei gleichem Wert (ein Risiko, kein Fehlverhalten),
// KRITISCH bei verschiedenen Werten (ein stiller Textfehler). Die Stufe sieht beide
// Werte und kann das unterscheiden. Eine Prüfung, die bei jedem harmlosen Duplikat
// Alarm schlägt, liest nach kurzer Zeit niemand mehr — dieselbe Begründung wie bei
// umlaute.mjs.

import { readFileSync, readdirSync, existsSync } from "node:fs";
import { basename, dirname, join } from "node:path";

const appFile = process.argv[2];
const htmlFile = process.argv[3];
const src = readFileSync(appFile, "utf8");
const html = htmlFile ? readFileSync(htmlFile, "utf8") : "";

// ------------------------------------------------------------- Deutsch aus app.js
// Zwei Schreibweisen, weil die Auslagerung (B15) die Struktur geändert hat:
// früher `const I18N = { de: { … } }`, heute `const I18N_DE = { … }`. Beide werden
// erkannt, damit die Prüfung nicht an einer Umbenennung scheitert — und damit sie
// auf einem älteren Stand (etwa beim Bisect) weiterhin greift.
let deStart = src.search(/\nconst\s+I18N_DE\s*=\s*\{/);
if (deStart < 0) deStart = src.search(/\n\s{2}de:\s*\{/);
if (deStart < 0) {
  console.log("  FEHLER  Deutsche i18n-Tabelle in app.js nicht gefunden — weder 'const I18N_DE' noch 'de:'");
  process.exit(1);
}
// Blockende auf Klammerebene statt am nächsten Sprachblock: Seit der Auslagerung
// folgt auf `de:` kein `en:` mehr, an dem man abschneiden könnte.
let depth = 0, deEnd = src.length;
for (let i = src.indexOf("{", deStart); i < src.length; i++) {
  if (src[i] === "{") depth++;
  else if (src[i] === "}") { depth--; if (depth === 0) { deEnd = i + 1; break; } }
}
const deBlock = src.slice(deStart, deEnd);

// Zeilenweise sammeln, damit Duplikate sichtbar bleiben. Ein Set allein verschluckt
// sie — genau das war die Lücke aus L130.
const collectPairs = (text) => {
  const pairs = [];
  for (const m of text.matchAll(/^[ \t]*"([a-z0-9_]+(?:\.[a-z0-9_]+)+)"\s*:\s*("(?:[^"\\]|\\.)*")/gim)) {
    pairs.push([m[1], m[2]]);
  }
  return pairs;
};

const tables = new Map();   // Sprachcode -> [[key, rohwert], …]
tables.set("de", collectPairs(deBlock));

// ------------------------------------------------- weitere Sprachen aus i18n/*.json
const i18nDir = join(dirname(appFile), "i18n");
if (existsSync(i18nDir)) {
  for (const f of readdirSync(i18nDir).filter((n) => n.endsWith(".json")).sort()) {
    const code = basename(f, ".json");
    const text = readFileSync(join(i18nDir, f), "utf8");
    // Erst als JSON parsen: Eine Datei, die nicht lädt, ist auf dem Gerät nutzlos,
    // und das muss hier auffallen und nicht erst im Browser des Nutzers.
    try {
      JSON.parse(text);
    } catch (e) {
      console.log(`  KRITISCH  i18n/${f} ist kein gueltiges JSON (${e.message}) — die Sprache laedt am Geraet nicht`);
      process.exitCode = 1;
    }
    tables.set(code, collectPairs(text));
  }
}

// ------------------------------------------------------------------ Verwendung
const used = new Map();
for (const m of src.matchAll(/\btranslate(?:Format)?\s*\(\s*"([^"]+)"/g)) {
  if (!used.has(m[1])) used.set(m[1], appFile);
}
for (const m of html.matchAll(/\bdata-i18n(?:-[a-z-]+)?\s*=\s*"([^"]+)"/g)) {
  if (!used.has(m[1])) used.set(m[1], htmlFile);
}

let critical = 0, hoch = 0;
const sets = new Map([...tables].map(([c, p]) => [c, new Set(p.map(([k]) => k))]));

console.log("  Sprachen: " + [...tables].map(([c, p]) => `${c}=${p.length} Zeilen/${sets.get(c).size} Keys`).join(", ")
            + `, verwendet=${used.size}`);

// ------------------------------------------------- Duplikate (L130), je Sprache
for (const [code, pairs] of tables) {
  const seen = new Map();
  for (const [k, v] of pairs) {
    if (!seen.has(k)) { seen.set(k, v); continue; }
    if (seen.get(k) === v) {
      console.log(`  HOCH      '${k}' steht in '${code}' doppelt, beide Werte gleich — heute folgenlos, aber JSON kann das nicht`);
      hoch++;
    } else {
      console.log(`  KRITISCH  '${k}' steht in '${code}' doppelt mit VERSCHIEDENEN Werten — die UI zeigt still den zweiten`);
      console.log(`            erst:  ${seen.get(k).slice(0, 60)}`);
      console.log(`            dann:  ${v.slice(0, 60)}`);
      critical++;
    }
  }
}

// ------------------------------------------------------------- undefinierte Keys
const inAny = (k) => [...sets.values()].some((s) => s.has(k));
for (const k of [...used.keys()].filter((k) => !inAny(k))) {
  console.log(`  KRITISCH  Key '${k}' wird verwendet, steht aber in KEINER Tabelle — erscheint woertlich in der UI`);
  critical++;
}

// --------------------------------------------------- Parität gegen Deutsch
const de = sets.get("de");
for (const [code, set] of sets) {
  if (code === "de") continue;
  const fehlt = [...de].filter((k) => !set.has(k));
  const extra = [...set].filter((k) => !de.has(k));
  if (fehlt.length) { console.log(`  HOCH      ${fehlt.length} Keys fehlen in '${code}' (Fallback auf Deutsch): ${fehlt.slice(0, 5).join(", ")}${fehlt.length > 5 ? " …" : ""}`); hoch++; }
  if (extra.length) { console.log(`  HOCH      ${extra.length} Keys in '${code}' ohne deutsches Gegenstueck: ${extra.slice(0, 5).join(", ")}${extra.length > 5 ? " …" : ""}`); hoch++; }
}

const unused = [...de].filter((k) => !used.has(k));
if (unused.length) console.log(`  INFO      ${unused.length} definierte Keys werden nie verwendet`);

if (critical === 0 && hoch === 0) console.log("  OK  Tabellen paritaetisch, keine Duplikate, keine undefinierten Keys");
process.exit(critical > 0 ? 1 : 0);
