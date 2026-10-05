// Die Messungen selbst -- als Zeichenkette, die IM BROWSER ausgefuehrt wird.
//
// Ein Stueck, zwei Wirte: die Vorschau (tools/ui-mess/vorschau.mjs) und das Geraet
// (tools/ui-mess/geraet.mjs). Vorher wurde fuer jede Messung ein eigenes Skript
// geschrieben und danach weggeworfen -- sechsmal in zwei Tagen (E19). Zahlen aus
// Vorschau und Geraet waren deshalb nie vergleichbar (L229).
export const MESSUNG = `(() => {
  const sx = window.scrollX, sy = window.scrollY;
  const sichtbar = (e) => e.offsetParent !== null || getComputedStyle(e).position === "fixed";
  const box = (e) => { const r = e.getBoundingClientRect();
    return { x: Math.round((r.x + sx) * 100) / 100, y: Math.round((r.y + sy) * 100) / 100,
             w: Math.round(r.width * 100) / 100, h: Math.round(r.height * 100) / 100 }; };

  // --- Kacheln je Panel -------------------------------------------------------
  const panels = [...document.querySelectorAll(".panel")].filter(sichtbar).map((p) => ({
    name: (p.id || p.className || "").slice(0, 40),
    ...box(p),
  }));

  // --- Ankreuzfelder, mit ihrer Pille ------------------------------------------
  const felder = [...document.querySelectorAll('input[type=checkbox], input[type=radio]')]
    .filter(sichtbar).map((e, i) => {
      const l = e.closest("label");
      return { i, typ: e.type, angekreuzt: e.checked,
               name: (l ? l.textContent : "").trim().replace(/\\s+/g, " ").slice(0, 40),
               klasse: l ? l.className : "", feld: box(e), pille: l ? box(l) : null };
    });

  // --- Treffflaechen unter 44 px ------------------------------------------------
  // Die Zahl ist keine Meinung: 44x44 CSS-Pixel sind die Mindestgroesse aus den
  // Apple Human Interface Guidelines und decken sich mit WCAG 2.5.5 (AAA, 44 CSS-px).
  //
  // GEMESSEN WIRD DIE TREFFFLAECHE, NICHT DAS ELEMENT. Sitzt ein Bedienelement in
  // einem <label>, trifft der Finger das Label -- ein Klick darauf schaltet das Feld.
  // Die erste Fassung mass das <input> selbst und meldete in einem Modul 16 Felder
  // "unter 44px": Die Kaestchen sind 18x18, ihre Pille aber 88x44. Das waere ein
  // Fehlalarm gegen eine Massnahme gewesen, die laengst umgesetzt ist (B30), und er
  // haette ueberzeugend ausgesehen -- 16 Treffer sind kein Rauschen.
  const treff = (e) => {
    const l = e.closest("label");
    if (!l) return box(e);
    const b1 = box(e), b2 = box(l);
    // Nur wenn das Label das Element wirklich umschliesst; ein danebenstehendes
    // Label (for=...) ist keine gemeinsame Flaeche.
    const umschliesst = b2.x <= b1.x + 1 && b2.y <= b1.y + 1 &&
                        b2.x + b2.w >= b1.x + b1.w - 1 && b2.y + b2.h >= b1.y + b1.h - 1;
    return umschliesst ? b2 : b1;
  };
  const klein = [...document.querySelectorAll("button, a, select, input, [role=button]")]
    .filter(sichtbar).map((e) => ({ tag: e.tagName.toLowerCase(),
      name: (e.textContent || e.value || e.id || "").trim().slice(0, 30),
      ...treff(e), eigen: box(e) }))
    .filter((e) => (e.w < 44 || e.h < 44) && e.w > 0 && e.h > 0);

  return {
    viewport: { w: window.innerWidth, h: window.innerHeight },
    doc: { w: document.documentElement.scrollWidth, h: document.documentElement.scrollHeight },
    modul: document.querySelector("[data-module].is-active, .module.is-active")?.id
           || localStorage.getItem("wordclock-app-active-module") || "(unbekannt)",
    panels, felder, klein,
  };
})()`;

// Tastaturfokus: ECHT ueber Tabulator, nicht ueber .focus().
//
// Der Unterschied ist der ganze Punkt. :focus-visible verhaelt sich bei einem
// programmatischen .focus() anders als bei einer Tabulatortaste -- Chromium zeigt den
// Ring dort je nach Vorgeschichte nicht. Wer mit .focus() misst, misst eine Lage, die
// ein Nutzer nie herstellt, und bekommt ein Ergebnis, das aussieht wie eine Messung.
// Deshalb geht das ueber Input.dispatchKeyEvent des Wirts, nicht ueber diese Datei.
export const FOKUS_LESEN = `(() => {
  const e = document.activeElement;
  if (!e || e === document.body) return null;
  const cs = getComputedStyle(e);
  const r = e.getBoundingClientRect();
  return {
    tag: e.tagName.toLowerCase(),
    name: (e.textContent || e.value || e.id || "").trim().slice(0, 30),
    sichtbarerRing: cs.outlineStyle !== "none" && parseFloat(cs.outlineWidth) > 0,
    outline: cs.outlineStyle + " " + cs.outlineWidth + " " + cs.outlineColor,
    schatten: cs.boxShadow === "none" ? null : cs.boxShadow.slice(0, 60),
    box: { x: Math.round(r.x + window.scrollX), y: Math.round(r.y + window.scrollY),
           w: Math.round(r.width), h: Math.round(r.height) },
  };
})()`;
