// Aus tdz/harness.mjs uebernommen (DOM-Attrappen), unveraendert.
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

export { makeElement };
