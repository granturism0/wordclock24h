// Pruefstand c35 (Paket 2026-10-11, P.1, AKC.1).
//
//   node c35.mjs <app.js> [bezeichnung]
//
// Schneidet loadSecondaryData() samt allem, was sie aus app.js aufruft, aus der Datei aus
// (nicht abgeschrieben) und fuehrt es in einem vm-Kontext mit simulierter Uhr aus. Gezaehlt
// wird jeder Aufruf von settleFetchJson() mit der update_status-Adresse. Ein- und Ausgabe
// (Netz, DOM) sind durch Attrappen ersetzt; die Entscheidung, OB abgerufen wird, laeuft
// im Originalcode.
//
// Faelle:
//   A  Hauptmodul 30 min: Oeffnen bei t=0, dann 120 Selbstaktualisierungen im 15-s-Takt
//      => 1 bis 7 Abrufe, der erste beim Oeffnen
//   B  wie A, jeder Abruf scheitert => 1 bis 7 (Fehlschlag zaehlt als Abruf)
//   C  Updatemodul 30 min => mindestens 100 (wie bisher jeder Lauf)
//   D  Wartungsmodul 30 min => mindestens 100
//   E  Hauptmodul offen, nach 60 s Wechsel ins Updatemodul => dieser Lauf holt sofort
//   F  Hauptmodul, Rueckkehr aus dem Updatemodul nach 2 min => kein zusaetzlicher Abruf
//      vor Ablauf von 5 min seit dem letzten Abruf
// Exit 0 nur, wenn alle Faelle stimmen; 1 bei Abweichung; 2 bei Werkzeugfehler.
import { readFileSync } from "node:fs";
import vm from "node:vm";

const [appPath, bezeichnung = "PWA"] = process.argv.slice(2);
if (!appPath) {
  console.error("Aufruf: node c35.mjs <app.js> [bezeichnung]");
  process.exit(2);
}
const quelle = readFileSync(appPath, "utf8");

function ueberspringeText(text, i) {
  const c = text[i];
  for (i++; i < text.length && text[i] !== c; i++) if (text[i] === "\\") i++;
  return i;
}

// Ende eines Blocks/Ausdrucks ab startIndex: bis zur schliessenden Klammer der ersten
// oeffnenden, Zeichenketten und Kommentare uebersprungen.
function schneideBlock(startIndex, oeffner = "{") {
  let i = quelle.indexOf(oeffner, startIndex);
  let tiefe = 0;
  for (; i < quelle.length; i++) {
    const c = quelle[i];
    const n = quelle[i + 1];
    if (c === "/" && n === "/") { i = quelle.indexOf("\n", i); continue; }
    if (c === "/" && n === "*") { i = quelle.indexOf("*/", i) + 1; continue; }
    if (c === '"' || c === "'" || c === "`") { i = ueberspringeText(quelle, i); continue; }
    if (c === "{" || c === "(" || c === "[") tiefe++;
    if ((c === "}" || c === ")" || c === "]") && --tiefe === 0) return i + 1;
  }
  throw new Error("Klammer nicht geschlossen ab " + startIndex);
}

// Top-Level-Deklaration eines Namens: function, async function, let, const.
function deklaration(name) {
  const f = new RegExp("^(async )?function " + name + "\\(", "m").exec(quelle);
  if (f) return quelle.slice(f.index, schneideBlock(f.index));
  const v = new RegExp("^(let|const) " + name + " = ", "m").exec(quelle);
  if (!v) return null;
  const zeilenende = quelle.indexOf("\n", v.index);
  const zeile = quelle.slice(v.index, zeilenende);
  if (/;\s*(\/\/.*)?$/.test(zeile)) return zeile.replace(/^const /, "var ").replace(/^let /, "var ");
  const ende = quelle.indexOf(";", schneideBlock(v.index + v[0].length - 1, quelle[v.index + v[0].length]));
  return quelle.slice(v.index, ende + 1).replace(/^const /, "var ").replace(/^let /, "var ");
}

function bezeichner(text) {
  // Kommentare und Zeichenketten entfernen, dann Namen sammeln.
  let rein = "";
  for (let i = 0; i < text.length; i++) {
    const c = text[i], n = text[i + 1];
    if (c === "/" && n === "/") { const e = text.indexOf("\n", i); i = e < 0 ? text.length : e; rein += "\n"; continue; }
    if (c === "/" && n === "*") { i = text.indexOf("*/", i) + 1; continue; }
    if (c === '"' || c === "'" || c === "`") { i = ueberspringeText(text, i); rein += " 0 "; continue; }
    rein += c;
  }
  return new Set(rein.match(/[A-Za-z_$][\w$]*/g) || []);
}

// Ein- und Ausgabe: durch Attrappen ersetzt, NICHT ausgeschnitten.
const zaehler = { updateStatus: 0, scheitern: false, abrufZeiten: [] };
let simJetzt = 0;
const nichts = () => undefined;
const attrappen = {
  settleFetchJson: (url, fallback) => {
    if (url === "/api/update_status") {
      zaehler.updateStatus++;
      zaehler.abrufZeiten.push(simJetzt);
      if (zaehler.scheitern) return Promise.reject(new Error("simulierter Fehlschlag"));
    }
    return Promise.resolve(fallback);
  },
  settleFetchText: (url, fallback) => Promise.resolve(fallback),
  getUpdateStatusUrl: () => "/api/update_status",
  CONNECTION_STABILITY: new Proxy({}, { get: () => 1000 }),
  loadRequestSerial: 1,
  getActiveModuleName: () => "main",
  console: { warn: nichts, error: nichts, log: nichts },
  document: { getElementById: () => null, querySelector: () => null },
  Date: class extends Date { static now() { return simJetzt; } },
  Promise, Object, Array, Number, String, Boolean, JSON, Math, Symbol, Set, Map, Error
};
const ausgeschnitten = new Set(["loadSecondaryData", "getModuleDataProfile"]);
// Alles andere, was loadSecondaryData() aufruft und in app.js steht, wird ausgeschnitten,
// solange es Logik ist; reine Anzeige- und Ablagefunktionen werden zu Attrappen.
const anzeige = /^(set|refresh|render|update|note|begin|end|show|announce|get(Current|Normalized)|load|get.*Url$)/;

const teile = [];
const offen = ["loadSecondaryData"];
const gesehen = new Set();
while (offen.length) {
  const name = offen.shift();
  if (gesehen.has(name)) continue;
  gesehen.add(name);
  const text = deklaration(name);
  if (!text) {
    if (name === "loadSecondaryData") {
      console.error(`WERKZEUGFEHLER (${bezeichnung}): loadSecondaryData nicht gefunden`);
      process.exit(2);
    }
    continue;
  }
  teile.push(text);
  for (const b of bezeichner(text)) {
    if (gesehen.has(b) || b in attrappen) continue;
    if (!ausgeschnitten.has(b) && anzeige.test(b)) {
      if (deklaration(b)) {
        // Rueckgabe muss fuer "await" und Eigenschaftszugriffe taugen.
        attrappen[b] = () => (b.startsWith("load") ? Promise.resolve({}) : {});
      }
      gesehen.add(b);
      continue;
    }
    offen.push(b);
  }
}
if (!teile.some((t) => /getModuleDataProfile/.test(t.slice(0, 60)))) {
  console.error(`WERKZEUGFEHLER (${bezeichnung}): getModuleDataProfile nicht ausgeschnitten`);
  process.exit(2);
}

const ausgeschnittenNamen = teile.map((t) => (/^(?:async )?function (\w+)|^var (\w+)/.exec(t) || []).slice(1).find(Boolean));
console.log(`  ${bezeichnung}: ausgeschnitten ${ausgeschnittenNamen.join(", ")}`);

function neuerKontext() {
  const kontext = vm.createContext({ ...attrappen });
  vm.runInContext(teile.join("\n\n") + "\n;globalThis.__lsd = loadSecondaryData;", kontext, { filename: "c35-auszug.js" });
  return kontext;
}

async function lauf(kontext, moduleName, opts) {
  await kontext.__lsd(1, { numvars: {}, strvars: {}, tmvars: {} }, { displayPower: "on" }, {}, { moduleName, ...opts });
}

const TAKT = 15000;
const DAUER = 30 * 60 * 1000;
const fehler = [];

async function modulDreissigMinuten(moduleName, scheitern) {
  zaehler.updateStatus = 0; zaehler.abrufZeiten = []; zaehler.scheitern = scheitern;
  simJetzt = 1_700_000_000_000;
  const start = simJetzt;
  const k = neuerKontext();
  const opts = moduleName === "maintenance" ? { maintenancePriority: true } : {};
  await lauf(k, moduleName, { startup: true, ...opts });
  const nachOeffnen = zaehler.updateStatus;
  for (simJetzt = start + TAKT; simJetzt <= start + DAUER; simJetzt += TAKT) {
    await lauf(k, moduleName, { auto: true, ...opts });
  }
  return { gesamt: zaehler.updateStatus, nachOeffnen };
}

function pruefe(fall, ok, text) {
  console.log(`  ${ok ? "OK  " : "FEHL"} ${fall}: ${text}`);
  if (!ok) fehler.push(fall);
}

try {
  const a = await modulDreissigMinuten("main", false);
  pruefe("A Hauptmodul 30 min", a.nachOeffnen === 1 && a.gesamt >= 1 && a.gesamt <= 7,
    `${a.gesamt} Abrufe (Soll 1..7), beim Oeffnen ${a.nachOeffnen} (Soll 1)`);

  const b = await modulDreissigMinuten("main", true);
  pruefe("B Hauptmodul 30 min, Abrufe scheitern", b.nachOeffnen === 1 && b.gesamt <= 7,
    `${b.gesamt} Abrufe (Soll 1..7)`);

  const c = await modulDreissigMinuten("update", false);
  pruefe("C Updatemodul 30 min", c.gesamt >= 100, `${c.gesamt} Abrufe (Soll >= 100, wie bisher)`);

  const d = await modulDreissigMinuten("maintenance", false);
  pruefe("D Wartungsmodul 30 min", d.gesamt >= 100, `${d.gesamt} Abrufe (Soll >= 100, wie bisher)`);

  // E: Wechsel ins Updatemodul holt sofort, auch innerhalb der 5 min.
  zaehler.updateStatus = 0; zaehler.scheitern = false;
  simJetzt = 1_700_000_000_000;
  const start = simJetzt;
  let k = neuerKontext();
  await lauf(k, "main", { startup: true });
  simJetzt = start + 60000;
  const vorWechsel = zaehler.updateStatus;
  await lauf(k, "update", { moduleChange: true });
  pruefe("E Wechsel ins Updatemodul nach 60 s", zaehler.updateStatus === vorWechsel + 1,
    `${zaehler.updateStatus - vorWechsel} Abruf beim Wechsel (Soll 1)`);

  // F: Rueckkehr ins Hauptmodul 2 min spaeter, 15-s-Laeufe bis 4 min nach dem letzten Abruf.
  const vorRueckkehr = zaehler.updateStatus;
  const letzterAbruf = simJetzt;
  for (simJetzt = start + 120000; simJetzt < letzterAbruf + 5 * 60 * 1000 - TAKT; simJetzt += TAKT) {
    await lauf(k, "main", simJetzt === start + 120000 ? { moduleChange: true } : { auto: true });
  }
  pruefe("F Rueckkehr ins Hauptmodul innerhalb 5 min", zaehler.updateStatus === vorRueckkehr,
    `${zaehler.updateStatus - vorRueckkehr} Abrufe (Soll 0)`);
} catch (error) {
  console.error(`WERKZEUGFEHLER (${bezeichnung}): ${error && error.stack ? error.stack : error}`);
  process.exit(2);
}

if (fehler.length) {
  console.log(`FEHL ${bezeichnung}: ${fehler.length} Fall/Faelle abweichend (${fehler.join("; ")})`);
  process.exit(1);
}
console.log(`OK ${bezeichnung}: update_status-Abrufe je Modul wie in AKC.1 verlangt`);
process.exit(0);
