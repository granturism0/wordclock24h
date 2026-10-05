# Musterseiten — für Inhalte, die die Vorschau nicht liefert

Die Vorschau (`tools/preview/server.py`) bildet die Geräte-API nach, aber **nicht alles**:
Release Notes, Dateilisten und andere vom Server gelieferte Inhalte fehlen dort. Elemente,
die nur mit solchen Inhalten erscheinen, sind in der Vorschau **nicht messbar** — sie sind
gar nicht im DOM.

Das ist kein Randfall. Genau so blieben zwei Verweise mit halber Trefffläche über Monate
unentdeckt (L282): In der Vorschau meldet derselbe Messlauf 0 Treffer, am Gerät 2.

## Wie eine Musterseite gebaut wird

1. Den **echten** Inhalt lesend vom Gerät holen (`/api/update_status` o. ä.) — nicht erfinden.
2. Ihn durch denselben Filter denken, den `app.js` anwendet. Für Release Notes ist das
   `sanitizeReleaseNotesChildren()`: Attribute fallen weg, Inline-Stile also auch.
3. In die **echte** Umgebung stellen — dieselbe Verschachtelung (`.shell > .panel > …`),
   dieselbe `styles.css`, dasselbe Viewport-Meta wie `index.html`.
4. Mit `tools/ui-mess/wirt.mjs` vermessen, nicht mit einem eigenen Browser-Aufbau.

Punkt 3 ist der, an dem es schiefgeht: Eine Musterseite ohne die echte Verschachtelung misst
andere Breiten, und das fällt nicht auf, weil die Zahlen plausibel aussehen.

## Was hier liegt

| Datei | Inhalt | Befund |
|---|---|---|
| `server-html.html` | Release Notes, wie das Gerät sie liefert, in der echten Umgebung | L282 |

**Die Messung gegen die Musterseite ersetzt den Gerätelauf nicht.** Sie macht ihn
wiederholbar und erlaubt, eine Änderung zu prüfen, bevor sie gebaut und hochgeladen ist.
Der Nachweis bleibt `vermessen.mjs --geraet`.
