// Gemeinsame Chrome-Anbindung fuer beide Wirte (Vorschau und Geraet).
//
// Ueber das DevTools-Protokoll, ohne Playwright oder Puppeteer: Node bringt seit v22
// ein WebSocket mit, und mehr braucht es nicht. Dieselbe Entscheidung wie in
// tools/check-pwa.mjs -- eine Abhaengigkeit weniger, die in zwei Jahren fehlen kann.
import { spawn } from "node:child_process";
import { setTimeout as sleep } from "node:timers/promises";
import { rmSync } from "node:fs";

const CHROME = process.env.CHROME_BIN
  || "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome";

export async function starteBrowser({ port = 9340, profil }) {
  rmSync(profil, { recursive: true, force: true });
  const chrome = spawn(CHROME, [
    "--headless=new", "--disable-gpu", "--no-sandbox", "--hide-scrollbars",
    "--force-device-scale-factor=1",          // sonst misst ein Retina-Mac doppelt
    `--remote-debugging-port=${port}`, `--user-data-dir=${profil}`, "about:blank",
  ], { stdio: "ignore" });

  let url;
  for (let i = 0; i < 80; i++) {
    try { url = (await (await fetch(`http://127.0.0.1:${port}/json/version`)).json()).webSocketDebuggerUrl; break; }
    catch { await sleep(250); }
  }
  if (!url) { chrome.kill(); throw new Error("Chrome kam nicht hoch"); }

  const ws = new WebSocket(url);
  await new Promise((r) => ws.addEventListener("open", r));
  let id = 0; const offen = new Map();
  ws.addEventListener("message", (ev) => {
    const m = JSON.parse(ev.data);
    if (m.id && offen.has(m.id)) {
      const { res, rej } = offen.get(m.id); offen.delete(m.id);
      m.error ? rej(new Error(JSON.stringify(m.error))) : res(m.result);
    }
  });
  const cdp = (method, params = {}, sessionId) => new Promise((res, rej) => {
    const mid = ++id; offen.set(mid, { res, rej });
    ws.send(JSON.stringify({ id: mid, method, params, sessionId }));
  });

  const { targetId } = await cdp("Target.createTarget", { url: "about:blank" });
  const { sessionId } = await cdp("Target.attachToTarget", { targetId, flatten: true });
  await cdp("Page.enable", {}, sessionId);
  await cdp("Runtime.enable", {}, sessionId);

  const js = async (expr) => {
    const r = await cdp("Runtime.evaluate",
      { expression: expr, returnByValue: true, awaitPromise: true }, sessionId);
    if (r.exceptionDetails) throw new Error(JSON.stringify(r.exceptionDetails));
    return r.result.value;
  };

  // Tabulator als ECHTES Tastenereignis -- siehe Begruendung in messungen.mjs.
  const tab = async (mitShift = false) => {
    for (const type of ["rawKeyDown", "char", "keyUp"]) {
      await cdp("Input.dispatchKeyEvent", {
        type: type === "char" ? "char" : type, key: "Tab", code: "Tab",
        windowsVirtualKeyCode: 9, nativeVirtualKeyCode: 9,
        text: type === "char" ? "\t" : undefined,
        modifiers: mitShift ? 8 : 0,
      }, sessionId);
    }
  };

  const schuss = async (clip) => {
    const r = await cdp("Page.captureScreenshot",
      { format: "png", captureBeyondViewport: true, clip }, sessionId);
    return Buffer.from(r.data, "base64");
  };

  return {
    cdp, js, tab, schuss, sessionId,
    viewport: (w, h) => cdp("Emulation.setDeviceMetricsOverride",
      { width: w, height: h, deviceScaleFactor: 1, mobile: w < 700 }, sessionId),
    gehe: async (url) => { await cdp("Page.navigate", { url }, sessionId); await sleep(900); },
    ende: () => { ws.close(); chrome.kill(); },
  };
}
