// Findet Aufrufe von Funktionen, die nirgends definiert sind.
// Kommentare, String-Literale und Regex werden vorher ausgeblendet, sonst
// erzeugen i18n-Texte und CSS-Funktionen wie rgba() massenhaft Fehlalarme.
import { readFileSync } from "node:fs";

const file = process.argv[2];
const raw = readFileSync(file, "utf8");

// Ersetzt Inhalte von Kommentaren, Strings und Regex durch Leerzeichen,
// behaelt aber jede Zeilennummer bei.
function blank(src) {
  const out = src.split("");
  let i = 0, prev = "";
  const keep = (n) => { for (let k = i; k < n && k < out.length; k++) if (out[k] !== "\n") out[k] = " "; i = n; };
  while (i < src.length) {
    const c = src[i], d = src[i + 1];
    if (c === "/" && d === "/") { let n = src.indexOf("\n", i); if (n < 0) n = src.length; keep(n); continue; }
    if (c === "/" && d === "*") { let n = src.indexOf("*/", i + 2); n = n < 0 ? src.length : n + 2; keep(n); continue; }
    if (c === '"' || c === "'" || c === "`") {
      let n = i + 1;
      while (n < src.length) { if (src[n] === "\\") { n += 2; continue; } if (src[n] === c) { n++; break; } n++; }
      keep(n); continue;
    }
    if (c === "/" && /[(,=:[!&|?{};+\-*%~^\n]/.test(prev || "\n")) {
      let n = i + 1, inClass = false, ok = false;
      while (n < src.length) {
        if (src[n] === "\\") { n += 2; continue; }
        if (src[n] === "[") inClass = true;
        else if (src[n] === "]") inClass = false;
        else if (src[n] === "/" && !inClass) { n++; ok = true; break; }
        else if (src[n] === "\n") break;
        n++;
      }
      if (ok) { keep(n); continue; }
    }
    if (!/\s/.test(c)) prev = c;
    i++;
  }
  return out.join("");
}

const src = blank(raw);

const GLOBALS = new Set(`
alert atob btoa clearInterval clearTimeout confirm decodeURI decodeURIComponent
encodeURI encodeURIComponent escape unescape eval fetch isFinite isNaN parseFloat
parseInt prompt queueMicrotask requestAnimationFrame cancelAnimationFrame
setInterval setTimeout structuredClone importScripts
Array Boolean Date Error TypeError RangeError SyntaxError Function JSON Map Math
Number Object Promise Proxy Reflect RegExp Set String Symbol WeakMap WeakSet WeakRef
BigInt Intl URL URLSearchParams FinalizationRegistry
Blob File FileReader FormData Headers Request Response AbortController AbortSignal
TextEncoder TextDecoder Image Audio Worker Event CustomEvent MutationObserver
ResizeObserver IntersectionObserver DOMParser XMLHttpRequest WebSocket Notification
CompressionStream DecompressionStream ReadableStream WritableStream TransformStream
Element HTMLElement Node NodeList Option DocumentFragment
document window navigator location history localStorage sessionStorage console
caches clients crypto performance screen self globalThis top parent indexedDB
registration skipWaiting addEventListener removeEventListener dispatchEvent
matchMedia getComputedStyle scrollTo scrollBy open close print focus blur require
`.trim().split(/\s+/));

const KEYWORDS = new Set(`
if else for while do switch case catch try finally return typeof instanceof new
delete void in of function class extends yield await async let const var throw
break continue default with debugger
`.trim().split(/\s+/));

const defined = new Set();
const add = (n) => { if (n && /^[A-Za-z_$][\w$]*$/.test(n)) defined.add(n); };

for (const m of src.matchAll(/(?:async\s+)?function\s*\*?\s*([A-Za-z_$][\w$]*)/g)) add(m[1]);
for (const m of src.matchAll(/\b(?:const|let|var)\s+([A-Za-z_$][\w$]*)/g)) add(m[1]);
for (const m of src.matchAll(/\bclass\s+([A-Za-z_$][\w$]*)/g)) add(m[1]);
for (const m of src.matchAll(/\b(?:const|let|var)\s*[{[]([^}\]]*)[}\]]/g))
  for (const p of m[1].split(",")) add(p.split(":").pop().replace(/[^\w$]/g, ""));
for (const m of src.matchAll(/^\s*(?:async\s+)?([A-Za-z_$][\w$]*)\s*\([^)]*\)\s*\{/gm)) add(m[1]);
for (const m of src.matchAll(/\(([^()]*)\)\s*(?:=>|\{)/g))
  for (const p of m[1].split(",")) add(p.trim().replace(/^\.\.\./, "").split(/[=:]/)[0].trim());
for (const m of src.matchAll(/(?:^|[(,=;{}\s])([A-Za-z_$][\w$]*)\s*=>/g)) add(m[1]);
for (const m of src.matchAll(/catch\s*\(\s*([A-Za-z_$][\w$]*)/g)) add(m[1]);
for (const m of src.matchAll(/for\s*\(\s*(?:const|let|var)?\s*([A-Za-z_$][\w$]*)\s+(?:of|in)\b/g)) add(m[1]);

const called = new Map();
const lines = src.split("\n");
for (let i = 0; i < lines.length; i++)
  for (const m of lines[i].matchAll(/(^|[^.\w$])([A-Za-z_$][\w$]*)\s*\(/g)) {
    const n = m[2];
    if (KEYWORDS.has(n) || GLOBALS.has(n) || defined.has(n) || called.has(n)) continue;
    called.set(n, i + 1);
  }

if (called.size === 0) { console.log(`  OK  ${file}: keine undefinierten Funktionsaufrufe`); process.exit(0); }
for (const [n, l] of called) console.log(`  KRITISCH  ${file}:${l}  Aufruf von '${n}()' — nirgends definiert`);
process.exit(1);
