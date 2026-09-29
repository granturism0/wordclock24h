// Messskript fuer die Vorschau. Wird von server.py bei ?diag=1 in die Seite
// eingehaengt und schreibt seine Messwerte sichtbar ueber die Oberflaeche,
// damit sie auf einem Screenshot lesbar sind.
(function () {
  function measure() {
    const de = document.documentElement, L = [];
    const add = (k, v) => L.push(k + ": " + v);
    add("VIEWPORT", innerWidth + " x " + innerHeight);
    add("scrollWidth", de.scrollWidth);
    add("clientWidth", de.clientWidth);
    add("H-UEBERLAUF", (de.scrollWidth - de.clientWidth) + " px");
    add("colorScheme", getComputedStyle(de).colorScheme);

    const over = [];
    document.querySelectorAll("*").forEach((el) => {
      const r = el.getBoundingClientRect();
      if (r.width === 0 || r.height === 0) return;
      if (r.right > de.clientWidth + 1) {
        const c = (typeof el.className === "string" && el.className) ? "." + el.className.trim().split(/\s+/)[0] : "";
        over.push([Math.round(r.right - de.clientWidth), el.tagName.toLowerCase() + (el.id ? "#" + el.id : "") + c]);
      }
    });
    over.sort((a, b) => b[0] - a[0]);
    add("ELEMENTE UEBER RAND", over.length);
    over.slice(0, 6).forEach(([px, sel]) => add("   +" + px + "px", sel));

    let clipped = 0, samples = [];
    document.querySelectorAll("p,h1,h2,h3,span,label,strong").forEach((el) => {
      if (el.children.length) return;
      if (el.scrollWidth > el.clientWidth + 2 && el.clientWidth > 0) {
        clipped++;
        if (samples.length < 4) samples.push((el.textContent || "").trim().slice(0, 28) + " [" + el.clientWidth + "/" + el.scrollWidth + "]");
      }
    });
    add("ABGESCHNITTENER TEXT", clipped);
    samples.forEach((s) => add("   ", s));

    let small = 0, ssamp = [];
    document.querySelectorAll("button,a,select,input").forEach((el) => {
      const r = el.getBoundingClientRect();
      if (r.width === 0 || r.height === 0) return;
      if (r.height < 44) { small++; if (ssamp.length < 4) ssamp.push(el.tagName.toLowerCase() + (el.id ? "#" + el.id : "") + " " + Math.round(r.width) + "x" + Math.round(r.height)); }
    });
    add("TOUCH < 44px HOCH", small);
    ssamp.forEach((s) => add("   ", s));

    const b = document.querySelector("button");
    if (b) { b.focus(); const cs = getComputedStyle(b);
      add("FOKUS outline", cs.outlineStyle + " " + cs.outlineWidth); }
    const sel = document.querySelector("select");
    if (sel) { const cs = getComputedStyle(sel);
      add("SELECT bg", cs.backgroundColor); add("SELECT color", cs.color);
      add("SELECT hoehe", Math.round(sel.getBoundingClientRect().height) + "px"); }
    add("SW registriert", navigator.serviceWorker && navigator.serviceWorker.controller ? "ja" : "nein");
    add("isSecureContext", String(window.isSecureContext));

    const pre = document.createElement("pre");
    pre.textContent = L.join("\n");
    pre.style.cssText = "position:fixed;inset:0;z-index:2147483647;margin:0;padding:10px;" +
      "background:#fff;color:#000;font:11px/1.35 monospace;white-space:pre;overflow:hidden";
    document.body.appendChild(pre);
  }
  addEventListener("load", () => setTimeout(measure, 1800));
})();
