# Design — F1, IR-Codes ins Backup

## Belegte Faktenlage

Die Analyse aus Befund **L84** ist die Grundlage dieser Spec. Alle dort genannten
Fundstellen sind beim Schreiben nachgelesen worden; das Ergebnis steht hier, damit der
umsetzende Agent nicht erneut suchen muss.

| Aussage | Fundstelle | Stand |
|---|---|---|
| 20 Tasten, nicht 32 | `src/remote-ir/remote-ir.h:69` (`N_REMOTE_IR_CMDS 20`), Schleifen `remote-ir.c:196`, `:226` | ✔ bestätigt |
| 160 Byte reserviert, 100 belegt | `eeprom-data.h:107` (`EEPROM_MAX_IR_CODES 32`), `:45` (`PACKED_IRMP_DATA_SIZE 5`), `:124` | ✔ bestätigt |
| Packung little endian, 5 Byte | `remote-ir.c:228-232` lesend `:200-202` | ✔ bestätigt |
| `flags` wird nicht gespeichert | `remote-ir.c:228-232` — nur protocol, address, command | ✔ bestätigt |
| ESP→STM schneidet nach 127 Zeichen still ab | `esp8266.h:68`, `esp8266.c:386` | ✔ bestätigt (L84) |
| STM→ESP nimmt 123 Zeichen Nutzlast | `ESP-uclock.ino:71` (`CMD_BUFFER_SIZE 128`), `:1148` — verwirft ohne `else` | ✔ bestätigt |
| `sprintf` in 160-Byte-Stackpuffer ohne Längenprüfung | `src/vars/vars.c:119-126`, `:190-200` | ✔ bestätigt (= L86) |
| RX-Ring 256 Byte, verwirft still | `esp8266-uart.c:69`, `uart-driver.h:698` | ✔ aus L84 übernommen |
| `var_send_all_variables()` ohne `watchdog_reload()` | `src/vars/vars.c:951-1019`, aufgerufen `main.c:2744` | ✔ bestätigt (= L85) |
| `irmp_data_array` ist `static` | `remote-ir.c:22` | ✔ bestätigt |
| EEPROM-Reset nullt nur die Version | `main.c:1590-1596`; Verlust erst beim Neustart über `main.c:1245` | ✔ bestätigt |
| Backup nimmt ältere Versionen an | `app.js:4249-4278` (`readBackupFileVersion`, `parseSettingsBackupFile`) | ✔ bestätigt (= L82) |
| Fehlerkennungen 1..5 | `http.cpp:161-170` | ✔ bestätigt |

**Ein Punkt ist neu und ändert den empfohlenen Weg.** Siehe „Abweichung von der
Analyse" weiter unten.

---

## Lösungsweg

Drei Schichten, jede mit genau einem Besitzer.

### 1. Protokoll

Ein neues Kommando in **beiden** Richtungen, fest 13 Zeichen:

```
I <idx:2> <protocol:2> <address:4> <command:4>
```

Beispiel: `I000212340001` — Taste 0, Protokoll `0x02`, Adresse `0x1234`,
Kommando `0x0001`. Alles hexadezimal, Grossbuchstabe `I` als Kommandocode.

Längenrechnung, gegen beide Puffergrenzen:

| Richtung | Leitungsform | Länge | Grenze | Reserve |
|---|---|---|---|---|
| STM→ESP | `var I000212340001\r\n` | 17 | 127 (`CMD_BUFFER_SIZE`) | 110 |
| ESP→STM | `CMD I000212340001\r\n` | 17 | 127 (`ESP8266_MAX_CMD_LEN`) | 110 |

Der Code `I` ist in beiden Dispatchern frei: STM belegt `R N n S T D A C M O t a l G`
(`main.c:2592-2677`), ESP belegt `N n S T D A C M O t a l` (`vars.cpp:764-...`).

**Konvention „nie angelernt":** `protocol == 0x00` oder `protocol == 0xFF`. Beides
sind keine gültigen IRMP-Protokolle. Ein echtes Gültigkeitsflag würde ein zusätzliches
Byte je Taste, einen `EEPROM_VERSION`-Bump und einen Migrationslauf kosten — für den
Gewinn ist das zu teuer.

### 2. STM (`src/**`, `stm-developer`)

**a) Zugriff auf `irmp_data_array`.** Das Array ist `static` (`remote-ir.c:22`).
Statt es zu entstatisieren, bekommt `remote-ir.c` zwei Funktionen, deklariert in
`remote-ir.h`:

```c
uint_fast8_t remote_ir_get_code (uint_fast8_t idx, uint_fast8_t * protocol,
                                 uint_fast16_t * address, uint_fast16_t * command);
uint_fast8_t remote_ir_set_code (uint_fast8_t idx, uint_fast8_t protocol,
                                 uint_fast16_t address, uint_fast16_t command);
```

Beide liefern 0 bei `idx >= N_REMOTE_IR_CMDS` und rühren dann nichts an.

`remote_ir_set_code()` tut **zwei** Dinge und zwar in dieser Reihenfolge:
1. `irmp_data_array[idx]` setzen (protocol, address, command; `flags` auf 0),
2. genau die 5 Byte an `EEPROM_DATA_OFFSET_IRMP_DATA + idx * PACKED_IRMP_DATA_SIZE`
   schreiben, mit derselben Packung wie `remote_ir_write_codes_to_eep()`
   (`remote-ir.c:228-232`).

Damit sind RAM und EEPROM nach **jedem** einzelnen Aufruf in Deckung. Das ist dieselbe
Sorgfalt, die `remote-ir.c:168-172` für den abgebrochenen Lernvorgang bereits aufbringt.

**b) Neue RPC-Nummer.** `GET_IR_CODES_RPC_VAR` wird in `src/vars/vars.h`
**unmittelbar vor** `MAX_RPC_VARIABLES` angehängt — nie dazwischen. Bestehende
Nummern dürfen sich nicht verschieben, sonst muss man STM und ESP im exakten
Gleichschritt flashen, und ein Fehlgriff löste still den falschen RPC aus.

**c) Export als Zustand im Hauptloop, nicht als Schleife.** Der RPC-Zweig in
`schedule_esp8266_remote_procedure()` (`main.c:1525`) setzt nur:

```c
case GET_IR_CODES_RPC_VAR:
    debug_log_message ("rpc: send IR codes");
    ir_export_idx = 0;                      // 0..N_REMOTE_IR_CMDS, N == fertig
    break;
```

Im Hauptloop (`main.c:3212` ff., nach `watchdog_reload()`), im Zweig für
`esp8266.is_online`:

```c
if (ir_export_idx < N_REMOTE_IR_CMDS)
{
    var_send_ir_code (ir_export_idx);       // ein Kommando pro Durchlauf
    ir_export_idx++;
}
```

Initialwert von `ir_export_idx` ist `N_REMOTE_IR_CMDS`, also „nichts zu tun".

**d) `var_send_ir_code()` in `src/vars/vars.c`**, Muster von `var_send_num_variable()`
(`vars.c:132-142`), fester Puffer `char buf[32]`, `sprintf` mit festem Format —
13 Zeichen plus Nullbyte, die Grösse ist statisch bekannt und der Befund **L86** damit
nicht berührt. Liest über `remote_ir_get_code()`.

**e) Empfang `case 'I'`** in `schedule_esp8266_cmd()` (`main.c:2592`), eigene Funktion
`schedule_esp8266_ir_code()` im Stil von `schedule_esp8266_numeric_array()`:
`htoi` für idx (2), protocol (2), address (4), command (4), dann
`remote_ir_set_code()`. Der Rückgabewert wird ausgewertet und bei 0 in den Debuglog
geschrieben — ein verworfener Index darf nicht stumm bleiben.

### 3. ESP (`ESP8266/ESP-uclock/*.cpp`, `*.h`, `esp-developer`)

**a) Puffer in `vars.cpp`**, neben den übrigen STM-Spiegelvariablen:

```c
#define MAX_IR_CODES 20        // spiegelt N_REMOTE_IR_CMDS, src/remote-ir/remote-ir.h:69

typedef struct { uint8_t protocol; uint16_t address; uint16_t command; } IR_CODE;

static IR_CODE   ir_codes[MAX_IR_CODES];
static uint32_t  ir_codes_received_mask;    // Bit i == Taste i eingetroffen
static uint8_t   ir_codes_requested;        // 1 == seit dem letzten Anstoss gueltig
```

104 Byte RAM. **Nicht** ins ESP-EEPROM — der Puffer ist bewusst flüchtig, die Quelle
der Wahrheit ist der STM.

Stimmen `MAX_IR_CODES` und `N_REMOTE_IR_CMDS` einmal nicht überein, entsteht kein
stiller Schaden: Der STM sendet den Index mit, der ESP prüft ihn gegen seine eigene
Grenze und verwirft darüber hinausgehende. Sie fehlen dann in der Maske, `complete`
bleibt `false`, und der Abzug scheitert **sichtbar**.

**b) `case 'I'` in `var_set_parameter()`** (`vars.cpp:764`): `htoi` wie oben,
`idx < MAX_IR_CODES` prüfen, Wert ablegen, `ir_codes_received_mask |= (1UL << idx)`.

**c) Drei Endpunkte** in `http.cpp`, eingehängt im Dispatcher bei den übrigen
`/api/*` (`http.cpp:11740` ff.).

### 4. PWA (`data/app/app.js`, `pwa-developer`)

Export, Import, Bestätigung, Gegenprobe — Einzelheiten unter „JSON-Format" und
„Ablauf in der PWA".

---

## Abweichung von der Analyse

Die Analyse in L84 empfiehlt den Export über einen eigenen RPC, der die 20 Kommandos
sendet. **Sie sagt nicht, wie.** Der naheliegende Weg — eine `for`-Schleife im
RPC-Zweig — ist gefährlich, und zwar aus einem Grund, der in L84 nicht steht:

`var_send_buf()` (`vars.c:51-99`) wartet nach **jedem** Kommando auf die Quittung des
ESP, mit `VAR_SEND_TIMEOUT_SEC` = **3 s** (`vars.c:47`). Bleiben die Quittungen aus —
genau der Fall, für den der Timeout überhaupt eingebaut wurde (`vars.c:82-85`) —,
kostet eine Schleife über 20 Kommandos **bis zu 60 s**. Der Watchdog steht bei 20 s,
und in diesem Pfad gäbe es keinen `watchdog_reload()`. Das ist derselbe Mechanismus,
der `var_send_all_variables()` zum Befund **L85** macht; eine Schleife im RPC-Zweig
würde ihn in kleinerem Massstab reproduzieren.

**Deshalb: ein Kommando pro Hauptloop-Durchlauf.** `watchdog_reload()` steht an
`main.c:3214` ganz oben im Loop. Zwischen zwei `I`-Kommandos liegt damit garantiert
ein Reload — ohne eine einzige neue Aufrufstelle. Das Watchdog-Problem verschwindet
nicht durch Sorgfalt, sondern durch die Struktur.

Nebenwirkung, ebenfalls erwünscht: Zwischen den Kommandos läuft der reguläre Loop
weiter (Display-Refresh, RTC, Ticker). Ein Abzug friert die Uhr nicht ein.

Zweite, kleinere Abweichung: Die Analyse lässt offen, ob `/api/ir_code_set` den
ESP-Puffer mitführen soll. **Diese Spec legt das Gegenteil fest** — der Schreibaufruf
*invalidiert* den Puffer. Begründung unter AK9 und unter „Prüfbarkeit ohne
Fernbedienung": Ein Puffer, der die eigene Eingabe zurückspiegelt, macht die einzige
verfügbare Gegenprobe wertlos.

---

## JSON-Format

Neuer Abschnitt `settings.ir`, angefügt in `buildSettingsBackupSections()`
(`app.js:4114-4129`). Kein bestehendes Feld wird berührt.

```json
"ir": {
  "keys": [
    { "name": "power",                      "index": 0,  "protocol": 2,    "address": 4660, "command": 1 },
    { "name": "ok",                         "index": 1,  "protocol": 2,    "address": 4660, "command": 2 },
    { "name": "decrement_display_mode",     "index": 2,  "protocol": null, "address": null, "command": null }
  ]
}
```

**20 Einträge, immer.** Die Zahlen sind dezimal, wie überall sonst im Backup.

**Der Name ist der Schlüssel beim Import, der Index ist informativ.**
`src/remote-ir/remote-ir.h:19-34` enthält einen auskommentierten Block „New Modes
(future use)", der die Nummerierung vollständig umstellen würde. Tritt das je ein,
bleibt eine namensbasierte Zuordnung richtig und eine indexbasierte wird still falsch.
Der Index steht trotzdem in der Datei — er macht sie lesbar und erlaubt die Gegenprobe
beim Teil-Restore.

Namensliste, 1:1 die Konstanten aus `remote-ir.h:38-67` in Kleinschrift ohne Präfix:

| Idx | `name` | | Idx | `name` |
|---|---|---|---|---|
| 0 | `power` | | 10 | `decrement_brightness_red` |
| 1 | `ok` | | 11 | `increment_brightness_red` |
| 2 | `decrement_display_mode` | | 12 | `decrement_brightness_green` |
| 3 | `increment_display_mode` | | 13 | `increment_brightness_green` |
| 4 | `decrement_animation_mode` | | 14 | `decrement_brightness_blue` |
| 5 | `increment_animation_mode` | | 15 | `increment_brightness_blue` |
| 6 | `decrement_hour` | | 16 | `decrement_brightness` |
| 7 | `increment_hour` | | 17 | `increment_brightness` |
| 8 | `decrement_minute` | | 18 | `auto_brightness_control` |
| 9 | `increment_minute` | | 19 | `get_temperature` |

Die Liste steht in `app.js` mit einem Kommentar, der `src/remote-ir/remote-ir.h:38-67`
als Quelle nennt. *Empfehlung an den Lead:* eine Prüfung in `tools/checks/`, die beide
Listen gegeneinander hält. `tools/**` gehört dem Lead, deshalb hier nur als Hinweis.

**`null` bedeutet „nie angelernt".** Export: Liefert der Abzug `protocol` 0 oder 255,
schreibt die PWA alle drei Felder als `null` — nicht nur `protocol`, damit die Zeile
nicht halb gefüllt aussieht. Import: Ein Eintrag mit `protocol === null` wird
**übersprungen**, es geht kein `ir_code_set` raus. Er löscht also keinen vorhandenen
Code.

Das ist eine bewusste Festlegung: Die Datei kann „absichtlich leer" nicht von „nie
angelernt" unterscheiden, beides ist dieselbe Konvention. Ein Restore, der löschte,
würde im Zweifel Arbeit vernichten. Die Zahl der übersprungenen Tasten geht in den
vorhandenen Übersprungen-Hinweis (`getSkippedImportFieldsSummary`, `app.js:4892`).

---

## API-Endpunkte

Alle drei liegen im bestehenden Dispatcher und folgen `<bereich>_<verb>` wie
`timer_set`, `ldr_min_set`, `weather_get_now`.

### `GET /api/ir_codes_request`

Verwirft den Puffer, setzt `ir_codes_received_mask = 0`, `ir_codes_requested = 1`,
ruft `rpc (GET_IR_CODES_RPC_VAR)`.

```json
{"ok":true,"expected":20}
```

Kein Parameter, also keine Fehlerkennung. Der Endpunkt ist **schreibend im Sinne von
DIR-008** — er löst ein STM-Kommando aus — aber folgenlos: Er ändert keinen
persistenten Zustand, weder auf dem ESP noch auf dem STM.

### `GET /api/ir_codes_get`

Rein lesend, kein STM-Kommando.

```json
{"ok":true,"requested":true,"expected":20,"received":18,"complete":false,
 "missing":[7,13],
 "codes":[{"idx":0,"protocol":2,"address":4660,"command":1},
          {"idx":1,"protocol":2,"address":4660,"command":2}]}
```

- `codes[]` enthält **nur** eingetroffene Indizes, jeder mit seinem `idx`. Fehlende
  stehen in `missing[]`. Das ist eindeutig und spart Bytes gegenüber 20 Einträgen mit
  Füllwerten.
- `complete` ist `ir_codes_received_mask == ((1UL << MAX_IR_CODES) - 1)`. Keine
  Heuristik, keine Plausibilität.
- `requested` ist `false`, solange seit dem ESP-Start kein `ir_codes_request` kam oder
  seit dem letzten `ir_code_set` der Puffer invalidiert ist.

Antwortgrösse im Vollausbau rund 1,1 kB — ein Bruchteil von `/api/stm32_log`
(`http.cpp:11485`, 64 Zeilen à 120 Zeichen).

### `GET /api/ir_code_set?idx=&protocol=&address=&command=`

**Alle vier Parameter sind Pflicht.** Gelesen über `http_get_int_param()`
(`http.cpp:174`), nicht über blankes `atoi` — das ist genau die Lektion aus **L70**
und **L71**.

| Prüfung | Kennung | `detail` |
|---|---|---|
| ein Parameter fehlt oder ist nicht numerisch | 1 | `idx missing or not numeric` usw. |
| `idx` ausserhalb `0..19` | 2 | `idx out of range (0..19)` |
| `protocol` ausserhalb `1..254` | 2 | `protocol out of range (1..254)` |
| `address` ausserhalb `0..65535` | 2 | `address out of range (0..65535)` |
| `command` ausserhalb `0..65535` | 2 | `command out of range (0..65535)` |

`protocol` 0 und 255 werden **abgewiesen**, nicht zurechtgebogen: Sie sind die
Leer-Konvention, und ein Schreibaufruf, der sie annähme, löschte eine Taste — etwas,
das diese Spec ausdrücklich nicht anbietet.

Erfolg: `{"ok":true}` nach `CMD I...` an den STM **und** Invalidierung des Puffers.

Dass dieses `ok:true` nur „abgeschickt" heisst und nicht „angekommen", ist Eigenschaft
**jedes** Setters dieses Systems. Die Gegenprobe ist der erneute Abzug (AK16) — und
weil der Puffer invalidiert wurde, ist dieser Abzug zwangsläufig ein frischer Wert vom
STM und nicht das Echo der eigenen Eingabe.

**Der Endpunkt ist gefährlich** und gehört in die Aufzählung in `CLAUDE.md`, R5, sowie
in den Abweisungsfilter von `tools/hooks/no-danger.py`. Beide Dateien gehören dem Lead
bzw. dem `doc-writer`; die Task-Liste führt das.

---

## Ablauf in der PWA

### Export

1. `POST`-los, aber schreibend gedacht: `apiFetch("/api/ir_codes_request")`.
2. Polling `apiFetch("/api/ir_codes_get")` alle **400 ms**, höchstens **15 Versuche**
   (6 s). Nicht kürzer: Jeder HTTP-Request erzeugt ESP-Debugtext auf der UART zum STM.
3. `complete:true` → Abschnitt bauen.
4. `requested:false` → der ESP ist zwischendurch neu gestartet. Abbruch mit eigener
   Meldung, kein stiller Rückfall auf einen leeren Satz.
5. Timeout → **ein** Wiederholungsversuch ab Schritt 1.
6. Auch der scheitert → Abschnitt `settings.ir` **weglassen**, Nutzer über den
   vorhandenen Hinweismechanismus informieren. Die Sicherung entsteht trotzdem; sie
   ist dann nur ohne IR-Codes, und das sieht man ihr an.

20 Kommandos auf der UART, einmalig, auf Anforderung. Keine Dauerlast.

### Import

Neue Stufe in `buildPrimaryImportStages()` (`app.js:4931`), **hinter** den Timern und
**vor** dem abschliessenden `maintenance_reset_stm32` (`app.js:4907`).

Ablauf der Stufe:

1. Abschnitt fehlt, ist leer oder hat nicht genau 20 Einträge → Stufe entfällt
   vollständig, mit Hinweis. Das ist die Verteidigung gegen eine von Hand bearbeitete
   Datei.
2. Zahl der zu schreibenden Tasten ermitteln (`protocol !== null`). Ist sie 0 → Stufe
   entfällt stumm.
3. **Vorab-Abzug** des aktuellen Stands (derselbe Ablauf wie beim Export) und Download
   als `wordclock-ir-vorher-<zeitstempel>.json`. Das ist der Rückweg.
4. `window.confirm` Nr. 1 — nennt die Zahl der zu überschreibenden Tasten und sagt,
   dass der einzige andere Rückweg erneutes Anlernen über `/api/learn_ir` ist.
   `window.confirm` ist das etablierte Muster dieser App (`app.js:3488`, `:4608`,
   `:10074`); es braucht kein neues Markup und keine neue Stilregel.
5. Ist der Vorab-Abzug **nicht** vollständig gelungen: `window.confirm` Nr. 2 sagt das
   ausdrücklich. Vorbelegung ist Abbruch — der Nutzer muss aktiv bestätigen. Dasselbe
   zweistufige Muster wie `maintenance.reset_eeprom_confirm_1/_2` (`app.js:10074-10078`).
6. Bis zu 20 Aufrufe `/api/ir_code_set`, **sequenziell**, einer je Taste. Ein Request =
   ein STM-Kommando ist das faktische Flusskontrollmuster dieses Systems.
7. **Gegenprobe:** erneuter vollständiger Abzug, feldweiser Vergleich gegen das, was
   geschrieben werden sollte. Jede Abweichung geht in den Hinweis. Die Erfolgsmeldung
   hängt an diesem Vergleich.

Dass danach `maintenance_reset_stm32` läuft, ist unschädlich und sogar nützlich: Die
Codes stehen im EEPROM, `read_configuration_from_eep()` liest sie beim Start
(`main.c:1165-1166`) — und ein Abzug nach dem Neustart beweist genau das.

### Versionierung

`BACKUP_VERSION` von 2 auf 3 (`app.js:1932`). Das ist zulässig, **weil F1 nur anfügt**:
`readBackupFileVersion()` und `parseSettingsBackupFile()` (`app.js:4249-4278`) nehmen
seit **L82** jede Datei mit `version <= BACKUP_VERSION` an. Bestehende Sicherungen
bleiben gültig, ihr fehlender `ir`-Abschnitt lässt die Stufe schlicht entfallen. Eine
Migration ist nicht nötig — sie wäre es nur, wenn ein bestehendes Feld umgedeutet
würde, und das tut F1 nicht (AK18).

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `src/remote-ir/remote-ir.h` | zwei Deklarationen | `stm-developer` |
| `src/remote-ir/remote-ir.c` | `remote_ir_get_code()`, `remote_ir_set_code()` | `stm-developer` |
| `src/vars/vars.h` | `GET_IR_CODES_RPC_VAR` vor `MAX_RPC_VARIABLES`; `var_send_ir_code()` | `stm-developer` |
| `src/vars/vars.c` | `var_send_ir_code()` | `stm-developer` |
| `src/main.c` | RPC-Zweig, `ir_export_idx` im Hauptloop, `case 'I'` + `schedule_esp8266_ir_code()` | `stm-developer` |
| `ESP8266/ESP-uclock/vars.h` | `GET_IR_CODES_RPC_VAR`, `MAX_IR_CODES`, Zugriffsdeklarationen | `esp-developer` |
| `ESP8266/ESP-uclock/vars.cpp` | Puffer, Maske, `case 'I'` in `var_set_parameter()` | `esp-developer` |
| `ESP8266/ESP-uclock/http.cpp` | drei Endpunkte, drei Dispatcher-Zweige | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | Export, Import, Bestätigung, Gegenprobe, Namensliste, `BACKUP_VERSION`, i18n-Schlüssel | `pwa-developer` |
| `BEFUNDE.md`, `TESTPLAN-PWA.md`, `CLAUDE.md` | Stand, Testphase, Gefahrenliste | `doc-writer` bzw. Lead |
| `src/main.h`, `version.h`, `APP_VERSION`, `CACHE_NAME` | Versionen | `release-engineer` (R4) |

Die i18n-Schlüssel liegen in den Sprachtabellen innerhalb von `app.js` — also derselbe
Besitzer, kein Task über zwei Besitzer hinweg. Braucht der Bestätigungsdialog wider
Erwarten neues Markup oder eine neue Stilregel, ist das ein **eigener** Task für
`ui-developer`; der Entwurf oben vermeidet das bewusst durch `window.confirm`.

---

## Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Die PWA bleibt parallel zur Legacy-Oberfläche; Legacy bekommt
keine IR-Seite und bleibt unverändert Stabilitäts-Referenz. Die Wetter-Endpunkte sind
nicht berührt, ebensowenig die vierteilige Restore-Bedingung um
`pending_weather_ticker_restore`. App-Assets bleiben `.gz`-only, es kommt kein Asset
dazu — die Weissliste `APP_INSTALL_ASSETS` (`http.cpp:215-229`) bleibt unverändert,
also auch kein neuer Fall der Falle vom 29.04.2026.

Zur Schicht: Die IR-Codes liegen im STM-EEPROM, und genau dort wird gelesen und
geschrieben. Der ESP ist Durchreiche und flüchtiger Puffer, die PWA ist
Darstellung und Ablaufsteuerung. Nichts wird in `app.js` umgangen, was weiter unten
liegt. Ausdrücklich **nicht** gewählt: die IR-Codes in `var_send_all_variables()`
aufzunehmen. Das wäre die bequemere Stelle gewesen und hätte ausgerechnet den
Startpfad verlängert, bei dem der F411 „teils exakt bei Anzeige von IP" hängt
(`CLAUDE.md`, offenes Thema 2; `main.c:2744`, Befund **L85**).

**Scalable systems** —

*Export:* 1 Kommando ESP→STM (der RPC), 20 Kommandos STM→ESP. 20 × 17 Byte = 340 Byte,
einmalig und nur auf ausdrückliche Anforderung. Je Kommando ein Hauptloop-Durchlauf,
dazwischen der reguläre `watchdog_reload()` aus `main.c:3214`. Der Hauptloop blockiert
pro Kommando so lange wie jedes andere `var_send_buf()` auch — im Normalfall
Millisekunden, im Quittungsausfall höchstens 3 s für **ein** Kommando, nie 60 s für
zwanzig. Kein EEPROM-Zugriff beim Export, er liest den RAM-Spiegel.

*Restore:* 20 HTTP-Requests, je 1 STM-Kommando, je 5 Byte EEPROM ≈ **80 ms**
Hauptloop-Blockade. Nicht 1,6 s am Stück. Zum Vergleich: Der Overlay-Import löscht in
einem Zug bis zu 32 Einträge, das sind rund 8,6 s (Befund **L80**) — F1 bleibt um zwei
Grössenordnungen darunter.

*Polling:* 400 ms Takt, höchstens 15 Abfragen je Abzug, und nur während eines Exports
oder Imports. Keine Dauerlast; `ir_codes_get` erzeugt kein STM-Kommando.

*RX-Ring:* Der kritische 256-Byte-Ring (`esp8266-uart.c:69`) ist die Richtung
ESP→STM. Dort geht je Taste genau **ein** 17-Byte-Kommando, ausgelöst von einem
eigenen HTTP-Request. Burst ausgeschlossen. In die Gegenrichtung ist der ESP durch die
Quittungswartezeit in `var_send_buf()` ohnehin getaktet.

*Hartkodierte Grenzen:* `N_REMOTE_IR_CMDS` (STM) und `MAX_IR_CODES` (ESP) müssen
übereinstimmen. Laufen sie auseinander, verwirft der ESP die überzähligen Indizes, die
Maske bleibt unvollständig und `complete` bleibt `false` — ein sichtbarer Fehlschlag,
keine stille Teilsicherung.

**Secure by design** — Kein `innerHTML` mit Fremddaten: Die Werte sind Zahlen, die
Tastennamen stammen aus einer festen Liste in `app.js`, nicht aus der Datei. Namen aus
einer Sicherungsdatei werden zur **Zuordnung** gegen diese Liste geprüft und nie
angezeigt; ein unbekannter Name führt zum Überspringen. Keine Credentials in
Logausgaben — das neue Debuglog schreibt Index, Protokoll, Adresse, Kommando, und das
sind Fernbedienungscodes, keine Geheimnisse. Der Update-Server ist nicht beteiligt.

Zum Punkt „neue Endpunkte, die Konfiguration schreiben, brauchen eine bewusste
Entscheidung über Bestätigung und Reichweite": Sie ist getroffen und steht unter
„Ablauf in der PWA" und bei AK14/AK15. Reichweite: genau eine Taste pro Aufruf, nie
alle auf einmal. `/api/ir_code_set` gehört in die Gefahrenliste in `CLAUDE.md` R5 und
in `tools/hooks/no-danger.py`.

**Stable & reliable** —

- *Jeder Fehler ausgewertet:* `remote_ir_set_code()` liefert 0 bei ungültigem Index,
  und `schedule_esp8266_ir_code()` schreibt das in den Debuglog. Keine stille
  Verwerfung ohne Spur.
- *Keine leeren `catch`:* Der PWA-Teil nutzt die bestehenden Stufenfehlerpfade; der
  Timeout beim Abzug führt zu einer sichtbaren Meldung, nicht zu einem leeren Abschnitt.
- *Stille Verwerfungen:* Der Fall „Kommando geht verloren" ist nicht verhindert, aber
  er ist **sichtbar** — die Empfangsmaske benennt die fehlenden Indizes namentlich.
  Das ist der ganze Zweck von `missing[]`.
- *Werte werden nicht zurechtgebogen:* `protocol` 0 und 255 werden abgewiesen, nicht
  auf einen gültigen Wert gezogen. Ein Index ausserhalb 0..19 wird abgewiesen, nicht
  maskiert — anders als `from_day` es vor **L71** wurde.
- *Zustand nach Abbruch mitten in der Sequenz:* definiert. Jedes `ir_code_set`
  schreibt RAM und EEPROM gemeinsam; ein Abbruch nach Taste 12 hinterlässt 12 neue und
  8 alte Tasten, und der nachfolgende Abzug zeigt genau das. Keine Indexverschiebung
  wie beim Overlay-Import, weil jede Taste ihren festen Platz hat.
- *Flags werden auf jedem Pfad aufgelöst:* `ir_export_idx` zählt deterministisch bis
  `N_REMOTE_IR_CMDS` und bleibt dort stehen; es gibt keine Bedingung, die den Zähler
  hängen lassen könnte. Ein erneuter RPC setzt ihn auf 0 zurück, auch mitten im Lauf —
  und weil der ESP seinen Puffer **vor** dem RPC leert, bleibt das konsistent. Auf der
  ESP-Seite ist `ir_codes_requested` das Gegenstück und wird von `ir_code_set`
  ausdrücklich wieder auf 0 gesetzt.

---

## Prüfbarkeit ohne Fernbedienung

`/api/learn_ir` ist gesperrt, auf dem Testgerät lässt sich kein Code anlernen. Der
**von aussen sichtbare Zustand ist ausschliesslich der Abzug selbst** — am Display ist
nichts zu sehen, solange niemand eine Taste drückt.

Genau deshalb muss der Abzug ein echter Abzug vom STM sein. Würde `/api/ir_code_set`
den ESP-Puffer mitführen, spiegelte die Gegenprobe die eigene Eingabe zurück und bewiese
gar nichts. Die Invalidierung aus AK9 ist nicht Kosmetik, sondern die Voraussetzung
dafür, dass dieser Nachweis trägt.

**Ablauf, Schritt für Schritt.** Die Schritte 1 bis 3 sind lesend und frei (DIR-008);
ab Schritt 4 ist die ausdrückliche Freigabe des Nutzers im selben Gespräch nötig.

1. `GET /api/ir_codes_request`, dann `GET /api/ir_codes_get` bis `complete:true`.
   Ergebnis ist der **Referenzabzug R0**.
2. R0 beantwortet zugleich die Risikofrage: Sind alle 20 Tasten leer (`protocol` 0
   oder 255), hat das Gerät gar keine angelernten Codes — dann ist der ganze folgende
   Ablauf risikofrei. Stehen echte Codes darin, ist R0 der Rückweg und muss vor
   Schritt 4 gesichert sein.
3. **Negativprüfungen, rein lesend**, weil jeder Aufruf abgewiesen wird: die fünf
   Zeilen aus AK8. Sie weisen nach, dass sich **L70**/**L71** hier nicht wiederholen —
   und sie schreiben per Definition nichts.
4. Schreibnachweis mit einem synthetischen Wert, der keiner echten Fernbedienung
   entspricht: `GET /api/ir_code_set?idx=0&protocol=2&address=4660&command=1`.
5. Erneuter Abzug. Taste 0 muss **exakt** `protocol:2, address:4660, command:1`
   zeigen. Das ist der Nachweis des gesamten Schreibpfads PWA → ESP → STM → EEPROM,
   ohne eine einzige Fernbedienung.
6. `GET /api/maintenance_reset_stm32`, dann erneuter Abzug. Taste 0 zeigt weiterhin
   dieselben Werte. **Das** ist der Nachweis, dass der Wert im EEPROM steht und nicht
   nur im RAM — der Unterschied, an dem nach einem EEPROM-Reset alles hängt.
7. R0 Taste für Taste zurückschreiben, erneuter Abzug, feldweiser Vergleich gegen R0.
   Erst wenn der stimmt, ist der Ablauf abgeschlossen.
8. Vollständigkeitsnachweis unter Last: Schritt 1 wiederholen, während ein
   Ticker-Overlay läuft. `complete` muss weiterhin `true` werden. Gelingt das nicht,
   ist die Flusskontrolle zu dünn und `missing[]` nennt die betroffenen Indizes.

**Was dieser Ablauf nicht beweist:** dass die Uhr auf die wiederhergestellten Codes
auch *reagiert*. Dafür braucht es eine Fernbedienung, und das bleibt beim Nutzer. Die
Spec verlangt es nicht als Akzeptanzkriterium, weil es im Testrahmen nicht erbringbar
ist — aber es gehört als ausdrücklicher Restpunkt in die Übergabe, nicht als
stillschweigende Annahme.

**Zusätzlich sichtbar:** `/api/stm32_log` zeigt die `var_send: I...`-Zeilen des
Exports (`vars.c:61`) und die Debugzeile des Empfangs. Befund **L75** einplanen: Unter
LED-Aktivität ist der 64-Zeilen-Ring nach gut einer Sekunde überschrieben. Also direkt
nach dem Abzug abfragen, nicht später.

---

## Verworfene Alternativen

**A — Ein Hexblock über die Brücke, je ein Kommando pro Richtung.** Der Entwurf, der
jahrelang im Massnahmenkatalog stand. **Verworfen, und zwar hart.** 100 Byte ergeben
200 Hexzeichen. ESP→STM schneidet `strncpy(…, ESP8266_MAX_CMD_LEN)` nach 127 Zeichen
**still** ab (`esp8266.h:68`, `esp8266.c:386`) — kein Log, kein Fehler, nur falsche
Codes. STM→ESP ist es schlimmer als nur wirkungslos: Der ESP verwirft zwar alles über
123 Zeichen (`ESP-uclock.ino:1148`), aber **vorher** formatiert der STM mit `sprintf`
in einen 160-Byte-Stackpuffer ohne jede Längenprüfung (`vars.c:190-200`). Ein
200-Zeichen-Wert überschreibt den Stack. Dieser Entwurf hätte die Uhr beschädigt, nicht
bloss nicht funktioniert.

**B — Export in `var_send_all_variables()` aufnehmen.** Bequem, weil die PWA dann gar
nichts anstossen müsste. **Verworfen:** Die Funktion sendet bereits rund 190 Kommandos
mit je bis zu 3 s Quittungswartezeit **ohne einen einzigen** `watchdog_reload()`
(`vars.c:951-1019`, Befund **L85**), und sie läuft im `ESP8266_IPADDRESS`-Zweig
(`main.c:2744`) — genau dem Startabschnitt, bei dem der F411 hängt. 20 weitere
quittungspflichtige Kommandos dort hineinzulegen wäre die unklügste denkbare
Reihenfolge.

**C — Die 20 Kommandos in einer `for`-Schleife im RPC-Zweig senden.** Der naheliegende
Weg und der, den die Analyse offenlässt. **Verworfen:** bis zu 20 × 3 s = 60 s gegen
ein 20-s-Watchdog-Budget, ohne Reload im Pfad. Siehe „Abweichung von der Analyse".

**D — Alle 20 Tasten im STM sammeln und mit einem Commit-Kommando ins EEPROM
schreiben.** Spart 19 EEPROM-Schreibvorgänge und macht den Restore „alles oder
nichts". **Verworfen:** Bleibt der Commit aus — Verbindungsabbruch, geschlossener
Browser-Tab, ESP-Neustart —, stehen RAM und EEPROM auseinander, und die Uhr reagiert
bis zum nächsten Neustart auf Codes, die nirgends gespeichert sind. Genau diesen
Zustand räumt `remote-ir.c:168-172` nach einem abgebrochenen Lernvorgang eigens wieder
auf. Die Einzelschreibung hält beide immer in Deckung; 80 ms je Taste sind der Preis
dafür, und er ist niedrig.

**E — Ein echtes Gültigkeitsflag je Taste statt der `protocol`-Konvention.**
Sauberer, weil „absichtlich leer" und „nie angelernt" unterscheidbar würden.
**Verworfen:** Ein sechstes Byte je Taste ändert das EEPROM-Layout, verlangt einen
`EEPROM_VERSION`-Bump und einen Migrationslauf für jedes bestehende Gerät. Der Gewinn
— eine Unterscheidung, die heute niemand braucht — trägt das nicht.

**F — Zuordnung beim Import über den Index statt über den Namen.** Einfacher.
**Verworfen:** `remote-ir.h:19-34` enthält einen auskommentierten Block „New Modes
(future use)", der die Nummerierung vollständig umstellen würde. Tritt das ein, wird
eine indexbasierte Zuordnung **still falsch** — jede Taste bekäme den Code einer
anderen, und auffallen würde es erst mit der Fernbedienung in der Hand.

**G — Den ESP-Puffer bei `/api/ir_code_set` mitführen statt invalidieren.** Spart der
PWA einen Abzug. **Verworfen:** Damit spiegelte die Gegenprobe die eigene Eingabe
zurück. Auf einem Gerät ohne Fernbedienung ist der Abzug der **einzige** von aussen
sichtbare Zustand; ihn zur Selbstbestätigung zu machen, nähme ihm genau die
Beweiskraft, für die er gebraucht wird. Das ist dieselbe Klasse wie die Lehre aus
**L81**: Wer seine eigene Schreibarbeit bestätigt, bestätigt seine Absicht, nicht den
Gerätezustand.

**H — Den Abzug beim Wiederholungsversuch aufaddieren statt den Puffer zu leeren.**
Fände bei einem einzeln verlorenen Kommando schneller ans Ziel. **Verworfen:** Das
Ergebnis wäre aus zwei Durchläufen zusammengesetzt und damit kein Abzug eines
einzelnen Zeitpunkts mehr. Deterministisch von vorn ist hier mehr wert als schnell.

---

## Versionsfolgen

- [ ] STM `src/main.h` anheben
- [ ] ESP `ESP8266/ESP-uclock/version.h` anheben
- [ ] App `APP_VERSION` anheben
- [ ] `CACHE_NAME` in `sw.js` anheben — zusammen mit `APP_VERSION`, nie einzeln

Alle vier Komponenten sind geändert, also steigen auch alle vier (DIR-004). Nur der
`release-engineer` führt das aus (R4). **Keine Versionsnummern in dieser Spec** —
`specs/**` ist Momentaufnahme (DIR-006), und den gültigen Stand zeigt
`./tools/guardrails.sh` in Stufe S4.

Nach dem Release: `./tools/install-app.sh --check`, dann `./tools/install-app.sh`. Die
PWA kommt nicht über den Rollout aufs Gerät, und ein ESP-Update bringt sie nicht mit.
