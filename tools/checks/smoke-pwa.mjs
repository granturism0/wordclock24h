// Smoke-Test PWA: laedt app.js mit gestubbtem Browser-Umfeld.
// Faengt Fehler ab, die beim Laden der Datei auftreten — nicht nur Syntax.
import { readFileSync } from "node:fs";
import vm from "node:vm";

const file = process.argv[2];
const src = readFileSync(file, "utf8");

const stub = () => new Proxy(function () {}, {
  get: (t, p) => {
    if (p === Symbol.toPrimitive) return () => "";
    if (p === Symbol.iterator) return function* () {};
    if (p === "then") return undefined;
    if (p === "length") return 0;
    if (p === "nodeName" || p === "tagName") return "DIV";
    return stub();
  },
  set: () => true,
  has: () => true,
  apply: () => stub(),
  construct: () => stub(),
});

const storage = { getItem: () => null, setItem: () => {}, removeItem: () => {}, clear: () => {} };
const sandbox = {
  console: { log() {}, warn() {}, error() {}, info() {}, debug() {} },
  document: stub(), window: stub(), navigator: stub(), location: stub(),
  localStorage: storage, sessionStorage: storage,
  fetch: () => Promise.resolve(stub()),
  setTimeout: () => 0, clearTimeout: () => {}, setInterval: () => 0, clearInterval: () => {},
  requestAnimationFrame: () => 0, matchMedia: () => stub(), caches: stub(),
  L: stub(), performance: { now: () => 0 }, screen: stub(), history: stub(),
};
for (const fn of ["addEventListener", "removeEventListener", "dispatchEvent",
                   "scrollTo", "scrollBy", "alert", "confirm", "prompt", "focus", "blur", "open", "close"]) {
  sandbox[fn] = () => {};
}
sandbox.matchMedia = () => ({ matches: false, addEventListener() {}, removeEventListener() {}, addListener() {}, removeListener() {} });
sandbox.getComputedStyle = () => new Proxy({}, { get: () => "" });
sandbox.globalThis = sandbox;
sandbox.self = sandbox;
sandbox.window = sandbox;

try {
  vm.createContext(sandbox);
  new vm.Script(src, { filename: file }).runInContext(sandbox, { timeout: 15000 });
  console.log(`  OK  ${file}: laedt ohne Fehler`);
  process.exit(0);
} catch (e) {
  console.log(`  KRITISCH  ${file}: wirft beim Laden — ${e.constructor.name}: ${e.message}`);
  if (e.stack) console.log(`            ${e.stack.split("\n")[1]?.trim() ?? ""}`);
  process.exit(1);
}
