// Pruefstand b48 (Paket 2026-10-13, P2.5, AKF.1): Fehlerzeile am Feld, im echten Browser.
//
//   node b48.mjs <app-verzeichnis> <port> [bezeichnung]
//
// Startet die Vorschau (tools/preview/server.py, PREVIEW_APP=<app-verzeichnis>) und ein
// headless Chrome, laedt /app/ und prueft
//   A  statisch: jede Meldefunktion ruft showFieldError; Zahl ihrer Aufrufstellen (Bestand)
//   B  Vertrag an JEDEM Feld der Bestandsliste (fest und dynamisch): aria-invalid,
//      <span class="field-error" id="<id>-error" aria-hidden="true"> ohne role (Lead-Entscheid
//      Review P2.6 M2: sonst liest ein Screenreader die Meldung bis zu dreimal) direkt nach dem Feld, Text per
//      textContent, aria-describedby ergaenzt ohne fremde IDs zu verlieren, zweiter Aufruf
//      ohne Dublette, naechstes input-Ereignis raeumt alles ab
//   C  Schutz: Feld nicht direkt in .field => keine Zeile
//   D  Ende zu Ende ueber den echten Speicherpfad: Datumsformat, WLAN-Schluessel zu lang,
//      Uhrzeit (die drei Felder aus AKF.2)
// Exit 0 OK, 1 FEHL, 2 Werkzeugfehler.
import { spawn } from "node:child_process";
import { mkdtempSync, readFileSync, rmSync } from "node:fs";
import { tmpdir } from "node:os";
import path from "node:path";

const [appDir, port = "8131", bez = "PWA"] = process.argv.slice(2);
const ROOT = process.env.WORDCLOCK_ROOT;   // von run.sh gesetzt und geprueft
const CHROME = process.env.CHROME || "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome";
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const quelle = readFileSync(path.join(appDir, "app.js"), "utf8");
// Zaehlung ohne Kommentare (Review P2.6 N-2: eine Erwaehnung im Kommentar zaehlte als Aufrufstelle).
// Nur ganze //-Zeilen und der Kopfkommentar am Dateianfang: Eine Regex-Erkennung fuer /* */
// hielte das Literal /^\.?\/*/ (app.js) fuer einen Kommentarbeginn -- derselbe Fehler wie K1.
const ohneKommentar = quelle.replace(/^\/\*[\s\S]*?\*\//, "").replace(/^[ \t]*\/\/.*$/gm, "");
const fehler = [];
let faelle = 0;
const pruefe = (name, ok, info = "") => { faelle++; if (!ok) fehler.push(name + (info ? ": " + info : "")); };

// ---------- A statisch ----------
const MELDER = ["reportInvalidInput", "checkTextValueOrReport", "readNumberInputOrReport", "readTimeInputOrReport", "showTimeFieldError"];
const rumpf = (name) => {
  const m = new RegExp("^(async )?function " + name + "\\(", "m").exec(quelle);
  if (!m) return null;
  let i = quelle.indexOf("{", m.index), t = 0;
  for (; i < quelle.length; i++) { if (quelle[i] === "{") t++; if (quelle[i] === "}" && --t === 0) break; }
  return quelle.slice(m.index, i + 1);
};
// Bestand der Aufrufstellen (Review P2.6 M3: frueher nur ausgegeben, nie verglichen). Aendert
// sich eine Zahl, ist ein Meldeweg dazugekommen oder weggefallen -- dann Bestandsliste und
// FELDER/DYNAMISCH nachfuehren, nicht nur die Zahl.
const BESTAND_SOLL = { reportInvalidInput: 4, checkTextValueOrReport: 3, readNumberInputOrReport: 11, readTimeInputOrReport: 3,
  showTimeFieldError: 2, readTextInputOrReport: 13, readNumberFieldsOrReport: 4 };
const bestand = {};
for (const f of ["readTextInputOrReport", "readNumberFieldsOrReport"]) {
  bestand[f] = (ohneKommentar.match(new RegExp("[^\\w.]" + f + "\\(", "g")) || []).length - 1;
}
for (const f of MELDER) {
  const r = rumpf(f);
  pruefe("A " + f + " vorhanden", !!r);
  if (!r) continue;
  pruefe("A " + f + " ruft showFieldError", /showFieldError\(/.test(r) || /showTimeFieldError\(/.test(r));
  bestand[f] = (ohneKommentar.match(new RegExp("[^\\w.]" + f + "\\(", "g")) || []).length - 1;
}

// N-2: readNumberInputOrReport hat drei Fehlerzweige -- jeder muss die Zeile setzen.
{ const r = rumpf("readNumberInputOrReport"); const n = r ? (r.match(/showFieldError\(/g) || []).length : 0;
  pruefe("A readNumberInputOrReport: showFieldError in allen drei Zweigen", n >= 3, "gezaehlt " + n); }
for (const [f, soll] of Object.entries(BESTAND_SOLL)) {
  pruefe("A Aufrufstellen " + f + " = " + soll, bestand[f] === soll, "gezaehlt " + bestand[f]);
}

// Feste Felder aus der Bestandsliste (P2.2, pwa-developer 14.10.2026); dynamische (Timer,
// Overlays, Alarme) entstehen erst mit Geraetedaten und laufen ueber denselben Code.
const FELDER = ["date-format-input", "weather-lon-input", "weather-lat-input", "weather-map-lon-input", "weather-map-lat-input",
  "ticker-text-input", "weather-appid-input", "weather-city-input", "weather-map-city-input", "network-ssid-manual-input",
  "network-ssid-select", "network-key-input", "network-ap-ssid-input", "network-ap-key-input", "network-timeserver-input",
  "update-host-input", "update-path-input", "dfplayer-silence-start-input", "dfplayer-silence-stop-input",
  "ticker-deceleration-input", "network-timezone-input", "temperature-rtc-correction-input", "temperature-ds18xx-correction-input",
  "ambilight-leds-input", "ambilight-offset-input", "dfplayer-speak-cycle-input", "datetime-year-input", "datetime-month-input",
  "datetime-day-input", "datetime-hour-input", "datetime-minute-input", "dfplayer-folder-input", "dfplayer-track-input"];

// ---------- Vorschau und Chrome ----------
const prof = mkdtempSync(path.join(tmpdir(), "b48-"));
const server = spawn("python3", [path.join(ROOT, "tools/preview/server.py"), port], { env: { ...process.env, PREVIEW_APP: path.resolve(appDir) }, stdio: "ignore" });
const cdp = String(Number(port) + 1000);
const chrome = spawn(CHROME, ["--headless=new", "--remote-debugging-port=" + cdp, "--user-data-dir=" + prof, "about:blank"], { stdio: "ignore" });
const ende = (code) => { try { chrome.kill("SIGKILL"); } catch {} try { server.kill("SIGKILL"); } catch {} try { rmSync(prof, { recursive: true, force: true }); } catch {} process.exit(code); };
let seiten;
for (let i = 0; i < 60; i++) { try { seiten = await (await fetch("http://127.0.0.1:" + cdp + "/json/list")).json(); if (seiten.length) break; } catch {} await sleep(200); }
const seite = seiten && seiten.find((t) => t.type === "page");
if (!seite) { console.log("KAPUTT b48 " + bez + ": Chrome ohne Debug-Port"); ende(2); }
const ws = new WebSocket(seite.webSocketDebuggerUrl);
await new Promise((r) => (ws.onopen = r));
let nr = 0; const offen = new Map();
ws.onmessage = (m) => { const d = JSON.parse(m.data); if (d.id && offen.has(d.id)) { offen.get(d.id)(d); offen.delete(d.id); } };
const send = (method, params = {}) => new Promise((r) => { const i = ++nr; offen.set(i, r); ws.send(JSON.stringify({ id: i, method, params })); });
const ev = async (expr) => { const r = await send("Runtime.evaluate", { expression: expr, awaitPromise: true, returnByValue: true }); if (!r.result) throw new Error("CDP: " + JSON.stringify(r.error || r).slice(0, 200)); if (r.result.exceptionDetails) throw new Error(r.result.exceptionDetails.exception ? r.result.exceptionDetails.exception.description : JSON.stringify(r.result.exceptionDetails)); return r.result.result.value; };
// Erster Aufruf: Der Service Worker registriert sich (127.0.0.1 ist ein sicherer Kontext),
// uebernimmt die Seite und laedt sie neu (controllerchange). Mitten in einer Pruefung hiess
// das "Inspected target navigated". Deshalb einmal vorladen und warten, dann pruefen.
await send("Page.navigate", { url: "http://127.0.0.1:" + port + "/app/" });
await sleep(4000);
let geladen = false;
let dynZahl = 0;
for (let i = 0; i < 30 && !geladen; i++) {
  await send("Page.navigate", { url: "http://127.0.0.1:" + port + "/app/" });
  await sleep(1500);
  try { geladen = await ev("typeof announceStatus === 'function' && !!document.getElementById('date-format-input')"); } catch {}
}
if (!geladen) { console.log("KAPUTT b48 " + bez + ": PWA in der Vorschau nicht geladen"); ende(2); }

try {
  const hat = await ev("typeof showFieldError === 'function'");
  pruefe("B showFieldError vorhanden", hat);
  // ---------- B Vertrag je Feld ----------
  // Dynamische Felder: die Vorschau liefert Timer und Overlays (tools/preview/README.md); nach
  // dem Laden stehen ihre Zeilen im DOM. Erfasst wird per Muster, nicht per Hand.
  const DYNAMISCH = "^(t|at)-time-\\d+$|^df-alarm-time-\\d+$|^ov-(value|icon|folder|track|interval|duration|days)-\\d+$|^alm-dec-\\d+$";
  await ev(`(async () => { for (const m of ["timers", "overlays", "dfplayer", "animations"]) { try { setActiveModule(m); } catch (e) {} await new Promise((r) => setTimeout(r, 300)); } })()`);
  const dyn = await ev(`[...document.querySelectorAll("input, select, textarea")].map((e) => e.id).filter((id) => new RegExp(${JSON.stringify(DYNAMISCH)}).test(id))`);
  dynZahl = dyn.length;
  const gruppen = new Set(dyn.map((id) => id.replace(/-\d+$/, "")));
  pruefe("B dynamische Felder gerendert (Timer, Alarm-Timer, Overlay)", ["t-time", "at-time", "ov-value"].every((g) => gruppen.has(g)), [...gruppen].join(", ") || "keine");
  const b = await ev(`(async () => {
    const out = [];
    for (const id of ${JSON.stringify(FELDER)}.concat(${JSON.stringify(dyn)})) {
      const el = document.getElementById(id);
      if (!el) { out.push({ id, fehlt: true }); continue; }
      const r = { id, f: [] };
      const f = (ok, t) => { if (!ok) r.f.push(t); };
      f(el.parentNode && el.parentNode.classList && el.parentNode.classList.contains("field"), "nicht direkt in .field");
      el.setAttribute("aria-describedby", "fremd-1");
      showFieldError(el, "Probe <b>1</b>");
      const s = el.nextElementSibling;
      f(el.getAttribute("aria-invalid") === "true", "aria-invalid fehlt");
      f(s && s.tagName === "SPAN" && s.classList.contains("field-error") && s.id === id + "-error" && s.getAttribute("aria-hidden") === "true" && !s.hasAttribute("role"), "Zeile fehlt/falsch: " + (s ? s.outerHTML.slice(0, 80) : "keine"));
      f(s && s.textContent === "Probe <b>1</b>" && !s.querySelector("b"), "Text nicht per textContent");
      const d1 = (el.getAttribute("aria-describedby") || "").split(/\\s+/);
      f(d1.includes("fremd-1") && d1.includes(id + "-error"), "describedby: " + d1.join(" "));
      showFieldError(el, "Probe 2");
      f(document.querySelectorAll("#" + CSS.escape(id + "-error")).length === 1, "Dublette");
      f(el.nextElementSibling && el.nextElementSibling.textContent === "Probe 2", "nicht aktualisiert");
      f((el.getAttribute("aria-describedby") || "").split(/\\s+/).filter((x) => x === id + "-error").length === 1, "describedby doppelt");
      el.dispatchEvent(new Event("input", { bubbles: true }));
      f(!document.getElementById(id + "-error"), "Zeile bleibt nach input");
      f(!el.hasAttribute("aria-invalid"), "aria-invalid bleibt nach input");
      f(el.getAttribute("aria-describedby") === "fremd-1", "describedby nach input: " + el.getAttribute("aria-describedby"));
      el.removeAttribute("aria-describedby");
      out.push(r);
    }
    return out;
  })()`);
  const fehlend = b.filter((x) => x.fehlt).map((x) => x.id);
  pruefe("B alle Felder der Bestandsliste in index.html", fehlend.length === 0, fehlend.join(", "));
  for (const r of b.filter((x) => !x.fehlt)) pruefe("B " + r.id, r.f.length === 0, r.f.join("; "));
  // ---------- C Schutz ----------
  const c = await ev(`(() => {
    const field = document.querySelector(".field");
    const div = document.createElement("div"); const inp = document.createElement("input"); inp.id = "b48-schutz";
    div.appendChild(inp); field.appendChild(div);
    showFieldError(inp, "x");
    const ok = !document.getElementById("b48-schutz-error") && !inp.hasAttribute("aria-invalid");
    div.remove(); return ok;
  })()`);
  pruefe("C Feld nicht direkt in .field: keine Zeile", c);
  // ---------- D Ende zu Ende ----------
  const d = await ev(`(async () => {
    const r = {};
    const zeile = (id) => { const s = document.getElementById(id + "-error"); return s ? s.textContent : null; };
    const gesendet = []; const f0 = window.fetch;
    window.fetch = (u, o) => { gesendet.push(String(u)); return f0(u, o); };
    // Das Feld muss sichtbar sein, sonst nimmt es keinen Fokus an: Modul des Felds aktivieren.
    const zeige = async (id) => { const s = document.getElementById(id).closest(".module-section"); if (s) setActiveModule(s.dataset.module); await new Promise((r) => setTimeout(r, 300)); };
    // Ein ausgeblendetes Modul (dfplayer in der Vorschau) laesst keinen Fokus zu: dort zaehlt
    // der Fokus nicht, die Pruefung sagt es aber ("sichtbar": false) statt still durchzuwinken.
    const pruef = (id) => ({ sichtbar: !!document.getElementById(id).offsetParent, fokus: document.activeElement && document.activeElement.id === id, banner: ((document.getElementById("status-banner") || {}).textContent || "").length > 0 });
    await zeige("date-format-input"); document.getElementById("date-format-input").value = "QQQ";
    await saveDateTickerFormat(); r.datum = zeile("date-format-input"); r.datumP = pruef("date-format-input");
    await zeige("network-key-input");
    document.getElementById("network-ssid-manual-input").value = "Heim";
    document.getElementById("network-key-input").value = "ä".repeat(40);
    await saveNetworkClient(); r.schluessel = zeile("network-key-input"); r.schluesselP = pruef("network-key-input");
    await zeige("dfplayer-silence-start-input"); document.getElementById("dfplayer-silence-start-input").value = "";
    await saveDfplayerSilenceStart(); r.zeit = zeile("dfplayer-silence-start-input"); r.zeitP = pruef("dfplayer-silence-start-input");
    // M1 (Review P2.6): Eine Fehlerzeile darf ein spaeteres erfolgreiches Speichern ueber das
    // Gegenfeld nicht ueberleben. Laenge leer + Breite gesetzt => Zeile an der Laenge; dann beide
    // leer => gespeichert, und die Zeile an der Laenge muss weg sein.
    await zeige("weather-lon-input");
    document.getElementById("weather-lon-input").value = ""; document.getElementById("weather-lat-input").value = "47.3";
    const vorPaar = gesendet.length;
    await saveWeatherCoordinates(); r.paarVorher = !!(zeile("weather-lon-input") || zeile("weather-lat-input"));
    r.paarAnfragenUnvollstaendig = gesendet.slice(vorPaar).filter((u) => /weather_coordinates_set/.test(u)).length;
    document.getElementById("weather-lat-input").value = "";
    const vorGueltig = gesendet.length;
    await saveWeatherCoordinates(); r.paarNachher = zeile("weather-lon-input") || zeile("weather-lat-input");
    r.paarAnfragenGueltig = gesendet.slice(vorGueltig).filter((u) => /weather_coordinates_set/.test(u)).length;
    // M-A (Nachreview P2.6): Ein programmatisch gesetzter Wert darf keine alte Zeile stehen lassen.
    // a) Kartenuebernahme: Laenge 999 speichern => Zeile; dann Ort ueber die Karte uebernehmen.
    await zeige("weather-lon-input");
    document.getElementById("weather-lon-input").value = "999"; document.getElementById("weather-lat-input").value = "47.3";
    await saveWeatherCoordinates(); r.karteVorher = !!zeile("weather-lon-input");
    for (const [id, v] of [["weather-map-city-input", "Bern"], ["weather-map-lon-input", "7.4474"], ["weather-map-lat-input", "46.948"]]) { const e = document.getElementById(id); if (e) e.value = v; }
    try { await applyWeatherMapSelection(); } catch (e) { r.karteFehler = String(e && e.message || e); }
    r.karteNachher = zeile("weather-lon-input");
    // b) Verwerfen: Datumsformat QQQ => Zeile; dann verwerfen.
    await zeige("date-format-input"); document.getElementById("date-format-input").value = "QQQ";
    await saveDateTickerFormat(); r.verwerfenVorher = !!zeile("date-format-input");
    discardUnsavedEdits(); r.verwerfenNachher = zeile("date-format-input");
    window.fetch = f0; r.gesendet = gesendet.filter((u) => /_set([?]|$)|network_client/.test(u) && !/weather_coordinates_set|weather_city_set/.test(u));
    r.banner = (document.getElementById("status-banner") || {}).textContent || "";
    return r;
  })()`);
  pruefe("D Datumsformat ungueltig => Zeile", !!d.datum, JSON.stringify(d));
  pruefe("D WLAN-Schluessel zu lang => Zeile", !!d.schluessel && /63/.test(d.schluessel), JSON.stringify(d));
  pruefe("D Uhrzeit leer => Zeile", !!d.zeit, JSON.stringify(d));
  for (const k of ["datumP", "schluesselP", "zeitP"]) { pruefe("D " + k + " Fokus auf dem Feld (wenn sichtbar) und Banner gesetzt", d[k] && (d[k].fokus || !d[k].sichtbar) && d[k].banner, JSON.stringify(d[k])); if (d[k] && !d[k].sichtbar) console.log("      Hinweis: " + k + " Feld in der Vorschau ausgeblendet, Fokus nicht pruefbar"); }
  pruefe("D M1 Koordinatenpaar unvollstaendig => Zeile", d.paarVorher === true, JSON.stringify(d.paarVorher));
  pruefe("D M1 nach erfolgreichem Speichern ueber das Gegenfeld keine Zeile mehr", !d.paarNachher, JSON.stringify(d.paarNachher));
  pruefe("D M-A Kartenuebernahme raeumt die Zeile am Hauptfeld", d.karteVorher === true && !d.karteNachher, JSON.stringify({ v: d.karteVorher, n: d.karteNachher, f: d.karteFehler }));
  pruefe("D M-A Verwerfen raeumt die Zeile", d.verwerfenVorher === true && !d.verwerfenNachher, JSON.stringify({ v: d.verwerfenVorher, n: d.verwerfenNachher }));
  pruefe("D Koordinaten: keine Anfrage beim unvollstaendigen Paar, genau eine beim gueltigen", d.paarAnfragenUnvollstaendig === 0 && d.paarAnfragenGueltig === 1, d.paarAnfragenUnvollstaendig + " / " + d.paarAnfragenGueltig);
  // Ausgenommen ist nur das GUELTIGE Speichern des leeren Koordinatenpaars im M1-Fall.
  pruefe("D keine Anfrage bei ungueltiger Eingabe", d.gesendet.filter((u) => !/weather_coordinates_set/.test(u)).length === 0, d.gesendet.join(" "));
} catch (e) {
  pruefe("Lauf ohne Ausnahme", false, String(e.message || e).slice(0, 200));
}
ws.close();
for (const f of fehler) console.log("      FEHLER: " + f);
console.log((fehler.length ? "FEHL" : "OK  ") + "  b48 " + bez + ": " + fehler.length + " Fehler in " + faelle + " Faellen (" + FELDER.length + " feste Felder, " + (typeof dynZahl === "number" ? dynZahl : 0) + " dynamische; Aufrufstellen " +
  Object.entries(bestand).map(([k, v]) => k + " " + v).join(", ") + ")");
ende(fehler.length ? 1 : 0);
