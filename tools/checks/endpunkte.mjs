#!/usr/bin/env node
//
// Die Liste der gefaehrlichen Endpunkte steht an genau EINEM Ort und laeuft nicht auseinander.
//
// WARUM ES DIESE PRUEFUNG GIBT -- ein eingetretener Fall, kein Risiko
//
// Am 05.10.2026 fuehrte CLAUDE.md unter R5 sieben gefaehrliche Endpunkte, DIR-008 im
// Katalog dieselbe Liste als Fliesstext mit sechs. Es fehlte /api/ir_code_set: am
// 03.10. in die Tabelle aufgenommen, in den Katalog nie nachgetragen, und DIR-008 stand
// unveraendert auf seinem alten seit:-Datum. Wer den Katalog als Quelle las, hielt den
// Endpunkt fuer unbedenklich -- er ueberschreibt einen angelernten IR-Code, und der
// einzige Rueckweg ist learn_ir, das selbst einen Watchdog-Reset garantiert.
//
// Das ist eine Ebene tiefer als eine fehlende Kennung (L225): auseinandergelaufener
// INHALT unter derselben Kennung. Gefunden hat es ein Agent beim Nachtragen, nicht die
// Pruefung -- die gab es nicht.
//
// DIE ENTSCHEIDUNG, DIE DAHINTERSTEHT
//
// Die Liste wird in CLAUDE.md gefuehrt, als Tabelle mit der Folge je Endpunkt, weil sie
// dort beim Arbeiten gelesen wird. Der Katalog fuehrt die REGEL.
//
// Die Stufe verlangt ENTWEDER VOLLSTAENDIG ODER GAR NICHT, und das ist der Kern:
// Ein erster Entwurf liess den Katalog kuerzen, solange er nicht abweicht. Das waere
// gruen gewesen und haette den Befund nur wegdefiniert -- wer den Katalog liest, haelt
// einen dort fehlenden Endpunkt weiterhin fuer unbedenklich, und genau das ist passiert.
// Eine unvollstaendige Liste ist gefaehrlicher als keine: Sie sieht aus wie eine Liste.
import { readFileSync } from "node:fs";

const claude = readFileSync("CLAUDE.md", "utf8");
const kat = readFileSync("knowledge/directives.md", "utf8");
let fehler = 0;

// Die Gefahrentabelle: der Abschnitt ab "Diese Endpunkte sind aus unseren eigenen
// Befunden heraus gefaehrlich" bis zur naechsten Ueberschrift.
const tabStart = claude.indexOf("Diese Endpunkte sind aus unseren eigenen Befunden");
if (tabStart < 0) {
  console.log("  HOCH      CLAUDE.md: die Gefahrentabelle unter R5 ist nicht auffindbar");
  process.exit(1);
}
const tab = claude.slice(tabStart, claude.indexOf("\n## ", tabStart));
const inTabelle = new Set([...tab.matchAll(/\/api\/([a-z_0-9]+)/g)].map((m) => m[1]));

// DIR-008 im Katalog
const d8 = kat.slice(kat.indexOf("DIR-008:"), kat.indexOf("  seit:", kat.indexOf("DIR-008:")));
const imKatalog = new Set([...d8.matchAll(/\b([a-z_]+(?:_[a-z_0-9]+)+|fs_remove|learn_ir)\b/g)]
  .map((m) => m[1])
  .filter((w) => /^(maintenance_|fs_|test_|learn_|ir_|remote_|update_|backup)/.test(w)));

console.log(`      Tabelle in CLAUDE.md: ${inTabelle.size} Endpunkte, DIR-008 nennt ${imKatalog.size}`);

const fremd = [...imKatalog].filter((e) => !inTabelle.has(e)).sort();
const fehlend = [...inTabelle].filter((e) => !imKatalog.has(e)).sort();

if (fremd.length) {
  fehler = 1;
  console.log(`  HOCH      DIR-008 nennt Endpunkte, die in der CLAUDE.md-Tabelle fehlen: ${fremd.join(", ")}`);
} else if (imKatalog.size === 0) {
  console.log("  OK  DIR-008 fuehrt keine eigene Endpunktliste, sondern verweist");
} else if (fehlend.length) {
  fehler = 1;
  console.log(`  HOCH      DIR-008 fuehrt eine UNVOLLSTAENDIGE Endpunktliste — es fehlen: ${fehlend.join(", ")}`);
  console.log("            Entweder vollstaendig oder gar nicht. Eine unvollstaendige Liste");
  console.log("            sieht aus wie eine Liste und wird als solche gelesen (L278).");
} else {
  console.log(`  OK  DIR-008 fuehrt die Liste vollstaendig (${imKatalog.size} Endpunkte)`);
}
process.exit(fehler);
