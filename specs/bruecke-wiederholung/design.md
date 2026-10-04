# Design — Wiederholung auf der Kommandobrücke (A32 / L230)

**Momentaufnahme vom 2026-10-04** (DIR-006). Gehört zu
`specs/bruecke-wiederholung/requirements.md`.

Zeilennummern sind **Fundhilfen, keine Anker**. Sie verschieben sich durch die eigenen
Patches — in diesem Projekt belegt. Wer patcht, sucht über den Inhalt und prüft, dass
der Anker **genau einmal** vorkommt.

---

## Leitsatz

Zwei Fehler sehen gleich aus und sind es nicht:

| | Was passiert | Was der Sender sieht | Was hilft |
|---|---|---|---|
| **Verlust** | Die Zeile kommt gar nicht an | keine Quittung | **Nachsendung** |
| **Verfälschung** | Die Zeile kommt beschädigt an und sieht gültig aus | **Quittung** | **Integritätsprüfung** |

Der Auftrag heisst „Wiederholung". Die Wiederholung allein deckt die linke Spalte ab.
Die rechte Spalte ist der Fall L205, und sie ist der gefährlichere — ein fehlender
Wert fällt auf, ein falscher nicht.

---

## 1 — Der Befundzusammenhang, geprüft statt übernommen

### 1.1 L42 und L103: die These trägt, aber über eine Ecke

Der statische Variablensatz geht **nicht** verloren, weil einzelne Kommandos im
Timeout verschwinden. Er geht verloren, weil der **Empfänger seinen Speicher
verliert**: Der ESP startet neu, `vars.cpp` legt seine Felder neu an, und
`vars_init()` setzt `HARDWARE_CONFIGURATION` ausdrücklich auf `0xFFFF`
(`ESP8266/ESP-uclock/vars.cpp:1332`). **Das ist die 65535 aus L42** — kein verlorenes
Kommando, sondern ein Vorgabewert.

Die Reparatur dafür existiert bereits: Der ESP meldet nach dem Hochfahren
`IPADDRESS` (`wifi.cpp:101`, `:164`, `:186`, `:202`), und der STM antwortet mit
`var_send_all_variables()` — rund 194 Kommandos am Stück (`main.c:2925`,
`vars.c:1181`).

**Und genau dieser Stoss ist der Ort des Schadens.** 194 Kommandos ohne
Flusskontrolle in einen 256-Byte-Empfangsring, mit einem Gegenüber, das gerade
hochfährt. Was dort verlorengeht, ist für immer weg, weil es **einmalig
angekündigte** Werte sind.

**Daraus folgt die Einordnung von A32, und sie ist schmaler als die Befundzeile
nahelegt:** A32 wirkt nicht gegen den ESP-Neustart. Es wirkt gegen die Verluste in
dem Nachsendestoss, den der Neustart auslöst. Das ist wertvoll — ohne A32 ist dieser
Stoss der einzige Versuch, den das System hat.

### 1.2 L205: die These trägt **nicht** — und das ist der wichtigste Befund dieser Spec

Der ESP quittiert **unbedingt**:

```cpp
if (! strncmp (cmd_buffer, "var ", 4))
{
    parameter = cmd_buffer + 4;
    var_set_parameter (parameter);
    Serial.println (".");                                       // "silent" OK
    Serial.flush ();
}
```
`ESP8266/ESP-uclock/ESP-uclock.ino:525-534`

Der Punkt heisst **„Zeile gelesen"**, nicht „Wert übernommen" und erst recht nicht
„Wert war plausibel". `var_set_parameter()` hat keinen Rückgabewert.

**Eine Wiederholung, die am ausbleibenden Punkt hängt, wird bei Verfälschung also nie
ausgelöst.** Damit ist die These des Auftrags für L205 widerlegt.

#### Wie aus 2 eine 14 wird — aus dem Quelltext hergeleitet

Das Kommando lautet `OT0002` (`var_send_byte("OT", 0, 2)`, `vars.c:568` → `"%s%02x%02x"`).
Auf der Leitung steht `var OT0002\r\n`.

L141 und L188 nennen die Verlustform **nicht** als Einzelzeichen, sondern als
**Sprünge von rund 77 Byte**. Fällt ein solcher zusammenhängender Block aus, sieht
der ESP den Anfang der einen und das Ende einer **späteren** Zeile als **eine** Zeile:

```
gesendet:    var OT0002\r\nvar OI0003\r\nvar OD000e\r\n
Verlust:         ^^^^^^ ... bis ................^      (zusammenhängender Block)
empfangen:   var OT000e\r\n
```

Der ESP liest: `cmd_code='O'`, `cmd_code='T'`, `var_idx = htoi("00",2) = 0`,
`type = htoi("0e",2) = 14` (`vars.cpp:1180-1187`). **Das ist exakt der gemessene
Zustand aus L205** — `overlay[0].type = 14`, gültig aussehend, ausserhalb des
Bereichs 0..10, und **quittiert**.

Ob genau dieser Block fehlte, ist nachträglich nicht mehr feststellbar; der
Mitschnitt des Augenblicks liegt nicht vor. **Belegt ist die Klasse, nicht der
Einzelfall** (DIR-003).

#### Zweiter, unabhängiger Weg zur Verfälschung — `htoi()` liest über das Zeilenende hinaus

```c
for (i = 0; i < max_digits && *buf; i++)        // *buf ist buf[0] -- konstant
{
    x = buf[i];
    ... else { x = 0; }                         // Nicht-Hex wird still zu 0
    sum <<= 4; sum += x;
}
```
`ESP8266/ESP-uclock/base.cpp:36-67`

Die Abbruchbedingung prüft `buf[0]`, gelesen wird `buf[i]`. Ist das Feld **kürzer**
als `max_digits`, liest die Schleife das Abschlussbyte mit und behandelt es als
Ziffer 0. Nachgerechnet:

| empfangen | gemeint | ESP setzt |
|---|---|---|
| `OT0002` | type = 2 | 2 ✔ |
| `OT000` (ein Zeichen fehlt) | type = 2 | **0** |
| `OT002` (ein Zeichen fehlt) | type = 2 | **32** (`0x20`) |

**Ein einzelnes verlorenes Zeichen erzeugt also keinen fehlenden Wert, sondern einen
falschen** — rechtsbündig mit Nullen aufgefüllt. Und auch dieser Fall wird quittiert.

Das ist dieselbe Defektklasse wie L88 im STM (`src/base/base.c:321-352`), dort schon
beschrieben; hier ist sie auf der Gegenseite und wirkt auf **jede** Variable.

### 1.3 A6: zwei Wege, A32 deckt einen

| Weg | Was passiert | Deckt A32? |
|---|---|---|
| (a) Die `IPADDRESS`-Zeile geht im Empfangsring des STM verloren | **Kein** Nachsendestoss, der ganze Satz bleibt weg | **nein** — die Gegenrichtung ist unquittiert |
| (b) Der Stoss läuft, einzelne seiner 194 Kommandos gehen verloren | Teilsatz, einzelne Werte falsch | **ja** |
| (c) Der Stoss läuft, einzelne Kommandos kommen verfälscht an | Werte falsch, Prüfungen blind | **erst mit der Prüfsumme** |

### 1.4 Ergebnis in einem Satz

**Die These „alles dasselbe Loch" trifft für den Verlust zu und für die Verfälschung
nicht.** Deshalb hat diese Spec zwei Bausteine und nicht einen.

---

## 2 — Idempotenz, Kommandoart für Kommandoart

Die Frage des Auftrags: Ein zweimal gesetztes `set_numvar` ist harmlos, ein zweimal
ausgeführtes Display-Kommando nicht.

**Die Antwort hängt an der Richtung, und das ist der Kern:** Die beiden Richtungen
dieser Brücke tragen völlig verschiedene Dinge.

### 2.1 Richtung STM → ESP (`var …`, durch `var_send_buf()`, **hier wird wiederholt**)

Abgezählt aus `var_set_parameter()` (`ESP8266/ESP-uclock/vars.cpp:867-1327`), alle 14
Fälle:

| Code | Was | Wirkung auf dem ESP | Wiederholung unschädlich? |
|---|---|---|---|
| `N` | numerische Variable | `numvars[idx] = val` | **ja** |
| `n` | `uint8_t`-Feld (Dimmkurven) | `dimmed_*_colors[n] = val` | **ja** |
| `S` | Zeichenkette | `strncpy` in `strvars[idx].str` | **ja** |
| `T` | Zeit/Datum | Felder von `tmvars[idx]` | **ja** (siehe 2.3) |
| `DC` | Anzeigefarbe | `dspcolorvars[idx].{r,g,b,w}` | **ja** |
| `AN`/`AD`/`AE`/`AF` | Animation | `displayanimationvars[idx].*` | **ja** |
| `CN`/`CD`/`CE`/`CF` | Farbanimation | `coloranimationvars[idx].*` | **ja** |
| `MN`/`MD`/`ME`/`MF` | Ambilight-Modus | `ambilightmodevars[idx].*` | **ja** |
| `OT`/`OI`/`OD`/`OC`/`OS`/`OY`/`ON`/`OF` | Overlay | `overlays[idx].*` | **ja** |
| `t`/`a` | Nachtzeittabellen | `nighttimevars[idx].*` | **ja** |
| `l` | Alarmzeittabelle | `alarmtimevars[idx].*` | **ja** |
| `I` | IR-Code | `ir_codes[idx].*`, `mask \|= 1<<idx` | **ja** — die Maske ist ein Oder, idempotent |

Zwei Nebenwirkungen, eigens nachgesehen, weil sie nicht wie Zuweisungen aussehen:

- `max_display_animation_variables` wächst monoton (`vars.cpp:1025-1028`) — idempotent.
- `ir_codes_received_mask |= (1 << idx)` (`vars.cpp:1317`) — idempotent.

**Ergebnis: Diese Richtung ist vollständig idempotent. Es gibt in ihr kein einziges
Kommando, das etwas *tut*; alle 14 Arten *setzen* nur.** Eine Wiederholung kann
fachlich nichts anrichten.

**Deshalb braucht A32 keine Sequenznummer.** Der einzige verbleibende
Reihenfolgefehler ist der *veraltete* Wert (R-1), und der wird über die Kennung
gelöst, nicht über eine Nummer (Abschnitt 3.4).

### 2.2 Richtung ESP → STM (`CMD …`, `main.c:2758-2860`, **hier wird nicht wiederholt**)

Diese Richtung trägt neben Settern auch `R` — den entfernten Prozeduraufruf
(`main.c:1542`, Liste in `src/vars/vars.h:22-32`):

| RPC | Wiederholung unschädlich? |
|---|---|
| `LDR_MIN_VALUE`, `LDR_MAX_VALUE` | ja (speichert den aktuellen Messwert) |
| `GET_NET_TIME`, `GET_WEATHER`, `GET_WEATHER_FC` | ja, kostet aber je einen HTTP-Abruf |
| `DISPLAY_TEMPERATURE`, `DISPLAY_DATE` | **nein** — sichtbare Wirkung an der Wand, doppelt heisst doppelt |
| `TEST_DISPLAY` | **nein** — 45 s Blockade, garantierter Watchdog-Reset (`CLAUDE.md`, R5) |
| `LEARN_IR` | **nein** — unbegrenzte Blockade, überschreibt einen angelernten Code |
| `RESET_EEPROM` | **nein** — Datenverlust |
| `GET_IR_CODES` | ja (setzt nur `ir_export_idx = 0`) |
| `GTs`/`GSs` (Spiele) | **nein** |

**Vier bis fünf der elf RPC dürfen nicht wiederholt werden.** Das ist die Antwort auf
die Frage nach dem „Display-Kommando" — und sie liegt in der Richtung, in der A32
nichts ändert. **Richtig so.** Wer diese Richtung je absichern will, braucht
Sequenznummern und eine Unterscheidung „setzend / ausführend"; das ist eine andere
Spec und ein grösserer Eingriff.

### 2.3 Die einzige Ausnahme in Richtung STM → ESP, und warum sie keine ist

`T` (aktuelle Zeit) und `N` mit `UPTIME_SECONDS_LO/HI` sind idempotent, aber
**veraltend**: Eine zwei Sekunden alte Uhrzeit nachzusenden ist nicht falsch, nur
sinnlos — und sie würde die Liste belegen, die für die einmalig angekündigten Werte
gedacht ist. Dieselbe Überlegung gilt für den LDR-Rohwert und die Temperaturindizes.

**Diese Kennungen werden nicht vorgemerkt** (Abschnitt 3.3, Ausschlussliste). Sie
kommen im nächsten Zyklus ohnehin wieder.

---

## 3 — Die Nachsendeliste (Baustein 1, STM)

### 3.1 Der Grundentscheid: nicht länger warten, sondern später noch einmal senden

Der naheliegende Entwurf — in der Warteschleife einfach ein zweites und drittes Mal
senden — ist **abzulehnen**, und zwar aus dem Grund, den der Auftrag selbst nennt:
Dreimal drei Sekunden sind **neun Sekunden Hauptloop-Blockade**. A29 und A31 haben
diesen Pfad am 04.10.2026 von genau solchen Blockaden befreit (L204, L211, L226 —
1'482'480 Durchläufe je 10 s, 1,6 % Einbruch statt sieben Sekunden Stillstand). Eine
Wiederholung, die wartet, wäre ein Rückfall dahinter.

**Der Entwurf hier kostet im Sendepfad null zusätzliche Wartezeit.** Beim Timeout
wird das Kommando **vorgemerkt** und `var_send_buf()` kehrt zurück wie heute. Der
schlechteste Fall je Kommando bleibt bei `VAR_SEND_TIMEOUT_SEC` = 3 s — **unverändert**.

Gesendet wird später, **im Hauptloop, höchstens ein Versuch je Sekunde**, an
derselben Stelle und nach demselben Muster wie der IR-Abzug (`main.c:3605-3633`):

> „genau EIN Kommando je Hauptloop-Durchlauf … damit zwischen zwei Kommandos
> garantiert der `watchdog_reload()` vom Kopf dieses Loops liegt und keine neue
> Aufrufstelle nötig ist."

Das Muster ist im Baum, es ist begründet, und es ist dasselbe, das der Wetterticker
seit Langem produktiv fährt. **Keine neue Mechanik, eine zweite Anwendung einer
bestehenden.**

### 3.2 Form

```c
#define VAR_RETRY_SLOTS           4     // vorgemerkte Kommandos, gleichzeitig
#define VAR_RETRY_CMD_LEN        72     // laengstes Kommando: "S" + 2 + 63 = 66 (EEPROM_MAX_HOSTNAME_LEN)
#define VAR_RETRY_MAX_ATTEMPTS    2     // zusaetzliche Versuche je Kommando
#define VAR_RETRY_DELAY_SEC       2     // fruehestens so spaet nach dem Fehlschlag
#define VAR_RETRY_SPACING_SEC     1     // hoechstens ein Versuch je Sekunde, ueber alle Eintraege

typedef struct
{
    char            cmd[VAR_RETRY_CMD_LEN + 1];
    uint8_t         idlen;              // Laenge der Kennung: Praefix ohne Wert, siehe 3.4
    uint8_t         attempts;           // bereits unternommene Nachsendeversuche
    uint32_t        due;                // uptime, ab der gesendet werden darf
} VAR_RETRY_SLOT;
```

**Speicherbedarf:** 4 × 80 Byte ≈ **320 Byte** plus vier sättigende Zähler ≈ **330 Byte**.
AK7 misst es auf **beiden** Zielen. Reicht es auf dem F103 nicht, fallen die Plätze
auf 2 — die Konstante ist genau dafür eine Konstante.

### 3.3 Was vorgemerkt wird und was nicht

Vorgemerkt wird beim Timeout — **ausser**:

| Nicht vorgemerkt | Warum |
|---|---|
| `T` (aktuelle Zeit), `N` mit `UPTIME_SECONDS_LO/HI` | veraltend, kommt im nächsten Zyklus wieder (2.3) |
| LDR-Rohwert, RTC- und DS18xx-Temperaturindex | dito, zyklisch |
| Kommando länger als `VAR_RETRY_CMD_LEN` | passt nicht; gezählt in `var_retry_toolong_cnt`, damit es nicht still geschieht |
| Liste voll | gezählt in `var_retry_dropped_cnt`, höchstens **eine** Logzeile je 60 s |

Die Ausschlussliste steht als **eine** Funktion `var_retry_is_worth_it(buf)` mit der
Begründung im Kopfkommentar — nicht verstreut an den Aufrufstellen.

**Dass die Liste volläuft, ist kein Fehler, sondern eine Aussage:**
`var_retry_dropped_cnt > 0` heisst „die Nachsendung auf Kommandoebene ist
überfordert, der Empfängerzustand ist zweifelhaft". Das ist genau die Lage, in der
ein **Vollabgleich** das richtige Mittel ist — Abschnitt 5.

### 3.4 Die Kennung — der Teil, ohne den die Spec abzulehnen ist (R-1)

Jeder Eintrag führt `idlen`: die Zahl der führenden Zeichen, die die **Variable**
bezeichnen, ohne ihren Wert.

| Sender | Format | `idlen` |
|---|---|---|
| `var_send_byte(id,var,…)` / `var_send_short` / `var_send_string` | `<id><idx:2>` | `strlen(id) + 2` |
| `var_send_num_variable` | `N<idx:2>` | 3 |
| `var_send_num8_array` | `n<idx:2><i:2>` | 5 |
| `var_send_tm_variable` | `T<idx:2>` | 3 |
| `var_send_dsp_color_variable` | `DC<idx:2>` | 4 |
| Nacht-/Alarmtabellen | `<t\|a\|l><idx:2>` | 3 |
| IR-Code | `I<idx:2>` | 3 |

`var_send_buf()` bekommt dafür einen zweiten Parameter. Alle Aufrufstellen liegen in
**derselben Datei** (`src/vars/vars.c`), sind privat und abzählbar.

Zwei Regeln, beide zwingend:

1. **Vor jedem Senden** wird die Liste nach derselben Kennung durchsucht; ein
   gefundener Eintrag wird **entfernt**. Der neue Wert ist der jüngere.
2. **Nach jeder eingetroffenen Quittung** dasselbe. Ein Eintrag, dessen Variable
   inzwischen bestätigt gesetzt ist, hat nichts mehr nachzusenden.

Ohne diese beiden Regeln baut die Nachsendung genau den Schaden ein, gegen den sie
antritt: einen **falschen, gültig aussehenden Wert** (L205).

### 3.5 Der Ablauf im Hauptloop

Direkt nach dem IR-Abzug (`main.c:3610-3633`), vor dem LDR-Zweig:

```
wenn (esp8266.is_online)
  und (uptime - var_retry_last_attempt >= VAR_RETRY_SPACING_SEC)
  und es gibt einen Eintrag mit due <= uptime:
        aeltesten faelligen Eintrag nehmen
        attempts++
        var_retry_last_attempt = uptime
        var_send_buf (eintrag.cmd, eintrag.idlen)     // normale Quittungspruefung
        Quittung?  -> Eintrag entfernen, log "var retry: ok <kennung> nach <n> Versuchen"
        kein Ack?  -> attempts >= VAR_RETRY_MAX_ATTEMPTS
                      ? Eintrag entfernen, log "var retry: aufgegeben <kennung> nach <n>"
                      : due = uptime + VAR_RETRY_DELAY_SEC
```

**Die Wiedereintrittsfalle des IR-Abzugs gilt hier genauso** (`main.c:3618-3626`):
`var_send_buf()` ruft in seiner Warteschleife `schedule_esp8266_messages()` und
führt dabei eintreffende Kommandos aus. Eines davon kann die Liste verändern. Der
Eintrag wird deshalb **vor** dem Senden aus der Liste genommen und nur dann
zurückgelegt, wenn er danach noch gebraucht wird und kein anderer Eintrag mit
derselben Kennung dazugekommen ist.

### 3.6 Die Logzeilen — ohne Werte, und gedrosselt

```
var retry: ok OT00 nach 1 Versuchen
var retry: aufgegeben S03 nach 2 Versuchen
var retry: Liste voll, 3 verworfen
```

Rund 35 Zeichen, nur im Störfall. **Kennung statt Wert** — die Zeichenkettenvariablen
tragen Hostnamen und Zugangsdaten (A20/L115). Die bestehende Timeout-Zeile bleibt
inhaltlich wie sie ist (A20 ist nicht Teil dieser Spec); geändert wird **nur** ihr
Text, weil „weiter ohne" nach dieser Änderung eine Falschaussage wäre:

```
var_send_buf: keine Quittung nach 3s, vorgemerkt: <wie bisher>
```

**Höchstens eine `Liste voll`-Zeile je 60 s.** Grund ist L109: Jede Logzeile legt rund
60 Byte auf genau die Leitung, deren Überlastung die Quittung gekostet hat. Eine
Meldung je verworfenem Eintrag wäre Mitkopplung.

### 3.7 Keine neuen Diagnosefelder — die Zeile ist voll

Die Diagnosezeile steht bei **118 von 119** Zeichen (`main.c:3452-3464`). Es passt
kein Feld mehr hinein, und auch keine weitere Stelle in einem bestehenden. Die vier
neuen Zähler sind deshalb **ereignisgetrieben sichtbar** (3.6), nicht periodisch.
Zugriffsfunktionen in `vars.h` gibt es trotzdem — für den Tag, an dem jemand Platz
schafft.

---

## 4 — Die Obergrenze: trägt `VAR_SEND_RELOAD_BUDGET_SEC`?

**Nein. Es braucht eine zweite, und sie misst etwas anderes.**

`VAR_SEND_RELOAD_BUDGET_SEC = 30` (`vars.c:79`) begrenzt, wie lange der Watchdog
**innerhalb eines Hauptloop-Durchlaufs** auf Quittungen hin zurückgestellt wird. Sein
Nullpunkt wird am **Kopf des Hauptloops** gelöscht (`main.c:3424`).

Die Nachsendung läuft im Loopkörper, also ist **jeder Versuch ein neuer Durchlauf mit
frischem Budget**. Das Budget begrenzt die Nachsendung damit überhaupt nicht — es
würde sie, wäre sie blockierend gebaut, sogar am Leben halten.

Die Obergrenze dieser Spec hat drei Stufen, und jede begrenzt etwas anderes:

| Stufe | Konstante | Begrenzt |
|---|---|---|
| je Kommando | `VAR_RETRY_MAX_ATTEMPTS` = 2 | wie oft ein einzelner Wert nachgefragt wird |
| je System | `VAR_RETRY_SLOTS` = 4 | wie viele Kommandos **gleichzeitig** offen sein können — der eigentliche Deckel bei totem ESP |
| je Zeit | `VAR_RETRY_SPACING_SEC` = 1 | wie oft überhaupt nachgesendet wird, über alle Einträge |

**Der schlechteste Fall, ausgerechnet:** Brücke tot, Liste voll. Vier Einträge × drei
Versuche, höchstens einer je Sekunde, jeder bis zu 3 s im Timeout.

- **längster zusammenhängender Stillstand: 3 s** — unverändert gegenüber heute,
  weil nie zwei Versuche in **demselben** Durchlauf liegen (AK1 misst es)
- **zusätzliche Last: höchstens rund 16 Byte/s**, gegen eine Grundlast von 37 bis
  80 B/s (L141), und nur im Störfall
- **Deckung in der Zeit:** rund 10 s Brückenstörung je Kommando. Ein ESP-Neustart
  liegt darunter, ein OTA-Schreibvorgang darüber — dagegen hilft nur der
  Vollabgleich (Abschnitt 5)

Bleibt die Brücke tot, greift weiterhin der Watchdog: Der Reload hängt an `got_ack`,
und das bleibt beim Timeout 0. **Die Selbstheilung durch Reset wird nicht angetastet**
— sie hat am 02.10.2026 die Uhr nach sieben Sekunden zurückgeholt (L25).

---

## 5 — Verhältnis zu Runde 4a (A6, Weg B)

**Die Vermutung des Auftrags bestätigt sich nicht. 4a bleibt nötig.** Die Begründung
in drei Schritten:

1. **A32 repariert Übertragung, nicht Empfängerzustand.** Was der ESP durch seinen
   Neustart verliert, war längst erfolgreich zugestellt. Es gibt kein offenes
   Kommando, das man nachsenden könnte (1.1).
2. **Der Auslöser des Vollabgleichs ist selbst ungesichert.** Er hängt an der
   `IPADDRESS`-Zeile des ESP — in der **Gegenrichtung**, die gar keine Quittung hat.
   Geht sie verloren (bei `d=769` verworfenen Zeichen nach einem OTA, L42, keine
   Spekulation), passiert **gar nichts**, und kein Wiederholmechanismus im STM merkt
   es. Weg B setzt genau hier an: Der **Empfänger** fordert an, und er kann es
   wiederholen, bis er ein Ergebnis sieht (`HARDWARE_CONFIGURATION != 65535` ist das
   natürliche Abbruchkriterium).
3. **Umgekehrt gilt aber:** Ohne A32 schickt Weg B 194 Kommandos über denselben
   verlustbehafteten Kanal und kann denselben halb beschädigten Satz erzeugen, den er
   reparieren soll. L103 und L205 sind der Beleg, dass das real geschieht.

**Die Reihenfolge ist damit umgekehrt zur Vermutung, aber nicht die Streichung:**

> **A32 macht 4a nicht überflüssig — A32 macht 4a erst wirksam.**
> Erst A32, dann 4a. Streichen lässt sich 4a nicht.

Die Zeile in `BEFUNDE.md` zum Stand von Runde 4a („blockiert durch A32 — erst A32
bewerten") ist damit beantwortet: **bewertet, 4a bleibt, Reihenfolge bestätigt.**

Eine Ergänzung, die sich aus 3.3 anbietet und **nicht** Teil dieser Spec ist:
`var_retry_dropped_cnt > 0` wäre ein guter zusätzlicher Auslöser für den Vollabgleich
— „die Einzelreparatur ist überfordert". Gehört in die Spec von 4a, nicht hierher.

---

## 6 — Die Prüfsumme (Baustein 2, beide Seiten)

Dies ist der Teil, der L205 schliesst. Baustein 1 schliesst ihn **nicht** (1.2).

### 6.1 Form

Der STM hängt an die Nutzlast an: `*` und **vier** Hexziffern.

```
var OT0002*1f3a
```

**Warum 16 Bit und nicht 8:** +2 Byte je Kommando sind +388 Byte je Vollabgleich auf
rund 3 kB — nicht messbar. Das Restrisiko fällt von rund 0,4 % auf rund 0,0015 % je
verfälschter Zeile. Bei einem Fehlerbild, dessen Schaden ein **stiller falscher Wert**
ist, ist das der richtige Tausch.

Berechnet wird über die Nutzlast **ohne** `var ` und **ohne** die Prüfsumme selbst,
mit der Länge als Startwert — damit eine Längenänderung, also der belegte
Burst-Verlust, sich auswirkt:

```c
sum1 = (uint8_t) len;
sum2 = 0;
fuer jedes Zeichen ch:
    sum1 = (uint8_t) (sum1 + (uint8_t) ch);
    sum2 = (uint8_t) (sum2 + sum1);
crc = (uint16_t) ((sum2 << 8) | sum1);
```

Fletcher-artig, ohne Tabelle, ohne Division. Erkennt Verlust, Einfügung,
Einzelbytefehler und Vertauschungen. **Garantien gibt sie keine** — es ist eine
Prüfsumme, keine Sicherung.

**Dieselben Zeilen stehen zweimal im Baum** (STM C, ESP C++). Das ist eine zweite
Wahrheit und wird entsprechend behandelt: Die Prüfvektoren werden **einmal** erzeugt
(Lead, `tools/checks/`) und in `tasks.md` eingetragen; beide Umsetzungen werden gegen
dieselben Vektoren geprüft.

### 6.2 Auf dem ESP

In `ESP-uclock.ino`, im `var `-Zweig, **vor** `var_set_parameter()`:

- Endet die Zeile auf `*` + vier Hexziffern? **Nein** → wie heute: anwenden,
  quittieren. (Rückwärtskompatibel, Runde 2 ist dadurch inert.)
- **Ja** → Prüfsumme über den Rest rechnen.
  - stimmt → Marke abschneiden, anwenden, `.` quittieren
  - stimmt nicht → **nicht anwenden**, `!v` antworten, `var_form_error_cnt++`, eine
    Zeile `- var abgewiesen len=<n>` über `Serial.println()` **und**
    `stm32_log_append()` (das Muster aus C14 — ohne den zweiten Aufruf bleibt die
    Messung über `/api/stm32_log` unsichtbar)

`!v` ist frei: Der einzige Ein-Zeichen-Token in dieser Richtung ist heute der Punkt
(`ESP-uclock.ino:532`), geprüft über den ganzen Sketch.

### 6.3 Auf dem STM

- `esp8266_get_message()` erkennt `!v` und liefert einen eigenen Rückgabewert
  `ESP8266_NAK`. **Das kommt in Runde 1 mit und ist bis Runde 2 wirkungslos** — kein
  ESP sendet es.
- `var_send_buf()` trennt zwei Dinge, die heute dasselbe `got_ack` sind:
  - `got_answer` (Punkt **oder** `!v`) → die Brücke lebt → Watchdog-Reload im Budget
  - `applied` (nur Punkt) → Erfolg
  Bei `!v`: **nicht erneut warten**, sondern sofort vormerken mit `due = uptime`. Der
  Hauptloop sendet im nächsten Durchlauf nach, gedrosselt wie alle anderen. Der
  Sendepfad bleibt bei höchstens 3 s.
- Angehängt wird die Prüfsumme **erst in Runde 3**, und **nur**, wenn
  `esp8266.cap_var_crc` gesetzt ist.

### 6.4 Die Fähigkeitsmeldung — gegen die Rückroll-Falle (R-3)

Der ESP sendet in `setup()`, **vor** jeder WLAN-Meldung, einmal:

```
CAP var-crc
```

Der STM setzt daraufhin `esp8266.cap_var_crc = 1`. Gelöscht wird das Flag nur durch
einen STM-Neustart.

**Warum das nötig ist:** Ein ESP ohne Prüfsummenverständnis nimmt `var S03meinhost*1f3a`
entgegen und speichert den Hostnamen **mit** der Marke — ein stiller falscher Wert,
also genau der Schaden, gegen den die Prüfsumme antritt. OTA-Rückrollen ist in diesem
Projekt Routine, der Fall ist nicht theoretisch.

**Restrisiko, benannt:** Geht die `CAP`-Zeile im Empfangsring verloren, bleibt die
Prüfung bis zum nächsten ESP-Neustart aus — stillschweigend. Das ist eine bewusst
hingenommene Verschlechterung gegenüber „immer anhängen", und sie ist die sichere
Richtung: Ohne Prüfsumme verhält sich das System wie heute, mit falsch angehängter
Prüfsumme würde es Werte beschädigen.

**Zweites Netz:** AK13 lässt `smoke-device.sh` nach Zeichenkettenwerten suchen, die
auf `*<4 Hexziffern>` enden. Tritt der Fall je ein, meldet ihn die Prüfung statt ihn
zu überleben.

---

## 7 — Betroffene Module

| Datei | Änderung | Agent | Runde |
|---|---|---|---|
| `src/vars/vars.c` | Nachsendeliste, Kennung, Ausschlussliste, Zähler, `applied`/`got_answer`, später Prüfsumme anhängen | `stm-developer` | 1, 3 |
| `src/vars/vars.h` | Zugriffsfunktionen auf die vier Zähler, Prototyp des Drains | `stm-developer` | 1 |
| `src/main.c` | Drain im Hauptloop neben dem IR-Abzug; `ESP8266_NAK` im `switch` | `stm-developer` | 1 |
| `src/esp8266/esp8266.c`, `.h` | `!v` → `ESP8266_NAK`; `CAP var-crc` → `cap_var_crc` | `stm-developer` | 1 |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | Prüfsummenprüfung im `var `-Zweig, `!v`, `CAP var-crc` in `setup()`, Zähler, Logzeile | `esp-developer` | 2 |
| `tools/checks/var-crc.c` | Referenzrechnung und Prüfvektoren | Lead | 2 |
| `tools/smoke-device.sh` | Suche nach `*<4 Hex>` am Ende von Zeichenkettenwerten | Lead | 3 |
| `tools/measure-log.sh` | `var retry:`-Zeilen zählen und melden | Lead | 1 |
| `src/main.h`, `ESP8266/ESP-uclock/version.h` | Versionen (DIR-004) | `release-engineer` | 1, 2, 3 |
| `BEFUNDE.md`, `CHANGELOG.md` | Stand nachführen | `doc-writer` | Abschluss |

**Kodierung:** `src/**` und `ESP-uclock.ino` sind ASCII/ISO-8859-1 — dort gilt die
Umschrift (`Bruecke`, `waehrend`, `zurueck`). **Vor dem Patchen die Datei prüfen**:
`http.cpp` und `stm32flash.cpp` sind UTF-8, ein `latin-1`-Patcher beschädigt sie
(`CLAUDE.md`). Beide stehen nicht auf dieser Liste.

---

## 8 — Prüfung gegen die Architektur-Checkliste

Pflicht, `knowledge/architecture-checklist.md`. Beantwortet, nicht abgehakt.

### Proper architecture

> **Bleibt die PWA parallel zu Legacy?** Ja — diese Spec fasst keine Datei unter
> `data/app/**` an und ändert keinen HTTP-Endpunkt. Beide Oberflächen lesen danach
> dieselben, nur zuverlässigeren Werte.
>
> **Wetter-Endpunkte?** Unberührt. `/api/weather_get_now` und
> `/api/weather_get_forecast` kommen in keiner geänderten Datei vor; der entfernte
> Legacy-Bypass wird nicht angefasst.
>
> **Restore-Bedingung um `pending_weather_ticker_restore`?** Vollständig erhalten. Der
> Drain liegt im Hauptloop **nach** `schedule_esp8266_messages()` und **vor** dem
> LDR-Zweig; er liest die vier Teilbedingungen nicht und schreibt keine davon.
>
> **Richtige Schicht?** Ja, und das ist hier die eigentliche Frage. Der Defekt liegt
> im Protokoll zwischen STM und ESP. Behoben wird er dort — nicht in der PWA (die
> Werte „nachladen" würde), nicht im ESP allein (er kann Verlust nicht bemerken) und
> nicht im STM allein (er kann Verfälschung nicht bemerken). **Die Verfälschung kann
> nur der Empfänger sehen, der Verlust nur der Sender.** Deshalb zwei Bausteine auf
> zwei Seiten.

### Scalable systems

> **Wie viele STM-Kommandos?** Null zusätzliche im Normalbetrieb. Im Störfall
> höchstens ein nachgesendetes je Sekunde, höchstens 12 je Störung (4 Plätze × 3
> Versuche).
>
> **Byte je Minute auf der UART?** Zusätzlich höchstens rund **960 Byte/min** im
> Störfall (16 B/s), null in Ruhe. Grundlast 37–80 B/s (L141). Nach Runde 3 dauerhaft
> **+5 Byte je `var`-Kommando** (`*` + 4 Hexziffern); bei rund 3 Kommandos je Minute
> in Ruhe sind das 15 Byte/min, beim Vollabgleich einmalig +970 Byte.
>
> **Unter 20 s Watchdog?** Ja. Der längste neue zusammenhängende Pfad ist **ein**
> Nachsendeversuch = `VAR_SEND_TIMEOUT_SEC` = 3 s, begrenzt durch
> `VAR_RETRY_SPACING_SEC`. Keine neue `watchdog_reload()`-Aufrufstelle, `WD_EXPECTED`
> bleibt (Guardrail S7).
>
> **Wie lange blockiert der Hauptloop?** Höchstens 3 s, wie heute. AK1 misst es gegen
> die 148'000 Durchläufe je Sekunde aus L226 — die Kennzahl existiert, seit A29/A31
> abgenommen wurden, und genau dafür ist sie da.
>
> **Hartkodierte Grenzen?** `VAR_RETRY_CMD_LEN = 72` hängt an
> `EEPROM_MAX_HOSTNAME_LEN = 64`. Wächst der, ist 72 zu klein — deshalb zählt
> `var_retry_toolong_cnt` statt still zu verwerfen, und der Zusammenhang steht als
> Kommentar an der Konstante.

### Secure by design

> **`innerHTML`?** Keine PWA-Änderung.
>
> **Fremddaten?** Der ESP bleibt für den STM eine nicht vertrauenswürdige Quelle. Die
> neue `CAP`-Zeile wird exakt verglichen und setzt nur ein Flag; `!v` wird exakt
> verglichen. Beides ohne Parameter, also ohne Angriffsfläche.
>
> **Credentials in Logausgaben?** Die neuen Zeilen tragen **Kennung und Zahl, nie
> einen Wert** — bewusst, weil Zeichenkettenvariablen Hostnamen und Zugangsdaten
> führen (A20/L115). Die bestehende Timeout-Zeile bleibt wie sie ist; A20 ist
> ausdrücklich nicht Teil dieser Spec und wird dadurch weder besser noch schlechter.
>
> **Neue schreibende Endpunkte?** Keine.

### Stable & reliable

> **Wird jeder Fehler ausgewertet?** Das ist der Kern dieser Spec. Heute meldet der
> ESP `.` für „Zeile gelesen" und der STM liest es als „Wert gesetzt". Nach Runde 3
> heisst `.` „Prüfsumme gestimmt, Wert gesetzt" und `!v` „abgelehnt". **Die
> Erfolgsmeldung hängt dann am Ergebnis**, nicht am Ablauf.
>
> **Leere `catch`?** Nicht anwendbar (C).
>
> **Stille Verwerfungen ohne Zähler?** Jede neue Verwerfung hat einen Zähler:
> `var_retry_dropped_cnt` (Liste voll), `var_retry_toolong_cnt` (passt nicht),
> `var_retry_gaveup_cnt` (aufgegeben), `var_form_error_cnt` (ESP, abgewiesen). Die
> Spec entfernt zusätzlich eine bestehende stille Verwerfung — das `break` aus L230.
>
> **Werte still zurechtgebogen und als gespeichert gemeldet?** Genau das passiert
> heute (`htoi` füllt mit Nullen auf, 1.2) und hört mit Runde 3 auf.
>
> **Zustand nach Abbruch mitten in einer Folge definiert?** Ja. Jedes Kommando ist
> ein unabhängiger Setzer (Abschnitt 2.1); ein Abbruch hinterlässt einen Teilsatz,
> keinen halben Datensatz. Das unterscheidet diesen Fall vom Overlay-Import, wo ein
> Fehlschlag alle folgenden Indizes verschiebt.
>
> **Flag auf jedem Pfad wieder aufgelöst?** Drei Flags sind zu prüfen:
> `var_send_busy` und `var_send_nested` werden wie heute auf **beiden** Endpfaden
> zurückgesetzt (`vars.c:216-217`) — der neue `!v`-Pfad verlässt die Schleife über
> dieselbe Stelle. Ein Listenplatz wird **vor** dem Senden belegt und auf jedem der
> drei Ausgänge (Erfolg, erneut fällig, aufgegeben) aufgelöst; der
> Wiedereintrittsfall aus 3.5 ist der, an dem das schiefgehen kann, und er ist
> ausdrücklich beschrieben.

---

## 9 — Verworfene Alternativen

| Verworfen | Warum |
|---|---|
| **Blockierend wiederholen** (3× senden in der Warteschleife) | 9 s Hauptloop-Blockade. Rückfall hinter A29/A31 (L204/L211/L226). Ausdrücklich vom Auftrag ausgeschlossen, und zu Recht |
| **Timeout auf 1 s senken und 3× versuchen** (gleiche Gesamtzeit) | Sieht elegant aus und verfehlt den Fall: Die belegte Störung ist ein **ESP-Neustart**, der Sekunden bis Minuten dauert. Drei Versuche in 3 s fallen alle in dasselbe tote Fenster. Dazu das Risiko, dass 1 s für eine normale Quittung bei beschäftigtem ESP nicht reicht — dann steigt `v=` im Ruhebetrieb und die Brücke bekommt Last, wo heute keine ist |
| **Sequenznummer je Kommando** | Löst ein Problem, das diese Richtung nicht hat: Alle 14 Kommandoarten sind reine Zuweisungen (2.1). Kostet Bandbreite, Protokollbruch und beidseitigen Zustand. Für die **Gegenrichtung** wäre sie nötig — die ist nicht Teil dieser Spec |
| **Längentabelle je Kommandoart auf dem ESP** statt Prüfsumme | Fängt kurze und verschmolzene Zeilen, aber **nicht** den belegten Fall: `var OT000e` hat die korrekte Länge (1.2). Dazu 14 Einträge, die bei jeder Protokolländerung mitgepflegt werden müssen — eine zweite Wahrheit neben `vars.c`. Die Prüfsumme ist **eine** Stelle je Seite und fängt mehr |
| **Vollabgleich bei jedem Timeout** | 194 Kommandos, weil eines fehlte. Genau das Pflaster, das L230 kritisiert. Als **Ergänzung** bei überlaufender Liste sinnvoll — gehört in 4a |
| **Grössere Liste (16 oder 32 Plätze)** | Kauft Deckung für einen Fall, in dem die Einzelreparatur ohnehin das falsche Mittel ist, und kostet RAM auf dem F103. Vier Plätze decken den realistischen Fall (einzelne Verluste im Nachsendestoss); darüber hinaus ist der Vollabgleich richtig |
| **Prüfsumme immer anhängen, ohne `CAP`** | Beschädigt Zeichenkettenwerte auf einem zurückgerollten ESP (R-3). OTA-Rückrollen ist hier Routine |
| **Eigenes Verb `varc …` statt Marke am Zeilenende** | Ein alter ESP ignoriert die Zeile vollständig — kein Schaden, aber **alle** Variablen fallen aus. Lauter Ausfall statt leiser Beschädigung ist besser, aber `CAP` ist beides nicht |

---

## 10 — Versionsfolgen

- [x] STM `src/main.h` anheben — **zweimal** (Runde 1 und Runde 3)
- [x] ESP `ESP8266/ESP-uclock/version.h` anheben — einmal (Runde 2)
- [ ] App `APP_VERSION` — **nicht**, die PWA ändert sich nicht
- [ ] `CACHE_NAME` in `sw.js` — **nicht** (gehört immer zu `APP_VERSION`)

Ausgeführt ausschliesslich vom `release-engineer` (R4). Jedes der drei Fabrikate wird
einzeln ausgerollt, sofort committet und getaggt (DIR-011, DIR-005).
