// Probe fuer Runde P (Paket 2026-10-06) -- laeuft gegen die Vorschau, nicht gegen das Geraet.
//
//     python3 tools/preview/server.py 8137 &
//     node tools/ui-mess/proben/runde-p.mjs 8137
//
// Prueft B34 (Sprachwechsel leert keine Anzeigen), B35 (Byte-Pruefung vor dem Absenden,
// auch im Backup-Import), B36 (keine gruene Meldung nach einer Abweisung, auch beim
// Formatieren), B27, B33 (Binaersperre), B26, die zehn Umschalt- und Power-Knoepfe beim
// Sprachwechsel und nach einem Fehler. Abweisungen und Antworten werden im Browser
// gestellt (window.__reject, window.__respond) -- das Geraet wird nie angefasst.
//
// Gegenprobe (DIR-014): Eine zweite Vorschau, die eine alte app.js ausliefert, und
// --alt. Gemessen am 05.10.2026: neu 84/0, Stand vor B33/B26 77/6, HEAD 22/38.
//
// Einige Pruefungen vergleichen deutsche Texte woertlich. Aendert sich ein Wortlaut,
// schlaegt die Probe an -- das ist dann ein Probenpflegefall, kein Fehler im Code.
// Browserprobe fuer B34/B35/B36 gegen die Vorschau. Aufruf:
//   node runde-p.mjs <port> [--alt <pfad zu alter app.js>]
import { starteBrowser } from "../wirt.mjs";
import { tmpdir } from "node:os";
import { readFileSync } from "node:fs";
import { setTimeout as sleep } from "node:timers/promises";

const port = process.argv[2];
const altIdx = process.argv.indexOf("--alt");
const altPfad = altIdx > 0 ? process.argv[altIdx + 1] : null;
const b = await starteBrowser({ port: 9351 + (altPfad ? 1 : 0), profil: tmpdir() + "/runde-p-profil" + (altPfad ? "-alt" : "") });

let ok = 0, fail = 0;
const pruef = (name, bed, info = "") => {
  if (bed) { ok++; console.log("  OK   " + name); }
  else { fail++; console.log("  FAIL " + name + (info ? "  -- " + info : "")); }
};

const fehler = [];
b.cdp("Runtime.enable", {}, b.sessionId);

await b.cdp("Page.addScriptToEvaluateOnNewDocument", { source: "window.__pageErrors=[];window.addEventListener('error',e=>window.__pageErrors.push(String(e.message)));window.addEventListener('unhandledrejection',e=>window.__pageErrors.push('rej:'+String(e.reason&&e.reason.message||e.reason)));" }, b.sessionId);

try {
  await b.gehe(`http://127.0.0.1:${port}/app/`);
  await sleep(2500);

  pruef("Seite geladen, app.js ohne Ladefehler", await b.js(`typeof applyStaticTranslations === "function" && typeof loadData === "function"`));

  // Instrumentierung: Statusmeldungen und Abrufe mitschreiben, Antworten steuerbar.
  await b.js(`(() => {
    window.__status = [];
    const orig = window.announceStatus;
    window.announceStatus = (m, t) => { window.__status.push([t, m]); return orig(m, t); };
    window.__calls = [];
    window.__reject = {};
    window.__focus = null;
    const of2 = HTMLElement.prototype.focus;
    HTMLElement.prototype.focus = function () { window.__focus = this.id; return of2.apply(this, arguments); };
    const of = window.fetch.bind(window);
    window.fetch = (url, opts) => {
      const u = String(url && url.url ? url.url : url);
      window.__calls.push(u);
      for (const key of Object.keys(window.__respond || {})) {
        if (u.indexOf(key) >= 0) {
          const r = window.__respond[key];
          return Promise.resolve(new Response(new Uint8Array(r.bytes), { status: 200, headers: { "Content-Type": r.type || "text/plain" } }));
        }
      }
      for (const key of Object.keys(window.__reject)) {
        if (u.indexOf(key) >= 0) {
          return Promise.resolve(new Response(JSON.stringify(window.__reject[key]), { status: 200, headers: { "Content-Type": "application/json" } }));
        }
      }
      return of(url, opts);
    };
    return true;
  })()`);

  // ------------------------------------------------------------------ B36
  console.log("B36 / L288");
  await b.js(`(async () => {
    window.__status.length = 0; window.__calls.length = 0;
    window.__reject["/api/update_host_set"] = { ok: false, error: 1, detail: "value missing or empty" };
    document.getElementById("update-host-input").value = "abgewiesen.example";
    await saveUpdateHost();
    return true;
  })()`);
  let st = await b.js(`window.__status`);
  let last = st[st.length - 1] || [];
  pruef("Abweisung: letzte Meldung ist der Fehler", last[0] === "error", JSON.stringify(st));
  pruef("Abweisung: keine 'abgeschlossen'-Meldung danach", !st.some(([t, m]) => t === "ok"), JSON.stringify(st));
  pruef("Abweisung: Statuszeile zeigt rot", await b.js(`document.getElementById("updated-at").classList.contains("is-error")`));

  await b.js(`(async () => {
    window.__status.length = 0;
    delete window.__reject["/api/update_host_set"];
    document.getElementById("update-host-input").value = "gut.example";
    await saveUpdateHost();
    return true;
  })()`);
  st = await b.js(`window.__status`);
  last = st[st.length - 1] || [];
  pruef("Erfolg: Nachpruefung laeuft und meldet ok", last[0] === "ok" && st.some(([t]) => t === "warn"), JSON.stringify(st));

  await b.js(`(async () => {
    window.__status.length = 0;
    window.__reject["/api/update_path_set"] = { ok: false, error: 1, detail: "value missing or empty" };
    document.getElementById("update-path-input").value = "pfad";
    await saveUpdatePath();
    delete window.__reject["/api/update_path_set"];
    return true;
  })()`);
  st = await b.js(`window.__status`);
  pruef("Pfad-Abweisung: kein ok danach", !st.some(([t]) => t === "ok") && (st[st.length - 1] || [])[0] === "error", JSON.stringify(st));

  // formatLittleFsFromFiles: Fehlschlag darf nicht "formatiert" schreiben
  if (!altPfad) {
    await b.js(`(async () => {
      window.confirm = () => true;
      window.__reject["/api/maintenance_format_fs"] = { ok: false, error: 3, detail: "busy" };
      document.getElementById("fs-action-status").textContent = "vorher";
      await formatLittleFsFromFiles();
      delete window.__reject["/api/maintenance_format_fs"];
      return true;
    })()`);
    pruef("Format-Fehlschlag: Dateiansicht meldet kein 'formatiert'",
      await b.js(`document.getElementById("fs-action-status").textContent !== translate("maintenance.format_fs_done")`),
      await b.js(`document.getElementById("fs-action-status").textContent`));
  }

  // ------------------------------------------------------------------ B35
  console.log("B35 / L287");
  const hatB35 = await b.js(`typeof countUtf8Bytes === "function"`);
  if (hatB35) {
    // Bytezahl gegen das, was der ESP nach normalize_http_parameters mit strlen misst
    const vergleich = await b.js(`(() => {
      const proben = ["", "a", "ä", "Zürich", "€", "😀", "Grüße aus Genève", "%d.%m", "a b+c&d=e", "\\u00e4".repeat(32)];
      return proben.map((s) => {
        const enc = encodeURIComponent(s);
        const espBytes = enc.replace(/%[0-9A-F]{2}/g, "x").length;
        return [s, countUtf8Bytes(s), espBytes];
      });
    })()`);
    pruef("countUtf8Bytes == Bytes nach Prozentdekodierung (10 Proben)", vergleich.every(([, a, e]) => a === e), JSON.stringify(vergleich));

    const probe = async (inputId, fn, endpoint, value) => b.js(`(async () => {
      window.__status.length = 0; window.__calls.length = 0; window.__focus = null;
      document.getElementById(${JSON.stringify(inputId)}).value = ${JSON.stringify(value)};
      await ${fn}();
      return { sent: window.__calls.some((u) => u.indexOf(${JSON.stringify(endpoint)}) >= 0), status: window.__status.slice(),
               focus: window.__focus };
    })()`);

    const faelle = [
      ["ticker-text-input", "saveTickerText", "/api/ticker_set", "ä".repeat(32), false],
      ["ticker-text-input", "saveTickerText", "/api/ticker_set", "ä".repeat(16), true],
      ["ticker-text-input", "saveTickerText", "/api/ticker_set", "ä".repeat(16) + "a", false],
      ["ticker-text-input", "saveTickerText", "/api/ticker_set", "a".repeat(32), true],
      ["ticker-text-input", "saveTickerText", "/api/ticker_set", "", true],
      ["date-format-input", "saveDateTickerFormat", "/api/date_ticker_format_set", "D.M.Y", true],
      ["date-format-input", "saveDateTickerFormat", "/api/date_ticker_format_set", "D.M.Yä", false],
      ["weather-appid-input", "saveWeatherAppId", "/api/weather_appid_set", "x".repeat(33), false],
      ["weather-city-input", "saveWeatherCity", "/api/weather_city_set", "Zürich", true],
      ["weather-city-input", "saveWeatherCity", "/api/weather_city_set", "ü".repeat(17), false],
      ["network-timeserver-input", "saveTimeServer", "/api/network_timeserver_set", "x".repeat(17), false],
      ["network-timeserver-input", "saveTimeServer", "/api/network_timeserver_set", "pool.ntp.org", true],
      ["update-host-input", "saveUpdateHost", "/api/update_host_set", "h".repeat(64), false],
      ["update-host-input", "saveUpdateHost", "/api/update_host_set", "h".repeat(63), true],
      ["update-path-input", "saveUpdatePath", "/api/update_path_set", "p".repeat(64), false],
    ];
    for (const [id, fn, ep, val, soll] of faelle) {
      const r = await probe(id, fn, ep, val);
      const meldung = r.status.find(([t]) => t === "error");
      if (soll) {
        pruef(`${fn} ${val.length} Zeichen/${new TextEncoder().encode(val).length} Byte: gesendet`, r.sent, JSON.stringify(r));
      } else {
        pruef(`${fn} ${val.length} Zeichen/${new TextEncoder().encode(val).length} Byte: abgewiesen vor dem Abruf, Meldung in Byte, Fokus`,
          !r.sent && meldung && /Byte/.test(meldung[1]) && r.focus === id, JSON.stringify(r));
      }
    }
    // Koordinaten: zweites Feld zu lang
    const koord = await b.js(`(async () => {
      window.__status.length = 0; window.__calls.length = 0;
      document.getElementById("weather-lon-input").value = "8.54";
      document.getElementById("weather-lat-input").value = "47.37000001";
      await saveWeatherCoordinates();
      return { sent: window.__calls.some((u) => u.indexOf("/api/weather_coordinates_set") >= 0), status: window.__status.slice(), focus: window.__focus };
    })()`);
    pruef("Koordinaten: Breitengrad 11 Byte abgewiesen, Fokus dort", !koord.sent && koord.focus === "weather-lat-input", JSON.stringify(koord));
    // Kartenauswahl
    const karte = await b.js(`(async () => {
      window.__status.length = 0; window.__calls.length = 0;
      document.getElementById("weather-city-input").value = "vorher";
      document.getElementById("weather-map-city-input").value = "ö".repeat(20);
      document.getElementById("weather-map-lon-input").value = "8.54";
      document.getElementById("weather-map-lat-input").value = "47.37";
      await applyWeatherMapSelection();
      return { sent: window.__calls.some((u) => u.indexOf("/api/weather_") >= 0), main: document.getElementById("weather-city-input").value,
               note: document.getElementById("weather-map-status").textContent };
    })()`);
    pruef("Kartenauswahl: 40-Byte-Ort abgewiesen, Hauptmaske unberuehrt, Meldung im Dialog", !karte.sent && karte.main === "vorher" && /40 Byte/.test(karte.note), JSON.stringify(karte));
    // Meldungstext
    const text = await b.js(`describeTextFieldOverflow(getTextFieldOverflow("ticker_text", "ä".repeat(32)))`);
    console.log("       Meldung: " + text);
    // Wetterkarte liest Grenzen aus der Tabelle
    pruef("WEATHER_CITY_MAX_BYTES/COORD aus der Tabelle", await b.js(`WEATHER_CITY_MAX_BYTES === 32 && WEATHER_COORDINATE_MAX_CHARS === 8`));

    // Import
    const imp = await b.js(`(async () => {
      window.__calls.length = 0;
      resetTooLongImportFields(); resetSkippedImportFields(); resetAdjustedImportFields();
      await importMaintenanceSettings({ update_host: "h".repeat(70), update_path: "firmware" });
      await importWeatherLocationSettings({ weather_city: "ü".repeat(20), weather_lon: "8.54", weather_lat: "47.37" });
      await importNetworkTimeSettings({ timeserver: "ntp.example", timezone_offset: 1, summertime: true });
      return { calls: window.__calls.filter((u) => u.indexOf("/api/") >= 0).map((u) => u.replace(/^.*\\/api\\//, "").replace(/\\?.*/, "")),
               summary: getImportNoticeSummary() };
    })()`);
    pruef("Import: zu langer Host nicht gesendet, Pfad gesendet", !imp.calls.includes("update_host_set") && imp.calls.includes("update_path_set"), JSON.stringify(imp.calls));
    pruef("Import: zu langer Ort -> ganze Ortsangabe stehen gelassen", !imp.calls.includes("weather_city_set") && !imp.calls.includes("weather_coordinates_set"), JSON.stringify(imp.calls));
    pruef("Import: Zeitserver passend -> gesendet, Stufe lief weiter", imp.calls.includes("network_timeserver_set") && imp.calls.includes("network_summertime_set"), JSON.stringify(imp.calls));
    pruef("Import: Sammelmeldung nennt beide Felder mit Bytezahl", imp.summary.includes("Update-Host (70 statt höchstens 63 Byte)") && imp.summary.includes("Ort für das Wetter (40 statt"), imp.summary);
    console.log("       Import-Meldung: " + imp.summary);
    const ticker = await b.js(`(async () => {
      window.__calls.length = 0; resetTooLongImportFields();
      window.sleep = async () => {};
      await importDisplaySettings({ ticker_text: "ä".repeat(20), date_ticker_format: "D.M.Y", brightness: 5, mode: 0, ticker_deceleration: 5, color: null, dim_curve: null });
      return { calls: window.__calls.filter((u) => u.indexOf("/api/") >= 0).map((u) => u.replace(/^.*\\/api\\//, "").replace(/\\?.*/, "")), summary: getTooLongImportFieldsSummary() };
    })()`);
    pruef("Import: zu langer Ticker nicht gesendet, Datumsformat danach gesendet", !ticker.calls.includes("ticker_set") && ticker.calls.includes("date_ticker_format_set"), JSON.stringify(ticker));
  } else {
    pruef("B35-Helfer vorhanden", false, "countUtf8Bytes fehlt");
  }

  // ------------------------------------------------------------------ B34
  console.log("B34 / L284");
  await b.js(`(async () => { await requestLanguage("de", false); return true; })()`);
  await b.js(`(() => {
    updateStm32Log({ lines: ["zeile eins", "zeile zwei"], count: 2 });
    document.getElementById("fs-preview-content").textContent = "INHALT DER DATEI";
    document.getElementById("fs-action-status").textContent = "Datei x angezeigt";
    return true;
  })()`);
  const ids = ["stm32-log-output", "stm32-log-meta", "fs-preview-content", "fs-action-status", "datetime-preview", "update-release-notes", "network-status-note", "weather-map-status"];
  const vorher = await b.js(`(${JSON.stringify(ids)}).map((id) => [id, document.getElementById(id).textContent])`);
  // ein Platzhalter, der nie gefuellt wird
  const platz = await b.js(`(() => { const e = document.getElementById("local-app-folder-note"); return [e.textContent, e.getAttribute("data-i18n")]; })()`);
  await b.js(`(async () => { await requestLanguage("en", false); return currentLanguage; })()`);
  const nachher = await b.js(`(${JSON.stringify(ids)}).map((id) => [id, document.getElementById(id).textContent])`);
  for (let i = 0; i < ids.length; i++) {
    const [id, v] = vorher[i];
    const istPlatz = await b.js(`I18N_DE[document.getElementById(${JSON.stringify(ids[i])}).getAttribute("data-i18n")] === ${JSON.stringify(v)}`);
    if (istPlatz) {
      pruef(`${id}: war Platzhalter, jetzt Englisch`, nachher[i][1] !== v, `vorher=${v} nachher=${nachher[i][1]}`);
    } else {
      pruef(`${id}: gefuellt, nach Sprachwechsel unveraendert`, nachher[i][1] === v, `vorher=${v.slice(0, 60)} nachher=${nachher[i][1].slice(0, 60)}`);
    }
  }
  const platzEn = await b.js(`document.getElementById("local-app-folder-note").textContent`);
  pruef("Platzhalter ohne Fuellung wird uebersetzt", platzEn !== platz[0] && platzEn === await b.js(`translate(${JSON.stringify(platz[1])})`), `${platz[0]} -> ${platzEn}`);
  const knopf = await b.js(`(() => { const e = document.getElementById("update-host-save-button"); return [e.textContent, e.dataset.restoreText]; })()`);
  pruef("Schaltflaeche weiter uebersetzt", knopf[0] === knopf[1] && knopf[0] === await b.js(`translate("maintenance.save_update_host")`), JSON.stringify(knopf));

  // Rueckweg in den Ladezustand, waehrend Englisch aktiv ist, dann zurueck nach Deutsch
  await b.js(`(() => {
    updateStm32Log({ lines: [] });
    document.getElementById("fs-preview-content").textContent = translate("maintenance.preview_placeholder");
    return true;
  })()`);
  await b.js(`(async () => { await requestLanguage("de", false); return true; })()`);
  const rueck = await b.js(`[document.getElementById("stm32-log-output").textContent, I18N_DE["system.logs_empty"],
    document.getElementById("fs-preview-content").textContent, I18N_DE["maintenance.preview_placeholder"]]`);
  pruef("Logfenster wieder im Ladezustand -> nach Wechsel Deutsch", rueck[0] === rueck[1], JSON.stringify(rueck));
  pruef("Dateivorschau wieder Platzhalter -> nach Wechsel Deutsch", rueck[2] === rueck[3], JSON.stringify(rueck));
  // und noch einmal hin: (b) greift
  await b.js(`(async () => { await requestLanguage("en", false); return true; })()`);
  const hin = await b.js(`[document.getElementById("stm32-log-output").textContent, translate("system.logs_empty")]`);
  pruef("zweiter Wechsel: Platzhalter folgt weiter", hin[0] === hin[1], JSON.stringify(hin));

  // ------------------------------------------------------------------ Umschaltknoepfe
  console.log("Umschaltknoepfe beim Sprachwechsel");
  const TOGGLES = {
    "display-it-is-button": ["display.keep_it_is_disable", "display.keep_it_is"],
    "auto-brightness-button": ["climate.disable_auto_brightness", "climate.enable_auto_brightness"],
    "network-summertime-button": ["network.summertime_disable", "network.summertime"],
    "display-use-rgbw-button": ["display.use_rgbw_disable", "display.use_rgbw"],
    "sync-ambilight-button": ["display.unsync_ambilight", "display.sync_ambilight"],
    "sync-markers-button": ["display.unsync_markers", "display.sync_markers"],
    "fade-clock-seconds-button": ["display.fade_clock_seconds_disable", "display.fade_clock_seconds"],
    "ambilight-markers-button": ["display.ambilight_markers_disable", "display.ambilight_markers"],
  };
  // Zaehlung im Quelltext: jede Aufrufstelle muss in der Liste oben stehen
  const quelle = await b.js(`fetch("/app/app.js").then((r) => r.text())`);
  const toggleIds = [...quelle.matchAll(/setActionToggleButton\("([^"]+)"/g)].map((m) => m[1]);
  console.log("       Aufrufstellen setActionToggleButton: " + toggleIds.length + " -> " + toggleIds.join(", "));
  pruef("Aufrufstellen == 8 und alle in der Probe", toggleIds.length === 8 && toggleIds.every((id) => TOGGLES[id]), JSON.stringify(toggleIds));

  const umschalt = await b.js(`(async () => {
    const T = ${JSON.stringify(TOGGLES)};
    await requestLanguage("en", false); await requestLanguage("de", false);   // beide Tabellen geladen
    const setze = (id, an) => setActionToggleButton.length >= 5
      ? setActionToggleButton(id, translate(T[id][0]), translate(T[id][1]), an)       // alter Stand
      : setActionToggleButton(id, an);
    const ids = Object.keys(T);
    ids.forEach((id, i) => setze(id, i % 2 === 0));                                  // abwechselnd an/aus
    const besetzt = document.getElementById("sync-markers-button");                 // Index 5 -> aus
    besetzt.classList.add("is-busy"); besetzt.textContent = "BUSY";
    const vorAbrufe = window.__calls.filter((u) => u.indexOf("/api/") >= 0).length;
    requestLanguage("en", false);                                                  // Tabelle geladen -> synchron
    const nachAbrufe = window.__calls.filter((u) => u.indexOf("/api/") >= 0).length;
    const en = ids.map((id, i) => { const e = document.getElementById(id);
      return { id, an: i % 2 === 0, text: e.textContent, restore: e.dataset.restoreText, isOn: e.classList.contains("is-on"),
               soll: translate(T[id][i % 2 === 0 ? 0 : 1]) }; });
    besetzt.classList.remove("is-busy");
    requestLanguage("de", false);
    const de = ids.map((id, i) => { const e = document.getElementById(id);
      return { id, text: e.textContent, restore: e.dataset.restoreText, soll: translate(T[id][i % 2 === 0 ? 0 : 1]) }; });
    return { apiAbrufe: nachAbrufe - vorAbrufe, lang: currentLanguage, en, de };
  })()`);
  pruef("Sprachwechsel loest keine Geraeteanfrage aus", umschalt.apiAbrufe === 0, String(umschalt.apiAbrufe));
  for (const r of umschalt.en) {
    if (r.id === "sync-markers-button") {
      pruef(`${r.id} (laeuft gerade): Text bleibt, restoreText EN`, r.text === "BUSY" && r.restore === r.soll, JSON.stringify(r));
      continue;
    }
    pruef(`${r.id} ${r.an ? "an " : "aus"} -> EN: Text und restoreText passen zum Zustand, is-on ${r.an ? "bleibt" : "fehlt"}`,
      r.text === r.soll && r.restore === r.soll && r.isOn === r.an, JSON.stringify(r));
  }
  for (const r of umschalt.de) {
    if (r.id === "sync-markers-button") {
      continue;
    }
    pruef(`${r.id} -> zurueck DE`, r.text === r.soll && r.restore === r.soll, JSON.stringify(r));
  }

  // ------------------------------------------------------------------ Umschalten: Texte und Fehlerpfad
  console.log("Umschalten: Rueckmeldetexte und Fehlerpfad");
  const fehlerpfad = await b.js(`(async () => {
    const T = ${JSON.stringify(TOGGLES)};
    const setze = (id, an) => setActionToggleButton.length >= 5
      ? setActionToggleButton(id, translate(T[id][0]), translate(T[id][1]), an)
      : setActionToggleButton(id, an);
    const warte = (ms) => new Promise((r) => window.setTimeout(r, ms));
    const out = {};
    requestLanguage("en", false);

    // toggleFlagButton: Fehlschlag, Zustand bleibt "an"
    setze("fade-clock-seconds-button", true);
    window.__reject["/api/fade_clock_seconds_set"] = { ok: false, error: 3, detail: "busy" };
    const lauf = toggleFlagButton("fade-clock-seconds-button", getFadeClockSecondsSetUrl());
    const fb = document.getElementById("fade-clock-seconds-button");
    out.flagBusy = fb.textContent;
    await lauf;
    out.flagFehler = fb.textContent;
    await warte(1700);
    out.flagDanach = fb.textContent;
    out.flagState = fb.dataset.state;
    out.flagSoll = translate(T["fade-clock-seconds-button"][fb.dataset.state === "on" ? 0 : 1]);
    out.flagIsOn = fb.classList.contains("is-on") === (fb.dataset.state === "on");
    delete window.__reject["/api/fade_clock_seconds_set"];

    // runStateToggleButton: Fehlschlag, Zustand bleibt "aus"
    setze("display-it-is-button", false);
    window.__reject["/api/display_it_is_set"] = { ok: false, error: 3, detail: "busy" };
    window.__reject["/api/display_permanent_it_is_set"] = { ok: false, error: 3, detail: "busy" };
    const ib = document.getElementById("display-it-is-button");
    const lauf2 = togglePermanentItIs();
    out.stateBusy = ib.textContent;
    await lauf2;
    await warte(1700);
    out.stateDanach = ib.textContent;
    out.stateSoll = translate(T["display-it-is-button"][ib.dataset.state === "on" ? 0 : 1]);
    out.stateAbgewiesen = window.__calls.some((u) => /it_is/.test(u));
    delete window.__reject["/api/display_it_is_set"];
    delete window.__reject["/api/display_permanent_it_is_set"];

    // Erfolg ueber toggleFlagButton: Rueckmeldetext in der aktiven Sprache
    setze("sync-markers-button", false);
    const sb = document.getElementById("sync-markers-button");
    const lauf3 = toggleFlagButton("sync-markers-button", getSyncMarkersSetUrl());
    out.okBusy = sb.textContent;
    await lauf3;
    out.okText = sb.textContent;
    out.okSollEines = [translate("flags.enabled"), translate("flags.disabled")];
    out.running = translate("common.running");
    out.error = translate("common.error");
    requestLanguage("de", false);
    return out;
  })()`);
  pruef("toggleFlagButton: Busy-Text englisch", fehlerpfad.flagBusy === fehlerpfad.running, JSON.stringify(fehlerpfad.flagBusy));
  pruef("toggleFlagButton: Fehlertext englisch", fehlerpfad.flagFehler === fehlerpfad.error, JSON.stringify(fehlerpfad.flagFehler));
  // Soll aus data-state am Ende: Das Polling der Vorschau kann den Zustand in den 1,7 s neu
  // setzen (Attrappe meldet "aus"); der Knopf muss dann diesem Zustand folgen, nicht dem Busy-Text.
  pruef("toggleFlagButton: nach Fehler zurueck auf Beschriftung des Zustands", fehlerpfad.flagDanach === fehlerpfad.flagSoll && fehlerpfad.flagIsOn, JSON.stringify(fehlerpfad));
  pruef("toggleFlagButton: Erfolgstext englisch", fehlerpfad.okSollEines.includes(fehlerpfad.okText), JSON.stringify([fehlerpfad.okBusy, fehlerpfad.okText]));
  pruef("runStateToggleButton: Abruf wurde abgewiesen (Probe greift)", fehlerpfad.stateAbgewiesen, JSON.stringify(fehlerpfad));
  pruef("runStateToggleButton: nach Fehler zurueck auf Beschriftung des Zustands", fehlerpfad.stateDanach === fehlerpfad.stateSoll, JSON.stringify(fehlerpfad));

  // ------------------------------------------------------------------ Power-Knoepfe der Startseite
  console.log("Power-Knoepfe: Sprachwechsel und Fehlerpfad");
  const power = await b.js(`(async () => {
    const warte = (ms) => new Promise((r) => window.setTimeout(r, ms));
    const dp = document.getElementById("display-power"), ap = document.getElementById("ambilight-power");
    const db = document.getElementById("display-toggle-button"), ab = document.getElementById("ambilight-toggle-button");
    const rendere = () => {   // wie loadData: erst Overview, dann Knoepfe; Signatur alt (2 Param.) oder neu (1)
      updateDisplayButton(dp.dataset.state);
      if (updateAmbilightButton.length >= 2) { updateAmbilightButton(ap.dataset.state, true); } else { updateAmbilightButton(true); }
    };
    const soll = () => ({
      d: translate(dp.dataset.state === "on" ? "main.display_turn_off" : "main.display_turn_on"),
      a: translate(ap.dataset.state === "on" ? "main.ambilight_turn_off" : "main.ambilight_turn_on")
    });
    const out = {};
    requestLanguage("de", false);
    dp.dataset.state = "on"; ap.dataset.state = "off"; rendere();
    out.deVorher = [db.textContent, ab.textContent, soll()];
    const vor = window.__calls.filter((u) => u.indexOf("/api/") >= 0).length;
    requestLanguage("en", false);
    out.apiAbrufe = window.__calls.filter((u) => u.indexOf("/api/") >= 0).length - vor;
    out.en = [db.textContent, db.dataset.restoreText, ab.textContent, ab.dataset.restoreText, soll()];
    requestLanguage("de", false);
    out.de = [db.textContent, ab.textContent, soll()];

    // Fehlerpfad, englisch, Display-Knopf
    requestLanguage("en", false);
    window.__reject["/api/display_power_set"] = { ok: false, error: 3, detail: "busy" };
    const lauf = toggleDisplayPower();
    out.busy = db.textContent;
    await lauf;
    await warte(1700);
    delete window.__reject["/api/display_power_set"];
    out.fehlerDanach = db.textContent;
    out.fehlerSoll = soll().d;
    out.running = translate("common.running");
    out.abgewiesen = window.__calls.some((u) => u.indexOf("/api/display_power_set") >= 0);

    // Fehlerpfad, Ambilight-Knopf
    window.__reject["/api/ambilight_power_set"] = { ok: false, error: 3, detail: "busy" };
    await toggleAmbilightPower();
    await warte(1700);
    delete window.__reject["/api/ambilight_power_set"];
    out.ambiDanach = ab.textContent;
    out.ambiSoll = soll().a;
    requestLanguage("de", false);
    return out;
  })()`);
  pruef("Power: Ausgangslage DE stimmt (Probe greift)", power.deVorher[0] === power.deVorher[2].d && power.deVorher[1] === power.deVorher[2].a, JSON.stringify(power.deVorher));
  pruef("Power: Sprachwechsel ohne Geraeteanfrage", power.apiAbrufe === 0, String(power.apiAbrufe));
  pruef("Power: nach DE->EN Beschriftung und restoreText nach Zustand",
    power.en[0] === power.en[4].d && power.en[1] === power.en[4].d && power.en[2] === power.en[4].a && power.en[3] === power.en[4].a, JSON.stringify(power.en));
  pruef("Power: zurueck DE nach Zustand", power.de[0] === power.de[2].d && power.de[1] === power.de[2].a, JSON.stringify(power.de));
  pruef("Power: Abruf wurde abgewiesen (Probe greift)", power.abgewiesen && power.busy === power.running, JSON.stringify([power.abgewiesen, power.busy]));
  pruef("Power Display: nach Fehler zurueck auf Beschriftung des Zustands", power.fehlerDanach === power.fehlerSoll, JSON.stringify([power.fehlerDanach, power.fehlerSoll]));
  pruef("Power Ambilight: nach Fehler zurueck auf Beschriftung des Zustands", power.ambiDanach === power.ambiSoll, JSON.stringify([power.ambiDanach, power.ambiSoll]));

  // ------------------------------------------------------------------ B33 / L246 und B26 / L221
  console.log("B33 / L246 Binaersperre, B26 / L221 tote Funktion");
  const fsprobe = await b.js(`(async () => {
    requestLanguage("de", false);
    const gzip = [0x1f, 0x8b, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x61, 0x00, 0xff, 0xfe, 0x41, 0x42];
    window.__respond = {
      "app.js.gz": { bytes: gzip },
      "unbekannt.dat": { bytes: gzip },
      "ohnegroesse.gz": { bytes: gzip },
      "leer.gz": { bytes: [] },
      "latin1.txt": { bytes: [0x5a, 0xfc, 0x72, 0x69, 0x63, 0x68] },        // "Zuerich" in ISO-8859-1
      "text.txt": { bytes: [0x48, 0x61, 0x6c, 0x6c, 0x6f] }
    };
    const pv = () => document.getElementById("fs-preview-content").textContent;
    const fsShowCalls = (name) => window.__calls.filter((u) => u.indexOf("fs_show") >= 0 && u.indexOf(name) >= 0).length;
    const out = {};
    const fall = async (name, size) => {
      window.__status.length = 0;
      const vor = fsShowCalls(name);
      await showFsFile(name, size);
      return { text: pv(), abrufe: fsShowCalls(name) - vor, status: window.__status.slice(-1)[0] || null };
    };
    out.gz = await fall("app.js.gz", 96013);
    out.dat = await fall("unbekannt.dat", 16);
    out.ohneGroesse = await fall("ohnegroesse.gz", undefined);
    out.leer = await fall("leer.gz", 0);
    out.latin1 = await fall("latin1.txt", 6);
    out.text = await fall("text.txt", 5);
    out.sollBinaer = translateFormat("maintenance.preview_binary", { size: formatBytes(96013) });
    out.sollLeer = translate("maintenance.preview_empty");

    // Echter Klick ueber die gerenderte Dateiliste: kommt die Groesse aus fs_list an?
    try {
      renderFileSystem({}, [{ name: "app.js.gz", size: 96013 }, { name: "text.txt", size: 5 }], getCurrentSettingsSnapshot());
      const knopf = document.querySelector('[data-fs-show="app.js.gz"]');
      const vor = fsShowCalls("app.js.gz");
      knopf.click();
      await new Promise((r) => window.setTimeout(r, 300));
      out.klick = { text: pv(), abrufe: fsShowCalls("app.js.gz") - vor };
    } catch (e) {
      out.klick = { fehler: String(e && e.message || e) };
    }
    delete window.__respond;
    out.formatTot = typeof formatLittleFs;
    out.formatLebt = typeof formatLittleFsFromFiles;
    return out;
  })()`);
  const binaer = (r) => !/[\u0000\u001f\ufffd]/.test(r.text) && !r.text.startsWith("\u001f");
  pruef("B33 .gz mit Groesse: kein Abruf, kein Binaerinhalt, Meldung mit Groesse", fsprobe.gz.abrufe === 0 && fsprobe.gz.text === fsprobe.sollBinaer, JSON.stringify(fsprobe.gz));
  pruef("B33 .gz: Status ist keine Erfolgsmeldung 'wird angezeigt'", fsprobe.gz.status && fsprobe.gz.status[0] !== "ok" && /Binärdatei/.test(fsprobe.gz.status[1]), JSON.stringify(fsprobe.gz.status));
  pruef("B33 unbekannte Endung mit NUL-Bytes: Inhaltsnetz greift", binaer(fsprobe.dat) && /Binärdatei, 16 Bytes/.test(fsprobe.dat.text), JSON.stringify(fsprobe.dat));
  pruef("B33 .gz ohne bekannte Groesse: abgerufen, Inhaltsnetz greift", fsprobe.ohneGroesse.abrufe === 1 && binaer(fsprobe.ohneGroesse) && /Binärdatei/.test(fsprobe.ohneGroesse.text), JSON.stringify(fsprobe.ohneGroesse));
  pruef("B33 leere .gz ist kein Binaerfall: 'leer'", fsprobe.leer.text === fsprobe.sollLeer && fsprobe.leer.abrufe === 1, JSON.stringify(fsprobe.leer));
  pruef("B33 ISO-8859-1-Text bleibt Text", !/Binärdatei/.test(fsprobe.latin1.text) && fsprobe.latin1.text.length === 6, JSON.stringify(fsprobe.latin1));
  pruef("B33 normaler Text unveraendert angezeigt", fsprobe.text.text === "Hallo" && fsprobe.text.status && fsprobe.text.status[0] === "ok", JSON.stringify(fsprobe.text));
  pruef("B33 Klick in der Dateiliste: Groesse aus fs_list kommt an, kein Abruf", fsprobe.klick.abrufe === 0 && /Binärdatei/.test(fsprobe.klick.text || ""), JSON.stringify(fsprobe.klick));
  pruef("B26 formatLittleFs entfernt, formatLittleFsFromFiles da", fsprobe.formatTot === "undefined" && fsprobe.formatLebt === "function", JSON.stringify([fsprobe.formatTot, fsprobe.formatLebt]));

  // B27 Wortlaut
  console.log("B27 / L223");
  const fade = await b.js(`[translate("display.fade_clock_seconds"), translate("display.fade_clock_seconds_disable"), I18N_DE["display.fade_clock_seconds"], I18N_DE["display.fade_clock_seconds_disable"]]`);
  console.log("       " + JSON.stringify(fade));

  const ausnahmen = await b.js(`window.__pageErrors || []`);
  pruef("keine Seitenfehler waehrend des Laufs", ausnahmen.length === 0, JSON.stringify(ausnahmen));
} catch (e) {
  fail++;
  console.log("  FAIL Ausnahme: " + e.message.slice(0, 500));
} finally {
  b.ende();
}
console.log(`=== ${ok} OK, ${fail} FAIL`);
process.exit(fail ? 1 : 0);
