// PNG lesen, um FARBEN AM BILDPUNKT zu bestimmen -- nicht aus dem Stylesheet zu rechnen.
//
// WARUM DAS DER GANZE PUNKT IST
//
// Am 04.10.2026 wurde derselbe Kontrastbefund VIERMAL falsch berechnet: erst gegen
// Schwarz statt gegen das tatsaechliche Grau, dann mit der falschen Trägerfarbe, dann
// gegen die falsche Bezugsflaeche -- und zuletzt stand "am Geraet gemessen" ueber einer
// Zahl, die wieder gerechnet war. Die angeblich gemessenen Farben kamen im Bild
// NULL mal vor (BEFUNDE.md, L245).
//
// Eine Flaeche in dieser Oberflaeche ist fast nie die Farbe, die im Stylesheet steht:
// halbtransparente Ueberlagerungen, ein Radialverlauf auf dem Koerper, der beim
// Scrollen mitwandert, und die Eigenfarben der Browser-Bedienelemente. Wer das
// nachrechnet, rechnet eine Welt nach, die es so nicht gibt.
//
// Deshalb: Bild aufnehmen, Bildpunkt auslesen, fertig. Node bringt zlib mit, mehr
// braucht es nicht -- keine Abhaengigkeit, die in zwei Jahren nicht mehr installierbar ist.
import { inflateSync } from "node:zlib";

export function pngLesen(buf) {
  if (buf.readUInt32BE(0) !== 0x89504e47) throw new Error("kein PNG");
  let p = 8, breite = 0, hoehe = 0, tiefe = 0, farbtyp = 0;
  const teile = [];
  while (p < buf.length) {
    const len = buf.readUInt32BE(p);
    const typ = buf.toString("ascii", p + 4, p + 8);
    const daten = buf.subarray(p + 8, p + 8 + len);
    if (typ === "IHDR") {
      breite = daten.readUInt32BE(0); hoehe = daten.readUInt32BE(4);
      tiefe = daten[8]; farbtyp = daten[9];
      if (tiefe !== 8 || (farbtyp !== 2 && farbtyp !== 6)) {
        throw new Error(`nicht unterstuetzt: Tiefe ${tiefe}, Farbtyp ${farbtyp}`);
      }
    } else if (typ === "IDAT") teile.push(daten);
    else if (typ === "IEND") break;
    p += 12 + len;
  }
  const kanaele = farbtyp === 6 ? 4 : 3;
  const roh = inflateSync(Buffer.concat(teile));
  const zeile = breite * kanaele;
  const bild = Buffer.alloc(hoehe * zeile);

  // Entfiltern nach RFC 2083. Jede Scanline traegt ihr Filterbyte voran; ohne diesen
  // Schritt bekaeme man Differenzen statt Farben -- und die saehen plausibel aus.
  for (let y = 0; y < hoehe; y++) {
    const filter = roh[y * (zeile + 1)];
    const ein = roh.subarray(y * (zeile + 1) + 1, y * (zeile + 1) + 1 + zeile);
    const aus = bild.subarray(y * zeile, (y + 1) * zeile);
    const oben = y > 0 ? bild.subarray((y - 1) * zeile, y * zeile) : null;
    for (let x = 0; x < zeile; x++) {
      const a = x >= kanaele ? aus[x - kanaele] : 0;
      const b = oben ? oben[x] : 0;
      const c = oben && x >= kanaele ? oben[x - kanaele] : 0;
      let v = ein[x];
      if (filter === 1) v += a;
      else if (filter === 2) v += b;
      else if (filter === 3) v += (a + b) >> 1;
      else if (filter === 4) {
        const pp = a + b - c, pa = Math.abs(pp - a), pb = Math.abs(pp - b), pc = Math.abs(pp - c);
        v += pa <= pb && pa <= pc ? a : pb <= pc ? b : c;
      }
      aus[x] = v & 0xff;
    }
  }
  return {
    breite, hoehe,
    punkt(x, y) {
      x = Math.round(x); y = Math.round(y);
      if (x < 0 || y < 0 || x >= breite || y >= hoehe) return null;
      const i = y * zeile + x * kanaele;
      return [bild[i], bild[i + 1], bild[i + 2]];
    },
    // Haeufigste Farbe in einem Bereich. Fuer Flaechen besser als ein Einzelpunkt:
    // Kantenglaettung und Verlaeufe erzeugen sonst Ausreisser, die niemand sieht.
    haeufigste(x, y, w, h) {
      const zaehler = new Map();
      for (let j = Math.round(y); j < Math.round(y + h); j++) {
        for (let i = Math.round(x); i < Math.round(x + w); i++) {
          const c = this.punkt(i, j);
          if (!c) continue;
          const k = c.join(",");
          zaehler.set(k, (zaehler.get(k) || 0) + 1);
        }
      }
      let best = null, max = 0;
      for (const [k, n] of zaehler) if (n > max) { max = n; best = k; }
      return best ? { farbe: best.split(",").map(Number), anteil: max } : null;
    },
  };
}

// WCAG 2.1 relative Leuchtdichte und Kontrastverhaeltnis.
const lin = (c) => { c /= 255; return c <= 0.04045 ? c / 12.92 : ((c + 0.055) / 1.055) ** 2.4; };
export const leuchtdichte = ([r, g, b]) => 0.2126 * lin(r) + 0.7152 * lin(g) + 0.0722 * lin(b);
export function kontrast(a, b) {
  const x = leuchtdichte(a), y = leuchtdichte(b);
  return (Math.max(x, y) + 0.05) / (Math.min(x, y) + 0.05);
}
