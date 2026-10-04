#!/usr/bin/env node
//
// S11 -- Jede im Repo zitierte Direktive steht auch im Katalog.
//
// WARUM ES DIESE PRUEFUNG GIBT
//
// Am 04.10.2026 fuehrte knowledge/directives.md DIR-000 bis DIR-009, waehrend im
// Repo DIR-000 bis DIR-014 zitiert wurden -- FUENF fehlten, darunter das STM-Flashen
// (DIR-010), die Commit- und Tag-Pflicht (DIR-011) und die Logwache waehrend Tests
// (DIR-013). Der Katalog sagt ueber sich selbst "Nichts steht hier ohne Bestaetigung";
// wer ihn als Quelle las, bekam seit DIR-009 ein unvollstaendiges Bild.
//
// Gemeldet hat es ein Agent, der eine Direktive zitieren wollte und sie nicht fand --
// nicht der, der sie eingefuehrt hat. Das ist dieselbe Gattung wie ein Befund, der in
// keiner ToDo-Liste steht (S10), und wie eine Versionsnummer in der Doku (S9): eine
// Angabe, die still veraltet, weil niemand sie nachschlaegt.
//
// WAS SIE BEWUSST NICHT PRUEFT
//
// Ob der INHALT eines Eintrags noch stimmt. Das ist statisch nicht entscheidbar, und
// eine Pruefung, die es vorgibt, waere schlimmer als keine. Sie prueft die Existenz,
// und das ist genau der Fehler, der aufgetreten ist.
//
// Sie prueft ausserdem nicht die Gegenrichtung (Katalogeintrag, den niemand zitiert).
// Eine Direktive darf gelten, ohne zitiert zu werden -- das waere ein Fehlalarm.
import { readFileSync } from "node:fs";
import { execSync } from "node:child_process";

// Der Katalogpfad ist ueberschreibbar, damit die Fehlschlagprobe nach DIR-014 an einer
// ATTRAPPE laufen kann statt an einer versionierten Datei:
//
//     node tools/checks/direktiven.mjs /pfad/zu/leerer-attrappe.md     # muss rot werden
//
// Der erste Nachweis am 04.10.2026 wurde stattdessen durch ein Testzitat "DIR-999" in
// TESTPLAN-PWA.md gefuehrt -- waehrend vier Agenten liefen. Einer sah es im Guardrail-Lauf
// und musste raten, ob es ein echter Tippfehler ist; haette er es "korrigiert", waere daraus
// Durcheinander geworden. Gemeldet statt angefasst hat ihn gerettet, nicht die Vorgehensweise.
// Eine Probe, die den Arbeitsbaum anfasst, ist bei paralleler Arbeit selbst ein Risiko.
const KATALOG = process.argv[2] || "knowledge/directives.md";
const MUSTER = /\bDIR-(\d{3})\b/g;

// DIR-9xx ist als BEISPIELBEREICH reserviert und wird nie als Zitat gewertet -- wie
// example.com bei Domains oder 555-0100 bei Telefonnummern.
//
// Der Grund ist teuer erarbeitet: Diese Pruefung kann ein Zitat nicht von der
// BESCHREIBUNG eines Zitats unterscheiden, und sie sucht den ganzen Arbeitsbaum ab.
// Am 04./05.10.2026 ist sie deshalb DREIMAL auf Text angeschlagen, der ueber sie
// schreibt: auf das Testzitat der eigenen Fehlschlagprobe, auf den Kommentar, der
// dieses Testzitat erklaerte, und auf die Befundzeile, die beide Vorfaelle festhielt.
// Jedes Mal hat jemand Zeit darauf verwendet, ein Nicht-Problem zu melden.
//
// Der erste Flicken -- diese Datei von der Suche ausnehmen -- half nur ihr selbst und
// liess BEFUNDE.md auflaufen. Ein reservierter Bereich loest es fuer ALLE Dokumente,
// und er macht die Falle zugleich unschaedlich: Wer kuenftig ueber einen Fehlschlag
// schreiben will, nimmt eine 9xx-Nummer und darf sie woertlich hinschreiben.
//
// Echte Kennungen duerfen hier nie hineinwachsen. Bei 15 vergebenen und einem Abstand
// von ueber 980 ist das keine reale Gefahr; sollte es je eng werden, meldet die Stufe
// einen Konflikt, statt still das Falsche zu tun.
const BEISPIELBEREICH = /^9\d\d$/;
let fehler = 0;

const katalog = readFileSync(KATALOG, "utf8");
const gefuehrt = new Set();
for (const m of katalog.matchAll(/^DIR-(\d{3}):/gm)) gefuehrt.add(m[1]);

const dateien = execSync("git ls-files", { encoding: "utf8" }).split("\n").filter(Boolean);
const zitiert = new Map();          // nummer -> Set von Dateien
for (const f of dateien) {
  if (f === KATALOG) continue;
  let txt;
  try { txt = readFileSync(f, "latin1"); } catch { continue; }
  for (const m of txt.matchAll(MUSTER)) {
    if (BEISPIELBEREICH.test(m[1])) continue;        // reserviert, siehe oben
    if (!zitiert.has(m[1])) zitiert.set(m[1], new Set());
    zitiert.get(m[1]).add(f);
  }
}

// DIR-014 verlangt, den Umfang zu nennen: Eine Pruefung, die zu wenig SIEHT, meldet
// OK und deckt nichts. Die Zahlen gehoeren deshalb in die Ausgabe, nicht nur das Urteil.
const imBereich = [...gefuehrt].filter((n) => BEISPIELBEREICH.test(n)).sort();
if (imBereich.length) {
  fehler = 1;
  console.log(`  HOCH      Katalog vergibt Kennungen im Beispielbereich DIR-9xx: ${imBereich.join(", ")}`);
  console.log("            Der Bereich ist reserviert und wird von dieser Stufe nicht geprueft.");
}

const fehlend = [...zitiert.keys()].filter((n) => !gefuehrt.has(n)).sort();
console.log(`      ${gefuehrt.size} Direktiven im Katalog, ${zitiert.size} im Repo zitiert`);

if (fehlend.length) {
  fehler = 1;
  for (const n of fehlend) {
    const wo = [...zitiert.get(n)].sort().slice(0, 4).join(", ");
    console.log(`  HOCH      DIR-${n} wird zitiert, steht aber nicht im Katalog — ${wo}`);
  }
  console.log(`            Nachtragen in ${KATALOG} (Revier: doc-writer).`);
} else {
  console.log("  OK  jede zitierte Direktive steht im Katalog");
}
process.exit(fehler);
