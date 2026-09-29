---
name: pwa-vorschau
description: Startet die PWA ohne WordClock in einem Mock-Server und vermisst die Oberfläche — horizontaler Überlauf, abgeschnittener Text, Touch-Targets unter 44 px, Fokusstil, color-scheme. Nutzen für jede Beurteilung von Layout, Darstellung oder Bedienbarkeit, wenn kein Gerät erreichbar ist.
when_to_use: Bei Fragen nach dem Aussehen der PWA, bei UI/UX-Prüfungen, bei Layout-Befunden und immer, wenn eine Behauptung über die Darstellung belegt werden soll.
allowed-tools: Read Grep Glob Bash
---

# PWA ohne Gerät ansehen und vermessen

```
python3 tools/preview/server.py 8099      # Terminal 1
./tools/preview/shot.sh --diag 390x844    # Terminal 2
```

Der Server liefert `data/app` aus und simuliert die Geräte-API. Damit lässt sich die
Oberfläche ansehen und vermessen, ohne dass eine WordClock erreichbar ist.

## Drei Chrome-Eigenheiten, die hier eingebaut sind

Sie zu kennen ist der halbe Zweck dieses Werkzeugs — **eine davon hat schon zu einem
falschen Befund geführt**.

1. **`--window-size` setzt nur die Bildgrösse, nicht den Layout-Viewport.** Wer damit
   misst, bekommt ein Bild in der gewünschten Grösse, aber ein Layout in einer anderen
   Breite. Genau so entstand einmal die Behauptung, Text werde bei 390 px abgeschnitten
   — gemessen waren es dann 0 px Überlauf bei 320, 390 und 852. Deshalb `/frame?w=&h=`,
   das die Seite in einen iframe exakter Grösse setzt.
2. **Chrome beendet sich nach `--screenshot` nicht.** Ohne Zeitbegrenzung bleibt der
   Prozess stehen.
3. **Der Profil-Lock verlangt ein frisches `--user-data-dir`** je Lauf.

`--dump-dom` funktioniert unter `--headless=new` nicht.

## Was diese Vorschau nicht kann

**Sie ersetzt keinen Test am Gerät.** Chrome statt iOS Safari, kein Notch also keine
Safe-Area, `127.0.0.1` ist ein sicherer Kontext und das echte Gerät nicht — deshalb
registriert hier der Service Worker, auf dem Gerät nie. Die Mock-Daten sind ausserdem
kürzer als echte, Überlauf durch lange Texte zeigt sich also nicht.

Der Mock meldet einen festen Gerätestand und folgt den Quellen absichtlich nicht.

Details in `tools/preview/README.md`.
