// Prueft, dass BEFUNDE.md den Massnahmenkatalog LUECKENLOS fuehrt.
//
// Hintergrund: Die beiden Reviews sind Momentaufnahmen und werden nicht
// fortgeschrieben (DIR-006). Der Stand ihrer Massnahmen lebt in BEFUNDE.md.
// Ohne Pruefung kann dort still eine Zeile fehlen — und genau das ist die
// Fehlerklasse, die schon einmal zugeschlagen hat: "fuehre die Doku nach" ist
// eine Absichtserklaerung, keine Pruefung.
//
// Geprueft wird sechserlei:
//   1. Jede Massnahmennummer aus REVIEW.md hat eine Zeile in BEFUNDE.md
//   2. Dasselbe fuer REVIEW-2026-09-29.md
//   3. Die L-Nummern der laufenden Arbeit sind lueckenlos ab L1
//   4. Jeder Befund mit Status "offen", "zurueckgestellt" oder "teilweise" ist
//      im Abschnitt "ToDo" genannt
//   5. Umgekehrt: kein ToDo-Eintrag verweist ausschliesslich auf Erledigtes
//   6. Kein gestrichener Eintrag traegt im Titel einen Befund, der noch offen ist
// Pruefung 4 gibt es, weil die Tabellen den STAND fuehren, aber niemand aus
// ihnen ablesen kann, was als Naechstes zu tun ist. Eine Arbeitsliste, die nur
// von Hand nachgezogen wird, veraltet still -- dieselbe Fehlerklasse wie die
// Versionsnummern, die in README-CMAKE.md monatelang falsch standen.
// Pruefung 5 kam dazu, weil 4 nur die vergessene AUFNAHME faengt. Die vergessene
// STREICHUNG ist haeufiger: Beim Abschliessen pflegt man die Tabelle und nicht
// die Arbeitsliste. Sechs Eintraege hatten das ueberlebt (siehe dort).
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

  // LUECKENLOS IST NICHT EINDEUTIG -- und das ist am 05.10.2026 teuer geworden (L280).
  //
  // Zwei Agenten haben im selben Zeitraum dieselben L-Nummern vergeben. Die Stufe
  // meldete im selben Lauf "L1 bis L277 lueckenlos" und OK: Solange jede Nummer
  // MINDESTENS einmal vorkommt, faellt eine doppelt vergebene nicht auf. Die Ursache
  // sitzt eine Ebene tiefer als der fehlende Test -- catalogNums() liefert ein Set,
  // und darin ist eine Dublette per Konstruktion unsichtbar.
  //
  // Fuer die ToDo-Kennungen wurde die Eindeutigkeit ausdruecklich geprueft und hat im
  // selben Lauf prompt eine Dublette gemeldet. Dieselbe Datei, dieselbe Stufe, eine
  // Haelfte geprueft, die andere nicht. Gefunden hat es ein Agent von Hand mit
  // sort | uniq -d, weil er fragte, ob die L-Seite denn geprueft sei.
  const roh = [...cat.matchAll(/^\|\s*L(\d+)\s*\|/gm)].map((m) => m[1]);
  const doppelt = [...new Set(roh.filter((x, i) => roh.indexOf(x) !== i))];
  if (doppelt.length) {
    fail(`BEFUNDE.md: L-Nummer doppelt vergeben — ${doppelt.map((d) => "L" + d).join(", ")}`);
  } else {
    console.log(`  OK  L-Nummern eindeutig: ${roh.length} Zeilen, ${live.size} verschiedene`);
  }
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

  // ---- 5. Die Gegenrichtung: ToDo-Punkte, deren Befunde ALLE erledigt sind
  //
  // Pruefung 4 allein genuegt nicht. Sie faengt die vergessene Aufnahme, nicht die
  // vergessene Streichung -- und die ist haeufiger, weil beim Abschliessen die
  // Tabelle gepflegt wird und die Arbeitsliste nicht. Am 03.10.2026 gemessen:
  // SECHS Eintraege (A4, A8, B0, C1, E5, E6) verwiesen auf laengst erledigte
  // Befunde. Eine Arbeitsliste, in der ein Drittel der Punkte schon getan ist,
  // ist keine Arbeitsliste mehr -- man sucht sich die echten heraus oder laesst es.
  //
  // Gemeldet wird nur, wenn JEDE im Eintrag genannte Kennung erledigt ist. Ein
  // Eintrag darf einen erledigten Befund als Begruendung zitieren: A1 nennt L15
  // ("seit 3.2.8 laeuft der Watchdog ueberhaupt erst") und ist selbst offen, weil
  // es zusaetzlich L85 nennt. Die Regel haette die sechs echten Faelle alle
  // gefunden und A1 in Ruhe gelassen -- an genau diesem Bestand geprueft.
  const statusOf = new Map();
  for (const [heading, numPattern, label] of [
    ["Review 1", /^(\d+)$/, "Massnahme "],
    ["Review 2", /^(\d+)$/, "R2-"],
    ["Befunde aus der laufenden Arbeit", /^L(\d+)$/, "L"]
  ]) {
    const sec = cat.split(/^## /m).find((s) => s.startsWith(heading));
    if (!sec) continue;
    for (const line of sec.split("\n")) {
      if (!line.startsWith("|")) continue;
      const cols = line.split("|").map((c) => c.trim());
      if (cols.length < 4) continue;
      const id = (cols[1].match(numPattern) || [])[1];
      if (!id) continue;
      statusOf.set(label + id, /^\*\*erledigt/i.test(cols[3]));
    }
  }

  const stale = [];
  for (const line of todo.split("\n")) {
    // Das Muster hat ZWEIMAL nicht gereicht, und das ist der eigentliche Lehrsatz.
    //
    // 04.10.2026: [A-F]\d+ sah B1b, C6d und sechs weitere nie -- acht erledigte
    // Eintraege standen in der Arbeitsliste, waehrend die Stufe OK meldete.
    // Korrigiert zu [A-F]\d+[a-z]?.
    //
    // 05.10.2026, keinen Tag spaeter: Dasselbe nochmal, eine Stufe feiner. Nach dem
    // Buchstabensuffix kann eine ZIFFER stehen -- C9c0 bis C9c6. Sieben weitere
    // Eintraege unsichtbar, zwei davon wieder Altlast. Gefunden hat es ein Agent,
    // der die Stufe NICHT geglaubt und von Hand nachgezaehlt hat.
    //
    // Deshalb steht unten die Abdeckungspruefung: Ein Muster laesst sich immer noch
    // enger denken, als die Wirklichkeit ist. Die Stufe meldet jetzt selbst, wenn
    // sie weniger sieht, als dasteht -- statt darauf zu warten, dass das wieder
    // jemandem auffaellt.
    const entry = line.match(/^\| \*\*([A-F]\d+[a-z]?\d*)\*\* \|/);
    if (!entry) continue;
    const refs = [
      ...[...line.matchAll(/\bMassnahme (\d+)\b/g)].map((m) => "Massnahme " + m[1]),
      ...[...line.matchAll(/\bR2-(\d+)\b/g)].map((m) => "R2-" + m[1]),
      ...[...line.matchAll(/\bL(\d+)\b/g)].map((m) => "L" + m[1])
    ].filter((r) => statusOf.has(r));
    if (refs.length && refs.every((r) => statusOf.get(r))) {
      stale.push(`${entry[1]} (${[...new Set(refs)].join(", ")})`);
    }
  }
  // --- Abdeckung: sieht diese Stufe ueberhaupt alles? (DIR-014, L235, L276)
  //
  // Absichtlich WEITER als das Muster oben: jede Zeile, die wie ein Eintrag aussieht,
  // auch gestrichene. Weicht die Zahl ab, ist das Muster zu eng -- und die Stufe sagt
  // es, statt stillschweigend ein Teilergebnis zu melden.
  const sichtbar = todo.split("\n").filter((l) => /^\| ~?~?\*\*[A-F]\d+[a-z]?\d*\*\*/.test(l)).length;
  const vorhanden = todo.split("\n").filter((l) => /^\| ~?~?\*\*[A-Z]+[0-9][^*|]*\*\*/.test(l)).length;
  if (sichtbar < vorhanden) {
    fail(`BEFUNDE.md: diese Pruefung sieht nur ${sichtbar} von ${vorhanden} Eintraegen — das Kennungsmuster ist zu eng`);
  } else {
    console.log(`  OK  Abdeckung vollstaendig: ${sichtbar} Eintraege gesehen`);
  }

  if (stale.length) fail("BEFUNDE.md: ToDo-Eintrag erledigt, aber nicht gestrichen — " + stale.join("; "));
  else console.log("  OK  kein ToDo-Eintrag verweist ausschliesslich auf Erledigtes");

  // ---- 6. Die dritte Richtung: gestrichener Eintrag, dessen EIGENER Befund noch offen ist
  //
  // Am 10.10.2026 vom Nutzer gefragt ("Hast du die ToDo-Liste sauber nachgefuehrt?") und
  // von Hand nachgezaehlt: C43 war gestrichen, seine L338 stand weiter auf "offen";
  // C9c2 ("Heap (L175), Logring (L176)") war gestrichen, L176 stand seit ESP 3.2.18 auf
  // "offen", obwohl umgesetzt. Pruefung 4 sah nichts, weil die Kennung ja im (gestrichenen)
  // Eintrag steht; Pruefung 5 sah nichts, weil sie nur ungestrichene Eintraege liest.
  //
  // Massgeblich sind nur die Kennungen im FETT GESETZTEN TITEL des Eintrags -- das sind
  // die Befunde, die er traegt. Im Text darf ein gestrichener Eintrag auf offene Befunde
  // verweisen (A58 nennt L42, der zu Recht offen ist).
  const offenSt = new Map();
  {
    const sec = cat.split(/^## /m).find((s) => s.startsWith("Befunde aus der laufenden Arbeit"));
    for (const line of (sec || "").split("\n")) {
      const cols = line.split("|").map((c) => c.trim());
      const id = (cols[1] || "").match(/^L(\d+)$/);
      if (id && cols.length >= 4) offenSt.set("L" + id[1], /^\*\*(offen|zur(ü|ue)ckgestellt|teilweise)/i.test(cols[3]));
    }
  }
  // Nur der TITEL eines offenen Eintrags zaehlt als Weitertraeger, nicht sein Fliesstext:
  // Die erste Fassung nahm jede Erwaehnung und liess damit genau C43/L338 durch, weil ein
  // anderer Eintrag L338 beilaeufig zitiert (Gegenprobe gegen den Stand vor der Korrektur).
  const offeneEintraege = todo.split("\n").filter((l) => /^\| \*\*[A-F]\d/.test(l))
    .map((l) => (l.split("|")[2] || "").match(/^\s*\*\*(.*?)\*\*/)?.[1] || "");
  const verwaist = [];
  let gestrichen = 0;
  for (const line of todo.split("\n")) {
    const e = line.match(/^\| ~~\*\*([A-F]\d+[a-z]?\d*)\*\*~~ \| ([^|]*)/);
    if (!e) continue;
    gestrichen++;
    const titel = (e[2].match(/^\*\*(.*?)\*\*/) || [])[1] || "";
    // Traegt ein OFFENER Eintrag denselben Befund weiter (L150 lebt in A23, L199 in C19),
    // ist das kein Waisenkind, sondern eine Teilerledigung -- erst dann melden, wenn
    // keine offene Zeile der Liste ihn mehr nennt.
    const offen = [...titel.matchAll(/\bL(\d+)\b/g)].map((m) => "L" + m[1])
      .filter((r) => offenSt.get(r) && !offeneEintraege.some((t) => new RegExp("\\b" + r + "\\b").test(t)));
    if (offen.length) verwaist.push(`${e[1]} (${[...new Set(offen)].join(", ")})`);
  }
  if (verwaist.length) fail("BEFUNDE.md: ToDo gestrichen, eigener Befund steht noch auf offen — " + verwaist.join("; "));
  else console.log(`  OK  gestrichene Eintraege: ${gestrichen} geprueft, kein eigener Befund mehr offen`);
}


// ---------------------------------------------- doppelte ToDo-Kennungen (L219)
//
// Am 04.10.2026 hat der Lead an einem Tag VIER Kennungen doppelt vergeben --
// B19, B20, B21, B22 standen danach je zweimal in der Arbeitsliste, mit
// verschiedenen Inhalten. Gemerkt hat er es nur, weil eine Einfuegung per
// assert auf "genau einmal" fehlschlug; ohne diesen Zufall waeren sie stehen
// geblieben.
//
// Warum das mehr ist als Unordnung: Die Liste ist das Arbeitsmittel. Wer "B20
// erledigt" meldet, meint einen von zwei Eintraegen, und der andere gilt still
// als miterledigt. Dieselbe Mechanik wie bei den doppelten i18n-Schluesseln
// (L130) -- ein Duplikat faellt lautlos zusammen, und die Pruefung, die es
// finden muesste, sah bisher nur auf Vollstaendigkeit, nicht auf Eindeutigkeit.
const todoIds = [...cat.matchAll(/^\|\s*\*\*([A-Z]\d+[a-z]?)\*\*\s*\|/gm)].map((m) => m[1]);
const seenTodo = new Map();
let dupes = 0;
for (const id of todoIds) {
  seenTodo.set(id, (seenTodo.get(id) || 0) + 1);
}
for (const [id, n] of seenTodo) {
  if (n > 1) {
    console.log(`  HOCH      Kennung '${id}' steht ${n}x in der Arbeitsliste — "erledigt" traefe nur einen davon`);
    dupes++;
  }
}
if (dupes === 0) console.log(`  OK  ${seenTodo.size} ToDo-Kennungen, alle eindeutig`);
else bad += dupes;

process.exit(bad === 0 ? 0 : 1);
