// Prueft innerHTML-Zuweisungen auf ungefilterten Fremdinhalt.
//
// Die fruehere zeilenbasierte Regel in guardrails.sh irrte in BEIDE Richtungen:
// sie meldete 21 korrekte .map()-Stellen, bei denen escapeHtml erst im Rumpf steht,
// und uebersah app.js:5667, weil dort escapeHtml fuer den FALLBACK auf derselben
// Zeile steht, waehrend der eigentliche Wert roh eingesetzt wird.
//
// Diese Fassung liest die ganze Zuweisung bis zum Anweisungsende, entfernt jeden
// escapeHtml(...)-Aufruf SAMT Argument, und meldet, was danach an Bezeichnern
// uebrig bleibt.
import { readFileSync } from "node:fs";

const file = process.argv[2];
const src = readFileSync(file, "utf8");

// Sichere Bezeichner: Konstanten, Hilfsfunktionen und Muster, die kein Fremdinhalt sind.
const SAFE = new Set(`
join map filter length slice toString String Number Boolean Math JSON Object Array
translate translateFormat escapeHtml
index idx i n key entry item file mode step label value selected disabled
`.trim().split(/\s+/));

// Einmal geprueft und als unbedenklich befunden: lokal aus Konstanten berechnete
// Werte, die kein Fremdinhalt sind. Wer hier etwas ergaenzt, muss die Stelle
// vorher gelesen haben — sonst ist die Pruefung wertlos.
const REVIEWED = new Set(["active", "checked", "state", "prefix", "badgeText", "stepNumber"]);

// Hilfsfunktionen, die selbst Markup erzeugen (buildWeekdayOptions, buildDayOptions,
// buildMonthOptions, buildNamedOptions, buildIconOptions). Ihr Rueckgabewert ist
// absichtlich HTML; sie escapen ihre eigenen Eingaben.
const MARKUP_BUILDER = /^build[A-Z]\w*$/;

// Entfernt escapeHtml( ... ) samt balanciertem Argument.
function stripEscaped(expr) {
  let out = "", i = 0;
  while (i < expr.length) {
    const at = expr.indexOf("escapeHtml(", i);
    if (at < 0) { out += expr.slice(i); break; }
    out += expr.slice(i, at);
    let depth = 0, j = at + "escapeHtml".length;
    for (; j < expr.length; j++) {
      if (expr[j] === "(") depth++;
      else if (expr[j] === ")") { depth--; if (depth === 0) { j++; break; } }
    }
    out += '""';           // durch ein harmloses Literal ersetzen
    i = j;
  }
  return out;
}

// Liest die Zuweisung ab einer Position bis zum Anweisungsende auf Klammertiefe 0.
function readAssignment(from) {
  let depth = 0, quote = null, i = from;
  for (; i < src.length; i++) {
    const c = src[i];
    if (quote) {
      if (c === "\\") { i++; continue; }
      if (c === quote) quote = null;
      continue;
    }
    if (c === '"' || c === "'" || c === "`") { quote = c; continue; }
    if ("([{".includes(c)) depth++;
    else if (")]}".includes(c)) { if (depth === 0) break; depth--; }
    else if (c === ";" && depth === 0) break;
  }
  return src.slice(from, i);
}

const findings = [];
for (const m of src.matchAll(/\.innerHTML\s*=\s*/g)) {
  const start = m.index + m[0].length;
  const expr = readAssignment(start);
  const line = src.slice(0, m.index).split("\n").length;

  let rest = stripEscaped(expr);
  // String-Literale entfernen — die sind statisches Markup, kein Fremdinhalt.
  rest = rest.replace(/`(?:[^`\\]|\\.)*`/g, '""')
             .replace(/"(?:[^"\\]|\\.)*"/g, '""')
             .replace(/'(?:[^'\\]|\\.)*'/g, '""');

  // Nur direkte Verkettung zaehlt: ein Wert, der unmittelbar neben einem
  // String-Literal mit + steht, wird roh ins Markup gesetzt. Eine Variable, die
  // bloss Iterationsquelle ist (meta.files.map(...)), wird NICHT eingesetzt.
  const raw = new Set();
  for (const x of rest.matchAll(/""\s*\+\s*([A-Za-z_$][\w$.]*)/g)) raw.add(x[1]);
  for (const x of rest.matchAll(/([A-Za-z_$][\w$.]*)\s*\+\s*""/g)) raw.add(x[1]);
  // Ein alleinstehender Wert ohne jede Verkettung ist ebenfalls eine Zuweisung.
  const bare = rest.trim().match(/^([A-Za-z_$][\w$.]*)\s*(\|\||$)/);
  if (bare) raw.add(bare[1]);

  // Zahlenartige Member sind kein Fremdtext-Risiko.
  const NUMERIC = /\.(idx|length|count|index|size|percent|value)$/;
  const ids = [...raw].filter((id) => {
    const head = id.split(".")[0];
    return !SAFE.has(head) && !SAFE.has(id) && !NUMERIC.test(id) &&
           !REVIEWED.has(head) && !MARKUP_BUILDER.test(head) &&
           !/^(?:if|else|return|const|let|var|function|new|typeof|true|false|null|undefined)$/.test(head);
  });

  if (ids.length) {
    findings.push({ line, ids: ids.slice(0, 4), snippet: expr.replace(/\s+/g, " ").slice(0, 70) });
  }
}

if (!findings.length) {
  console.log(`  OK  ${file}: jede innerHTML-Zuweisung ist gefiltert`);
  process.exit(0);
}
for (const f of findings) {
  console.log(`  HOCH      ${file}:${f.line}  ungefiltert: ${f.ids.join(", ")}`);
  console.log(`            ${f.snippet}`);
}
process.exit(0);   // meldet, blockiert nicht — Bewertung bleibt beim Review
