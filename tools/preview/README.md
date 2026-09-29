# Vorschau der PWA ohne Gerät

Zeigt und vermisst die Oberfläche unter `ESP8266/ESP-uclock/data/app`, ohne dass eine
WordClock erreichbar sein muss. Gedacht für zwei Fälle: eine UI-Änderung vorher und
nachher vergleichen, und Layoutfragen über die Gerätespanne beantworten, statt sie aus
dem Stylesheet zu erraten.

## Benutzen

```
python3 tools/preview/server.py 8099      # Terminal 1, läuft weiter
./tools/preview/shot.sh                   # Terminal 2, rendert den Standardsatz
./tools/preview/shot.sh --diag 390x844    # eine Grösse, mit Messung im Bild
```

Die PNGs landen unter `tools/preview/shots/` und sind gitignored.

Im Browser direkt:

| URL | Zweck |
|---|---|
| `http://127.0.0.1:8099/app/` | die App, wie der ESP sie ausliefern würde |
| `http://127.0.0.1:8099/frame?w=390&h=844` | in einem iframe exakter Grösse |
| `…&diag=1` | zusätzlich mit eingeblendeter Messung |

## Was die Messung ausgibt

Horizontaler Überlauf des Dokuments, Elemente die über den Viewport ragen, abgeschnittener
Text, Bedienelemente unter 44 px Höhe, der berechnete Fokusstil, `color-scheme`, die
tatsächlichen Farben des Sprach-Selects, und ob der Service Worker registriert ist.

## Was der Server simuliert

`/api/settings_xml` nach dem Schema aus `http.cpp` — `numvar`, `strvar`, `tmvar`,
`dspcolor`, `num8array`, `dispmode`, `dispanim`, `coloranim`, `almode`, `overlay`,
`alarmtime`, `nighttime`, `ambinighttime` — mit plausiblen Werten: Display und Ambilight
an, Firmware 3.2.5, drei Overlays, acht Timer, DS18xx vorhanden. Dazu `update_status`,
`update_table_files`, `eeprom_settings`, `stm32_log`, `fs_list`. Jeder andere
`/api/`-Pfad antwortet `{"ok":true}`.

**Anpassen, wenn du einen Randfall sehen willst:** die Tabellen `NUM`, `STR`, `OVERLAYS`
und `JSON_API` oben in `server.py`. Ein sehr langer Ortsname oder ein Overlay-Text am
Längenlimit ist genau der Fall, den die Vorschau mit Standardwerten **nicht** zeigt.

## Grenzen — wichtig

Die Vorschau ersetzt keinen Test am Gerät.

- **Chrome, nicht iOS Safari.** Alles, was iOS-spezifisch ist, bleibt unbeantwortet:
  Safe-Area am Notch, native Picker, `dvh`-Verhalten, Tastaturüberdeckung, das Verhalten
  im Homescreen-Modus
- **Kein Notch.** `env(safe-area-inset-*)` ist hier überall 0. Der Befund „Safe-Area
  links/rechts fehlt" lässt sich damit **nicht** prüfen
- **127.0.0.1 ist ein sicherer Kontext**, das echte Gerät über `http://192.168.x.x` nicht.
  Der Service Worker verhält sich hier also anders als in der Realität
- **Mock-Daten sind kürzer als echte.** Überlaufprobleme, die erst mit langen echten
  Werten auftreten, zeigt die Vorschau nicht
- **Keine echte Latenz.** Der ESP8266 antwortet langsamer und einthreadig; Timing- und
  Ladeprobleme treten hier nicht auf

## Erkenntnisse, die hier schon drinstecken

Drei Chrome-Eigenheiten haben beim Bauen Zeit gekostet und sind in `shot.sh` fest
verdrahtet, damit sie das nicht nochmal tun:

1. `--window-size` setzt im Headless-Modus **nur die Bildgrösse**, nicht das
   Layout-Viewport. Wer ohne den iframe-Umweg misst, misst die falsche Breite — und hält
   den Zuschnitt des Screenshots fälschlich für abgeschnittenen Text
2. Chrome **beendet sich nach `--screenshot` nicht zuverlässig**. Der Prozess wird nach
   einer Wartezeit beendet; die Datei ist dann bereits geschrieben
3. Ein zweiter Lauf auf demselben `--user-data-dir` **blockiert am Profil-Lock**. Jeder
   Lauf bekommt ein frisches Profil

`--dump-dom` funktioniert unter `--headless=new` nicht mehr. Deshalb schreibt das
Messskript seine Werte sichtbar in die Seite, statt sie über den DOM zurückzugeben.
