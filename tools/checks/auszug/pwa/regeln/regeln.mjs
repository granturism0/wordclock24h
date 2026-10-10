// Pruefstand Regelgleichheit C55/C44 (Paket 2026-10-10, P.3, AKR.5).
//
//   node pwa-p-regeln.mjs <app.js> <regeln.tsv> [bezeichnung]
//
// Fuehrt die Regelfunktionen der PWA gegen DIESELBE Fall-Tabelle aus, gegen die t17 den ESP
// prueft. Die Funktionen werden aus app.js ausgeschnitten (nicht abgeschrieben) und in einem
// eigenen vm-Kontext ausgefuehrt.
//
// Gemessen wird, was die PWA vor dem Senden durchlaesst -- also Laengenpruefung UND Regel,
// genau wie im Speicherweg (readTextInputOrReport, dann isValid...). Fehlen die
// Regelfunktionen (Ausgangsstand), bleibt nur die Laengenpruefung; so laesst der alte Stand
// "<\"&" durch, und der Pruefstand meldet FEHL.
//
// Jede Abweichung wird beim Namen gemeldet (Feld, Eingabe, Tabelle, PWA, Notiz). Exit 0 nur,
// wenn jeder Fall gleich entschieden wird; 1 bei Abweichung; 2 bei Werkzeugfehler.
import { readFileSync } from "node:fs";
import vm from "node:vm";

const [appPath, tsvPath, bezeichnung = "PWA"] = process.argv.slice(2);
if (!appPath || !tsvPath) {
  console.error("Aufruf: node pwa-p-regeln.mjs <app.js> <regeln.tsv> [bezeichnung]");
  process.exit(2);
}

const quelle = readFileSync(appPath, "utf8");

// Ausschneiden mit Klammerzaehlung, die Zeichenketten und Kommentare ueberspringt.
function schneide(startIndex) {
  let i = quelle.indexOf("{", startIndex);
  let tiefe = 0;
  for (; i < quelle.length; i++) {
    const c = quelle[i];
    const n = quelle[i + 1];
    if (c === "/" && n === "/") { i = quelle.indexOf("\n", i); continue; }
    if (c === "/" && n === "*") { i = quelle.indexOf("*/", i) + 1; continue; }
    if (c === '"' || c === "'" || c === "`") {
      for (i++; i < quelle.length && quelle[i] !== c; i++) if (quelle[i] === "\\") i++;
      continue;
    }
    if (c === "{") tiefe++;
    if (c === "}" && --tiefe === 0) return quelle.slice(startIndex, i + 1);
  }
  throw new Error("Klammer nicht geschlossen ab " + startIndex);
}

function funktion(name) {
  const m = new RegExp("^function " + name + "\\(", "m").exec(quelle);
  return m ? schneide(m.index) : null;
}

function konstante(name) {
  const m = new RegExp("^const " + name + " = ", "m").exec(quelle);
  if (!m) return null;
  const ende = quelle.indexOf("});", m.index);
  return quelle.slice(m.index, ende + 3);
}

const teile = [];
const pflicht = ["countUtf8Bytes", "getTextFieldOverflow"];
for (const name of pflicht) {
  const f = funktion(name);
  if (!f) { console.error(`WERKZEUGFEHLER (${bezeichnung}): ${name} nicht gefunden`); process.exit(2); }
  teile.push(f);
}
const grenzen = konstante("TEXT_FIELD_LIMITS");
if (!grenzen) { console.error(`WERKZEUGFEHLER (${bezeichnung}): TEXT_FIELD_LIMITS nicht gefunden`); process.exit(2); }
teile.push(grenzen.replace(/^const /, "var "));
const hilfe = funktion("utf8ByteLengthOfCodePoint");
if (hilfe) teile.push(hilfe);

const regelnamen = ["isValidDateTickerFormat", "normalizeWeatherCoordinate", "isValidWeatherCoordinate"];
const regeln = regelnamen.map(funktion);
const regelVorhanden = regeln.every(Boolean);
if (regelVorhanden) {
  teile.push(...regeln);
} else {
  console.log(`  HINWEIS ${bezeichnung}: Regelfunktionen fehlen (${regelnamen.filter((_, i) => !regeln[i]).join(", ")}) -- gemessen wird nur die Laengenpruefung`);
}

const kontext = vm.createContext({ TextEncoder });
vm.runInContext(teile.join("\n\n") + `
  var __feldname = { datumsformat: "date_ticker_format", laenge: "weather_lon", breite: "weather_lat" };
  var __grenze = { laenge: 180, breite: 90 };
  function __pwaNimmtAn(feld, eingabe) {
    if (getTextFieldOverflow(__feldname[feld], eingabe)) return false;
    ${regelVorhanden ? `
    if (feld === "datumsformat") return isValidDateTickerFormat(eingabe);
    return isValidWeatherCoordinate(eingabe, __grenze[feld]);` : `
    return true;`}
  }
`, kontext, { filename: "app.js-auszug" });

// Tabelle lesen: dieselbe Datei wie t17. Kommentare (#) und die Kopfzeile fallen weg.
const zeilen = readFileSync(tsvPath, "utf8").split("\n");
const faelle = [];
for (const [nr, zeile] of zeilen.entries()) {
  if (!zeile || zeile.startsWith("#") || zeile.startsWith("feld\t")) continue;
  const spalten = zeile.split("\t");
  if (spalten.length < 3 || !["datumsformat", "laenge", "breite"].includes(spalten[0])
      || !["gueltig", "ungueltig"].includes(spalten[2])) {
    console.error(`WERKZEUGFEHLER (${bezeichnung}): Zeile ${nr + 1} unlesbar: ${JSON.stringify(zeile)}`);
    process.exit(2);
  }
  faelle.push({ nr: nr + 1, feld: spalten[0], eingabe: spalten[1], erwartet: spalten[2] === "gueltig", notiz: spalten[3] || "" });
}

const jeFeld = {};
let abweichungen = 0;
for (const f of faelle) {
  jeFeld[f.feld] = (jeFeld[f.feld] || 0) + 1;
  const pwa = kontext.__pwaNimmtAn(f.feld, f.eingabe);
  if (pwa !== f.erwartet) {
    abweichungen++;
    console.log(`  ABWEICHUNG ${f.feld} ${JSON.stringify(f.eingabe)} (Zeile ${f.nr}): Tabelle ${f.erwartet ? "gueltig" : "ungueltig"}, PWA ${pwa ? "nimmt an" : "weist ab"}${f.notiz ? "  -- " + f.notiz : ""}`);
  }
}

const zaehlung = Object.entries(jeFeld).map(([k, v]) => `${k} ${v}`).join(", ");
console.log(`${abweichungen ? "FEHL" : "OK  "} Regelgleichheit ${bezeichnung}: ${faelle.length} Faelle (${zaehlung}), ${abweichungen} abweichend`);
process.exit(abweichungen ? 1 : 0);
