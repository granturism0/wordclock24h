// Prueft, dass BEFUNDE.md den Massnahmenkatalog LUECKENLOS fuehrt.
//
// Hintergrund: Die beiden Reviews sind Momentaufnahmen und werden nicht
// fortgeschrieben (DIR-006). Der Stand ihrer Massnahmen lebt in BEFUNDE.md.
// Ohne Pruefung kann dort still eine Zeile fehlen — und genau das ist die
// Fehlerklasse, die schon einmal zugeschlagen hat: "fuehre die Doku nach" ist
// eine Absichtserklaerung, keine Pruefung.
//
// Geprueft wird dreierlei:
//   1. Jede Massnahmennummer aus REVIEW.md hat eine Zeile in BEFUNDE.md
//   2. Dasselbe fuer REVIEW-2026-09-29.md
//   3. Die L-Nummern der laufenden Arbeit sind lueckenlos ab L1
// Der Status selbst wird NICHT geprueft — den kann nur ein Mensch oder ein
// gezielter Codecheck setzen. Geprueft wird die Vollstaendigkeit.

import { readFileSync } from "node:fs";

const read = (f) => { try { return readFileSync(f, "utf8"); } catch { return null; } };

// Nummern aus dem Abschnitt "Priorisierte Massnahmen" eines Reviews
function reviewMeasures(file) {
  const text = read(file);
  if (text === null) return { err: `${file} nicht lesbar` };
  const sec = text.split(/^# /m).find((s) => s.startsWith("Priorisierte Massnahmen"));
  if (!sec) return { err: `${file}: Abschnitt "Priorisierte Massnahmen" nicht gefunden` };
  const nums = [...sec.matchAll(/^\|\s*(\d+)\s*\|/gm)].map((m) => Number(m[1]));
  return { nums: [...new Set(nums)].sort((a, b) => a - b) };
}

// Nummern aus einem Abschnitt von BEFUNDE.md
function catalogNums(text, headingStartsWith, pattern) {
  const sec = text.split(/^## /m).find((s) => s.startsWith(headingStartsWith));
  if (!sec) return null;
  return new Set([...sec.matchAll(pattern)].map((m) => m[1]));
}

const cat = read("BEFUNDE.md");
if (cat === null) { console.log("  KRITISCH  BEFUNDE.md fehlt — der Massnahmenkatalog ist das lebende Gegenstueck zu den Reviews"); process.exit(1); }

let bad = 0;
const fail = (msg) => { console.log(`  HOCH      ${msg}`); bad++; };

for (const [file, heading] of [["REVIEW.md", "Review 1"], ["REVIEW-2026-09-29.md", "Review 2"]]) {
  const { nums, err } = reviewMeasures(file);
  if (err) { fail(err); continue; }
  const have = catalogNums(cat, heading, /^\|\s*(\d+)\s*\|/gm);
  if (!have) { fail(`BEFUNDE.md: Abschnitt "${heading}" fehlt`); continue; }
  const missing = nums.filter((n) => !have.has(String(n)));
  if (missing.length) fail(`BEFUNDE.md, ${heading}: Massnahme(n) ${missing.join(", ")} aus ${file} fehlen im Katalog`);
  else console.log(`  OK  ${heading}: alle ${nums.length} Massnahmen aus ${file} im Katalog`);
}

const live = catalogNums(cat, "Befunde aus der laufenden Arbeit", /^\|\s*L(\d+)\s*\|/gm);
if (!live) fail('BEFUNDE.md: Abschnitt "Befunde aus der laufenden Arbeit" fehlt');
else {
  const n = [...live].map(Number).sort((a, b) => a - b);
  const gaps = [];
  for (let i = 1; i <= (n[n.length - 1] || 0); i++) if (!live.has(String(i))) gaps.push(`L${i}`);
  if (gaps.length) fail(`BEFUNDE.md: Luecke in der L-Nummerierung — ${gaps.join(", ")} fehlen`);
  else console.log(`  OK  laufende Arbeit: L1 bis L${n[n.length - 1]} lueckenlos`);
}

process.exit(bad === 0 ? 0 : 1);
