# Messwerkzeug für die Oberfläche

Ein Messstück, zwei Wirte: **Vorschau** und **Gerät**. Vorher wurde für jede Frage ein
eigenes Skript gebaut und danach weggeworfen — sechsmal in zwei Tagen (`BEFUNDE.md`, E19),
und Vorschau- und Gerätezahlen standen deshalb nie nebeneinander (L229).

```
python3 tools/preview/server.py 8099 &                       (nur für die Vorschau)
node tools/ui-mess/vermessen.mjs --vorschau 390x844 1512x982
node tools/ui-mess/vermessen.mjs --geraet --module maintenance 1512x982
node tools/ui-mess/auswerten.mjs
```

| Datei | Was |
|---|---|
| `messungen.mjs` | die Messungen, als Zeichenkette für den Browser — Kacheln, Ankreuzfelder, Treffflächen, Fokus |
| `wirt.mjs` | Chrome über das DevTools-Protokoll, ohne Playwright |
| `png.mjs` | PNG entfiltern, **Farbe am Bildpunkt**, WCAG-Kontrast |
| `png.probe.mjs` | Selbsttest für `png.mjs` — läuft mit allen fünf Filtertypen |
| `vermessen.mjs` | der Lauf, beide Wirte |
| `auswerten.mjs` | Höhendifferenzen, Treffflächen, Fokus, Kontraste aus einer Ergebnisdatei |

## Drei Dinge, die hier bewusst anders gemacht werden

**Farben werden gelesen, nicht gerechnet.** Derselbe Kontrastbefund wurde viermal falsch
berechnet; zuletzt stand „am Gerät gemessen" über Zahlen, deren Farben im Bild **null mal**
vorkamen (L245). Halbtransparente Überlagerungen, ein mitscrollender Verlauf und die
Eigenfarben der Bedienelemente machen jede Stylesheet-Rechnung wertlos.

**Gemessen wird die Trefffläche, nicht das Element.** Sitzt ein Kästchen in einem Label,
trifft der Finger das Label. Die erste Fassung mass das `input` selbst und meldete 16 Felder
„unter 44 px" — die Kästchen sind 18×18, ihre Pille 88×44. Ein überzeugend aussehender
Fehlalarm gegen eine längst umgesetzte Massnahme.

**Der Tastaturfokus wird über echte Tabulatortasten geprüft**, nicht über `.focus()`.
`:focus-visible` verhält sich bei programmatischem Fokus anders; wer so misst, misst eine
Lage, die ein Nutzer nie herstellt.

## Stand der Eichung (W.4, 05.10.2026)

Gegen die von Hand ermittelten Zahlen aus L227, Modul `maintenance`:

| Paar | L227 | gemessen | |
|---|---|---|---|
| `backup` / `service` | 406 px | **406 px** | ✓ exakt |
| `source` / `remote` | 302 px | 900 px | ✗ |

Die entscheidende Zahl — die grösste Höhendifferenz, an der AK3.1 hängt — wird **exakt**
reproduziert. Bei `source` weicht die **Vorschau** ab: Sie rendert die Kachel 1248 px hoch
statt der ~650, die zur Handmessung passen. Die beiden Kacheln sind im Markup Geschwister,
also kein Auswertungsfehler.

**Das ist kein Mangel des Werkzeugs, sondern sein erster Befund** — und genau die Frage, für
die es gebaut wurde: Die Vorschau zeigt nicht dasselbe wie das Gerät. Welche der beiden
Zahlen gilt, entscheidet ein Lauf mit `--geraet`.
