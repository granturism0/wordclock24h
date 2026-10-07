# Prüfstände

Hier liegt Code, der **Firmware-Logik auf dem Entwicklungsrechner** nachrechnet — um eine
Umsetzung gegen eine unabhängige zweite zu stellen, oder um einen Fehlerfall zu erzeugen,
den man am Gerät nicht gefahrlos auslöst.

## Die eine Regel, die hier alles trägt

**Ein Prüfstand auf dem Entwicklungsrechner ist nicht die Zielplattform.** Der STM32 ist
32-bittig, Dein Rechner 64-bittig. Jeder Typ ohne feste Breite — `unsigned long`, `size_t`,
jeder Zeiger — ist damit eine **andere Zahl**, und das Ergebnis sieht trotzdem aus wie ein
Beleg.

Belegt an einer Überlaufrechnung: Am Gerät 5049 ms, im Prüfstand **4'294'962'345 ms**, weil
die Differenz dort nicht überlief (`BEFUNDE.md`, L256).

Deshalb: **`#include "pruefstand.h"`** und die `zp_*`-Typen benutzen. Der Header prüft seine
eigenen Breiten beim Übersetzen und bricht ab, statt still falsch zu rechnen.

**Firmware-Code mit `uint_fast8_t`/`uint_fast16_t`** braucht zusätzlich
`-DPRUEFSTAND_SCHNELLE_TYPEN`, und der Header muss **vor** dem Firmware-Code stehen. Auf
dem Ziel sind beide Typen 32 Bit, auf dem Mac 8 bzw. 16 — ohne den Schalter nimmt eine
Bereichsprüfung 288 als 32 an, die auf dem Gerät abweist (in Runde S zweimal gesehen).

## Was ein Prüfstand hier erfüllen muss

1. **Er entsteht vor oder unabhängig von der Umsetzung, die er prüft.** Wer seine Vektoren
   aus dem bestehenden Code ableitet, prüft, ob dieser mit sich selbst übereinstimmt — und
   bestätigt einen Fehler, den beide Seiten teilen.
2. **Er ist einmal fehlgeschlagen** (DIR-014). Nicht „der Code sieht richtig aus", sondern:
   eine Verletzung herstellen und sehen, dass er anschlägt.
3. **Sein Aufruf steht in `tools/guardrails.sh`** — oder ist dort mit Grund ausgelassen.
   Ein Prüfstand, den niemand startet, ist eine Datei.
4. **Im Kopf steht, warum es ihn gibt**, mit Befundkennung. Ohne das weiss in drei Monaten
   niemand, ob er noch etwas prüft, was es noch gibt.

## Bestand

| Datei | Prüft | Befund |
|---|---|---|
| `var-crc.c` | die Brücken-Prüfsumme, als unabhängige dritte Umsetzung gegen STM und ESP | A32 |
| `pruefstand.h` | die Typbreiten der Zielplattform | L256 |
| `auszug.sh` + `auszug/` | 14 Auszugs-Prüfstände aus Runde S: echter Code aus `src/**` bzw. `ESP8266/ESP-uclock/`, ausgeschnitten und nativ **und** mit Zieltypen übersetzt. Läuft als Stufe S16. `--gegen <rev>` fährt die Gegenprobe gegen einen alten Stand | AKS.5, A3, A5, A9, A13, A14, A20, A24, A46, A47, A48, A49, A51, C31, L308, L325 |

**Wie ein Auszugs-Prüfstand gebaut ist:** Ein `extract.py` schneidet die geprüften
Funktionen aus der Quelldatei (Latin-1 gelesen, CRLF entfernt), ein `run.sh` übersetzt sie
mit dem Prüfrahmen (`-DSNIPPET=…`) zweimal, nativ und mit `-DZIELTYPEN`, und gibt Exit 1
bei einem Fehlschlag. Weil die Skripte Zwischendateien neben sich schreiben, kopiert
`auszug.sh` den Ordner vor jedem Lauf nach `$TMPDIR`. Schneidet ein `extract.py` nach einer
Codeänderung nichts mehr aus, meldet der Lauf „nicht übersetzbar“ (Exit 2) — das ist ein
Pflegefall des Prüfstands, kein bestandener Test.

Die `.mjs`-Dateien daneben sind keine Prüfstände, sondern **Guardrail-Stufen**: Sie lesen den
Quelltext und melden Abweichungen, statt Logik nachzurechnen.
