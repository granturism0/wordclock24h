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
// Faelle (gezaehlt je update_status und update_table_files):
//   A  Hauptmodul 30 min: Oeffnen bei t=0, dann 120 Laeufe im 15-s-Takt => je 1..7, der
//      erste beim Oeffnen
//   B1 wie A, jeder Abruf wirft => je 1..7 (Fehlschlag zaehlt als Abruf)
//   B2 wie A, Abruf liefert wie das echte settleFetchJson den Rueckfallwert => je 1..7
//   C  Updatemodul 30 min => je >= 100;  D  Wartungsmodul 30 min => je >= 100
//   G  Anzeigemodul 30 min => update_table_files >= 100
//   E  Hauptmodul offen, nach 60 s Wechsel in Wartung bzw. Update => dieser Lauf holt sofort
//   F  danach Rueckkehr ins Hauptmodul => kein Abruf vor Ablauf von 5 min
//   H  der Oeffnungslauf wird waehrend des Abrufs ueberholt (loadRequestSerial steigt) =>
//      das Ergebnis ist trotzdem uebernommen, und es wird nicht vor 5 min nachgeholt (N1)
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
const URL_STATUS = "/api/update_status";
const URL_TABELLE = "/api/update_table_files";
// modus: "ok" liefert einen Wert, "wirft" verwirft das Promise, "rueckfall" liefert wie das
// echte settleFetchJson bei Fehler/Timeout den Rueckfallwert, ohne zu werfen.
// ueberholen: waehrend des Abrufs steigt loadRequestSerial (ein neuerer loadData() laeuft).
const zaehler = { status: 0, tabelle: 0, modus: "ok", ueberholen: false, gesetzt: { status: null, tabelle: null } };
let simJetzt = 0;
let aktuellerKontext = null;
const nichts = () => undefined;
const attrappen = {
  settleFetchJson: (url, fallback) => {
    const art = url === URL_STATUS ? "status" : url === URL_TABELLE ? "tabelle" : null;
    if (!art) return Promise.resolve(fallback);
    zaehler[art]++;
    if (zaehler.ueberholen && aktuellerKontext) aktuellerKontext.loadRequestSerial += 1;
    if (zaehler.modus === "wirft") return Promise.reject(new Error("simulierter Fehlschlag"));
    if (zaehler.modus === "rueckfall") return Promise.resolve(fallback);
    return Promise.resolve({ marke: art + "-" + zaehler[art] });
  },
  settleFetchText: (url, fallback) => Promise.resolve(fallback),
  getUpdateStatusUrl: () => URL_STATUS,
  getUpdateTableFilesUrl: () => URL_TABELLE,
  getNormalizedUpdateStatus: (v) => v || {},
  getNormalizedUpdateTableInfo: (v) => v || {},
  getCurrentUpdateTableInfo: () => zaehler.gesetzt.tabelle || {},
  setCurrentUpdateStatus: (v) => { zaehler.gesetzt.status = v; return v; },
  setCurrentUpdateTableInfo: (v) => { zaehler.gesetzt.tabelle = v; return v; },
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
    // Nur Funktionen werden zu Attrappen; Zustandsvariablen mit gleichem Praefix
    // (etwa updateStatusFetchSerial) werden ausgeschnitten.
    if (!ausgeschnitten.has(b) && anzeige.test(b) && !/^(let|const|var) /.test(deklaration(b) || "")) {
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
  aktuellerKontext = kontext;
  return kontext;
}

async function lauf(kontext, moduleName, opts) {
  await kontext.__lsd(kontext.loadRequestSerial, { numvars: {}, strvars: {}, tmvars: {} }, { displayPower: "on" }, {}, { moduleName, ...opts });
}

const TAKT = 15000;
const DAUER = 30 * 60 * 1000;
const T0 = 1_700_000_000_000;
const fehler = [];

function zuruecksetzen(modus = "ok") {
  zaehler.status = 0; zaehler.tabelle = 0; zaehler.modus = modus; zaehler.ueberholen = false;
  zaehler.gesetzt = { status: null, tabelle: null };
  simJetzt = T0;
}

async function modulDreissigMinuten(moduleName, modus = "ok") {
  zuruecksetzen(modus);
  const k = neuerKontext();
  const opts = moduleName === "maintenance" ? { maintenancePriority: true } : {};
  await lauf(k, moduleName, { startup: true, ...opts });
  const nachOeffnen = { status: zaehler.status, tabelle: zaehler.tabelle };
  for (simJetzt = T0 + TAKT; simJetzt <= T0 + DAUER; simJetzt += TAKT) {
    await lauf(k, moduleName, { auto: true, ...opts });
  }
  return { status: zaehler.status, tabelle: zaehler.tabelle, nachOeffnen };
}

function pruefe(fall, ok, text) {
  console.log(`  ${ok ? "OK  " : "FEHL"} ${fall}: ${text}`);
  if (!ok) fehler.push(fall);
}

const imRahmen = (n) => n >= 1 && n <= 7;

try {
  const a = await modulDreissigMinuten("main");
  pruefe("A Hauptmodul 30 min, update_status", a.nachOeffnen.status === 1 && imRahmen(a.status),
    `${a.status} Abrufe (Soll 1..7), beim Oeffnen ${a.nachOeffnen.status} (Soll 1)`);
  pruefe("A Hauptmodul 30 min, update_table_files", a.nachOeffnen.tabelle === 1 && imRahmen(a.tabelle),
    `${a.tabelle} Abrufe (Soll 1..7), beim Oeffnen ${a.nachOeffnen.tabelle} (Soll 1)`);

  const b1 = await modulDreissigMinuten("main", "wirft");
  pruefe("B1 Hauptmodul 30 min, Abrufe werfen", b1.nachOeffnen.status === 1 && imRahmen(b1.status) && imRahmen(b1.tabelle),
    `update_status ${b1.status}, update_table_files ${b1.tabelle} (Soll je 1..7)`);

  const b2 = await modulDreissigMinuten("main", "rueckfall");
  pruefe("B2 Hauptmodul 30 min, Rueckfallwert wie settleFetchJson", b2.nachOeffnen.status === 1 && imRahmen(b2.status) && imRahmen(b2.tabelle),
    `update_status ${b2.status}, update_table_files ${b2.tabelle} (Soll je 1..7)`);

  const c = await modulDreissigMinuten("update");
  pruefe("C Updatemodul 30 min", c.status >= 100 && c.tabelle >= 100,
    `update_status ${c.status}, update_table_files ${c.tabelle} (Soll je >= 100, wie bisher)`);

  const d = await modulDreissigMinuten("maintenance");
  pruefe("D Wartungsmodul 30 min", d.status >= 100 && d.tabelle >= 100,
    `update_status ${d.status}, update_table_files ${d.tabelle} (Soll je >= 100, wie bisher)`);

  const g = await modulDreissigMinuten("display");
  pruefe("G Anzeigemodul 30 min, update_table_files", g.tabelle >= 100,
    `${g.tabelle} Abrufe (Soll >= 100, wie bisher)`);

  // E: Wechsel ins Wartungs- bzw. Updatemodul holt sofort, auch innerhalb der 5 min.
  for (const ziel of ["maintenance", "update"]) {
    zuruecksetzen();
    const k = neuerKontext();
    await lauf(k, "main", { startup: true });
    simJetzt = T0 + 60000;
    const vor = { status: zaehler.status, tabelle: zaehler.tabelle };
    await lauf(k, ziel, { moduleChange: true, ...(ziel === "maintenance" ? { maintenancePriority: true } : {}) });
    pruefe(`E Wechsel nach ${ziel} nach 60 s`, zaehler.status === vor.status + 1 && zaehler.tabelle === vor.tabelle + 1,
      `update_status +${zaehler.status - vor.status}, update_table_files +${zaehler.tabelle - vor.tabelle} (Soll je +1)`);

    // F: Rueckkehr ins Hauptmodul 2 min spaeter, Laeufe bis kurz vor 5 min nach dem letzten Abruf.
    const vorRueckkehr = { status: zaehler.status, tabelle: zaehler.tabelle };
    const letzterAbruf = simJetzt;
    for (simJetzt = T0 + 120000; simJetzt < letzterAbruf + 5 * 60 * 1000 - TAKT; simJetzt += TAKT) {
      await lauf(k, "main", simJetzt === T0 + 120000 ? { moduleChange: true } : { auto: true });
    }
    pruefe(`F Rueckkehr aus ${ziel} ins Hauptmodul innerhalb 5 min`,
      zaehler.status === vorRueckkehr.status && zaehler.tabelle === vorRueckkehr.tabelle,
      `update_status +${zaehler.status - vorRueckkehr.status}, update_table_files +${zaehler.tabelle - vorRueckkehr.tabelle} (Soll je 0)`);
  }

  // H (N1): Der Oeffnungslauf wird waehrend des Abrufs ueberholt. Das Ergebnis muss trotzdem
  // uebernommen sein, und die Laeufe der naechsten 4 min holen nicht nach.
  zuruecksetzen();
  {
    const k = neuerKontext();
    zaehler.ueberholen = true;
    await lauf(k, "main", { startup: true });
    zaehler.ueberholen = false;
    const nachOeffnen = { status: zaehler.status, tabelle: zaehler.tabelle };
    for (simJetzt = T0 + TAKT; simJetzt < T0 + 4 * 60 * 1000; simJetzt += TAKT) {
      await lauf(k, "main", { auto: true });
    }
    const st = zaehler.gesetzt.status && zaehler.gesetzt.status.marke;
    pruefe("H ueberholter Oeffnungslauf, update_status uebernommen", st === "status-1" && zaehler.status === nachOeffnen.status,
      `uebernommen: ${st || "nichts"} (Soll status-1), Abrufe danach +${zaehler.status - nachOeffnen.status} (Soll 0)`);
    // Die Tabelleninfo erreicht ein ueberholter Lauf nur, wenn er nicht schon nach
    // update_status abbricht; deshalb eigener Lauf, der erst bei der Tabelle ueberholt wird.
  }
  zuruecksetzen();
  {
    const k = neuerKontext();
    const echt = k.settleFetchJson;
    k.settleFetchJson = (url, fallback) => {
      if (url === URL_TABELLE) k.loadRequestSerial += 1;
      return echt(url, fallback);
    };
    await lauf(k, "main", { startup: true });
    const nachOeffnen = zaehler.tabelle;
    k.settleFetchJson = echt;
    for (simJetzt = T0 + TAKT; simJetzt < T0 + 4 * 60 * 1000; simJetzt += TAKT) {
      await lauf(k, "main", { auto: true });
    }
    const tb = zaehler.gesetzt.tabelle && zaehler.gesetzt.tabelle.marke;
    pruefe("H ueberholter Oeffnungslauf, update_table_files uebernommen", tb === "tabelle-1" && zaehler.tabelle === nachOeffnen,
      `uebernommen: ${tb || "nichts"} (Soll tabelle-1), Abrufe danach +${zaehler.tabelle - nachOeffnen} (Soll 0)`);
  }
} catch (error) {
  console.error(`WERKZEUGFEHLER (${bezeichnung}): ${error && error.stack ? error.stack : error}`);
  process.exit(2);
}

if (fehler.length) {
  console.log(`FEHL ${bezeichnung}: ${fehler.length} Fall/Faelle abweichend (${fehler.join("; ")})`);
  process.exit(1);
}
console.log(`OK ${bezeichnung}: update_status- und update_table_files-Abrufe je Modul wie verlangt`);
process.exit(0);
