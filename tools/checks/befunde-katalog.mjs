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
//   4. Jeder Befund mit Status "offen", "zurueckgestellt" oder "teilweise" ist
//      im Abschnitt "ToDo" genannt
// Pruefung 4 gibt es, weil die Tabellen den STAND fuehren, aber niemand aus
// ihnen ablesen kann, was als Naechstes zu tun ist. Eine Arbeitsliste, die nur
// von Hand nachgezogen wird, veraltet still -- dieselbe Fehlerklasse wie die
// Versionsnummern, die in README-CMAKE.md monatelang falsch standen.
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

// ---- 4. Offene Befunde muessen in der ToDo-Liste stehen
// Zugeordnet wird ueber die Kennung, nicht ueber den Text: Review 1 ueber
// "Massnahme <n>", Review 2 ueber "R2-<n>", laufende Arbeit ueber "L<n>". Wer
// eine Zeile auf "offen" setzt und sie nicht in die Liste nimmt, faellt auf.
function openRows(text, headingStartsWith, numPattern) {
  const sec = text.split(/^## /m).find((s) => s.startsWith(headingStartsWith));
  if (!sec) return null;
  const out = [];
  for (const line of sec.split("\n")) {
    if (!line.startsWith("|")) continue;
    const cols = line.split("|").map((c) => c.trim());
    if (cols.length < 4) continue;
    const id = (cols[1].match(numPattern) || [])[1];
    if (!id) continue;
    // Spalte 3 ist der Status, und nur sie wird geprueft. Sonst schluege
    // "erledigt, Annahme korrigiert" an, weil im Belegtext "offen bleibt" steht.
    if (/^\*\*(offen|zur(ü|ue)ckgestellt|teilweise)/i.test(cols[3])) out.push(id);
  }
  return out;
}

const todo = cat.split(/^## /m).find((s) => s.startsWith("ToDo"));
if (!todo) {
  fail('BEFUNDE.md: Abschnitt "ToDo" fehlt — ohne ihn gibt es keine Arbeitsliste');
} else {
  const groups = [
    ["Review 1", /^(\d+)$/, (n) => new RegExp("Massnahme " + n + "\\b"), "Massnahme "],
    ["Review 2", /^(\d+)$/, (n) => new RegExp("R2-" + n + "\\b"), "R2-"],
    ["Befunde aus der laufenden Arbeit", /^L(\d+)$/, (n) => new RegExp("\\bL" + n + "\\b"), "L"]
  ];
  const missing = [];
  for (const [heading, numPattern, re, label] of groups) {
    const open = openRows(cat, heading, numPattern);
    if (open === null) { fail('BEFUNDE.md: Abschnitt "' + heading + '" fehlt'); continue; }
    for (const n of open) if (!re(n).test(todo)) missing.push(label + n);
  }
  if (missing.length) fail("BEFUNDE.md: offen, aber nicht in der ToDo-Liste — " + missing.join(", "));
  else console.log("  OK  ToDo-Liste nennt jeden offenen Befund");
}

process.exit(bad === 0 ? 0 : 1);
