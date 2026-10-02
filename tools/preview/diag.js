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

    // Wie viel von der Breite die Oberflaeche tatsaechlich belegt. Auf einem
    // 3440er Monitor lag das lange bei 1540 px -- der Rest blieb leer.
    const shell = document.querySelector(".shell");
    if (shell) {
      const w = Math.round(shell.getBoundingClientRect().width);
      add("GENUTZTE BREITE", w + " px (" + Math.round((w / innerWidth) * 100) + " % der Breite)");
    }

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

    // ---- Nicht-Text-Kontrast (WCAG 1.4.11). Gemessen wird der Rand eines echten
    // Eingabefelds gegen seine eigene Flaeche -- nicht die Token aus dem Stylesheet,
    // denn die sind halbtransparent und sagen allein nichts ueber das Ergebnis.
    const parse = (s) => {
      const m = String(s).match(/-?[\d.]+/g) || [];
      return [Number(m[0]) || 0, Number(m[1]) || 0, Number(m[2]) || 0, m[3] === undefined ? 1 : Number(m[3])];
    };
    const comp = (fg, bg) => fg.slice(0, 3).map((c, i) => c * fg[3] + bg[i] * (1 - fg[3]));
    const lin = (c) => { c /= 255; return c <= 0.04045 ? c / 12.92 : Math.pow((c + 0.055) / 1.055, 2.4); };
    const lum = (c) => 0.2126 * lin(c[0]) + 0.7152 * lin(c[1]) + 0.0722 * lin(c[2]);
    const ratio = (a, b) => { const x = lum(a), y = lum(b); return (Math.max(x, y) + 0.05) / (Math.min(x, y) + 0.05); };

    const probe = document.querySelector(".field input, .field select");
    if (probe) {
      // Die Flaeche hinter dem Feld: erster Vorfahr mit deckender Farbe.
      let back = [5, 9, 19, 1], node = probe.parentElement;
      while (node) {
        const c = parse(getComputedStyle(node).backgroundColor);
        if (c[3] >= 0.8) { back = c; break; }
        node = node.parentElement;
      }
      const face = comp(parse(getComputedStyle(probe).backgroundColor), back);
      const edge = comp(parse(getComputedStyle(probe).borderTopColor), face);
      add("RAND/FLAECHE Feld", ratio(edge, face).toFixed(2) + ":1 (gefordert 3.00)");
      add("RAND/UMGEBUNG Feld", ratio(edge, back.slice(0, 3)).toFixed(2) + ":1");
    }

    // ---- Modal: Dialogrolle, Fokus hinein, Escape, Fokus zurueck (Massnahme 14)
    const modal = document.getElementById("weather-map-modal");
    if (modal && typeof openWeatherMapPicker === "function") {
      const opener = document.querySelector("button");
      if (opener) { opener.focus(); }
      const before = document.activeElement;
      openWeatherMapPicker();
      const card = modal.querySelector(".modal-card");
      add("MODAL role", (card && card.getAttribute("role")) + "/" + (card && card.getAttribute("aria-modal")));
      add("MODAL Fokus drin", modal.contains(document.activeElement) ? "ja" : "NEIN");
      add("MODAL Seite gesperrt", document.body.classList.contains("has-modal") ? "ja" : "NEIN");
      const shellEl = document.querySelector(".shell");
      add("MODAL Hintergrund inert", shellEl && shellEl.inert ? "ja" : "nein (Browser ohne inert)");
      if (card) {
        const mh = parseFloat(getComputedStyle(card).maxHeight) || 0;
        // dvh folgt der sichtbaren Hoehe; 92vh laege bei ausgeklappter Leiste darueber.
        add("MODAL maxHeight", Math.round(mh) + " px bei " + innerHeight + " px Viewport");
      }
      document.dispatchEvent(new KeyboardEvent("keydown", { key: "Escape", bubbles: true }));
      add("MODAL Escape schliesst", modal.classList.contains("is-hidden") ? "ja" : "NEIN");
      add("MODAL Fokus zurueck", document.activeElement === before ? "ja" : "NEIN");
    }

    // Zusaetzlich an den Server melden. Der legt daraus diag-<breite>_<hoehe>.json an,
    // und erst damit laesst sich eine Reihe ueber zwanzig Formate auswerten, ohne
    // zwanzig Screenshots einzeln anzusehen.
    try {
      const payload = JSON.stringify({ viewport: innerWidth + "x" + innerHeight, lines: L });
      const url = "/diag-result?vp=" + innerWidth + "x" + innerHeight;
      if (navigator.sendBeacon) {
        navigator.sendBeacon(url, new Blob([payload], { type: "application/json" }));
      } else {
        fetch(url, { method: "POST", body: payload, keepalive: true });
      }
    } catch (_) { }

    const pre = document.createElement("pre");
    pre.textContent = L.join("\n");
    pre.style.cssText = "position:fixed;inset:0;z-index:2147483647;margin:0;padding:10px;" +
      "background:#fff;color:#000;font:11px/1.35 monospace;white-space:pre;overflow:hidden";
    document.body.appendChild(pre);
  }
  addEventListener("load", () => setTimeout(measure, 1800));
})();
