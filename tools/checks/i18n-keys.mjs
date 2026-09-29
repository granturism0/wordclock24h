// Prüft die i18n-Tabellen: benutzte-aber-undefinierte Keys und DE/EN-Parität.
// Hätte common.uploaded / common.setting / common.set aus REVIEW.md gefunden.
import { readFileSync } from "node:fs";

const appFile = process.argv[2];
const htmlFile = process.argv[3];
const src = readFileSync(appFile, "utf8");
const html = htmlFile ? readFileSync(htmlFile, "utf8") : "";

const deStart = src.search(/\n\s{2}de:\s*\{/);
const enStart = src.search(/\n\s{2}en:\s*\{/);
if (deStart < 0 || enStart < 0) {
  console.log("  FEHLER  i18n-Bloecke 'de:' oder 'en:' nicht gefunden — Struktur geaendert?");
  process.exit(1);
}
const collect = (text) => {
  const s = new Set();
  for (const m of text.matchAll(/^\s*"([a-z0-9_]+(?:\.[a-z0-9_]+)+)"\s*:/gim)) s.add(m[1]);
  return s;
};
const de = collect(src.slice(deStart, enStart));
const en = collect(src.slice(enStart));

const used = new Map();
for (const m of src.matchAll(/\btranslate(?:Format)?\s*\(\s*"([^"]+)"/g)) {
  if (!used.has(m[1])) used.set(m[1], appFile);
}
for (const m of html.matchAll(/\bdata-i18n(?:-[a-z-]+)?\s*=\s*"([^"]+)"/g)) {
  if (!used.has(m[1])) used.set(m[1], htmlFile);
}

let critical = 0;
console.log(`  Keys: de=${de.size} en=${en.size} verwendet=${used.size}`);

const undefinedKeys = [...used.keys()].filter((k) => !de.has(k) && !en.has(k));
for (const k of undefinedKeys) {
  console.log(`  KRITISCH  Key '${k}' wird verwendet, steht aber in KEINER Tabelle — erscheint woertlich in der UI`);
  critical++;
}
const missingEn = [...de].filter((k) => !en.has(k));
const missingDe = [...en].filter((k) => !de.has(k));
if (missingEn.length) console.log(`  HOCH      ${missingEn.length} Keys fehlen in 'en' (Fallback auf Deutsch): ${missingEn.slice(0, 5).join(", ")}${missingEn.length > 5 ? " …" : ""}`);
if (missingDe.length) console.log(`  HOCH      ${missingDe.length} Keys fehlen in 'de': ${missingDe.slice(0, 5).join(", ")}${missingDe.length > 5 ? " …" : ""}`);

const unused = [...de].filter((k) => !used.has(k));
if (unused.length) console.log(`  INFO      ${unused.length} definierte Keys werden nie verwendet`);

process.exit(critical > 0 ? 1 : 0);
