// Findet Klassen, die app.js setzt, fuer die es aber KEINE CSS-Regel gibt.
//
// Das ist die Gegenrichtung zu unused-css.mjs, und sie fehlte. Jene Stufe meldet
// CSS-Klassen ohne Verwender; hier geht es um Verwender ohne Regel.
//
// Der Anlass: Mit L59 stellte app.js sieben Schalter von "primary" auf "is-on" um --
// und "is-on" hatte keine Regel. Sieben Schalter zeigten ihren Ein-Zustand gar nicht
// mehr an, und keine Guardrail-Stufe hat das gemeldet. Die Klasse war ja benutzt,
// nur eben wirkungslos.
//
// Geprueft werden classList.add/toggle/replace mit Zeichenketten-Literal. Dynamisch
// zusammengesetzte Klassennamen bleiben aussen vor -- sie liessen sich nur raten,
// und eine Pruefung, die raet, erzeugt Fehlalarme.

import { readFileSync } from "node:fs";

const [jsFile, cssFile] = process.argv.slice(2);
const js = readFileSync(jsFile, "utf8");
const css = readFileSync(cssFile, "utf8");

const used = new Set();
// Das abschliessende (?!\s*\+) schliesst zusammengesetzte Namen aus: Bei
// classList.toggle("is-" + name, …) ist "is-" kein Klassenname, sondern ein
// Praefix. Ohne diese Bedingung meldete die Pruefung beim ersten Lauf genau das
// als fehlende Regel -- ein Fehlalarm, und davon lebt keine Pruefung lange.
const ADD = /classList\.(?:add|toggle|replace)\s*\(\s*"([A-Za-z0-9_-]+)"(?!\s*\+)/g;
for (const m of js.matchAll(ADD)) {
  if (m[1].endsWith("-")) continue;   // zweite Sicherung fuer Praefixe
  used.add(m[1]);
}

// Zweites Argument von replace() ist ebenfalls eine Klasse.
for (const m of js.matchAll(/classList\.replace\s*\(\s*"[A-Za-z0-9_-]+"\s*,\s*"([A-Za-z0-9_-]+)"/g)) used.add(m[1]);

// Klassen, die im erzeugten Markup stehen, deckt unused-css.mjs ab -- hier geht es
// um die, die zur Laufzeit geschaltet werden.
const defined = new Set();
for (const m of css.matchAll(/\.([A-Za-z0-9_-]+)/g)) defined.add(m[1]);

const missing = [...used].filter((c) => !defined.has(c)).sort();

if (missing.length) {
  for (const c of missing) {
    const line = js.split("\n").findIndex((l) => l.includes(`"${c}"`)) + 1;
    console.log(`  HOCH      Klasse "${c}" wird gesetzt (app.js:${line}), hat aber keine CSS-Regel — sie wirkt nicht`);
  }
  process.exit(1);
}

console.log(`  OK  alle ${used.size} zur Laufzeit gesetzten Klassen haben eine CSS-Regel`);
