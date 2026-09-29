#!/usr/bin/env python3
"""Vorschau-Server fuer die WordClock-PWA.

Liefert ESP8266/ESP-uclock/data/app aus und simuliert die Geraete-API, damit die
Oberflaeche ohne Hardware betrachtet und vermessen werden kann.

    python3 tools/preview/server.py 8099

    http://127.0.0.1:8099/app/                    die App
    http://127.0.0.1:8099/frame?w=390&h=844       in einem iframe exakter Groesse
    http://127.0.0.1:8099/frame?w=390&h=844&diag=1  zusaetzlich mit Messung

Der iframe-Umweg ist noetig, weil Chromes --window-size im Headless-Modus nur die
Bildgroesse setzt, nicht das Layout-Viewport. Ohne ihn misst man die falsche Breite.
"""
import json, os, re, sys, time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
APP = os.path.join(REPO, "ESP8266", "ESP-uclock", "data", "app")

NUM = {0:1, 1:1, 2:1, 3:1, 4:0, 5:0, 6:10, 7:1, 8:3, 9:1, 10:0, 11:0, 12:0, 13:0,
       14:8, 15:0, 16:0, 17:1, 18:0, 19:0, 20:1, 21:44, 22:0, 23:45, 24:0, 25:0,
       26:0, 27:3, 28:0, 29:0, 30:0, 31:0, 32:0, 33:0, 34:0, 35:0, 36:0, 37:0,
       38:0, 39:0}
STR = {0:"", 1:"3.2.5", 2:"1.0", 3:"3.2.1", 4:"ch.pool.ntp.org", 5:"",
       6:"Bern", 7:"7.4474", 8:"46.9480", 9:"update.wordclock.ch",
       10:"/firmware/", 11:"%d.%m.%Y", 12:"wc24h-de-ch.txt"}

MODES = ["ESisT","ESisT ohne Vorlauf","Berndeutsch","Wallis","Ostschweiz"]
ANIMS = ["Kein","Ueberblenden","Rollen","Explodieren","Schlange","Wuerfel","Fernschreiber","Zufall"]
COLANIMS = ["Kein","Regenbogen","Farbwechsel","Puls"]
ALMODES = ["Normal","Uhrzeit","Rahmen","Zufall"]
OVERLAYS = [(0,1,60,10,0,0,127,1,"Guten Morgen"), (1,2,0,8,1,0,31,1,"Muellabfuhr"),
            (2,0,30,6,0,0,127,0,"")]

def settings_xml():
    t = time.localtime()
    p = ["<settings>"]
    for i,v in NUM.items(): p.append(f'<numvar idx="{i}" value="{v}" />')
    for i,v in STR.items(): p.append(f'<strvar idx="{i}" value="{v}" />')
    for i in range(3):
        p.append(f'<tmvar idx="{i}" year="{t.tm_year}" month="{t.tm_mon}" day="{t.tm_mday}" '
                 f'hour="{t.tm_hour}" minute="{t.tm_min}" second="{t.tm_sec}" wday="{t.tm_wday}" />')
    cols = [(63,50,20,0),(20,40,63,10),(63,10,10,0),(10,63,20,0),
            (40,40,63,20),(63,63,40,30),(30,10,63,0),(63,30,0,5)]
    for i,(r,g,b,w) in enumerate(cols):
        p.append(f'<dspcolor idx="{i}" red="{r}" green="{g}" blue="{b}" white="{w}" />')
    for var in (0,1):
        for i in range(16):
            p.append(f'<num8array var="{var}" idx="{i}" value="{min(15, i)}" />')
    for i,n in enumerate(MODES):   p.append(f'<dispmode idx="{i}" name="{n}" />')
    for i,n in enumerate(ANIMS):   p.append(f'<dispanim idx="{i}" name="{n}" dcl="3" def_dcl="3" flags="1" />')
    for i,n in enumerate(COLANIMS):p.append(f'<coloranim idx="{i}" name="{n}" dcl="4" def_dcl="4" flags="1" />')
    for i,n in enumerate(ALMODES): p.append(f'<almode idx="{i}" name="{n}" dcl="2" def_dcl="2" flags="1" />')
    for i,(idx,ty,iv,du,dc,ds,dy,fl,tx) in enumerate(OVERLAYS):
        p.append(f'<overlay idx="{idx}" type="{ty}" interval="{iv}" duration="{du}" '
                 f'date_code="{dc}" date_start="{ds}" days="{dy}" flags="{fl}" text="{tx}" />')
    for i in range(8):
        p.append(f'<alarmtime idx="{i}" minutes="{420 + i*30}" flags="{1 if i < 2 else 0}" />')
    for i in range(8):
        p.append(f'<nighttime idx="{i}" minutes="{1320 + i*10}" flags="{1 if i == 0 else 0}" />')
        p.append(f'<ambinighttime idx="{i}" minutes="{1320 + i*10}" flags="0" />')
    p.append("</settings>")
    return "".join(p)

JSON_API = {
  "update_status": {"ok":True,"host":"update.wordclock.ch","path":"/firmware/",
      "stm32_filename":"wc12h-stm32f411ce-25-sk6812-rgbw.hex","esp_filename":"ESP-WordClock-4M.bin",
      "stm32_version":"3.2.5","esp_version":"3.2.1","releaseNotes":"Version 3.2.5",
      "update_available":False,"remote_esp_update_url":"/api/remote_esp_update"},
  "update_table_files": {"ok":True,"files":["wc24h-de-ch.txt"],"current":"wc24h-de-ch.txt"},
  "eeprom_settings": {"ok":True,"ssid":"Heimnetz","key":"geheim1234","ap_ssid":"WordClock",
      "ap_key":"wordclock24","boot_as_ap":False},
  "stm32_log": {"ok":True,"lines":[
      "main: call display_clock time flags=0x04 power=1 ambi=1",
      "sk6812_refresh: start leds=288 nextbuf=1",
      "show_time: display_clock_flag=0x04 power=1 ambi=1 hour=14 minute=23"]},
  "fs_list": {"ok":True,"files":[{"name":"wc24h-de-ch.txt","size":10240},
                                 {"name":"app.js.gz","size":96013}]},
  "device_ready": {"ok":True,"ready":True},
  "update_progress": {"ok":True,"active":False,"state":"idle","type":"","percent":0},
  "overlay_icons": {"ok":True,"icons":[]},
  "weather_get_now": {"ok":True,"text":"Bern: 18 Grad, bewoelkt","icon":"03d"},
}

class H(BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def _send(self, body, ctype, code=200):
        if isinstance(body, str): body = body.encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-cache")
        self.end_headers()
        self.wfile.write(body)
    def do_POST(self):
        path = urlparse(self.path).path
        if path == "/diag-result":
            n = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(n).decode("utf-8", "replace")
            vp = urlparse(self.path).query.replace("vp=", "").replace("%20", "")
            here = os.path.dirname(os.path.abspath(__file__))
            with open(os.path.join(here, "diag-%s.json" % vp.replace("x", "_")), "w") as fh:
                fh.write(body)
        self._send(json.dumps({"ok":True}), "application/json")
    def do_GET(self):
        path = urlparse(self.path).path
        if path == "/frame":
            from urllib.parse import parse_qs
            q = parse_qs(urlparse(self.path).query)
            w = int(q.get("w", ["390"])[0]); h = int(q.get("h", ["844"])[0])
            diag = "1" if q.get("diag") else ""
            html = ("<!doctype html><meta charset=utf-8>"
                    "<style>html,body{margin:0;background:#222}"
                    "iframe{width:%dpx;height:%dpx;border:0;display:block}</style>"
                    "<iframe src='/app/%s'></iframe>") % (w, h, "?diag=1" if diag else "")
            return self._send(html, "text/html; charset=utf-8")
        if path in ("/", "/legacy"):
            return self._send("<h1>Legacy</h1>", "text/html")
        if path.startswith("/api/"):
            name = path[5:]
            if name == "settings_xml":
                return self._send(settings_xml(), "text/xml; charset=utf-8")
            if name in ("display_power", "ambilight_power"):
                return self._send("on", "text/plain")
            if name in JSON_API:
                return self._send(json.dumps(JSON_API[name]), "application/json")
            return self._send(json.dumps({"ok":True}), "application/json")
        rel = path[5:] if path.startswith("/app/") else ("index.html" if path == "/app" else path.lstrip("/"))
        if rel in ("", "/"): rel = "index.html"
        full = os.path.join(APP, rel)
        if os.path.isfile(full):
            ext = os.path.splitext(full)[1]
            ctype = {".html":"text/html; charset=utf-8", ".js":"application/javascript; charset=utf-8",
                     ".css":"text/css; charset=utf-8", ".json":"application/json",
                     ".svg":"image/svg+xml", ".webmanifest":"application/manifest+json"}.get(ext, "text/plain")
            with open(full, "rb") as f: data = f.read()
            if rel == "index.html" and "diag=1" in (urlparse(self.path).query or ""):
                extra = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "diag.js"), "rb").read()
                data = data.replace(b"</body>", b"<script>" + extra + b"</script></body>")
            return self._send(data, ctype)
        self._send("not found", "text/plain", 404)

if __name__ == "__main__":
    ThreadingHTTPServer(("127.0.0.1", int(sys.argv[1])), H).serve_forever()
