// Selbsttest fuer png.mjs: PNG mit bekannten Farben bauen, zurueckleben, vergleichen.
//
//     node tools/ui-mess/png.probe.mjs
//
// WARUM ER IM REPO LIEGT UND NICHT IM SCRATCHPAD
//
// Das Messwerkzeug fuer die Oberflaeche ist am 04./05.10.2026 SECHSMAL von Hand
// nachgebaut worden -- mess.mjs, mess2.mjs, mess3.mjs, ui.mjs bis ui6.mjs -- und jedes
// Mal weggeworfen (BEFUNDE.md, E19). Beim siebten Mal faengt jemand wieder bei der
// PNG-Entfilterung an und macht dabei wieder einen Fehler, den niemand bemerkt.
//
// WAS DER TEST SELBST GELEHRT HAT (DIR-014)
//
// Seine erste Fassung lief mit absichtlich verdorbenem Paeth-Filter durch und meldete
// fuenfmal OK. Zwei Gruende, beide lehrreich:
//
//   1. Der erste Versuch, den Fehler einzubauen, nutzte sed -- und das Muster traf
//      wegen der && im Ausdruck nie. Die "Gegenprobe" lief gegen unveraenderten Code.
//      Seitdem wird jede solche Verderbnis mit einer ZAEHLUNG belegt, nicht mit dem
//      Gefuehl, der Befehl habe schon gewirkt.
//   2. Das Testmuster war ein linearer Verlauf. Damit liegen die Nachbarn a, b, c fast
//      immer so, dass die volle Paeth-Bedingung und eine verkuerzte Fassung dasselbe
//      waehlen. Ein Muster, das den Unterschied nicht herstellt, prueft nichts.
//
// Mit Pseudozufall meldet er jetzt 192 falsche Punkte, sobald der Paeth-Zweig faelsch
// ist -- und 0, wenn er stimmt.
import { deflateSync } from "node:zlib";
import { pngLesen, kontrast } from "./png.mjs";

function baue(breite, hoehe, farbeVon, filter) {
  const zeile = breite * 3;
  const roh = Buffer.alloc(hoehe * (zeile + 1));
  const bild = Buffer.alloc(hoehe * zeile);
  for (let y = 0; y < hoehe; y++)
    for (let x = 0; x < breite; x++) {
      const c = farbeVon(x, y);
      bild[y * zeile + x * 3] = c[0]; bild[y * zeile + x * 3 + 1] = c[1]; bild[y * zeile + x * 3 + 2] = c[2];
    }
  for (let y = 0; y < hoehe; y++) {
    roh[y * (zeile + 1)] = filter;
    for (let x = 0; x < zeile; x++) {
      const a = x >= 3 ? bild[y * zeile + x - 3] : 0;
      const b = y > 0 ? bild[(y - 1) * zeile + x] : 0;
      const c = y > 0 && x >= 3 ? bild[(y - 1) * zeile + x - 3] : 0;
      const v = bild[y * zeile + x];
      let f = v;
      if (filter === 1) f = v - a;
      else if (filter === 2) f = v - b;
      else if (filter === 3) f = v - ((a + b) >> 1);
      else if (filter === 4) {
        const pp = a + b - c, pa = Math.abs(pp - a), pb = Math.abs(pp - b), pc = Math.abs(pp - c);
        f = v - (pa <= pb && pa <= pc ? a : pb <= pc ? b : c);
      }
      roh[y * (zeile + 1) + 1 + x] = f & 0xff;
    }
  }
  const idat = deflateSync(roh);
  const teile = [];
  const crcT = []; for (let n = 0; n < 256; n++) { let c = n; for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1; crcT[n] = c >>> 0; }
  const crc = (b) => { let c = 0xffffffff; for (const x of b) c = crcT[(c ^ x) & 0xff] ^ (c >>> 8); return (c ^ 0xffffffff) >>> 0; };
  const chunk = (typ, daten) => { const l = Buffer.alloc(4); l.writeUInt32BE(daten.length); const t = Buffer.from(typ, "ascii"); const c = Buffer.alloc(4); c.writeUInt32BE(crc(Buffer.concat([t, daten]))); return Buffer.concat([l, t, daten, c]); };
  const ihdr = Buffer.alloc(13); ihdr.writeUInt32BE(breite, 0); ihdr.writeUInt32BE(hoehe, 4); ihdr[8] = 8; ihdr[9] = 2;
  teile.push(Buffer.from([0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a]), chunk("IHDR", ihdr), chunk("IDAT", idat), chunk("IEND", Buffer.alloc(0)));
  return Buffer.concat(teile);
}

let fehler = 0;
// Das Muster muss die Filter WIRKLICH fordern.
//
// Der erste Entwurf nahm einen linearen Verlauf ((x*7+y*13)%256). Damit lagen die drei
// Nachbarn a, b, c fast immer so, dass die volle Paeth-Bedingung und eine verkuerzte
// Fassung DASSELBE waehlen -- die Probe lief mit absichtlich verdorbenem Paeth-Filter
// durch und meldete fuenfmal OK. Ein Testmuster, das den Unterschied nicht herstellt,
// prueft nichts; es erzeugt nur das Gefuehl, geprueft zu haben.
//
// Deshalb Pseudozufall mit festem Startwert: reproduzierbar, aber ohne Struktur, der
// eine verkuerzte Formel folgen koennte.
let seed = 12345;
const zuf = () => { seed = (seed * 1103515245 + 12345) & 0x7fffffff; return (seed >> 8) % 256; };
const werte = [];
for (let i = 0; i < 23 * 11 * 3; i++) werte.push(zuf());
const muster = (x, y) => [werte[(y * 23 + x) * 3], werte[(y * 23 + x) * 3 + 1], werte[(y * 23 + x) * 3 + 2]];
for (const filter of [0, 1, 2, 3, 4]) {
  const bild = pngLesen(baue(23, 11, muster, filter));
  let schlecht = 0;
  for (let y = 0; y < 11; y++) for (let x = 0; x < 23; x++) {
    const soll = muster(x, y), ist = bild.punkt(x, y);
    if (soll.join() !== ist.join()) schlecht++;
  }
  console.log(`  Filter ${filter}: ${schlecht === 0 ? "OK" : "FEHLER in " + schlecht + " Punkten"}`);
  if (schlecht) fehler = 1;
}
const b2 = pngLesen(baue(10, 10, () => [10, 132, 255], 4));
console.log("  haeufigste Farbe:", JSON.stringify(b2.haeufigste(0, 0, 10, 10)));
console.log("  Kontrast #0a84ff gegen Weiss:", kontrast([10,132,255],[255,255,255]).toFixed(2),
            "/ gegen Schwarz:", kontrast([10,132,255],[0,0,0]).toFixed(2));
process.exit(fehler);
