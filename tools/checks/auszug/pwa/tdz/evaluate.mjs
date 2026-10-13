// pwa-developer: wertet die Ausgabe von harness.mjs aus. Exit 0/1/2 wie run.sh.
import fs from "node:fs";
const d = JSON.parse(fs.readFileSync(process.argv[2], "utf8"));
let broken = [];
let fails = [];
const mods = d.modulesInHtml;
if (mods.length === 0) broken.push("keine Module in index.html gefunden");
const chipsSorted = [...d.chipTargets].sort().join(",");
if (chipsSorted !== [...mods].sort().join(",")) broken.push("Module (" + mods.length + ") und Modul-Chips (" + d.chipTargets.length + ") in index.html stimmen nicht ueberein");
if (d.catchInstrumented !== d.catchTotal) broken.push("nur " + d.catchInstrumented + " von " + d.catchTotal + " catch-Klauseln instrumentiert");
const seen = new Set();
for (const r of d.results) {
  const label = (r.module === null ? "(kein gespeichertes Modul)" : r.module) + (r.full ? " [localStorage voll]" : " [localStorage leer]");
  if (r.module !== null) seen.add(r.module);
  // Positivkontrollen: Ohne sie bestuende ein Pruefstand, der gar nichts startet.
  const expected = mods.includes(r.module) ? r.module : "main";
  if (r.activeAfter !== expected) broken.push(label + ": aktives Modul nach dem Start '" + r.activeAfter + "', erwartet '" + expected + "'");
  if (r.module === "system" && !r.fatal && !r.refErrors.length && !r.fetched.some((u) => u.includes("stm32_log"))) broken.push(label + ": STM32-Logbuch wurde nicht abgerufen -- der gesuchte Pfad wird gar nicht erreicht");
  if (r.fatal) fails.push("FEHL " + label + ": Skript bricht ab: " + r.fatal);
  for (const e of r.refErrors) fails.push("FEHL " + label + ": " + e);
  if (!r.fatal && !r.refErrors.length) console.log("OK   " + label);
}
const missing = mods.filter((m) => !seen.has(m));
if (missing.length) broken.push("nicht gestartet: " + missing.join(", "));
console.log("Module in index.html: " + mods.length + ", gestartet: " + mods.filter((m) => seen.has(m)).length + " (+ ohne Eintrag, + ungueltiger Name), je 2 Speicherbelegungen = " + d.results.length + " Starts; catch-Klauseln instrumentiert " + d.catchInstrumented + "/" + d.catchTotal + ", Promise-.catch " + d.promiseCatchTotal + " ueber then-Haken");
for (const f of fails) console.log(f);
for (const b of broken) console.error("PRUEFSTAND KAPUTT: " + b);
if (broken.length) process.exit(2);
if (fails.length) { console.log("ERGEBNIS: FEHL (" + fails.length + ")"); process.exit(1); }
console.log("ERGEBNIS: OK");
process.exit(0);
