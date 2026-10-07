#!/usr/bin/env node
//
// Vermisst die PWA -- wahlweise in der VORSCHAU oder auf dem GERAET, mit demselben
// Messstueck (tools/ui-mess/messungen.mjs). Das ist der Punkt: Vorher wurde fuer jede
// Frage ein eigenes Skript gebaut und danach weggeworfen, sechsmal in zwei Tagen (E19),
// und Vorschau- und Geraetezahlen waren nie vergleichbar (L229).
//
//   node tools/ui-mess/vermessen.mjs --vorschau 390x844 1280x820
//   node tools/ui-mess/vermessen.mjs --geraet   390x844
//   node tools/ui-mess/vermessen.mjs --vorschau --module display,timers 390x844
//   node tools/ui-mess/vermessen.mjs --vorschau --module dfplayer 390x844    (Override automatisch)
//
// Legt je Lauf eine Ergebnisdatei unter tools/ui-mess/ergebnisse/ an. AUSSCHLIESSLICH
// LESEND -- es werden Module geoeffnet und Werte abgelesen, nichts gespeichert, keine
// Schaltflaeche mit Wirkung geklickt (DIR-008).
import { writeFileSync, mkdirSync, readFileSync, existsSync } from "node:fs";
import { setTimeout as sleep } from "node:timers/promises";
import { starteBrowser } from "./wirt.mjs";
import { MESSUNG, FOKUS_LESEN } from "./messungen.mjs";
import { pngLesen, kontrast } from "./png.mjs";

const args = process.argv.slice(2);
const ziel = args.includes("--geraet") ? "geraet" : "vorschau";
const mi = args.indexOf("--module");
const MODULE = mi >= 0 ? args[mi + 1].split(",") : ["display", "animations", "overlays", "timers"];
const VIEWS = args.filter((a) => /^\d+x\d+$/.test(a)).map((s) => s.split("x").map(Number));

// Ansichts-Overrides der PWA (System > Ansichts-Overrides, localStorage
// "wordclock-app-debug-overrides"), z. B. --override dfplayer=on,ambilight=on.
// E14/L119: Die Vorschau meldet keinen DFPlayer, das Modul ist dort ausgeblendet und
// misst 0 x 0 px -- wer es vermessen wollte, mass nichts und merkte es nicht. Steht
// dfplayer unter --module, setzt die VORSCHAU den Override deshalb von selbst. Am
// Geraet nie automatisch: Dort soll die Messung zeigen, was der Nutzer sieht.
const oi = args.indexOf("--override");
const OVERRIDES = {};
if (oi >= 0) for (const kv of args[oi + 1].split(",")) { const [k, v] = kv.split("="); OVERRIDES[k] = v; }
if (!VIEWS.length) VIEWS.push([390, 844]);

let BASIS;
if (ziel === "geraet") {
  // Die Adresse steht ausschliesslich in tools/device.conf (gitignored, R5/DIR-008).
  if (!existsSync("tools/device.conf")) {
    console.error("tools/device.conf fehlt — ohne sie ist die Geraeteadresse nicht bekannt.");
    process.exit(2);
  }
  const host = (readFileSync("tools/device.conf", "utf8").match(/^DEVICE_HOST=(.+)$/m) || [])[1];
  if (!host) { console.error("DEVICE_HOST fehlt in tools/device.conf."); process.exit(2); }
  BASIS = `http://${host.trim().replace(/^["']|["']$/g, "")}`;
} else {
  BASIS = `http://127.0.0.1:${process.env.PORT || 8099}`;
  if (MODULE.includes("dfplayer") && !OVERRIDES.dfplayer) OVERRIDES.dfplayer = "on";
  try { await fetch(`${BASIS}/app/`); }
  catch { console.error(`Vorschau-Server antwortet nicht auf ${BASIS}.\n  python3 tools/preview/server.py 8099`); process.exit(2); }
}

const AUS = "tools/ui-mess/ergebnisse";
mkdirSync(AUS, { recursive: true });
const b = await starteBrowser({ port: 9340, profil: "/tmp/ui-mess-profil" });
const lauf = { ziel, basis: ziel === "geraet" ? "(Geraet, Adresse nicht protokolliert)" : BASIS,
               zeitpunkt: new Date().toISOString(), module: [] };

try {
  for (const [W, H] of VIEWS) {
    await b.viewport(W, H);
    for (const modul of MODULE) {
      await b.gehe(`${BASIS}/app/`);
      await b.js(`localStorage.setItem('wordclock-app-active-module', ${JSON.stringify(modul)})`);
      if (Object.keys(OVERRIDES).length)
        await b.js(`localStorage.setItem('wordclock-app-debug-overrides', ${JSON.stringify(JSON.stringify(OVERRIDES))})`);
      await b.gehe(`${BASIS}/app/`);
      await sleep(ziel === "geraet" ? 3500 : 1500);

      const m = await b.js(MESSUNG);
      if (!m) continue;

      // Fokusreihenfolge ueber ECHTE Tabulatortasten (siehe messungen.mjs).
      await b.js("document.body.focus()");
      const fokus = [];
      for (let i = 0; i < 12; i++) {
        await b.tab();
        const f = await b.js(FOKUS_LESEN);
        if (!f) break;
        fokus.push(f);
      }

      // Farben AM BILDPUNKT: ganzseitige Aufnahme, dann je Ankreuzfeld auslesen.
      let farben = [];
      if (m.felder.length) {
        const png = pngLesen(await b.schuss(
          { x: 0, y: 0, width: m.doc.w, height: Math.min(m.doc.h, 16000), scale: 1 }));
        farben = m.felder.slice(0, 40).map((f) => {
          const mitte = png.haeufigste(f.feld.x + f.feld.w * 0.3, f.feld.y + f.feld.h * 0.3,
                                       Math.max(2, f.feld.w * 0.4), Math.max(2, f.feld.h * 0.4));
          const umfeld = f.pille
            ? png.haeufigste(f.pille.x + f.pille.w - 12, f.pille.y + f.pille.h / 2 - 3, 10, 6)
            : null;
          // Der UMRISS getrennt von der Fuellung. Beim unangekreuzten Feld traegt in
          // Chromium nicht die Fuellung den Kontrast, sondern der Rand -- wer nur die
          // Mitte misst, bekommt rund 1,3:1 und haelt das fuer einen Normverstoss,
          // waehrend der Rand die geforderten 3:1 deutlich erfuellt (L245, B31).
          // Gemessen wird ein schmaler Streifen auf der linken Kante des Kaestchens.
          const rand = png.haeufigste(f.feld.x, f.feld.y + f.feld.h * 0.4,
                                      Math.max(1, f.feld.w * 0.12), Math.max(2, f.feld.h * 0.2));
          return { i: f.i, name: f.name, angekreuzt: f.angekreuzt,
                   feldfarbe: mitte?.farbe ?? null, randfarbe: rand?.farbe ?? null,
                   umfeldfarbe: umfeld?.farbe ?? null,
                   kontrastFuellung: mitte && umfeld
                     ? Math.round(kontrast(mitte.farbe, umfeld.farbe) * 100) / 100 : null,
                   kontrastRand: rand && umfeld
                     ? Math.round(kontrast(rand.farbe, umfeld.farbe) * 100) / 100 : null };
        });
      }
      lauf.module.push({ viewport: `${W}x${H}`, modul, doc: m.doc,
                         panels: m.panels, felder: m.felder, klein: m.klein, fokus, farben });
      process.stdout.write(`  ${W}x${H} ${modul.padEnd(12)} ` +
        `${m.panels.length} Kacheln, ${m.felder.length} Felder, ` +
        `${m.klein.length} unter 44px, ${fokus.length} Fokusstationen\n`);
    }
  }
} finally { b.ende(); }

const name = `${AUS}/${ziel}-${new Date().toISOString().slice(0, 16).replace(/[:T]/g, "")}.json`;
writeFileSync(name, JSON.stringify(lauf, null, 2));
console.log(`\n  Ergebnis: ${name}`);
