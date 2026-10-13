// pwa-developer: TDZ-Pruefstand. Laedt app.js in einer vm mit DOM-/fetch-Attrappen und
// startet die Sitzung einmal je Modul aus index.html (vorbelegtes localStorage).
// Aufruf: node harness.mjs <app.js> <index.html> [modul]
// Gibt je Lauf eine JSON-Zeile aus: {module, refErrors:[...], fatal, activeAfter}
import fs from "node:fs";
import vm from "node:vm";

const [appPath, htmlPath, onlyModule] = process.argv.slice(2);
const rawCode = fs.readFileSync(appPath, "utf8");
// Jede catch-Klausel meldet ihren Fehler zusaetzlich an __tdzSeen: Ein TDZ-Fehler in einem
// stillen catch (_) {} waere sonst unsichtbar. Die Zahl der instrumentierten Klauseln wird
// mit der Zahl im Quelltext verglichen (DIR-014: den Gegenstand vollstaendig sehen).
const catchPattern = /\bcatch \(([A-Za-z_$][\w$]*)\) \{/g;
const catchTotal = (rawCode.match(/(?<![.\w$])catch\s*[({]/g) || []).length;
const promiseCatchTotal = (rawCode.match(/\.catch\(/g) || []).length;
let catchInstrumented = 0;
const code = rawCode.replace(catchPattern, (m, name) => { catchInstrumented++; return m + " __tdzSeen(" + name + ");"; });
const html = fs.readFileSync(htmlPath, "utf8");

const ids = new Set([...html.matchAll(/\bid="([^"]+)"/g)].map((m) => m[1]));
const modules = [...new Set([...html.matchAll(/class="[^"]*\bmodule-section\b[^"]*"[^>]*data-module="([^"]+)"|data-module="([^"]+)"[^>]*class="[^"]*\bmodule-section\b/g)].map((m) => m[1] || m[2]))];
const chipTargets = [...new Set([...html.matchAll(/data-module-target="([^"]+)"/g)].map((m) => m[1]))];

function isRefError(x) {
  if (!x) return false;
  if (typeof x === "object" && x.name === "ReferenceError") return true;
  if (typeof x === "string" && /ReferenceError|before initialization|is not defined/.test(x)) return true;
  return false;
}
function describe(x) {
  return typeof x === "object" ? (x.name + ": " + x.message + " @ " + String(x.stack || "").split("\n").slice(1, 4).map((s) => s.trim()).join(" | ")) : String(x);
}

function makeClassList(initial) {
  const set = new Set(initial || []);
  return {
    add: (...c) => c.forEach((v) => set.add(v)),
    remove: (...c) => c.forEach((v) => set.delete(v)),
    toggle: (c, force) => { const on = force === undefined ? !set.has(c) : !!force; on ? set.add(c) : set.delete(c); return on; },
    contains: (c) => set.has(c),
    get length() { return set.size; },
    _set: set
  };
}

function makeElement(tag, attrs) {
  const attributes = new Map(Object.entries(attrs || {}));
  const dataset = {};
  for (const [k, v] of attributes) if (k.startsWith("data-")) dataset[k.slice(5).replace(/-([a-z])/g, (_, c) => c.toUpperCase())] = v;
  const base = {
    tagName: String(tag || "div").toUpperCase(), nodeType: 1, id: attributes.get("id") || "",
    dataset, style: { setProperty() {}, removeProperty() {} }, classList: makeClassList((attributes.get("class") || "").split(/\s+/).filter(Boolean)),
    children: [], childNodes: [], options: [], files: [], value: "", checked: false, disabled: false, hidden: false,
    textContent: "", innerHTML: "", innerText: "", title: "", href: "", src: "", type: "", name: "",
    parentNode: null, parentElement: null, firstChild: null, lastChild: null, nextSibling: null, offsetWidth: 0, offsetHeight: 0,
    scrollWidth: 0, clientWidth: 0, scrollLeft: 0, scrollTop: 0, scrollHeight: 0, clientHeight: 0, selectedIndex: -1,
    getAttribute: (n) => attributes.has(n) ? attributes.get(n) : null,
    setAttribute: (n, v) => attributes.set(n, String(v)), removeAttribute: (n) => attributes.delete(n), hasAttribute: (n) => attributes.has(n),
    toggleAttribute: (n, f) => { const on = f === undefined ? !attributes.has(n) : !!f; on ? attributes.set(n, "") : attributes.delete(n); return on; },
    addEventListener() {}, removeEventListener() {}, dispatchEvent() { return true; },
    appendChild: (c) => c, append() {}, prepend() {}, remove() {}, insertBefore: (c) => c, replaceChildren() {}, removeChild: (c) => c, replaceWith() {},
    insertAdjacentHTML() {}, insertAdjacentElement() {},
    querySelector: () => null, querySelectorAll: () => [], getElementsByTagName: () => [], closest: () => null, contains: () => false, matches: () => false,
    cloneNode: () => makeElement(tag), focus() {}, blur() {}, click() {}, select() {}, scrollIntoView() {}, scrollTo() {}, reset() {}, submit() {}, requestSubmit() {},
    showModal() {}, close() {}, setCustomValidity() {}, reportValidity: () => true, checkValidity: () => true,
    getBoundingClientRect: () => ({ top: 0, left: 0, right: 0, bottom: 0, width: 0, height: 0 }), getContext: () => null
  };
  return base;
}

const FULL_STORAGE = {
  "wordclock-language": "en",
  "wordclock-app-debug-overrides": JSON.stringify({ ambilight: "on", dfplayer: "on", color: "on", tft: "on" }),
  "wordclock-app-ambilight-online": "on",
  "wordclock-app-layout-preview": JSON.stringify({ rows: ["ESKISCHFUNF"], mode: 0 }),
  "wordclock-app-live-display-color": JSON.stringify({ mode: 0, red: 1, green: 2, blue: 3, white: 0 })
};

function runOnce(storedModule, full) {
  const refErrors = [];
  const otherWarnings = [];
  const elementsById = new Map();
  for (const id of ids) elementsById.set(id, makeElement("div", { id }));
  const sections = modules.map((m) => makeElement("section", { class: "module-section", "data-module": m }));
  const chips = chipTargets.map((m) => makeElement("button", { class: "module-chip", "data-module-target": m }));

  const storageFactory = (init) => {
    const map = new Map(Object.entries(init || {}));
    return {
      getItem: (k) => map.has(k) ? map.get(k) : null, setItem: (k, v) => map.set(k, String(v)), removeItem: (k) => map.delete(k),
      clear: () => map.clear(), key: (i) => [...map.keys()][i] ?? null, get length() { return map.size; }
    };
  };

  const timers = [];
  let timerSeq = 1;
  const record = (kind) => (...args) => {
    for (const a of args) if (isRefError(a)) refErrors.push(kind + ": " + describe(a));
    if (kind === "warn" || kind === "error") otherWarnings.push(args.map((a) => typeof a === "object" && a ? (a.name || "") + " " + (a.message || "") : String(a)).join(" "));
  };

  const document = {
    hidden: false, visibilityState: "visible", readyState: "complete", title: "", documentElement: makeElement("html"), body: makeElement("body"), head: makeElement("head"),
    activeElement: null, cookie: "",
    getElementById: (id) => elementsById.get(id) || null,
    querySelector(sel) {
      let m;
      if ((m = /^\.module-section\[data-module="([^"]*)"\]$/.exec(sel))) return sections.find((s) => s.dataset.module === m[1]) || null;
      if ((m = /^\.module-chip\[data-module-target="([^"]*)"\]$/.exec(sel))) return chips.find((s) => s.dataset.moduleTarget === m[1]) || null;
      if (sel === ".module-section.is-active") return sections.find((s) => s.classList.contains("is-active")) || null;
      if ((m = /^#([\w-]+)$/.exec(sel))) return elementsById.get(m[1]) || null;
      return null;
    },
    querySelectorAll(sel) {
      if (sel === ".module-section") return sections;
      if (sel === ".module-chip") return chips;
      return [];
    },
    createElement: (t) => makeElement(t), createTextNode: (t) => ({ textContent: t, nodeType: 3 }), createDocumentFragment: () => makeElement("fragment"),
    addEventListener() {}, removeEventListener() {}
  };
  document.documentElement.lang = "de";

  const sandbox = {
    console: { log: record("log"), info: record("info"), debug: record("debug"), warn: record("warn"), error: record("error") },
    document,
    localStorage: storageFactory({ ...(full ? FULL_STORAGE : {}), ...(storedModule === null ? {} : { "wordclock-app-active-module": storedModule }) }),
    sessionStorage: storageFactory(full ? { "wordclock-progress-scroll-restore": "1" } : {}),
    navigator: { userAgent: "node-harness", language: "de-CH", languages: ["de-CH"], onLine: true },
    location: { href: "http://uhr.invalid/app/", origin: "http://uhr.invalid", protocol: "http:", host: "uhr.invalid", hostname: "uhr.invalid", pathname: "/app/", search: "", hash: "", reload() {}, replace() {}, assign() {} },
    history: { replaceState() {}, pushState() {}, state: null },
    isSecureContext: true, innerWidth: 1280, innerHeight: 800, devicePixelRatio: 1, scrollX: 0, scrollY: 0, pageYOffset: 0,
    setTimeout: (fn, ms) => { const id = timerSeq++; timers.push({ id, fn, ms: Number(ms) || 0 }); return id; },
    clearTimeout: (id) => { const i = timers.findIndex((t) => t.id === id); if (i >= 0) timers.splice(i, 1); },
    setInterval: () => timerSeq++, clearInterval() {},
    requestAnimationFrame: () => timerSeq++, cancelAnimationFrame() {},
    matchMedia: () => ({ matches: false, addEventListener() {}, removeEventListener() {}, addListener() {}, removeListener() {} }),
    getComputedStyle: () => ({ getPropertyValue: () => "" }),
    addEventListener() {}, removeEventListener() {}, scrollTo() {}, alert() {}, confirm: () => false, prompt: () => null, open: () => null,
    fetch: () => new Promise(() => {}),
    URL, URLSearchParams, AbortController, AbortSignal, TextDecoder, TextEncoder, Blob, FormData, Headers, Response, Request, Event, EventTarget, performance, crypto, structuredClone, queueMicrotask,
    CustomEvent: class CustomEvent { constructor(t, o) { this.type = t; this.detail = o && o.detail; } },
    MutationObserver: class { observe() {} disconnect() {} },
    ResizeObserver: class { observe() {} unobserve() {} disconnect() {} },
    IntersectionObserver: class { observe() {} unobserve() {} disconnect() {} },
    Image: class { constructor() { this.onload = null; } },
    HTMLElement: class {}, Element: class {}, Node: class {}
  };
  sandbox.__tdzSeen = (e) => { if (isRefError(e)) refErrors.push("caught: " + describe(e)); };
  sandbox.window = sandbox;
  sandbox.self = sandbox;
  sandbox.globalThis = sandbox;
  vm.createContext(sandbox);

  // Promise-Ablehnungen, die ein .catch()/then(_, f) schluckt, ebenfalls sehen.
  vm.runInContext(`(() => { const t = Promise.prototype.then; Promise.prototype.then = function (ok, bad) {
    return t.call(this, ok, typeof bad === "function" ? function (e) { __tdzSeen(e); return bad.apply(this, arguments); } : bad); }; })();`, sandbox);
  const fetched = [];
  sandbox.fetch = (url) => { fetched.push(String(url)); return new Promise(() => {}); };
  let fatal = null;
  try {
    vm.runInContext(code, sandbox, { filename: appPath });
  } catch (error) {
    fatal = describe(error);
    if (isRefError(error)) refErrors.push("uncaught: " + fatal);
  }
  const active = sections.find((s) => s.classList.contains("is-active"));
  return { refErrors, fatal, activeAfter: active ? active.dataset.module : null, otherWarnings, fetched };
}

const unhandled = [];
process.on("unhandledRejection", (r) => { unhandled.push(r); });

const cases = onlyModule ? [onlyModule] : [null, ...modules, "gibt-es-nicht"];
const results = [];
for (const full of [false, true]) for (const m of cases) {
  const r = runOnce(m, full);
  // Mikrotasks des Starts ablaufen lassen (async-Funktionen bis zum ersten echten await).
  await new Promise((res) => setImmediate(res));
  await new Promise((res) => setImmediate(res));
  for (const u of unhandled.splice(0)) if (isRefError(u)) r.refErrors.push("unhandledRejection: " + describe(u));
  results.push({ module: m, full, ...r });
}
console.log(JSON.stringify({ modulesInHtml: modules, chipTargets, catchTotal, catchInstrumented, promiseCatchTotal, results }, null, 1));
