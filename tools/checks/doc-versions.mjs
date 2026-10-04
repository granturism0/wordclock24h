// Findet Versionskopien in der LEBENDEN Dokumentation, die von der Quelle abweichen.
//
// Hintergrund: In README-CMAKE.md stand ueber Monate "STM-Version: 3.2.0 / ESP 3.2.0 /
// PWA 1.2.43", waehrend die Quellen laengst bei 3.2.6 / 3.2.2 / 1.4.70 standen. Niemand
// hat es bemerkt, weil nichts es geprueft hat. Eine Anweisung an einen Doc-Agenten
// ("fuehre die Doku nach") ist eine Absichtserklaerung, keine Pruefung.
//
// Geprueft wird nur die Kombination "Komponentenwort direkt vor einer Versionsnummer".
// Damit schlagen weder WCAG-Kriterien (2.1, 1.4.11) noch Kontrastwerte (3:1) noch
// Toolchain-Versionen (9.3.1) an — die tragen kein Komponentenwort.
//
// Momentaufnahmen sind ausgenommen: REVIEW*.md, gap-analysis.md, CHANGELOG.md und
// specs/** halten bewusst den Stand ihres Entstehungszeitpunkts fest (DIR-006).

import { readFileSync } from "node:fs";

const SRC = {
  STM: ["src/main.h", /^#define\s+VERSION\s+"([^"]+)"/m],
  ESP: ["ESP8266/ESP-uclock/version.h", /^#define\s+ESP_VERSION\s+"([^"]+)"/m],
  PWA: ["ESP8266/ESP-uclock/data/app/app.js", /^const\s+APP_VERSION\s*=\s*"([^"]+)"/m],
  CACHE: ["ESP8266/ESP-uclock/data/app/sw.js", /^const\s+CACHE_NAME\s*=\s*"([^"]+)"/m],
};

const cur = {};
for (const [k, [file, re]] of Object.entries(SRC)) {
  const m = readFileSync(file, "latin1").match(re);
  if (!m) { console.log(`  KRITISCH  Versionszeile in ${file} nicht lesbar`); process.exit(1); }
  cur[k] = m[1];
}
const known = new Set([cur.STM, cur.ESP, cur.PWA]);

// "STM-Version: `3.2.0`", "STM 3.2.6", "PWA-Version 1.4.70", "App: 1.4.70"
const VER = /\b(STM(?:32)?|ESP(?:8266)?|PWA(?:-App)?|App)(?:-Version|version)?\s*[:\s]\s*`?(\d+\.\d+\.\d+)`?/gi;
const CACHE = /`?(wordclock-app-v\d+)`?/g;

let bad = 0;
for (const file of process.argv.slice(2)) {
  let text;
  try { text = readFileSync(file, "utf8"); } catch { continue; }
  text.split("\n").forEach((line, i) => {
    // Bewusste Ausnahme fuer historische Angaben in einem sonst lebenden Dokument.
    // Explizit statt heuristisch: eine aufgeweichte Regel haette den Realfall
    // "STM-Version: 3.2.0" ebenfalls durchgelassen.
    if (line.includes("<!-- historisch -->")) return;

    // Zweite Ausnahme, bewusst eng: "in welcher Version wurde es behoben" ist eine
    // Angabe ueber die VERGANGENHEIT und veraltet nicht. BEFUNDE.md fuehrt das in
    // jeder Statuszelle -- 13 Meldungen je Lauf, alle falsch. Eine Pruefung, die
    // regelmaessig Fehlalarme produziert, liest irgendwann niemand mehr, und das
    // ist schlimmer als gar keine.
    //
    // Eng bleibt es dadurch, dass der Erledigt-Marker VOR der Versionsnummer stehen
    // muss. Der Realfall, der diese Pruefung ausgeloest hat -- "Aktueller
    // Abschlussstand: STM 3.2.0" -- traegt keinen solchen Marker und schlaegt
    // weiterhin an.
    // Rueckwaertsgewandte Formulierungen, die dieses Projekt tatsaechlich benutzt:
    // eine Statuszelle ("**erledigt** 3.2.5", "**groesstenteils erledigt** PWA 1.4.72")
    // oder Fliesstext ("seit ESP 3.2.4", "der Fix in ESP 3.2.5", "behoben mit 3.2.8").
    const doneMarker = /\*\*[^*]{0,40}(?:erledigt|behoben|gekl(?:ä|ae)rt|belegt)[^*]{0,24}\*\*|\b(?:seit|Seit|mit|Mit|ab|Ab)\s+(?:STM|ESP|PWA)\b|\b[Ff]ix in\b|\bbehoben\s+(?:in|mit)\b|\bBehoben\s+(?:in|mit)\b/;
    const doneAt = line.search(doneMarker);

    // Eine Versionsnummer ALLEIN in Klammern ist in diesem Projekt durchgehend eine
    // Herkunftsangabe: "die Abwehr eingebetteter Wartungsaufrufe (ESP 3.2.4)". Der
    // Realfall "Aktueller Abschlussstand: STM 3.2.0" steht nicht in Klammern.
    const PROVENANCE = /\((?:STM|ESP|PWA|App)\s+\d+\.\d+\.\d+\)/;

    // Dritte Ausnahme: die DATIERTE Messbedingung, "(04.10.2026, ESP 3.2.18)".
    //
    // Sie sagt, auf welchem Stand gemessen wurde, und das ist bei einem noch
    // OFFENEN Befund die wesentliche Angabe -- gerade weil kein Erledigt-Marker
    // davorsteht, greift doneMarker dort nicht. Ohne diese Ausnahme muesste man
    // entweder die Version weglassen (dann fehlt die Messbedingung) oder jede
    // Zeile einzeln mit <!-- historisch --> versehen (dann gewoehnt man sich das
    // Wegklicken an, und die Marke verliert ihre Bedeutung).
    //
    // Eng bleibt es durch das vorangestellte Datum: Eine Messung an einem
    // bestimmten Tag ist per Konstruktion Vergangenheit und veraltet nicht. Der
    // Realfall, den diese Pruefung fangen soll -- "Aktueller Abschlussstand:
    // STM 3.2.0" -- traegt kein Datum und schlaegt weiterhin an.
    const MESSUNG = /\(\d{2}\.\d{2}\.\d{4},\s*(?:STM|ESP|PWA|App)\s+\d+\.\d+\.\d+/;

    for (const m of line.matchAll(VER)) {
      if (doneAt !== -1 && doneAt < m.index) continue;
      if (PROVENANCE.test(line.slice(Math.max(0, m.index - 1), m.index + m[0].length + 1))) continue;
      if (MESSUNG.test(line)) continue;
      if (!known.has(m[2])) {
        console.log(`  HOCH      ${file}:${i + 1}  "${m[1]} ${m[2]}" — Quellen stehen bei STM ${cur.STM} / ESP ${cur.ESP} / PWA ${cur.PWA}`);
        bad++;
      }
    }
    for (const m of line.matchAll(CACHE)) {
      if (m[1] !== cur.CACHE) {
        console.log(`  HOCH      ${file}:${i + 1}  "${m[1]}" — Quelle steht bei ${cur.CACHE}`);
        bad++;
      }
    }
  });
}

if (bad === 0) console.log(`  OK  keine veralteten Versionsangaben in der lebenden Doku (STM ${cur.STM} / ESP ${cur.ESP} / PWA ${cur.PWA} / ${cur.CACHE})`);
process.exit(bad === 0 ? 0 : 1);
