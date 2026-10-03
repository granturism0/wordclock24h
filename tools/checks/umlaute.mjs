// Findet ASCII-Umschrift in den deutschen Texten der PWA.
//
// Hintergrund: In diesem Projekt gelten zwei verschiedene Schreibweisen, und sie
// haben beide ihren Grund.
//
//   C-Dateien (src/**, http.cpp)  ISO-8859-1 oder ASCII  ->  Umschrift: "Geraet"
//   PWA (app.js, index.html)      UTF-8                  ->  echte Umlaute: "Gerät"
//
// Die Umschrift in den C-Dateien ist KEINE Sprachregel, sondern Vorsicht: Schreibt
// ein Werkzeug ein 'ä' in eine ISO-8859-1-Datei und nimmt dabei UTF-8 an, ist die
// Datei beschaedigt. Das ist hier schon passiert.
//
// Wer in derselben Sitzung an http.cpp und an app.js arbeitet, schleppt die
// Gewohnheit aus der einen Welt in die andere. Genau so sind am 03.10.2026
// "Schluessel" und "ungueltig" in die deutschen Fehlertexte geraten -- aufgefallen
// ist es dem Nutzer, nicht einer Pruefung.
//
// Geprueft wird mit einer BENANNTEN Liste von Umschriften, nicht mit dem Digraph
// "ue"/"oe"/"ae". Sonst meldete die Pruefung "aktuell", "Quelle", "Steuerung",
// "neue", "Dauer", "manuell" und "zuerst" -- alles echte deutsche Woerter ohne
// gemeinten Umlaut. Eine Pruefung mit Fehlalarmen liest irgendwann niemand mehr.

import { readFileSync } from "node:fs";

// Stamm -> richtige Schreibung. Gross-/Kleinschreibung wird beim Pruefen ignoriert,
// der Vorschlag nennt die Kleinform.
const UMSCHRIFTEN = [
  ["geraet", "gerät"], ["waehrend", "während"], ["waehl", "wähl"], ["naechst", "nächst"],
  ["aendern", "ändern"], ["aenderung", "änderung"], ["zusaetzlich", "zusätzlich"],
  ["taeglich", "täglich"], ["spaeter", "später"], ["erklaert", "erklärt"],
  ["haeufig", "häufig"], ["ungefaehr", "ungefähr"], ["qualitaet", "qualität"],
  ["maerz", "märz"], ["gaeste", "gäste"],
  ["moeglich", "möglich"], ["koennen", "können"], ["oeffnen", "öffnen"],
  ["loeschen", "löschen"], ["hoeher", "höher"], ["groesser", "grösser"],
  ["schoen", "schön"], ["noetig", "nötig"], ["woechentlich", "wöchentlich"],
  ["fuer", "für"], ["ueber", "über"], ["muessen", "müssen"], ["zurueck", "zurück"],
  ["gueltig", "gültig"], ["ungueltig", "ungültig"], ["schluessel", "schlüssel"],
  ["pruef", "prüf"], ["fuehr", "führ"], ["duerfen", "dürfen"], ["kuerz", "kürz"],
  ["stueck", "stück"], ["ueblich", "üblich"], ["natuerlich", "natürlich"],
  ["verfuegbar", "verfügbar"], ["zukuenftig", "zukünftig"], ["fruehe", "frühe"],
  ["waehrung", "währung"], ["ausfuehr", "ausführ"], ["einfuehr", "einführ"],
];

// Nur die deutschen Zeichenketten. Die englische Tabelle und der Quelltext
// drumherum sind nicht gemeint.
function germanStrings(text) {
  const out = [];
  const lines = text.split("\n");
  let inGerman = false;

  lines.forEach((line, i) => {
    if (/^\s*(const\s+)?(TRANSLATIONS\s*=\s*\{|)?\s*de\s*:\s*\{/.test(line)) { inGerman = true; return; }
    if (inGerman && /^\s*\},?\s*$/.test(line)) { inGerman = false; return; }
    if (inGerman && /^\s*en\s*:\s*\{/.test(line)) { inGerman = false; return; }
    if (!inGerman) return;

    const m = line.match(/"[^"]*"\s*:\s*"(.*)"\s*,?\s*$/);
    if (m) out.push([i + 1, m[1]]);
  });

  return out;
}

let bad = 0;

for (const file of process.argv.slice(2)) {
  let text;
  try { text = readFileSync(file, "utf8"); } catch { continue; }

  // index.html und andere Markup-Dateien haben keine Tabelle -- dort wird der
  // sichtbare Text direkt gepruefkt.
  const entries = file.endsWith(".js")
    ? germanStrings(text)
    : text.split("\n").map((l, i) => [i + 1, l]);

  for (const [lineNo, value] of entries) {
    const lower = value.toLowerCase();
    for (const [wrong, right] of UMSCHRIFTEN) {
      if (lower.includes(wrong)) {
        console.log(`  HOCH      ${file}:${lineNo}  "${wrong}" statt "${right}" — die PWA ist UTF-8, Umschrift gehoert nur in die C-Dateien`);
        bad++;
        break;
      }
    }
  }
}

if (bad === 0) console.log("  OK  deutsche PWA-Texte ohne ASCII-Umschrift");
process.exit(bad === 0 ? 0 : 1);
