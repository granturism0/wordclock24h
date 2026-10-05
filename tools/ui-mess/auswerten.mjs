#!/usr/bin/env node
//
// Wertet eine Ergebnisdatei von vermessen.mjs aus.
//
//   node tools/ui-mess/auswerten.mjs                      juengster Lauf
//   node tools/ui-mess/auswerten.mjs <datei> --paare      Hoehendifferenzen (L227)
//
// Die Hoehendifferenz nebeneinanderliegender Kacheln ist die Groesse, an der AK3.1
// haengt. Sie wird hier GERECHNET und nicht im Browser bestimmt, damit dieselbe
// Rohmessung auch andere Fragen beantworten kann.
import { readFileSync, readdirSync } from "node:fs";

const AUS = "tools/ui-mess/ergebnisse";
const datei = process.argv[2]?.endsWith(".json") ? process.argv[2]
  : `${AUS}/${readdirSync(AUS).filter((f) => f.endsWith(".json")).sort().pop()}`;
const d = JSON.parse(readFileSync(datei, "utf8"));
console.log(`  ${datei}   Ziel: ${d.ziel}   ${d.zeitpunkt.slice(0, 16)}\n`);

for (const m of d.module) {
  const p = m.panels.filter((x) => !/banner/.test(x.name));
  // Nebeneinander heisst: die vertikalen Ausdehnungen ueberlappen sich.
  let groesste = 0, paar = "";
  for (let i = 0; i < p.length; i++)
    for (let j = i + 1; j < p.length; j++) {
      const a = p[i], b = p[j];
      if (a.y < b.y + b.h && b.y < a.y + a.h) {
        const diff = Math.abs(a.h - b.h);
        if (diff > groesste) { groesste = Math.round(diff); paar = `${a.name}(${Math.round(a.h)}) / ${b.name}(${Math.round(b.h)})`; }
      }
    }
  const randlos = m.klein.length;
  const ohneRing = m.fokus.filter((f) => !f.sichtbarerRing && !f.schatten).length;
  const schlechtester = m.farben.filter((f) => f.kontrastRand != null)
    .reduce((a, f) => (a == null || f.kontrastRand < a ? f.kontrastRand : a), null);
  console.log(`  ${m.viewport.padEnd(9)} ${m.modul.padEnd(13)} ` +
    `groesste Hoehendifferenz ${String(groesste).padStart(4)} px` +
    (paar ? `  (${paar})` : "") );
  if (randlos || ohneRing || (schlechtester != null && schlechtester < 3))
    console.log(`${"".padEnd(24)} ${randlos} Treffflaechen < 44px, ` +
      `${ohneRing} Fokusstationen ohne Ring, schlechtester Randkontrast ${schlechtester ?? "-"}`);
}
