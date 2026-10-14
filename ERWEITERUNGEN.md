# Erweiterungen — geplante Features

Hier stehen **neue Funktionen**, die bewusst geplant, aber noch nicht beauftragt sind. Sie sind
getrennt von `BEFUNDE.md`: Dort stehen Mängel und Verbesserungen am Bestand, jeweils mit Beleg und
Status, und die ToDo-Liste dort ordnet die laufende Arbeit. Ein Feature hier ist kein Befund und
drängt sich nicht in diese Liste. Wird eines beauftragt, bekommt es eine Spec unter `specs/` und
wandert damit in die Arbeit; hier bleibt der Eintrag mit Verweis darauf stehen.

Kennung `X<n>`, fortlaufend, wird nie neu vergeben. Status: **Idee** (aufgenommen, nicht bewertet),
**geplant** (Abhängigkeiten und Rahmen geklärt), **beauftragt** (Spec vorhanden), **erledigt**,
**verworfen** (mit Grund).

| | Erweiterung | Status | Hängt ab von | Betrifft |
|---|---|---|---|---|
| **X1** | **Live-Display der echten LEDs in der PWA** | Idee | C65 (Heap des ESP unter PWA-Last) | STM, ESP, PWA |

## X1 — Live-Display der echten LEDs in der PWA

Aufgenommen am 14.10.2026, Idee des Nutzers.

**Heute:** Die WordClock-Vorschau der PWA ist nachgebildet. Sie errechnet die Wörter selbst aus der
Layout-Tabelle (`layout-previews.json`) und der Gerätezeit; die Farbe holt sie periodisch vom Gerät
(`liveDisplayColorTimer`). Sie zeigt also, was leuchten *müsste* — nicht, was die Uhr tatsächlich
anzeigt.

**Ziel:** Die PWA zeigt die Anzeige nach den LEDs, die wirklich an sind. Damit wären Ticker, Icons,
Overlays, das Nachtfenster und vor allem ein Hänger in der Oberfläche sofort sichtbar — wertvoll für
die Diagnose (siehe die sporadischen Hänger in `CLAUDE.md`, „Offene technische Themen“).

**Randbedingungen aus dem Bestand:**

- Die UART-Brücke ist der einzige vollwertige UART des ESP. Jede Dauerlast darauf zählt, auch wenn
  niemand die Oberfläche offen hat.
- Der Heap des ESP ist unter PWA-Last schon knapp (`BEFUNDE.md`, C65, L449). Eine weitere periodische
  Abfrage kostet genau die Verbindungen, die heute fehlen. Deshalb erst nach C65.
- Naheliegend: Abruf auf Anforderung oder kompaktes Format (Wortindizes statt 288 einzelner LEDs)
  statt eines laufenden Stroms.

**Offen für die Spec:**

- Umfang: ganzes Panel einschliesslich Ticker und Icons, oder nur „welche Wörter sind an“?
- Takt: laufend, solange die Seite offen ist, oder auf Knopfdruck?
- Wo das Abbild entsteht (STM sendet Rohzustand, ESP verdichtet, PWA zeichnet) und was es an Flash
  auf der engsten STM-Variante kostet (Reserve über dem Gate siehe `BEFUNDE.md`, L435).
