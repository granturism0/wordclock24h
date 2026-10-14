// Pruefstand c65q (Paket 2026-10-13, E4.6/E4.7; AKV.3, AKV.6, AKV.7, AKV.8, AKV.9).
//
//   node c65q.mjs <app-verzeichnis> <port> [bezeichnung]
//
// Startet die Vorschau (tools/preview/server.py) HINTER einem eigenen Proxy. Der Proxy steht fuer
// das Geraet: Er verzoegert /api/-Antworten steuerbar (je Endpunkt, je Anfrage ueber __d=ms) und
// zaehlt jede /api/-Verbindung mit Beginn, Ende und Abbruch. Gleichzeitigkeit wird dort gemessen,
// wo sie dem ESP schadet -- an der Gegenstelle, nicht im Browser. Dazu ein headless Chrome ueber
// CDP; die Faelle rufen die Originalfunktionen der PWA auf, nichts ist abgeschrieben.
//
// Faelle (Soll fuer E4; gegen PWA 1.4.99 muss jeder mit [alt] markierte FEHL melden):
//   S1  CONNECTION_STABILITY gleich der Tabelle design-e4.md §19.6 (16 Schluessel)        [alt]
//   S2  loadCoreData ruft weder display_power noch ambilight_power                          [alt]
//   D1  Startphase: bis zum Ende der ersten Kernladung nie mehr als 1 /api/ gleichzeitig     [alt]
//   D8a display_power/ambilight_power laufen ab (6 s) => Anzeige trotzdem "an" (settings_xml)  [alt]
//   D2  6 Lesende auf einmal => hoechstens 2 gleichzeitig, alle 6 gesendet                   [alt]
//   D3  Schreibende vor Lesenden, je Klasse in Ankunftsreihenfolge                           [alt]
//   D4  ueberholter Lauf: update_status nicht gesendet, Zeitstempel nicht gesetzt (auch nicht
//       waehrend des Wartens)                                                               [alt]
//   D10 lesender Abruf nach Fristablauf nicht wiederholt (attempts 2)                        [alt]
//   D11 update_progress-Taktgeber: hoechstens 1 offen, kein Stau                             [alt]
//   D9  Ladesperre bis zum Ende der Nebenabrufe: Selbstaktualisierung startet keinen Lauf    [alt]
//   D6  Frist ab Senden: 10 s Wartezeit + 1 s Netz bei Frist 3'200 ms => echte Antwort       [alt]
//   D5  Lesender nach 15 s Wartezeit ohne Senden verworfen, Schreibender nie                 [alt]
//   D8b settings_xml ohne numvar 3 => Anzeige nie "aus"
//   D7  network_scan: Start mit Netzwerkmodul 1, 4 Takte auto 0, erneuter Klick aufs offene
//       Modul 0, Wechsel hinein 1, Knopf 1, Knopf mit Fristablauf 1 ohne Wiederholung, alte
//       Liste bleibt, Knopf meldet Fehler                                                     [alt]
//   D12 Server erneut pruefen (E4.5a): genau 1 update_status?refresh=1, danach genau 1 Nachlade-
//       Abruf 8-10 s nach dem Neuladen, keine Wiederholung                                  [alt]
//   G1  ueber den ganzen Lauf nie mehr als 2 /api/ gleichzeitig                               [alt]
//   G2  ueber den ganzen Lauf 0 Abrufe display_power/ambilight_power                          [alt]
// Exit 0 OK, 1 FEHL, 2 Werkzeugfehler.
import { spawn } from "node:child_process";
import http from "node:http";
import { mkdtempSync, readFileSync, rmSync } from "node:fs";
import { tmpdir } from "node:os";
import path from "node:path";

const [appDir, port = "8731", bez = "PWA"] = process.argv.slice(2);
const ROOT = process.env.WORDCLOCK_ROOT;
const CHROME = process.env.CHROME || "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome";
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const quelle = readFileSync(path.join(appDir, "app.js"), "utf8");
const fehler = [];
let faelle = 0;
const pruefe = (name, ok, info = "") => {
  faelle++;
  console.log(`  ${ok ? "OK  " : "FEHL"} ${name}${info ? ": " + info : ""}`);
  if (!ok) fehler.push(name);
};

// ---------------------------------------------------------------- statisch
const SOLL = {
  coreSettingsTimeoutMs: 4200, corePowerTimeoutMs: 3200, updateStatusTimeoutMs: 5500, updateTableInfoTimeoutMs: 5500,
  networkScanTimeoutMs: 4800, overlayIconsTimeoutMs: 3200, fsInfoTimeoutMs: 6500, eepromSettingsTimeoutMs: 6500,
  fsListTimeoutMs: 7500, stm32LogTimeoutMs: 4800, progressPollTimeoutMs: 2800, frameProbeTimeoutMs: 3500,
  apiWriteTimeoutMs: 6500, fastReadAttempts: 1, slowReadAttempts: 2, retryDelayMs: 220, progressPollIntervalMs: 900
};
{
  const m = /^const CONNECTION_STABILITY = \{([\s\S]*?)^\};/m.exec(quelle);
  const ist = {};
  if (m) for (const z of m[1].matchAll(/^\s*(\w+):\s*(\d+)/gm)) ist[z[1]] = Number(z[2]);
  const abw = Object.keys(SOLL).filter((k) => ist[k] !== SOLL[k]).map((k) => `${k}=${ist[k]} (Soll ${SOLL[k]})`);
  const fremd = Object.keys(ist).filter((k) => !(k in SOLL));
  pruefe("S1 CONNECTION_STABILITY gleich Tabelle §19.6", !!m && abw.length === 0 && fremd.length === 0,
    `${Object.keys(ist).length} Schluessel gelesen, Soll ${Object.keys(SOLL).length}` + (abw.length ? "; " + abw.join(", ") : "") + (fremd.length ? "; unbekannt " + fremd.join(", ") : ""));
  const lc = /^async function loadCoreData\([^)]*\) \{[\s\S]*?^\}/m.exec(quelle);
  const verboten = lc ? (lc[0].match(/getDisplayPowerUrl|getAmbilightPowerUrl|display_power|ambilight_power/g) || []) : ["loadCoreData fehlt"];
  pruefe("S2 loadCoreData ohne display_power/ambilight_power", !!lc && verboten.length === 0, verboten.join(", "));
}

// ---------------------------------------------------------------- Proxy (das "Geraet")
const pyPort = Number(port) + 1;
const st = { inFlight: 0, max: 0, log: [], defaultDelay: 0, epDelay: {}, strip3: false, scanSeq: 0 };
// Unabhaengig von reset: Hoechstwert und Stromabrufe ueber den ganzen Lauf (G1, G2).
const gesamt = { max: 0, power: 0, api: 0 };
function forward(req, res, rewrite) {
  const up = http.request({ host: "127.0.0.1", port: pyPort, path: req.url, method: req.method, headers: req.headers }, (r) => {
    if (!rewrite) {
      res.writeHead(r.statusCode, r.headers);
      r.pipe(res);
      return;
    }
    const teile = [];
    r.on("data", (c) => teile.push(c));
    r.on("end", () => {
      const body = Buffer.from(rewrite(Buffer.concat(teile).toString("utf8")), "utf8");
      const h = { ...r.headers, "content-length": String(body.length) };
      res.writeHead(r.statusCode, h);
      res.end(body);
    });
  });
  up.on("error", () => { try { res.writeHead(502); res.end("upstream"); } catch {} });
  req.pipe(up);
}
const proxy = http.createServer((req, res) => {
  const u = new URL(req.url, "http://x");
  if (u.pathname === "/__ctl") {
    // clear ZUERST: sonst loescht "clear=1&ep=..." die eben gesetzte Verzoegerung wieder.
    if (u.searchParams.has("clear")) st.epDelay = {};
    if (u.searchParams.has("reset")) { st.log = []; st.max = 0; }
    if (u.searchParams.has("default")) st.defaultDelay = Number(u.searchParams.get("default"));
    if (u.searchParams.has("ep")) st.epDelay[u.searchParams.get("ep")] = Number(u.searchParams.get("delay") || 0);
    if (u.searchParams.has("strip3")) st.strip3 = u.searchParams.get("strip3") === "1";
    res.end("ok");
    return;
  }
  if (u.pathname === "/__blank") { res.writeHead(200, { "content-type": "text/html" }); res.end("<!doctype html><title>c65q</title>"); return; }
  // Kein Service Worker: Er wuerde die Seite mitten im Fall neu laden (controllerchange).
  // /api/ laeuft ohnehin an ihm vorbei (sw.js: nur /app/).
  if (u.pathname === "/app/sw.js") { res.writeHead(404); res.end("no sw in c65q"); return; }
  if (!u.pathname.startsWith("/api/")) { forward(req, res); return; }
  const name = u.pathname.slice(5);
  const e = { name, q: u.search, start: Date.now(), end: 0, aborted: false, conc: 0 };
  st.log.push(e);
  st.inFlight++;
  e.conc = st.inFlight;
  st.max = Math.max(st.max, st.inFlight);
  gesamt.max = Math.max(gesamt.max, st.inFlight);
  gesamt.api++;
  if (name === "display_power" || name === "ambilight_power") gesamt.power++;
  let done = false;
  const fertig = (abgebrochen) => { if (done) return; done = true; e.end = Date.now(); e.aborted = abgebrochen; st.inFlight--; };
  res.on("finish", () => fertig(false));
  res.on("close", () => fertig(!res.writableFinished));
  const d = u.searchParams.has("__d") ? Number(u.searchParams.get("__d")) : (st.epDelay[name] ?? st.defaultDelay);
  setTimeout(() => {
    if (done) return;
    if (name === "network_scan") {
      st.scanSeq++;
      const body = JSON.stringify({ ok: true, networks: [{ ssid: "C65Q-" + st.scanSeq, rssi: -50, encryption: 4 }] });
      res.writeHead(200, { "content-type": "application/json", "cache-control": "no-cache" });
      res.end(body);
      return;
    }
    if (name === "settings_xml" && st.strip3) {
      forward(req, res, (t) => t.replace(/<numvar idx="3" value="[^"]*" \/>/, "").replace(/<numvar idx="30" value="[^"]*" \/>/, ""));
      return;
    }
    forward(req, res);
  }, d);
});
const statistik = () => st;
// Hoechste Ueberlappung einer Teilmenge (Sweep ueber Beginn/Ende).
function maxUeberlappung(eintraege) {
  const p = [];
  for (const e of eintraege) { p.push([e.start, 1]); p.push([e.end || Date.now(), -1]); }
  p.sort((a, b) => a[0] - b[0] || a[1] - b[1]);
  let n = 0, m = 0;
  for (const [, d] of p) { n += d; m = Math.max(m, n); }
  return m;
}
const mitMarke = (marke) => st.log.filter((e) => e.q.includes("c65q=" + marke));

// ---------------------------------------------------------------- Vorschau, Proxy, Chrome
const prof = mkdtempSync(path.join(tmpdir(), "c65q-"));
const py = spawn("python3", [path.join(ROOT, "tools/preview/server.py"), String(pyPort)], { env: { ...process.env, PREVIEW_APP: path.resolve(appDir) }, stdio: "ignore" });
await new Promise((r) => proxy.listen(Number(port), "127.0.0.1", r));
const cdp = String(Number(port) + 1000);
const chrome = spawn(CHROME, ["--headless=new", "--remote-debugging-port=" + cdp, "--user-data-dir=" + prof, "about:blank"], { stdio: "ignore" });
const ende = (code) => {
  try { chrome.kill("SIGKILL"); } catch {}
  try { py.kill("SIGKILL"); } catch {}
  try { proxy.close(); } catch {}
  try { rmSync(prof, { recursive: true, force: true }); } catch {}
  process.exit(code);
};
let seiten;
for (let i = 0; i < 60; i++) { try { seiten = await (await fetch("http://127.0.0.1:" + cdp + "/json/list")).json(); if (seiten.length) break; } catch {} await sleep(200); }
const seite = seiten && seiten.find((t) => t.type === "page");
if (!seite) { console.log("KAPUTT c65q " + bez + ": Chrome ohne Debug-Port"); ende(2); }
const ws = new WebSocket(seite.webSocketDebuggerUrl);
await new Promise((r) => (ws.onopen = r));
let nr = 0; const offen = new Map();
ws.onmessage = (m) => { const d = JSON.parse(m.data); if (d.id && offen.has(d.id)) { offen.get(d.id)(d); offen.delete(d.id); } };
const send = (method, params = {}) => new Promise((r) => { const i = ++nr; offen.set(i, r); ws.send(JSON.stringify({ id: i, method, params })); });
const ev = async (expr) => {
  const r = await send("Runtime.evaluate", { expression: expr, awaitPromise: true, returnByValue: true });
  if (!r.result) throw new Error("CDP: " + JSON.stringify(r.error || r).slice(0, 200));
  if (r.result.exceptionDetails) throw new Error(r.result.exceptionDetails.exception ? r.result.exceptionDetails.exception.description : JSON.stringify(r.result.exceptionDetails));
  return r.result.result.value;
};
const ctl = (q) => fetch(`http://127.0.0.1:${port}/__ctl?${q}`).then((r) => r.text());
// Netzruhe: 1'500 ms ohne offene und ohne neue /api/-Verbindung, hoechstens maxMs.
async function ruhe(maxMs = 30000) {
  const bis = Date.now() + maxMs;
  let still = 0;
  while (Date.now() < bis) {
    const letzte = st.log.length ? Math.max(...st.log.map((e) => e.end || Date.now()), ...st.log.map((e) => e.start)) : 0;
    if (st.inFlight === 0 && Date.now() - letzte >= 1500) { still++; if (still >= 2) return true; } else still = 0;
    await sleep(250);
  }
  return false;
}
async function oeffne(modul) {
  await send("Page.navigate", { url: `http://127.0.0.1:${port}/__blank` });
  await sleep(400);
  await ev(`localStorage.clear(); localStorage.setItem("wordclock-app-active-module", ${JSON.stringify(modul)}); true`);
  await send("Page.navigate", { url: `http://127.0.0.1:${port}/app/` });
  for (let i = 0; i < 80; i++) {
    await sleep(250);
    try { if (await ev("typeof getCurrentSettingsSnapshot === 'function' && !!getCurrentSettingsSnapshot()")) return true; } catch {}
  }
  return false;
}
const fall = async (name, fn) => { try { await fn(); } catch (e) { pruefe(name + " lief durch", false, String(e && e.message || e).slice(0, 300)); } };

try {
  // ---------------- D1 Startphase und D8a Displaystrom
  await ctl("reset=1&clear=1&default=300&ep=display_power&delay=6000");
  await ctl("ep=ambilight_power&delay=6000");
  const navStart = Date.now();
  if (!(await oeffne("main"))) { console.log("KAPUTT c65q " + bez + ": PWA in der Vorschau nicht geladen"); ende(2); }
  await ruhe(30000);
  await fall("D1 Startphase", async () => {
    const ersteKern = st.log.find((e) => e.name === "settings_xml" && e.start >= navStart);
    const bisKern = st.log.filter((e) => e.start >= navStart && ersteKern && e.start <= ersteKern.end);
    const m = maxUeberlappung(bisKern);
    pruefe("D1 Startphase: bis zum Ende der ersten Kernladung hoechstens 1 /api/ gleichzeitig", !!ersteKern && m <= 1,
      `${bisKern.length} Abrufe bis dahin (${bisKern.map((e) => e.name).join(", ")}), hoechstens ${m} gleichzeitig`);
  });
  await fall("D8a Displaystrom", async () => {
    const z = await ev(`({ d: document.getElementById("display-power").dataset.state, a: document.getElementById("ambilight-power").dataset.state })`);
    pruefe("D8a display_power/ambilight_power laufen ab, settings_xml sagt an => Anzeige an", z.d === "on", `display-power=${z.d}, ambilight-power=${z.a}`);
  });
  await ctl("clear=1&default=0");
  await ev("stopAlignedAutoRefresh(); true");

  // ---------------- D2 Grenze 2
  await fall("D2", async () => {
    await ctl("reset=1");
    const r = await ev(`Promise.all(Array.from({ length: 6 }, (_, i) => settleFetchJson("/api/fs_info?c65q=d2-" + i + "&__d=600", "FB", 4000))).then((a) => a.filter((x) => x !== "FB").length)`);
    await ruhe();
    const l = mitMarke("d2-");
    pruefe("D2 6 Lesende auf einmal: hoechstens 2 gleichzeitig, alle 6 gesendet und beantwortet", maxUeberlappung(l) <= 2 && l.length === 6 && r === 6,
      `gesendet ${l.length}, beantwortet ${r}, gleichzeitig hoechstens ${maxUeberlappung(l)}, gesamt hoechstens ${st.max}`);
  });

  // ---------------- D3 Schreibende vor Lesenden
  await fall("D3", async () => {
    await ctl("reset=1");
    await ev(`(async () => {
      const b = [0, 1].map((i) => apiFetch("/api/c65q_block_set?c65q=d3-b" + i + "&__d=1500"));
      const r = [1, 2, 3].map((i) => settleFetchJson("/api/fs_info?c65q=d3-r" + i, "FB", 4000));
      const w = [1, 2].map((i) => apiFetch("/api/c65q_x_set?c65q=d3-w" + i));
      await Promise.all([...b, ...r, ...w]);
      return true;
    })()`);
    await ruhe();
    const folge = st.log.filter((e) => /c65q=d3-[rw]/.test(e.q)).map((e) => /c65q=(d3-[rw]\d)/.exec(e.q)[1]);
    pruefe("D3 Schreibende vor Lesenden, je Klasse in Reihenfolge", folge.join(",") === "d3-w1,d3-w2,d3-r1,d3-r2,d3-r3", "Ankunft " + folge.join(","));
  });

  // ---------------- D4 ueberholter Lauf, Zeitstempel erst beim Senden
  await fall("D4", async () => {
    await ctl("reset=1");
    const r = await ev(`(async () => {
      const b = [0, 1].map((i) => apiFetch("/api/c65q_block_set?c65q=d4-b" + i + "&__d=2000"));
      await new Promise((x) => setTimeout(x, 100));
      lastUpdateStatusFetchAt = 0;
      lastUpdateTableFilesFetchAt = 0;
      const p = loadSecondaryData(loadRequestSerial, getCurrentSettingsSnapshot(), { displayPower: "on", ambilightPower: "on" }, getDebugOverrides(), { moduleName: "main" });
      await new Promise((x) => setTimeout(x, 150));
      const waehrend = lastUpdateStatusFetchAt;
      loadRequestSerial += 1;
      await Promise.all(b);
      await p;
      return { waehrend, danach: lastUpdateStatusFetchAt, tabelle: lastUpdateTableFilesFetchAt };
    })()`);
    await ruhe();
    const us = st.log.filter((e) => e.name === "update_status" || e.name === "update_table_files").length;
    pruefe("D4 ueberholter Lauf: update_status nicht gesendet, kein Zeitstempel", us === 0 && r.waehrend === 0 && r.danach === 0 && r.tabelle === 0,
      `gesendet update_status/update_table_files ${us}, Zeitstempel waehrend des Wartens ${r.waehrend}, danach ${r.danach}/${r.tabelle} (Soll je 0)`);
  });

  // ---------------- D10 keine Wiederholung lesender Abrufe nach Fristablauf
  await fall("D10", async () => {
    await ctl("reset=1");
    const r = await ev(`settleFetchJson("/api/fs_info?c65q=d10&__d=5000", "FB", 1500, 2)`);
    await sleep(2500);
    await ruhe();
    const l = mitMarke("d10");
    pruefe("D10 lesender Abruf nach Fristablauf nicht wiederholt", l.length === 1 && r === "FB", `gesendet ${l.length} (Soll 1), Ergebnis ${JSON.stringify(r)}`);
  });

  // ---------------- D11 Taktgeber update_progress
  await fall("D11", async () => {
    await ctl("reset=1&ep=update_progress&delay=2500");
    await ev("startUpdateProgressPolling(0); true");
    await sleep(3300);
    await ev("stopUpdateProgressPolling(); true");
    await ruhe();
    await ctl("clear=1");
    const l = st.log.filter((e) => e.name === "update_progress");
    pruefe("D11 update_progress-Takt: hoechstens 1 offen", maxUeberlappung(l) <= 1 && l.length <= 2,
      `${l.length} Abrufe in 3,3 s bei Takt 900 ms und Antwort nach 2,5 s, gleichzeitig hoechstens ${maxUeberlappung(l)}`);
  });

  // ---------------- D9 Ladesperre bis zum Ende der Nebenabrufe
  await fall("D9", async () => {
    await ctl("reset=1&ep=fs_list&delay=4000");
    const r = await ev(`(async () => {
      await loadData(buildLoadOptionsForModule("maintenance", {}));
      const gesperrt = activeLoadCount > 0;
      await refreshVisibleModuleData({ auto: true });
      return { gesperrt };
    })()`);
    await ruhe();
    await ctl("clear=1");
    const kern = st.log.filter((e) => e.name === "settings_xml").length;
    const fsl = st.log.filter((e) => e.name === "fs_list").length;
    pruefe("D9 Ladesperre bis zum Ende der Nebenabrufe", r.gesperrt && kern === 1 && fsl === 1,
      `Sperre nach der Kernladung ${r.gesperrt ? "gehalten" : "frei"}, settings_xml ${kern} (Soll 1), fs_list ${fsl}`);
  });

  // ---------------- D8b settings_xml ohne numvar 3/30
  await fall("D8b", async () => {
    await ctl("reset=1&strip3=1");
    const z = await ev(`(async () => { await loadData(); return { d: document.getElementById("display-power").dataset.state, t: document.getElementById("display-power").textContent, aus: translate("climate.status_off") }; })()`);
    await ruhe();
    await ctl("strip3=0");
    pruefe("D8b settings_xml ohne Displaystrom => nie \"aus\"", z.d !== "off" && z.t !== z.aus, `state=${z.d}, Text "${z.t}"`);
  });

  // ---------------- D6 Frist ab Senden
  await fall("D6", async () => {
    await ctl("reset=1");
    const r = await ev(`(async () => {
      const b = [0, 1].map((i) => apiFetch("/api/c65q_block_set?c65q=d6-b" + i + "&__d=10000", { timeoutMs: 15000 }));
      await new Promise((x) => setTimeout(x, 50));
      const t0 = Date.now();
      const v = await settleFetchJson("/api/fs_info?c65q=d6-r&__d=1000", "FB", 3200);
      await Promise.all(b);
      return { echt: v !== "FB", ms: Date.now() - t0 };
    })()`);
    await ruhe();
    const l = mitMarke("d6-r");
    const blockEnde = Math.min(...mitMarke("d6-b").map((e) => e.end));
    // Gewartet hat er nur, wenn er erst nach dem Ende eines Blockers beim Geraet ankam.
    const gewartet = l.length === 1 && l[0].start >= blockEnde - 50;
    pruefe("D6 Frist ab Senden: 10 s in der Schlange, 1 s Netz, Frist 3'200 ms => echte Antwort", r.echt && gewartet,
      `Antwort ${r.echt ? "echt" : "Rueckfallwert"} nach ${r.ms} ms, gesendet ${l.length}, ${gewartet ? "nach" : "VOR"} dem Ende der Blocker angekommen`);
  });

  // ---------------- D5 Lesender nach 15 s verworfen, Schreibender nie
  await fall("D5", async () => {
    await ctl("reset=1");
    const r = await ev(`(async () => {
      const b = [0, 1].map((i) => apiFetch("/api/c65q_block_set?c65q=d5-b" + i + "&__d=16500", { timeoutMs: 20000 }));
      await new Promise((x) => setTimeout(x, 50));
      const t0 = Date.now();
      let tr = 0;
      const lesend = settleFetchJson("/api/fs_info?c65q=d5-r", "FB", 4000).then((v) => { tr = Date.now() - t0; return v; });
      const schreibend = apiFetch("/api/c65q_x_set?c65q=d5-w").then(() => "ok", (e) => "fehler " + e.message);
      const v = await lesend;
      const w = await schreibend;
      await Promise.all(b);
      return { lesend: v === "FB" ? "FB" : "echt", tr, w };
    })()`);
    await ruhe();
    const lr = mitMarke("d5-r").length, lw = mitMarke("d5-w").length;
    pruefe("D5 Lesender nach 15 s ohne Senden verworfen, Schreibender gesendet", r.lesend === "FB" && lr === 0 && r.tr >= 14500 && r.tr <= 16500 && lw === 1 && r.w === "ok",
      `Lesender ${r.lesend} nach ${r.tr} ms, gesendet ${lr} (Soll 0); Schreibender ${r.w}, gesendet ${lw} (Soll 1)`);
  });

  // ---------------- D12 Server erneut pruefen: genau ein refresh=1, ein Nachlade-Abruf nach ~9 s
  await fall("D12", async () => {
    await ctl("reset=1&clear=1");
    const t0 = Date.now();
    const ton = await ev("refreshUpdateServerAvailability().then(() => lastAnnouncedTone)");
    const nachLaden = Date.now();
    await sleep(12000);
    await ruhe();
    const us = st.log.filter((e) => e.name === "update_status" && e.start >= t0);
    const mitRefresh = us.filter((e) => /[?&]refresh=1/.test(e.q));
    const nach = us.filter((e) => !/[?&]refresh=1/.test(e.q) && e.start >= nachLaden);
    const abstand = nach.length ? nach[0].start - nachLaden : -1;
    pruefe("D12 Server erneut pruefen: 1 x refresh=1, danach genau 1 Nachlade-Abruf nach 8-10 s, keine Wiederholung, Meldung ok",
      mitRefresh.length === 1 && nach.length === 1 && abstand >= 8000 && abstand <= 10500 && ton === "ok",
      `refresh=1 ${mitRefresh.length}, Nachlade-Abrufe ${nach.length} (erster ${abstand} ms nach dem Neuladen), Meldung ${ton}`);
  });

  // ---------------- D12b refresh=1 laeuft ab => Fehlermeldung, nicht "neu geprueft" (Review E4.10 H1)
  await fall("D12b", async () => {
    await ctl("reset=1&clear=1&ep=update_status&delay=7000");
    const ton = await ev("refreshUpdateServerAvailability().then(() => lastAnnouncedTone)");
    await sleep(11000);
    await ruhe();
    await ctl("clear=1");
    pruefe("D12b refresh=1 laeuft ab => Meldung Fehler", ton === "error", `Meldung ${ton}`);
  });

  // ---------------- D12c refresh=1 ist schreibend: vor wartenden Lesenden (Review E4.10 M1)
  await fall("D12c", async () => {
    await ctl("reset=1&clear=1");
    await ev(`(async () => {
      const b = [0, 1].map((i) => apiFetch("/api/c65q_block_set?c65q=d12c-b" + i + "&__d=1500"));
      const r = [1, 2].map((i) => settleFetchJson("/api/fs_info?c65q=d12c-r" + i, "FB", 4000));
      const u = refreshUpdateServerAvailability();
      await Promise.all([...b, ...r, u]);
      return true;
    })()`);
    await sleep(11000);
    await ruhe();
    const folge = st.log.filter((e) => /c65q=d12c-r/.test(e.q) || (e.name === "update_status" && /refresh=1/.test(e.q)))
      .map((e) => (e.name === "update_status" ? "refresh" : /c65q=(d12c-r\d)/.exec(e.q)[1]));
    pruefe("D12c refresh=1 vor wartenden Lesenden", folge[0] === "refresh", "Ankunft " + folge.join(","));
  });

  // ---------------- D7 WLAN-Scan
  await fall("D7", async () => {
    await ctl("reset=1&clear=1");
    const navNetz = Date.now();
    if (!(await oeffne("network"))) throw new Error("Netzwerkmodul nicht geladen");
    await ruhe();
    await ev("stopAlignedAutoRefresh(); true");
    const scans = () => st.log.filter((e) => e.name === "network_scan" && e.start >= navNetz).length;
    const start = scans();
    for (let i = 0; i < 4; i++) { await ev("refreshVisibleModuleData({ auto: true }).then(() => true)"); await ruhe(); }
    const auto = scans() - start;
    await ev(`setActiveModule("network"); true`); await ruhe();
    const nochmal = scans() - start - auto;
    await ev(`setActiveModule("main"); true`); await ruhe();
    const vorWechsel = scans();
    await ev(`setActiveModule("network"); true`); await ruhe();
    const wechsel = scans() - vorWechsel;
    const vorKnopf = scans();
    await ev("refreshNetworkScan().then(() => true)"); await ruhe();
    const knopf = scans() - vorKnopf;
    await ctl("ep=network_scan&delay=7000");
    const listeVor = await ev("JSON.stringify(getCurrentNetworkInfo())");
    const vorAblauf = scans();
    const k = await ev(`refreshNetworkScan().then(() => document.getElementById("network-scan-button").classList.contains("is-error"))`);
    await sleep(3000);
    await ruhe();
    await ctl("clear=1");
    const ablauf = scans() - vorAblauf;
    const listeNach = await ev("JSON.stringify(getCurrentNetworkInfo())");
    pruefe("D7a Start mit Netzwerkmodul: 1 Scan", start === 1, `${start}`);
    pruefe("D7b 4 Takte Selbstaktualisierung: 0 Scans", auto === 0, `${auto}`);
    pruefe("D7c Klick aufs schon offene Netzwerkmodul: 0 Scans", nochmal === 0, `${nochmal}`);
    pruefe("D7d Wechsel in das Netzwerkmodul: 1 Scan", wechsel === 1, `${wechsel}`);
    pruefe("D7e Knopf: 1 Scan", knopf === 1, `${knopf}`);
    pruefe("D7f Knopf mit Fristablauf: 1 Scan, keine Wiederholung, alte Liste, Fehler gemeldet", ablauf === 1 && listeVor === listeNach && k === true,
      `${ablauf} Scan(s), Liste ${listeVor === listeNach ? "unveraendert" : "veraendert"}, Knopf ${k ? "Fehler" : "kein Fehler"}`);
  });

} catch (e) {
  console.log("KAPUTT c65q " + bez + ": " + (e && e.stack || e));
  ende(2);
}

pruefe("G1 nie mehr als 2 /api/ gleichzeitig (ganzer Lauf)", gesamt.max <= 2, `hoechstens ${gesamt.max} bei ${gesamt.api} /api/-Abrufen`);
pruefe("G2 0 Abrufe display_power/ambilight_power (ganzer Lauf)", gesamt.power === 0, `${gesamt.power}`);

console.log(`${fehler.length ? "FEHL" : "OK  "} c65q ${bez}: ${fehler.length} Fehler in ${faelle} Faellen` + (fehler.length ? " (" + fehler.join("; ") + ")" : ""));
ende(fehler.length ? 1 : 0);
