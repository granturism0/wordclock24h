# Design — Paket 2026-10-06 (Runden P, P2, S, F, verteilte Runde K)

**Erstellt:** 2026-10-06, Stand `401aae2`. **Nachgeführt am selben Tag** nach der
Auslieferung von Runde P (PWA 1.4.91) und nach deren Gerätetest (L324). Momentaufnahme
(DIR-006), wird nach der Freigabe nicht mehr fortgeschrieben.

**Rundenfolge:** **P (ausgeliefert) → P2 → S → F**. Runde K fährt mit (§6). Kennungen:
„N1"/„N2" sind Befunde der Nachzählung; **B40/L321**, **C38/L323** und **L324** stammen aus
dem Gerätetest von 1.4.91; **L303** betrifft `runConfirmedButtonAction`, **L306** das
Umschalten von Flags; „Ent-n" sind Entscheidungen (`requirements.md`).

---

## 0. Was sich gegenüber `specs/paket-2026-10-05/` geändert hat

| Punkt | Alte Spec | Diese Spec | Grund |
|---|---|---|---|
| Runden V, W, E1, E2, H | geplant | **weg** | ausgeliefert bzw. E1 entfallen; Release `release/3.2.21-3.2.24-1.4.90`, auch im Testplan abgenommen |
| Runde U | eigene Runde | **aufgelöst**: B27, B32 nach P; B28, B31 warten | Entscheidung des Nutzers |
| Reihenfolge | … → S → P → U → F | **P → P2 → S → F** | Entscheidung des Nutzers; P2 aus dem Rest von P |
| Runde K | — | **neu**, ohne eigenen Flash, verteilt | Entscheidung des Nutzers, Nachzählung vom 06.10.2026 |
| Runde P | B33, B24 | **ausgeliefert als 1.4.91**: B33, B34, B35, B36, R1–R4, B27, B26, L304, B37, B38; B32 erledigt | B24 war erledigt |
| Runde P2 | — | Overlay-Text in `TEXT_FIELD_LIMITS`, **B40/L321**, **Massnahme 4 in engerer Form**, B17, **Vorschau-Probe für L303 und L306** | nicht in 1.4.91 enthalten bzw. im Gerätetest gefunden (L324) |
| Runde S | A39, L260, A35 T1, C26, A16 | dasselbe **plus C31, A46, N1** und acht STM-K-Punkte, **zweistufiges Flash-Gate**; S.26 mit Gerätenachweis für B35 und B36 | L298, L291, Nachzählung, L296, L324 |
| S hängt ab von | H.7 | **P2-Build** (P2.6) | geteilter Arbeitsbaum, §4 |
| Runde F | C9c6, 65535-Liste, Smoketest | dasselbe **plus C28/L272** in **einer** Änderung, **C38/L323**, fünf ESP-K-Punkte | §3.1, §3.5, §6 |
| Testdurchlauf | eine Abnahme je Runde | **je Runde ein eigener Task**, M2 als **Nutzerschritt** davor; P2 begründet ohne, getragen von Proben | L302, §1.7 |
| Zähler C26 | „Logring **oder** `/api/device_ready`" | **nur Logring** | §4.2 |
| Prüfstände, Proben | vom Umsetzer „unter `tools/checks/`" | Umsetzer baut, **Lead legt ab** | `tools/**` ist Revier des Leads |
| STM-Guards | Gerätetest | **nur Prüfstand** (S15) | über HTTP nicht herstellbar, §2.12 |
| Builds | „einmal am Ende" | **serielle Testbauten** zwischen den STM-Tasks | Ent-7, §2.7 |
| Commit/Tag | teils nach der Abnahme | **im Rollout-Task** | DIR-011: sofort |
| Fundstellen `app.js` | Zeilennummern | **Funktionsnamen** | die Datei wurde gerade bearbeitet |

**Was bleibt, wörtlich oder sinngemäss:** §2.1 (A39, Variante b), §2.2 (Eröffnungszeile und
Abschlussmarke samt der Prämisse aus L269), §2.3 (C26), §2.4 (A35), §2.5 (die beiden
Warnungen).

---

## 1. Runde P — ausgeliefert, und P2

**Stand:** P ist als PWA 1.4.91 ausgeliefert (`release/3.2.21-3.2.24-1.4.91`), die Abnahme
steht in L324. Der lesende Gerätetest hat zwei Befunde für P2 geliefert (§1.8, §1.9). §1.1
bis §1.5 beschreiben den Entwurf, nach dem gebaut wurde; sie bleiben stehen, weil die
Abnahme (P.18) und P2 sich darauf beziehen. **Offen ist P2:** Overlay-Text in
`TEXT_FIELD_LIMITS`, B40, Massnahme 4, B17 und der Nachweis für L303 und L306 (§1.10).

### 1.1 B35 — Byte statt Zeichen, ohne Markup-Änderung

**Was das Gerät misst.** `http_check_strvar_len()` (`http.cpp:10351`) vergleicht
`strlen (value)` gegen die Grenze — die URL-dekodierten UTF-8-Byte, die
`encodeURIComponent` erzeugt. **`new TextEncoder().encode(text).length` liefert exakt die
Zahl, die das Gerät prüft.**

| Endpunkt | Wert | Byte | Quelle | Gerät heute | PWA 1.4.91 |
|---|---|---|---|---|---|
| Ticker | `value` | 32 | `vars.h:183` | weist ab | prüft |
| Datumsformat | `value` | 5 | `vars.h:194` | weist ab | prüft |
| Wetter-APPID | `value` | 32 | `vars.h:188` | weist ab | prüft |
| Wetter-Ort | `value` | 32 | `vars.h:189` | weist ab | prüft |
| Koordinaten | `lon`, `lat` | je 8 | `vars.h:190-191` | weist ab | prüft |
| Zeitserver | `value` | 16 | `vars.h:187` | weist ab | prüft — **aber ein später Scan kann den Wert vorher ersetzen** (B40, §1.8) |
| Update-Host | `value` | 63 | `vars.h:192` | weist ab | prüft |
| Update-Pfad | `value` | 63 | `vars.h:193` | weist ab | prüft |
| **Overlay-Text** | `text` | 32 | `vars.h:414` | **kürzt still** — K-Punkt F | **fehlt — P2.1** |

**Der Overlay-Text gehört in dieselbe Tabelle**, obwohl das Gerät ihn heute noch kürzt. Die
PWA weist dann **vor** dem Gerät ab — bis zum F-OTA ist das der einzige Schutz gegen die
stille Kürzung. **Dass die PWA strenger ist als das Gerät von heute, ist gewollt:** Gekürzt
und als gespeichert gemeldet ist genau der Fall, den C22 auf dem Gerät beseitigt hat.

**`index.html` bleibt für B35 unverändert.** Die `maxlength`-Werte sind dieselben Zahlen in
Zeichen; die Byteprüfung fängt den Überschuss, die Meldung läuft über `announceStatus`.

**Die Prüfung sitzt an der Endpunktgrenze, nicht am Eingabefeld.** Eingabefeld,
Kartenauswahl, Overlay-Editor und Backup-Import beschreiben dieselben Endpunkte; eine
Prüfung je Feld vergässe einen Weg — L179 in der PWA. Daher eine Hilfsfunktion und **eine**
Tabelle mit Verweis auf `vars.h`, im Review abgeglichen (AKP.2). **P2.1 ergänzt sie um den
Overlay-Text, im Editor und in `importOverlaySettings()`.**

### 1.2 R1 und R2 — Kennung 6 aus `fs_remove` ist doppelt belegt

✔ am Code: `http_api_fs_remove()` meldet Kennung 6 für „file not found" (`http.cpp:11536`)
**und** für „remove failed" (`:11545`). Die PWA übersetzt „Diese Datei gibt es auf dem
Gerät nicht." — für den zweiten Fall falsch.

**Regel (ausgeliefert):** Kennung 6 wird erst nach einem erneuten Abruf des Verzeichnisses
gedeutet. Fehlt die Datei, ist das Ziel erreicht; steht sie noch da, ist es ein Fehlschlag.
Kein Vergleich auf den englischen Detailtext. Eine Hilfsfunktion für beide Aufrufer. Die
saubere Lösung läge auf dem ESP — Ent-4, vom Lead dem Nutzer vorgelegt; die PWA-Regel bleibt
auch danach richtig.

### 1.3 B36 — die Nachprüfung hängt am Ergebnis

`runButtonRequest()` gibt `true`/`false` zurück, `saveUpdateHost()`/`saveUpdatePath()`
prüfen nur bei `true` nach. AKP.4 verlangt den Nachweis, dass kein anderer Aufrufer den
Rückgabewert anders auswertet. **Nicht gewählt:** ein eigener Anfrageblock — zweite Fassung
der Rückmeldelogik (L289). **Gerätenachweis in S.26** über einen zu langen Update-Host, den
der ESP abweist (§5).

### 1.4 B34 — das Attribut, nicht der Platzhalter, ist falsch

`applyTranslations()` darf ein Element, das Daten trägt, nicht auf den Platzhalter
zurücksetzen. `data-i18n` bleibt im Markup. Ein übersetzter Satz darf bis zur nächsten
Aktualisierung in der alten Sprache stehen; der Platzhalter nicht, weil er **Inhalt
vernichtet**.

**Verbindung zu B17**, ● nicht geprüft: `stm32-log-output` gehörte zu den 17 Flächen. Das
erklärt L142 nicht vollständig (das Polling heilte nach 2,5 s), gehört aber in die
Eingrenzung P2.2.

### 1.5 R3, R4, B33, B27, B26, B32, B17

**R3** — ausgeliefert: einmal je Sitzung sichtbar. **R3 löst C32 nicht**; die Funktion ist
selbst eine Umgehung von C32 und wird nur ehrlich gemacht.

**R4** — ausgeliefert für die acht Endpunkte: zu langes Feld übersprungen und genannt,
nicht geworfen, **nie gekürzt**. **Für die Overlay-Texte in P2.1.**

**B33** — ausgeliefert: `.gz` zeigt „binär, n Byte".

**B27** — ausgeliefert mit „Sekunden am Ambilight-Ring weich ausblenden" (Ent-3). Länger
als der Vorschlag der Spec, dafür eindeutig — und Eindeutigkeit war der ganze Befund.

**B26** — ausgeliefert, mit Gegenprobe: tote Funktion ohne Aufrufer, entfernt.
`files-panel` und `local-update-panel` **bleiben**, weil `tools/ui-mess` sie als
Panelnamen führt; die Annahme „nirgends referenziert" war falsch.

**B32** — erledigt, die Gegenprobe ist gefahren.

**B17 — nur, wenn klein, jetzt in P2.** Der Code enthält bereits die Abmilderung gegen alle
drei Spuren aus L142. Offen ist die **Ursache**: Eingrenzung am Gerät (P2.2, Lead, lesend),
Umsetzung nur bei belegter Ursache in einer Funktion (P2.3). Sonst bleibt B17 offen — das
erfüllt AKP.13.

### 1.6 E31 ist erledigt — mit einem Vorbehalt, der weiterträgt

Das Release 3.2.21/3.2.24/1.4.90 ist im Umfang des `pwa-tester` abgenommen. **Vorbehalt:**
Ob die STM-Indexguards greifen, ist über HTTP nicht zeigbar, weil der ESP jeden ungültigen
Index vorher abweist. **Die Spec verlangt deshalb keinen Gerätenachweis für STM-Guards**,
sondern einen Prüfstand (AKK.6, §2.12).

### 1.7 Einspielen, und warum P2 keinen eigenen Testdurchlauf hat

**P:** ausgeliefert. **P2:** nur PWA, `install-app.sh --check`, dann `install-app.sh`;
keine Firmware vorausgesetzt.

**Warum P2 vor S:** P2 schreibt `app.js`, und jeder Release-Build von S baut `app-gz` mit
(§4.1). Ist P2 gebaut, bevor S.1 beginnt, liegt beim S-Build kein offener PWA-Task im Baum.

**Testdurchlauf — neu geprüft.** Die erste Fassung begründete die Ausnahme mit „eine
Tabellenzeile". **Mit B40 und Massnahme 4 trägt diese Begründung nicht mehr**: P2 ändert
jetzt das Verhalten des Netzwerk-Moduls bei einer nebenläufigen Antwort und die Erkennung
ungespeicherter Änderungen. Die Ausnahme bleibt trotzdem, **aus einem anderen Grund**:

- B40 ist ein **Wettlauf**. Ob der Scan vor oder nach der Eingabe zurückkommt, hängt vom
  WLAN ab; ein `pwa-tester`-Durchlauf am Gerät würde ihn **zufällig** treffen oder verfehlen
  — und ein Durchlauf, der ihn verfehlt, meldete grün. **In der Vorschau ist er gezielt
  herstellbar**, mit einer absichtlich verzögerten `network_scan`-Antwort. Das tragende
  Instrument ist deshalb die **Probe** P2.1c unter `tools/ui-mess/proben/`, und sie muss
  **einmal gegen 1.4.91 fehlschlagen** (DIR-014).
- Massnahme 4 ist in der Vorschau vollständig prüfbar.
- **L303 und L306** brauchen eine Fehlerantwort, die am Gerät nicht folgenlos entsteht
  (§1.10). Ihr Nachweis ist ebenfalls eine Probe, P2.1e.
- **Am Gerät zusätzlich, lesend und ohne Speichern** (P2.8). Das braucht kein M2.
- Der volle Durchlauf **S.26** läuft gegen einen Stand mit P2 und prüft **B35 am Zeitserver
  in Phase 2** — dort, wo der Gerätenachweis mit Speichern hingehört.

**Was offen bleibt, ausdrücklich:** Zwischen dem P2-Upload und S.26 ist B40 am Gerät nur
lesend belegt. Fiele der Fix aus, wäre das der heutige Fehler, kein neuer. L303 und L306
werden nie mit einem echten Fehlschlag am Gerät belegt.

### 1.8 B40 / L321 — dieselbe Regel wie bei der AP-SSID

**Befund (Gerätetest 1.4.91, 23:58:46):** `updateNetworkControlsFromMeta()` setzt
`network-timeserver-input.value` und `network-timezone-input.value` **ohne Bedingung**
(`app.js:3519-3520` laut Lead, Stand 1.4.91). Gerufen wird das nach **jeder**
`network_scan`-Antwort über `refreshNetworkUi()` (`app.js:2893-2897`). Ein Scan, der spät
zurückkommt, ersetzt die Eingabe; „Speichern" schickt danach **still den alten Wert**.

**Warum das B35 berührt:** Die Byteprüfung prüft den Wert im Feld. Ersetzt der Scan ihn
vorher, prüft sie den alten und meldet nichts — B35 ist für Zeitserver und Zeitzone erst
nachgewiesen, wenn der Feldinhalt der ist, den der Nutzer eingegeben hat.

**Lösung:** **dieselbe** `prefillDeviceValue()`-Regel wie bei der AP-SSID (L34) — ein
Gerätewert füllt nur ein Feld, das weder fokussiert noch vom Nutzer geändert ist. **Keine
zweite Regel daneben**: Zwei Fassungen derselben Schutzlogik sind das Muster aus L179 und
L289. Der Review P2.4 prüft das ausdrücklich.

**Nicht mitkorrigiert:** Haben weitere Felder in `updateNetworkControlsFromMeta()` dieselbe
Form, meldet der `pwa-developer` sie. Belegt ist der Befund für diese zwei Felder.

**Warum die richtige Schicht:** Der Scan ist nicht der Fehler — dass er den Netzwerkstand
nachzieht, ist gewollt. Falsch ist, dass eine Hintergrundantwort über eine laufende Eingabe
schreibt. Das ist eine Frage der Oberfläche, nicht des ESP.

### 1.9 Massnahme 4 in engerer Form — ändern heisst abweichen

**Befund (L324):** `handleDirtyFormInteraction()` (`app.js:2119` laut Lead) setzt
`hasUnsavedEdits = true` bei **jeder** Eingabe. Wer tippt und den Ausgangswert vollständig
wiederherstellt, bekommt beim Modulwechsel trotzdem den Dialog.

**Lösung:** Ein Feld gilt als geändert, wenn sein Wert **vom Ausgangswert abweicht**, nicht
wenn eine Eingabe stattfand. Der Ausgangswert ist **derselbe**, den `prefillDeviceValue()`
aus §1.8 kennt — zwei Antworten auf „was ist der Gerätewert" wären die Gattung aus L289,
und sie liefen auseinander, sobald einer von beiden geändert wird.

**Warum nicht nebensächlich:** Ein Dialog, der bei unveränderten Feldern warnt, lernt den
Nutzer, ihn wegzuklicken — auch dann, wenn er einmal recht hat.

**Engere Form heisst:** Es geht um diese Vergleichsregel, nicht um einen Umbau der
Dirty-Erkennung. Was darüber hinausgeht, meldet der `pwa-developer` zurück.

### 1.10 L303 und L306 — Nachweis über eine nachgebildete Fehlerantwort

**L303** betrifft `runConfirmedButtonAction` (`app.js:9378` laut Lead). Darüber laufen nur
`downloadUpdateAssets`, `resetStm32` und `formatLittleFsFromFiles`. **Keine** lässt sich am
Gerät folgenlos zum Scheitern bringen — ein fehlgeschlagenes Formatieren ist kein Testfall
für eine produktive Uhr, und ein zu langer Text erreicht diesen Weg nie. **L306** betrifft
`toggleFlagButton`/`finishButtonFeedback`; ein Flag-Umschalten scheitert nicht über einen zu
langen Text.

**Deshalb eine Vorschau-Probe mit nachgebildeter Fehlerantwort** (P2.1e, Lead,
`tools/ui-mess/proben/`):

- **L303:** Die Attrappe beantwortet den Formatier-Aufruf mit einem Fehler; die Oberfläche
  darf **nicht** „formatiert" melden.
- **L306:** Die Attrappe lässt ein Umschalten scheitern; der Knopf steht danach wieder auf
  seinem Zustand, **nicht** auf „schaltet…".

**Einmal fehlgeschlagen — gegen einen künstlich zurückgebauten Stand** (DIR-014). 1.4.91
enthält beide Korrekturen schon; ein Lauf dagegen wäre grün und bewiese nur, dass die Probe
läuft, nicht, dass sie etwas erkennt. Der zurückgebaute Stand liegt **im Scratchpad**, nicht
im Arbeitsbaum — eine Fehlschlagprobe im gemeinsamen Baum hat bei paralleler Arbeit schon
einmal einen anderen Agenten in die Irre geführt (L268).

**Warum das genügt:** Beide Befunde betreffen die **Darstellung** einer Fehlerantwort, nicht
ihr Zustandekommen. Was die Oberfläche aus einer Fehlerantwort macht, hängt nicht daran, ob
die Antwort vom Gerät oder von einer Attrappe kommt.

---

## 2. Runde S — die Brücke fertigbauen

### 2.0 Zwei Richtungen, und eine davon war im ersten Entwurf der alten Spec verkehrt

| Was | Richtung | Beleg |
|---|---|---|
| `CAP var-crc`, `FIRMWARE` | **ESP → STM** | `src/esp8266/esp8266.c`, Prüfung auf `"CAP var-crc"` (`:372` ff.); der STM hält `esp8266.cap_var_crc` |
| Prüfsummen-Marke an der Variablenzeile | **STM → ESP** | Der STM markiert, **wenn** der ESP `CAP var-crc` gemeldet hat |
| Quittung `.` und Abweisung `!v` | **ESP → STM** | `ESP-uclock.ino:884-891` sendet, `src/esp8266/esp8266.c` liest |

**Alle drei Neuerungen dieser Runde laufen STM → ESP.** Der ESP müsste wissen, was der STM
kann, und **dafür gibt es keinen Kanal.** **Die Information reist im Strom mit**, nicht in
einem Sitzungszustand. Was nicht überdauert, kann nicht veralten.

### 2.1 A39 / L259 — kein verschachtelter 194er-Stoss mehr

✔ unverändert im Code: `src/main.c:3217` ruft im `ESP8266_IPADDRESS`-Zweig direkt
`var_send_all_variables()`. Der `SYNCVARS`-Zweig (`:3243-3266`) merkt vor und begründet es:
`schedule_esp8266_messages()` wird **auch aus der Warteschleife von `var_send_buf()`**
gerufen; von dort liefe der Vollabgleich verschachtelt — der Schaden aus L102.

**Variante (b), vom Nutzer gewählt:** Der `IPADDRESS`-Zweig setzt `var_sync_pending = 1`
**und** merkt den IP-Lauftext vor; der Hauptloop sendet den Abgleich (`:3946-3949`) und
setzt **danach** den Ticker. Der lokale Puffer `buf[32]` entfällt.

**`pending_weather_ticker_restore` wandert mit dem Ticker mit** (heute `:3240`). Keine der
vier Teilbedingungen des Restores wird vereinfacht.

**A39 trägt doppelt:** Die Zuordnungszeilen aus §2.4 landen im selben 256-Byte-Ring. **Ohne
A39 wäre §2.4 riskant; mit A39 ist der Ring nie unbeaufsichtigt.**

### 2.2 L260 / A42 — ein Erfolgskriterium, das den ganzen Satz prüft

`var_sync_check()` (`ESP-uclock.ino:753`) prüft auf `HARDWARE_CONFIGURATION != 0xFFFF`, das
**dritte** von rund 194 Kommandos.

**Lösung:** `var_send_all_variables()` sendet als **erstes** eine Eröffnungszeile („ich
markiere", „ich schliesse mit einer Marke ab") und als **letztes** die Abschlussmarke:

| Lage | Verhalten des ESP |
|---|---|
| Eröffnungszeile gesehen | Der Abgleich gilt erst mit der Abschlussmarke als vollständig |
| keine Eröffnungszeile (alter STM) | heutiges Kriterium, `HARDWARE_CONFIGURATION != 0xFFFF` |

Kosten: **zwei** Kommandos von dann rund 196.

#### Die Prämisse, und warum sie trägt — drei Punkte statt einem (L269)

1. **`var_cmd_min_len()` liefert für einen unbekannten Buchstaben `0`**
   (`ESP8266/ESP-uclock/vars.cpp:914`); die Zeile passiert die Längenprüfung. **E2.1 hat das
   ausdrücklich erhalten** (L265).
2. **`var_set_parameter()` hat keinen `default:`-Zweig** — die Zeile fällt wirkungslos
   heraus.
3. **Der eigentliche Träger:** Die gefährliche Zeigerarithmetik der L237-Gattung steht
   **ausschliesslich in den `case`-Rümpfen**. Ein unbekannter Buchstabe erreicht keine.

**Punkt 3 macht die Prämisse versionsrobust** — auch für ESP-Stände vor A37.

#### Drei Bedingungen an die neuen Zeilen — Auflage für den `stm-developer`

1. **Kommandobuchstabe ausserhalb von `N n S T D A C M O t a l I`** (L269).
2. **Die Nutzlast endet nicht auf `*` + vier Hexziffern.**
3. **Die Nutzlast bleibt unter 72 Zeichen** (`VAR_RETRY_CMD_LEN`).

#### Verpackung: `var <freierBuchstabe>…`, verbindlich

> **Ein eigenes Top-Level-Präfix wäre gefährlich.** Die `if/else if`-Kette auf der
> ESP-Seite hat **kein abschliessendes `else`** — auf ein unbekanntes Top-Level-Präfix
> antwortet der ESP **gar nicht**. `var_send_buf()` liefe je Zeile 3 Sekunden leer, **ohne
> `watchdog_reload()`**; zwei Zeilen je Vollabgleich sind 6 Sekunden bei einem
> 20-Sekunden-Fenster. **Das ist exakt die Mechanik aus L266.**

Ein eigenes Präfix nur über `esp8266_uart_puts()` — **mit Vermerk im Code**. Die Taktung der
Nachforderung (bis zu drei Versuche, rund 10 s) bleibt.

### 2.3 C26 / L253 — Zähler **und** Markenpflicht, vom Nutzer entschieden

1. **Sichtbar machen:** Zähler für Zeilen **nach** der ersten Marke **ohne** Marke,
   **ablesbar über den Logring**.
2. **Markenpflicht** für `HARDWARE_CONFIGURATION`, Update-Host, Update-Pfad — innerhalb
   eines Abgleichs mit Eröffnungszeile sofort, ausserhalb sobald eine Marke gesehen wurde.

**Abgewiesen mit `!v`, nicht mit Schweigen** (L266). **Ausschlag:** Ein leerer Update-Host
hat zuletzt fremde Firmware geholt (L42, DIR-009). **Kein Dauerzustand:** schlimmster Fall
ist derselbe wie heute, nur erkennbar. Liste **aufgezählt, nicht gemustert**.

### 2.4 A35 / L233 — Zuordnung der Quittung, nach dem Nachweis L266

```
ACK c3
.
```

| Lage | Verhalten |
|---|---|
| **alter STM** | verwirft `ACK c3` und liest den Punkt **unverändert** als `OK` |
| **neuer STM** | vergleicht die Zuordnung mit dem ausstehenden Kommando; passt sie nicht: **verwerfen und zählen**, A32 sendet nach |

**Die Zuordnungszeile geht jeder Quittung voraus**, dem Punkt wie dem `!v`. **Herkunft der
zwei Zeichen:** aus der Prüfsumme; **in S.2 festzulegen**, ob gespiegelt oder selbst
gerechnet, und was bei einer unmarkierten Zeile gesendet wird; S.6 prüft, dass beide Seiten
dieselben Zeichen bilden. **Grenze:** Mit 1:256 trifft eine verspätete Quittung zufällig —
das Verhalten von heute. **Kosten (L266):** rund 1,5 kB je Vollabgleich; auf altem STM rund
135 ms. **Teil 2 ist nicht in diesem Paket.**

### 2.5 Zwei Warnungen für jeden künftigen Entwurf (aus der alten Spec §6.5)

> **Ein Mechanismus, dessen Sicherheit an einem Hardware-Nebeneffekt hängt, ist nicht
> abgesichert — er hat bisher Glück gehabt.**

`cap_var_crc` hängt an einem GPIO-Puls (`src/esp8266/esp8266.c:738-740`) **und** am
Einschaltstrom über `PB0` (L183). **Keine Neuerung dieser Runde verlässt sich auf eine
Fähigkeitsmeldung.**

> **Die `var `-Verpackung ist nicht stumm, sondern regulär quittiert — und gerade das
> rettet den Entwurf.**

**Wer die Verpackung ändert, ändert die Watchdog-Bilanz.**

### 2.6 A16 / L106 — Tetris und Snake

Null `watchdog_reload()`; nach rund 3,6 Steinen ist das 20-s-Fenster um. **Vom Nutzer mit
„Drin lassen" bestätigt (Ent-6)**, einschliesslich des Spiellaufs von 60 Sekunden nach dem
STM-Flash. S7 führt danach zwei Stellen mehr (S.20).

### 2.7 Das Flash-Gate, zweistufig (L296 und die Warnung der Nachzählung)

**Die Lage:** 1'924 Byte frei auf dem F103, Warnschwelle 2'048 unterschritten. `-flto` ist
gesetzt, `my_gmtime` gesperrt. **Betrifft nicht die F411-Uhr**, wohl aber den
Release-Build (L165). In diesen Flash sollen der Kern **und acht STM-K-Punkte**.

**Warum zwei Stufen.** Bei Punkten von wenigen Dutzend Byte ist die Schätzunsicherheit so
gross wie der Wert, und acht davon summieren sich — daher die Warnung des Prüfers, **zu
messen**. Eine Messung erst beim Release-Bau kommt zu spät: Dann fällt die Streichung unter
Zeitdruck, mit fertigem Code.

**Stufe 1 — Vorabschätzung (S.12, `firmware-analyst`).** Je Teil Unter- und Obergrenze aus
gemessenen Zuwächsen (L165, L289/L296) und neuen unbedingten Zeichenketten (L168). **Zweck:**
erkennen, ob **schon der Kern** nicht passt. Kein `Bash` beim Analysten; er liest die Map
des letzten Builds.

**Stufe 2 — Testbauten (S.18 und nach jedem S.19x, Lead).** Nach dem Kern und nach **jedem**
K-Punkt baut der Lead **nur das F103-Ziel**, ohne ZIP und ohne Rollout, und trägt den
Zuwachs in die K-Tabelle ein. Reihenfolge nach Schaden (Ent-2).

**Was bei Unterschreitung geschieht, regelt Ent-2**, nicht der Lead: Vorschlag „anhalten und
vorlegen" bei A20, A13+A47, A3+A48, „zurückstellen und weiter" bei den übrigen. Ein
zurückgestellter Punkt wird **ganz** zurückgenommen, **erneut gemessen** und steht mit
seiner Zahl in der K-Tabelle.

**Die Testbauten und R1 — vom Lead entschieden (Ent-7).** R1 soll verhindern, dass
**parallel** gebaut wird: zwei Ziele im selben `build/`, ein Teammate, das baut, während ein
anderer schreibt. Das bleibt gewahrt — die Testbauten macht nur der Lead, seriell, und nur
zwischen abgeschlossenen STM-Tasks. Der Wortlaut „der Lead baut einmal am Ende" stammt aus
einer Zeit ohne Flash-Gate; mit einem Gate, das je Punkt messen soll, ist er nicht mehr der
Zweck der Regel.

**Gegenprobe am Ende (S.23):** S8b neben Schätzung und Testbauten. Rest unter der Reserve ⇒
kein Rollout. Schätzung über ihrer Obergrenze ⇒ Befund über die Schätzmethode. Wie S8b die
unterschrittene Warnschwelle bewertet, prüft der Lead vor S.23.

### 2.8 C31 / L298 — keine Schlüssel auf der UART

✔ `eepromdata.cpp:252-253`, `:258-259`. Ersetzt durch `gesetzt`/`leer` — **kein Wert, keine
Länge**. In S, weil S den ESP als Erstes flasht; abgenommen in S.11. **Kodierung und
Zeilenende vor dem Patch feststellen.** **Zweitweg (AKS.11)** über die STM-Logausgabe
(`src/esp8266/esp8266.c:367-370`) und die Logspiegelung (A19) klärt S.7. **Nicht Teil:**
vorhandene Pi-Logdateien (Nutzer), C6b.

### 2.9 A46 — in S, mit Vorbehalt; A47 jetzt mit A13

**A46 / L291** löst AKH.4 der alten Spec für die Datenpfade ein; ändert nur die Zählung
abgewiesener Zeilen. Weit hinten in der Reihenfolge (Ent-2).

**A47 / L292** fährt **zusammen mit A13**: gleiche Datei, gleiche Gattung, rund acht Zeilen.
**L96: je Stelle den Verbraucher prüfen**, nicht pauschal ändern.

Beide ändern nichts an **angenommenen** Zeilen; ein Fehlschlag im Vollabgleich bleibt dem
Protokoll zuzuordnen.

### 2.10 C32 / L299 — bewusst nicht in S

Anderer Gegenstand, offene Entwurfsfrage, Flash, kein Sicherheitsbefund. **Kandidat der
nächsten Brückenrunde, zusammen mit A35 Teil 2.** Beim Lösen fällt die
`localStorage`-Umgehung in der PWA weg.

### 2.11 Einspielen

**Erst ESP, dann STM;** dazwischen der Rückfallnachweis AKS.6 (S.11). Nach dem ESP-OTA:
`install-app.sh --check`, Update-Quelle (DIR-009). `flash-stm.sh` setzt den STM vor dem
Flash zurück (DIR-010).

**Sichtbar:** IP-Lauftext wie bisher, vollständig; im Mitschnitt nur „gesetzt"/„leer";
Tetris und Snake über 20 Sekunden; die Legacy-Overlayseite nimmt keinen Index über dem
Bestand mehr an.

### 2.12 N1 — der Legacy-Overlayindex, und warum er mit S fährt

✔ `http_overlays()` liest `oidx` per `atoi` (`http.cpp:4689`, `:4693`), prüft nur
`oidx != 0xff` (`:4696`), erhöht bei `oidx == n_overlays` den Bestand (`:4702-4705`) und
schreibt ab `:4710` in `overlays[oidx]` — 32 Plätze. **Aus dem LAN, laut Nachzählung auch
als `<img>` von einer fremden Seite.**

**Die Typbreite ist Teil des Befunds** (L256): `oidx` ist `uint_fast8_t`, auf Xtensa breiter
als ein Byte; `atoi ("300")` bleibt 300. Auf einem Host mit 8-Bit-Typ bräche es auf 44 um
und sähe harmloser aus. Prüfstand mit den Typen des Ziels (AKS.14).

**Fix (K):** `oidx < MAX_OVERLAYS && oidx <= n_overlays`, **vor** dem ersten Schreibzugriff
**und** vor `n_overlays++`.

**Warum mit S und nicht mit F:** S-ESP ist der **nächste** ESP-Flash; F kommt erst nach dem
ganzen STM-Teil. Die Zusage an S.11 bleibt haltbar — der Legacy-Overlay-Handler berührt den
Vollabgleich nicht. Dass S damit `http.cpp` anfasst, ändert nichts an der Reihenfolge: S und
F durften ohnehin nicht parallel laufen, und sie teilen keine Funktion.

**Die STM-Seite:** `overlay_set_n_overlays` prüft nicht (A3), die Setter nur
`idx < n_overlays` (L293) — ● S.7 klärt die Kette. **A3+A48 schliessen sie** (S.19c).
**Abnahme über den Prüfstand, nicht am Gerät** (AKK.6): Nach N1 weist der ESP jeden Index
ab, bevor er den STM erreicht. S15 wird in S.20 auf A3+A48 und A24 erweitert.

**Mit-geprüft, nicht mit-korrigiert:** der `disp`-Zweig (`:4684`) — S.6 beantwortet am Code,
ob das ein eigener Befund ist.

---

## 3. Runde F — Flash-Überwachung

### 3.1 Eine Funktion für drei Stellen

✔ `http_remote_stm32_filename_matches()` (`http.cpp:974-992`) gibt `0` zurück ohne Filter
und bei Namen unter sechs Zeichen. Daraus: Der Legacy-Flashzweig weist bei 65535 jeden
Namen ab (C9c6); die Legacy-Liste ist bei 65535 **automatisch leer**; das ungeprüfte
`fname + 6` (C28) verschwindet. **Drei Befunde, eine Ursache**, ein Task (F.1).

**Der Rückweg bleibt offen:** Der Hinweis nennt `./tools/flash-stm.sh`, über
`stm32_log_append()` unter 120 Zeichen (L257). Die API-Liste (`:12063-12071`) — Ent-5.

### 3.2 Smoketest-Stufe

`smoke-device.sh` meldet 65535 als Fehlschlag, aus einem **lesenden** Endpunkt;
Gegenprobe gegen festen Testwert. **E11 schreibt dieselbe Datei** — nacheinander.

### 3.3 Warum F zuletzt, und warum ein STM-Flash als Probe

Nach S ist der STM-Stand bekannt; ein Flash mit vorhersagbarem Ergebnis ist die ehrlichste
Probe. **Der Legacy-Pfad wird nicht scharf gefahren.**

### 3.4 Einspielen

**Nur ESP**, danach der STM-Flash als Probe. **Sichtbar:** Die Legacy-Seite bietet bei
unbekannter Hardware keine Datei an und nennt den Rückweg; ein zu langer Overlay-Text und
eine zu lange SSID werden vom Gerät abgewiesen statt gekürzt; die Logwache zeigt zwei
Verlustzähler. **Braucht C38 eine PWA-Grenze (§3.5), kommt sie danach** — Firmware zuerst
(L241).

### 3.5 C38 / L323 — SSID und WLAN-Schlüssel abweisen statt still kürzen

`http_api_network_client_set()` (`http.cpp:8664-8670`) und `http_api_eeprom_settings_set()`
(`:8779-8785`) übernehmen SSID und Schlüssel per `toCharArray` mit fester Länge. **Ein still
gekürzter Schlüssel ist ein falscher Schlüssel**; die Uhr verbindet sich danach nicht mehr,
und gemeldet wurde Erfolg. Dieselbe Gattung wie C22 (L206).

**Lösung:** dieselbe Abweisung wie bei den übrigen Zeichenkettenfeldern —
`http_check_strvar_len()` oder dieselbe Byte-Regel, **keine zweite**. Gleiche Datei wie die
übrigen F-Tasks, deshalb seriell nach F.5e.

**Abnahme ohne schreibenden WLAN-Aufruf.** `network_client_set` bleibt für jeden Gerätetest
gesperrt — ein Fehler dort trennt die Uhr vom Netz, und der Rückweg ginge über den AP. Die
Abnahme läuft **nur am Quelltext und am Prüfstand** (F.5h: zu lang ⇒ abgewiesen, Grenzwert ⇒
angenommen, einmal fehlgeschlagen gegen die alte Fassung). Am Gerät nur lesend: Das WLAN
bleibt nach dem F-OTA verbunden (F.10).

**Offene Frage an den Review (F.6):** Kann die PWA eine SSID oder einen Schlüssel mit **mehr
Byte** senden, als der ESP annimmt — etwa Umlaute in der SSID bei `maxlength` in Zeichen?
Wenn ja, gehört eine PWA-Grenze dazu, als eigener Task **nach** dem F-OTA (L241). Bis dahin
würde der ESP abweisen statt zu kürzen — schlechter benutzbar, aber nicht mehr still falsch.

---

## 4. Abhängigkeiten und Parallelität (R3b)

### 4.1 Inhaltlich unabhängig heisst nicht baulich unabhängig

Jeder Release-Build (`release-zip`) baut STM, ESP **und** `app-gz` aus dem **gemeinsamen**
Arbeitsbaum (R1):

- **Kein Release-Build, solange irgendein Produktcode-Task offen ist** — sonst trägt das ZIP
  einer Runde den halbfertigen Code einer anderen, unter der Versionsnummer der ersten.
- **`app-gz` nur bei stillem Arbeitsverzeichnis** (R2).

**Daher:** P2 wird gebaut (P2.6), bevor S.1 beginnt. Die Abnahme von P (P.18, P.19) läuft
parallel — sie schreibt keinen Produktcode.

### 4.2 S und F

Der ESP-Teil von S liegt in `ESP-uclock.ino`, `eepromdata.cpp` und — **nur für N1** — in
`http_overlays()`. Der Zähler aus C26 geht **nur in den Logring**. **Braucht S weitere
Stellen in `http.cpp`, ist das eine Rückfrage an die Spec.**

**Nicht parallel:** Beide Runden bauen ein ESP-Release aus demselben Baum, und S.11
verspricht „neuer ESP gegen alten STM" mit **genau den S-Änderungen**. **F.1 beginnt nach
S.23.** F.4 und K.W (Werkzeug) jederzeit. **Wer zwingend parallel will, braucht ein
Worktree (R6).**

### 4.3 `src/**` in S, `http.cpp` in F

Nur der `stm-developer`, S.14 bis S.19h nacheinander, mit den Testbauten des Leads
dazwischen. In F schreibt der `esp-developer` F.1 bis F.5g nacheinander in `http.cpp`.

---

## 5. Testdurchlauf und M2 — als Nutzerschritt geplant

**Der Engpass ist das Passwort.** M2 entsteht nur im Terminal des Nutzers; seit L300 bricht
`snapshot-device.sh` ohne Passwort vor dem ersten Abruf ab.

1. **Nutzer-Sitzung:** Einspielen **und unmittelbar danach M2**. Der Lead liefert die Zeilen.
2. **Lead:** Smoketest, `install-app.sh --check`, Gerätabnahme, `watch-log.sh` mitgelesen.
3. **`pwa-tester`:** Phasen 0 bis 4 und 9; Phase 0 prüft, dass M2 **vorliegt**. Der Bericht
   nennt die enthaltenen K-Punkte.
4. **Unvollständig ist nicht abgenommen** (AKZ.4).

**S:** Flash, 60 Sekunden Spiel, M2 — eine Sitzung (S.24). **F:** zwei Nutzerschritte, eine
Sitzung, wenn F.10 dazwischen grün ist. **P2:** ohne eigenen Durchlauf; getragen von den
Proben P2.1c und P2.1e und der lesenden Geräteprobe P2.8, Gerätenachweis mit Speichern in
S.26 (§1.7).

**S.26 trägt zwei zusätzliche Nachweise aus 1.4.91:**

- **B35 am Zeitserver** in Phase 2 — erst nach B40 aussagekräftig.
- **B36** über einen Schreibaufruf, der **fehlschlägt, aber nichts verändert**: ein zu
  langer Update-Host, den der ESP mit `error=2` abweist. Die Fehlermeldung bleibt stehen
  und wird nicht von einer Erfolgsmeldung überschrieben, und **Phase 9 belegt im
  Abschlussvergleich, dass der Gerätewert gleich geblieben ist**. Eine Abweisung **ist**
  der Fall, den B36 betrifft — deshalb geht das, ohne die Konfiguration der produktiven
  Uhr anzufassen.

**L303 und L306 gehören nicht in S.26** — ihr Weg lässt sich am Gerät nicht folgenlos zum
Scheitern bringen (§1.10). Eine frühere Fassung dieser Spec hatte sie dort, mit einem zu
langen Zeitserver als Auslöser; der erreicht keinen der beiden Wege.

**Was ein Durchlauf nicht leisten kann:** einen STM-Guard nachweisen (§1.6), einen Wettlauf
gezielt herstellen (§1.7), eine Fehlerantwort einer Aktion erzeugen, die man an einer
produktiven Uhr nicht scheitern lässt (§1.10), und einen schreibenden WLAN-Aufruf sicher
abnehmen (§3.5). Dafür stehen Prüfstände und Proben.

---

## 6. Runde K — Kleinkram, verteilt auf die vorhandenen Flashes

### 6.1 Warum keine eigene Runde

Jeder Flash kostet eine Nutzer-Sitzung, ein M2, einen Testdurchlauf und beim ESP ein
OTA-Risiko (L180). Kleinkram, der ohnehin auf eine geflashte Laufzeit muss, **fährt mit**.
**Der Preis ist Zuordenbarkeit**; §6.2 hält K-Punkte von den Stellen fern, deren Isolation
die Spec zusagt.

### 6.2 Welcher Punkt mit welchem Flash fährt

| Träger | Dateien | Fährt mit | Tasks |
|---|---|---|---|
| **P2** | `app.js`, `sw.js`, `i18n/*.json`; `index.html`, `styles.css` | nächster PWA-Upload | P2.1 (B40, Massnahme 4 und die Probe L303/L306 sind keine K-Punkte, fahren aber mit) |
| **S-ESP** | ESP-Quellen **mit Brückenbezug**: `ESP-uclock.ino`, `vars.cpp`, `eepromdata.cpp` | ESP-OTA von S | — (dazu N1 als Ausnahme, S.5) |
| **S-STM** | `src/**` | STM-Flash von S, **gemessen** | S.19a–S.19h |
| **F** | ESP-Quellen **ohne Brückenbezug**: `http.cpp`, `httpclient.cpp` u. a. | ESP-OTA von F | F.5a–F.5e (C38 ist kein K-Punkt, fährt aber mit, F.5g) |
| **W** | `tools/**`, `.claude/**`, `CLAUDE.md` | kein Flash | K.W |

**Warum ESP-Punkte ohne Brückenbezug mit F:** S.11 sagt einen isolierten Rückfallnachweis
zu; fremde ESP-Änderungen im selben OTA verwischen einen Fehlschlag dort.

**Die eine Ausnahme ist N1** (§2.12) — kritisch, aus dem LAN auslösbar, S ist der nächste
ESP-Flash. Kein Präzedenzfall für andere `http.cpp`-Punkte.

**STM-K-Punkte im Brückenpfad** markiert S.21 einzeln, die Release-Notiz S.23 nennt sie.
**STM-K-Punkte im Anzeigepfad** (A24, A45) bekommen in S.25 einen Blick auf die Uhr.

**Wo es nicht passt:**

- **Schreibender Gerätezugriff zur Abnahme** (R5): Vorschau, Prüfstand oder Quelltext —
  oder zurückstellen.
- **STM-Guards:** nur über den Prüfstand (AKK.6).
- **Nach S.18 entschiedene S-STM-Punkte:** nächste STM-Runde.
- **C4** passt in keinen Träger — sein Nachweis verlangt einen Bau ohne andere Änderung.

### 6.3 Was ein K-Punkt ist, und was er nicht mehr ist

**K heisst:** höchstens rund zehn Zeilen **und** keine Entwurfsfrage. **Wächst ein Punkt
darüber hinaus, ist er kein K-Punkt mehr** (AKK.3) — der Umsetzer hört auf und meldet
zurück. Beim F103 ist das wörtlich zu nehmen.

### 6.4 Die Abnahme je Punkt ist Pflicht

- **Vor der Umsetzung:** „erledigt, wenn …" mit Instrument (AKK.1).
- **Nach der Umsetzung:** Beleg **dieses** Commits oder Messwert (AKK.4), kein Verweis auf
  einen anderen Befund.
- **Abgezählt** (AKK.2, K.0) — eine Liste, die weniger sieht als dasteht, ist L235.
- **Ein Befund kann beim Einplanen falsch sein** — so B26: Die Kennungen galten als
  ungenutzt und sind Panelnamen in `tools/ui-mess`. Die Abnahme hielt das fest, statt sie zu
  entfernen.

### 6.5 Testdurchlauf und Versionen

K-Punkte laufen im Durchlauf **ihres Trägers**; der Bericht nennt sie. Keine eigenen
Versionen.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Runde |
|---|---|---|---|
| `BEFUNDE.md` | P nachführen; B40/L321, Massnahme 4, L303/L306, Overlay-Text, B17; C26/A35-Text auf den Entscheidungsstand; S und F nachführen, C38/L323 | `doc-writer` | P, P2, S, F |
| `app.js` | **ausgeliefert:** B35, B36, B34, B33, R1–R4, B26, L304, B37, B38. **P2:** Overlay-Text in `TEXT_FIELD_LIMITS` (Editor und Import), B40 über `prefillDeviceValue()`, Massnahme 4 in `handleDirtyFormInteraction()`, B17 nur wenn klein | `pwa-developer` | P, P2 |
| `data/app/i18n/*.json` | ausgeliefert (B27 u. a.) | `pwa-developer` | P |
| `data/app/index.html` | ausgeliefert (Rückfalltext B27) | `ui-developer` | P |
| `tools/ui-mess/proben/` | Probe B40 mit verzögerter Antwort (P2.1c); Probe L303/L306 mit nachgebildeter Fehlerantwort (P2.1e) | Lead | P2 |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | Eröffnung/Abschluss, Zuordnungszeile, Markenpflicht mit `!v`, Zähler in den Logring | `esp-developer` | S |
| `ESP8266/ESP-uclock/eepromdata.cpp` | C31 | `esp-developer` | S |
| `ESP8266/ESP-uclock/http.cpp` | **S:** N1 in `http_overlays()`. **F:** Flashzweig, Legacy-Liste, C28, C27+N2, E12, C25, Overlay-Text, **C38**; je nach Ent-4/Ent-5 | `esp-developer` | S, F |
| `ESP8266/ESP-uclock/httpclient.cpp` | C9k | `esp-developer` | F |
| `src/vars/vars.c` | Eröffnung/Abschluss senden; A20; A14 | `stm-developer` | S |
| `src/main.c` | A39; A3+A48; A49; A46-Anteil | `stm-developer` | S |
| `src/esp8266/esp8266.c` | Zuordnung; A13+A47 | `stm-developer` | S |
| `src/tables/tables.c` | A24 | `stm-developer` | S |
| `src/display/display.c` | A45 | `stm-developer` | S |
| `src/tftled/tftled.c`, `src/esp-spiffs/esp-spiffs.c` | A46 | `stm-developer` | S |
| `src/tetris/tetris.c`, `src/tetris/snake.c` | A16 | `stm-developer` | S |
| `tools/checks/` | Prüfstände AKS.5, AKS.14, **C38**; A46-Fälle in `htoi-laenge.c`; **S15 auf A3+A48, A24**; S7 | Lead | S, F |
| `tools/smoke-device.sh` | Stufe 65535 (F); E11 (K) — nacheinander | Lead | F, K |
| `tools/watch-log.sh` | C25-Format | Lead | F |
| `tools/hooks/file-ownership.py`, `tools/ui-mess/`, `CLAUDE.md`, Skill `/release` | E20, E14-Rest, E24-DIR-002, B20 | Lead | K |
| Versionsdateien | je Einspielschritt | `release-engineer` | je Runde |

**Kodierung und Zeilenenden vor jedem Patch feststellen** (DIR-015). `http.cpp` ist UTF-8,
`ESP-uclock.ino` hat gemischte Zeilenenden; `eepromdata.cpp`, `httpclient.cpp` und die
STM-Quellen sind vor dem Patch zu prüfen. Umschrift in neuem Text in `src/**` und
`ESP8266/ESP-uclock/`. **`grep -a` in `src/**`** (B21).

---

## Prüfung gegen die Architektur-Checkliste

### Proper architecture

**Bleibt die PWA parallel zur Legacy-Oberfläche?** Ja. F **stärkt** die Legacy: Ihr
Flashpfad bekommt denselben Schutz wie der API-Pfad. N1 schliesst einen Schreibzugriff im
Legacy-Overlayformular, ohne das Formular anzutasten.

**Werden die Wetter-Endpunkte eingehalten?** Ja. B35 prüft Ort und Koordinaten vor ihren
Settern; die Wetterabfrage bleibt unberührt.

**Bleibt die Restore-Bedingung um `pending_weather_ticker_restore` vollständig?** **A39
berührt sie.** Variante (b) verschiebt das Setzen des Tickers in den Hauptloop — **das Flag
wandert mit**; keine Teilbedingung wird vereinfacht. Nachweis in S.15, Prüfung in S.21.
**E17 (Umbenennen) ist nicht Teil von S** — Empfehlung der Spec, Entscheidung des Nutzers.

**Werden App-Assets ausschliesslich als `.gz` ausgeliefert?** Ja. C27 lässt `fs_show` eine
leere Datei als eigenen Zustand melden — das **stärkt** die Invariante „Grösse > 0".

**Greift die Änderung an der richtigen Schicht an?**

- **B35** — Vorabprüfung an der Endpunktgrenze; beim Overlay-Text bis zum F-OTA strenger als
  das Gerät, danach zieht der ESP nach.
- **B40** — der Scan ist gewollt; falsch ist das Überschreiben einer laufenden Eingabe.
  Oberfläche, mit der bestehenden Regel `prefillDeviceValue()` (§1.8).
- **Massnahme 4** — „geändert" heisst „weicht vom Gerätewert ab", mit demselben Gerätewert
  wie B40 (§1.9).
- **L303, L306** — Darstellung einer Fehlerantwort; nachgewiesen dort, wo sie entsteht,
  in der Oberfläche, mit nachgebildeter Antwort (§1.10).
- **C38** — die Abweisung gehört auf den ESP, wo gespeichert wird; eine PWA-Grenze wäre nur
  die Vorabprüfung und folgt nach dem OTA, falls nötig (§3.5).
- **R2** — die richtige Schicht ist der ESP (Ent-4); die PWA-Regel ist robust ohne
  englischen Detailtext.
- **R3** — eine Umgehung von C32, nur ehrlich gemacht.
- **N1** — Fix auf dem ESP, wo der Wert ankommt, **und** ein eigener Guard auf dem STM.
- **A35** — eigene Zeile statt angehängter Zuordnung (L266).

### Scalable systems

**Wie viele STM-Kommandos?** P/P2: keine; B35 und R4 **verhindern** Kommandos. S: rund
**196** statt 194 je Vollabgleich; A39 ändert den Zeitpunkt. N1 **verhindert** Kommandos mit
ungültigem Index. F: keine; C38 **verhindert** einen Schreibvorgang mit gekürztem Wert.

**Wie viele Byte pro Minute zusätzlich auf der UART?**

| Posten | Richtung | Kosten |
|---|---|---|
| Eröffnungszeile + Abschlussmarke | STM → ESP | je Vollabgleich rund 22 Byte plus zwei Quittungen |
| Zuordnungszeile | ESP → STM | **rund 1,5 kB je Vollabgleich** |
| auf **altem** STM ein `log_flush()` je Zeile | — | **rund 135 ms über 194 Kommandos** (zwischen S.10 und S.24) |
| Zähler, Erfolgszeile | — | **0 Byte** auf der Brücke |
| C31, A20 | — | **weniger** |
| A46, A49 | STM → ESP | gedrosselt; A49 meldet je Grund einmal mehr |

Die 1,5 kB laufen **strikt im Wechsel** mit dem Warten des STM; häufen könnten sie sich nur
im verschachtelten Vollabgleich, den A39 beseitigt. **Darum gehören A39 und A35 zusammen.**

**Bleibt jeder Pfad unter 20 s?** Gefährlichster Pfad ist die ausbleibende Quittung (L266,
L269) — daher eigene Zeile, `!v`, `var …`. A16 beseitigt zwei Pfade, die heute garantiert
über 20 s laufen. Bestand: S7.

**Wie oft pollt die PWA?** Unverändert — auch durch B40 nicht: Der Scan läuft wie bisher,
nur sein Ergebnis schreibt nicht mehr über eine laufende Eingabe.

**Wie lange blockiert der Hauptloop?** AKS.2: höchstens rund 2 % gegen L226.

**Hartkodierte Grenzen?** `TEXT_FIELD_LIMITS` als eine Kopie von `vars.h`, im Review
abgeglichen; die Markenpflichtliste benannt; STM-Guards aus `sizeof`; C38 über dieselbe
Prüffunktion wie die übrigen Felder. **Härteste Grenze: der F103-Flash** — zweistufiges
Gate (§2.7).

### Secure by design

**`escapeHtml` bei jedem `innerHTML`?** B33 hält Binärdaten aus dem DOM; Meldungen über
`announceStatus`.

**Fremddaten als nicht vertrauenswürdig?** **N1 ist der schärfste Fall:** ein Index aus
einer URL, die jede fremde Seite per `<img>` auslösen kann, als Schreibadresse — geprüft vor
dem ersten Schreibzugriff, und der STM prüft unabhängig selbst. C26 und B36 schützen den
Update-Host von beiden Enden.

**Keine Credentials in Logausgaben?** **C31 und A20.** AKS.11 klärt einen möglichen
Zweitweg. Die Abnahme prüft die Form der Zeile, nicht die Abwesenheit des Schlüssels. C6b
bleibt offen.

**Neue Endpunkte, die Konfiguration schreiben?** Keine. **E12 ändert, was ein fehlender
Parameter bewirkt** — vom Löschen zum Abweisen; F.6 prüft die PWA darauf. **C38 ändert, was
ein zu langer WLAN-Wert bewirkt** — vom stillen Kürzen zum Abweisen; abgenommen ohne
schreibenden Gerätezugriff.

### Stable & reliable

**Wird jeder Fehler ausgewertet?** P: B36, R1, R3, R4 (ausgeliefert); L303 und L306 —
Fehlerantworten dürfen nicht als Erfolg oder als hängender Zwischenzustand enden (P2.1e).
Brücke: A35. K: ein Punkt ohne Abnahmesatz ist eine Erledigtmeldung ohne Grundlage. F:
E12, Overlay-Text und C38 — Erfolg beim Nichtstun bzw. Verbiegen wird zur Abweisung.

**Leere `catch`-Blöcke?** R3 beseitigt einen. **Zwei stehen bewusst:**
`getPersistedAmbilightState()`/`setPersistedAmbilightState()` fangen das Fehlen von
`localStorage` ab und fallen mit der C32-Umgehung. P2.4 prüft, dass keine neuen entstehen.

**Stille Verwerfungen ohne Zähler?** C26-Zähler; gezählte unpassende Quittung; A46; A49; C25
trennt zwei Ursachen. **Eine bleibt zählerlos und ist beabsichtigt:** Der alte ESP verwirft
die Eröffnungszeile — sie wird quittiert (§2.5).

**Werden Werte still zurechtgebogen?** **B40 ist genau das, nur nicht vom Gerät, sondern von
der Oberfläche:** Sie tauschte die Eingabe gegen den alten Wert und speicherte ihn ohne
Hinweis. P2.1b beseitigt es. Auf dem ESP kürzten zuletzt noch der Overlay-Text und **SSID
und WLAN-Schlüssel** (C38) still — beides in F. **Massnahme 4 ist die Gegenrichtung:** eine
Warnung ohne Grund, die das Vertrauen in die berechtigte untergräbt.

**Ist der Zustand nach einem Abbruch definiert?** Backup-Import: alles ausser dem genannten
Feld. Layout-Restore: Kennung 6 bei verschwundener Datei weiter, bei verbliebener abbrechen
und melden. Flag-Umschalten (L306): nach Fehlschlag wieder der Zustand vor dem Klick, nicht
„schaltet…". Vollabgleich: keine Marke ⇒ Neuanforderung, gewollt. Zuordnung ohne Quittung:
beim nächsten Kommando gelöscht. N1: abgewiesener Index ⇒ **nichts** geschrieben, auch
`n_overlays` nicht. C38: abgewiesener Wert ⇒ alter WLAN-Wert bleibt, Verbindung bleibt.
Zurückgestellter K-Punkt: ganz zurückgenommen und neu gemessen.

**Wird ein gesetztes Flag auf jedem Pfad aufgelöst?** IP-Ticker-Vormerker, gemerkte
Zuordnung, Markenerwartung: im selben Durchlauf gelöscht, in dem sie gewertet werden. Der
R3-Merker gilt für die Sitzung. Der „vom Nutzer geändert"-Zustand aus
`prefillDeviceValue()` gilt bis zum Speichern oder Neuladen — wie bei der AP-SSID; und
**`hasUnsavedEdits` fällt nach Massnahme 4 auch dann, wenn der Nutzer den Ausgangswert
wiederherstellt**. Der Busy-Zustand eines Umschaltknopfs fällt auch im Fehlerpfad (L306,
P2.1e). **Die A49-Maske wird nie zurückgesetzt** — gewollt: „seit dem Start schon
gemeldet". S.21 prüft die Firmware-Zustände.

---

## Verworfene Alternativen

**B35 über `maxlength` in Byte; B35 je Eingabefeld; zu lange Texte kürzen.** Siehe §1.1.

**Overlay-Text erst prüfen, wenn das Gerät abweist (nach F).** Bis dahin kürzt das Gerät
still und meldet `{"ok":true}`.

**P2 nach F oder mit dem S-Build zusammen.** Nach F wären Overlay-Text, B40 und Massnahme 4
bis zum Paketende offen; mit dem S-Build zusammen trüge das S-Release eine PWA-Änderung, die
nicht zur Runde gehört.

**Eigener `pwa-tester`-Durchlauf für P2.** Neu geprüft (§1.7) und weiter verworfen — nicht
mehr wegen der Grösse, sondern weil ein Gerätelauf den Wettlauf nur zufällig trifft und bei
Verfehlen grün meldete. Die Probe stellt ihn gezielt her.

**B40: den Scan während einer Eingabe aussetzen.** Zweiter Mechanismus neben
`prefillDeviceValue()`, und der Netzwerkstand würde während einer Eingabe nicht mehr
nachgezogen.

**B40 oder Massnahme 4 mit eigener Regel bzw. eigenem Ausgangswert.** Zweite Fassung
derselben Logik (L179, L289).

**Massnahme 4 als Umbau der ganzen Dirty-Erkennung.** Verworfen: „engere Form" heisst die
Vergleichsregel, nicht mehr.

**L303 und L306 in S.26 über einen zu langen Zeitserver nachweisen.** Verworfen — und eine
frühere Fassung dieser Spec hatte genau das: Ein zu langer Zeitserver erreicht weder
`runConfirmedButtonAction` noch das Flag-Umschalten. Der Nachweis hätte einen Weg geprüft,
den die Befunde nie nehmen — die Gattung aus L177/L185.

**L303 am Gerät mit einem echten Fehlschlag nachweisen.** Verworfen: Formatieren, STM-Reset
und Asset-Download sind keine Aktionen, die man an einer produktiven Uhr scheitern lässt.

**Die Fehlschlagprobe für P2.1e gegen 1.4.91 fahren.** Verworfen: 1.4.91 enthält die
Korrekturen schon; die Probe wäre grün und bewiese nichts. Daher ein künstlich
zurückgebauter Stand, im Scratchpad (L268).

**C38 am Gerät mit einem zu langen Schlüssel abnehmen.** Verworfen: Ein Fehler dort trennt
die Uhr vom Netz; `network_client_set` bleibt gesperrt. Quelltext und Prüfstand genügen,
weil die Prüffunktion dieselbe ist wie bei den übrigen Feldern, die am Gerät belegt sind.

**B36 mit einem erfolgreichen Schreibaufruf nachweisen.** Verworfen: Ein erfolgreicher
Aufruf ändert die Konfiguration der produktiven Uhr. Eine Abweisung ist genau der Fall,
den B36 betrifft, und Phase 9 belegt, dass nichts geändert wurde.

**R2: Kennung 6 pauschal als Erfolg; auf „file not found" vergleichen.** Das erste verschluckt
„remove failed", das zweite hängt am Diagnosetext.

**B36 über einen eigenen Anfrageblock; B34: `data-i18n` aus dem Markup nehmen.** §1.3, §1.4.

**B26 umsetzen wie gemeldet (Kennungen entfernen).** `tools/ui-mess` führt sie als
Panelnamen.

**C31: Länge statt Wert; C31 oder N1 in F; N1 mit eigenem Hotfix vor S.** Länge ist ein
Teilgeheimnis; S flasht den ESP früher; ein Hotfix kostete einen zusätzlichen Flash samt
Sitzung und Durchlauf für einen Gewinn von Tagen.

**C26-Zähler über `/api/device_ready`.** Zöge weitere `http.cpp`-Stellen in S.

**C32 in S.** §2.10.

**Flash-Gate nur als Schätzung; nur als Messung beim Release-Bau; alle K-Punkte gemeinsam
messen.** §2.7 — zu ungenau, zu spät, nicht zuordenbar.

**Flash-Gate als Entscheidung des Analysten oder des Leads.** Was entfällt, ist eine
Umfangsentscheidung des Nutzers (Ent-2).

**Runde K als eigene Runde mit eigenem Flash; alle ESP-K-Punkte mit S.** §6.1, §6.2.

**E17 mit S; F parallel zu S.** Unlesbarer Diff an der A39-Stelle; gemeinsamer Arbeitsbaum.

Aus der alten Spec weiter gültig: eigenes Top-Level-Präfix, Zuordnung an den Punkt (`.c3`),
Fähigkeitsmeldung für die Marke, Markenpflicht für alle Kommandoarten, Schweigen statt `!v`,
Folgenummer, A35 Teil 2, A39 Variante (a) (`specs/paket-2026-10-05/design.md`).

---

## Versionsfolgen

Je Einspielschritt, **nur die geänderte Komponente** (DIR-004). K-Punkte fahren unter der
Version ihres Trägers; die Testbauten heben keine Version an.

| Schritt | STM `src/main.h` | ESP `version.h` | `APP_VERSION` | `CACHE_NAME` |
|---|---|---|---|---|
| P — **ausgeliefert** (1.4.91) | — | — | ☑ | ☑ |
| P2 | — | — | ☐ | ☐ |
| S, ESP-Teil (mit N1) | — | ☐ | — | — |
| S, STM-Teil (mit gemessenen K-Punkten) | ☐ | — | — | — |
| F (mit C38 und K-Punkten; ggf. Ent-4/Ent-5) | — | ☐ | — | — |

`APP_VERSION` und `CACHE_NAME` **immer zusammen**. Nur der `release-engineer` (R4). Der Tag
`release/<stm>-<esp>-<app>` wird **je Einspielschritt** gesetzt und gepusht (DIR-011).
**Braucht die PWA nach F eine Anpassung** (C27, C38, Ent-5), ist das ein eigener PWA-Schritt
**nach** dem F-OTA (L241: Firmware zuerst).
