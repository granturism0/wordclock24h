// Meldet CSS-Klassen, die im CSS definiert, aber in HTML und JS nie gesetzt werden.
// Haette die fuenf Grid-Area-Klassen aus REVIEW.md gefunden.
import { readFileSync } from "node:fs";

const [cssFile, ...consumers] = process.argv.slice(2);
const css = readFileSync(cssFile, "utf8");
const haystack = consumers.map((f) => readFileSync(f, "utf8")).join("\n");

const classes = new Set();
for (const m of css.matchAll(/\.(-?[A-Za-z_][\w-]*)/g)) classes.add(m[1]);

const EXTERNAL = /^(leaflet|marker-cluster)-/; // von Fremdbibliotheken gesetzt
const unused = [...classes].filter((c) => !EXTERNAL.test(c)).filter((c) => !new RegExp(`\\b${c.replace(/[-]/g, "\\-")}\\b`).test(haystack)).sort();

if (unused.length === 0) {
  console.log("  OK  jede CSS-Klasse wird irgendwo verwendet");
  process.exit(0);
}
console.log(`  MITTEL    ${unused.length} CSS-Klassen ohne Verwendung in ${consumers.map((f) => f.split("/").pop()).join(", ")}:`);
for (const c of unused) console.log(`            .${c}`);
process.exit(0); // meldet, blockiert nicht
