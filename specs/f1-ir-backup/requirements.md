# Anforderungen — F1, IR-Codes ins Backup

**Status:** Entwurf
**Auslöser:** `BEFUNDE.md` Massnahme **F1** (ToDo), Befund **L35**, Firmware-Analyse **L84**
(`✔ verifiziert` — die Angaben aus L84 sind beim Schreiben dieser Spec an allen genannten
Fundstellen im Quelltext nachgeprüft worden, siehe `design.md`, Abschnitt „Belegte
Faktenlage")

## Problem

Die PWA sichert die Einstellungen der Uhr als JSON (`buildSettingsBackup`,
`app.js:4193`). Zehn Abschnitte sind enthalten — Anzeige, Netz, Wartung, Klima,
Animationen, TFT, Ambilight, DFPlayer, Overlays, Timer (`app.js:4117-4128`).

**Die gelernten IR-Fernbedienungscodes fehlen.** Sie sind damit die **einzige**
Konfiguration des Geräts ohne jeden Rückweg:

- Sie liegen ausschliesslich im EEPROM an Offset `EEPROM_DATA_OFFSET_IRMP_DATA`
  (`src/eeprom/eeprom-data.h:175`) und im RAM-Spiegel `irmp_data_array`
  (`src/remote-ir/remote-ir.c:22`, `static`, von aussen nicht erreichbar).
- Es gibt keinen API-Endpunkt, der sie liest. Es gibt keinen, der sie schreibt.
  Der einzige Weg, sie zu erzeugen, ist `/api/learn_ir` (`http.cpp:8577`).
- `/api/learn_ir` blockiert die Uhr, bis ein Mensch 20 Tasten gedrückt hat. Der
  Vorgang ist deshalb in Phase 8 des `TESTPLAN-PWA.md` gesperrt und für Agenten
  nicht auslösbar; die Lernschleife bedient den Watchdog eigens
  (`remote-ir.c:149-150`), weil sie ihn sonst zwangsläufig auslösen würde.

**Konkrete Folge, am Code belegt:** Nach `/api/maintenance_reset_eeprom` wird nur die
Versionsnummer genullt (`main.c:1590-1596`). Beim **nächsten STM-Neustart** schreibt
`read_configuration_from_eep()` den dann leeren RAM-Stand zurück (`main.c:1245`,
`remote_ir_write_codes_to_eep()`) — und damit sind die 20 Tastencodes endgültig weg.
Der Nutzer muss jede Taste neu anlernen, mit genau dem Vorgang, der die Uhr dabei
minutenlang blockiert.

**Der naheliegende Entwurf ist ausgeschlossen.** Der Massnahmenkatalog nannte
jahrelang „160 Byte hexkodiert, ein Lesekommando und ein Schreibkommando". Dieser
Weg beschädigt den STM-Stack — Beleg in `design.md`, Abschnitt „Verworfene
Alternativen".

## Ziel

Die IR-Codes aller 20 Tasten sind Teil der Sicherungsdatei und lassen sich daraus
vollständig wiederherstellen, ohne dass eine Fernbedienung oder der Lernvorgang
gebraucht wird. Der Abzug ist entweder vollständig oder er findet nicht statt — ein
Backup mit 19 von 20 Tasten darf nicht entstehen.

## Akzeptanzkriterien

### Protokoll und STM

- [ ] **AK1** — Ein neues Kommando `I<idx:2><protocol:2><addr:4><cmd:4>` (13 Zeichen,
      Hex, Grossbuchstabe `I`) existiert in **beiden** Richtungen. Nachweis: Der
      Buchstabe `I` ist in `schedule_esp8266_cmd()` (`src/main.c:2592`) und in
      `var_set_parameter()` (`ESP8266/ESP-uclock/vars.cpp:764`) belegt und jeweils
      mit Indexprüfung versehen.
- [ ] **AK2** — Der STM sendet die 20 Kommandos **ausschliesslich auf Anforderung**
      über einen eigenen RPC. Nachweis: `var_send_all_variables()`
      (`src/vars/vars.c:951-1019`) enthält danach **keinen** Aufruf, der ein
      `I`-Kommando erzeugt. Begründung: Befund **L85**.
- [ ] **AK3** — Zwischen zwei `I`-Kommandos läuft mindestens ein kompletter
      Durchlauf des Hauptloops. Nachweis: Der Sendezustand wird im Hauptloop
      getaktet, nicht in einer Schleife innerhalb des RPC-Zweigs. Damit liegt
      zwischen zwei Kommandos der reguläre `watchdog_reload()` aus `main.c:3214`.
- [ ] **AK4** — Ein eintreffendes `I`-Kommando schreibt **nur** die betroffenen
      5 Byte ins EEPROM und führt `irmp_data_array[idx]` mit. RAM und EEPROM laufen
      nicht auseinander. Nachweis: Nach einem Schreibvorgang und einem STM-Neustart
      liefert der Abzug denselben Wert.
- [ ] **AK5** — `EEPROM_VERSION` bleibt unverändert. Das Speicherlayout wird nicht
      angefasst, es gibt keinen Migrationslauf.

### ESP und API

- [ ] **AK6** — `GET /api/ir_codes_request` verwirft den ESP-Puffer, setzt die
      Empfangsmaske auf 0 und stösst den RPC an. Antwort `{"ok":true,"expected":20}`.
- [ ] **AK7** — `GET /api/ir_codes_get` liefert `requested`, `expected`, `received`,
      `complete`, `missing[]` und `codes[]`. `complete` ist genau dann `true`, wenn
      alle 20 Indizes eingetroffen sind — nicht, wenn der Puffer „plausibel aussieht".
- [ ] **AK8** — `GET /api/ir_code_set?idx=&protocol=&address=&command=` verlangt
      **alle vier** Parameter. Prüfbar mit diesen fünf Aufrufen, jeder muss
      `{"ok":false,...}` mit der genannten Kennung liefern:

      | Aufruf | erwartete Kennung |
      |---|---|
      | `?protocol=2&address=1&command=1` (kein `idx`) | 1 |
      | `?idx=&protocol=2&address=1&command=1` | 1 |
      | `?idx=20&protocol=2&address=1&command=1` | 2 |
      | `?idx=0&protocol=0&address=1&command=1` | 2 |
      | `?idx=0&protocol=2&address=65536&command=1` | 2 |

      Ein stilles `{"ok":true}` mit Vorgabeindex 0 ist der Fehler aus **L70**; er darf
      sich hier nicht wiederholen.
- [ ] **AK9** — `/api/ir_code_set` **invalidiert** den ESP-Puffer (`requested=false`,
      Maske 0), statt ihn mit dem geschriebenen Wert zu füllen. Nachweis: Ein
      `ir_codes_get` direkt nach einem `ir_code_set` liefert `complete:false`. Grund:
      Sonst spiegelte der Abzug die eigene Eingabe zurück und wäre als Gegenprobe
      wertlos.
- [ ] **AK10** — Der Puffer liegt ausschliesslich im ESP-RAM. Nach einem ESP-Neustart
      meldet `ir_codes_get` `requested:false`, und die PWA bricht einen laufenden
      Abzug mit klarer Meldung ab, statt einen leeren Satz zu sichern.

### PWA

- [ ] **AK11** — Die Sicherungsdatei enthält den Abschnitt `settings.ir` mit genau
      20 Einträgen, jeder mit `name`, `index`, `protocol`, `address`, `command`.
      Format und Namensliste: `design.md`.
- [ ] **AK12** — Bleibt der Abzug nach einem Wiederholungsversuch unvollständig, wird
      der Abschnitt `settings.ir` **weggelassen** und der Nutzer darauf hingewiesen.
      Ein teilbefüllter Abschnitt entsteht unter keinen Umständen.
- [ ] **AK13** — Eine Taste ohne angelernten Code steht im JSON mit `protocol: null`,
      `address: null`, `command: null`. Beim Import wird sie übersprungen; sie löscht
      keinen vorhandenen Code auf dem Gerät.
- [ ] **AK14** — Der Import des IR-Abschnitts läuft nur nach ausdrücklicher
      Bestätigung des Nutzers, und die Rückfrage nennt die Zahl der zu
      überschreibenden Tasten.
- [ ] **AK15** — Vor dem Überschreiben lädt die PWA den **aktuellen** IR-Stand als
      eigene Rückfalldatei herunter. Gelingt dieser Abzug nicht vollständig, sagt die
      zweite Rückfrage das ausdrücklich und ist mit **Abbruch** vorbelegt.
- [ ] **AK16** — Nach dem Import prüft die PWA durch einen **erneuten Abzug** feldweise
      nach und meldet jede Abweichung. Die Erfolgsmeldung hängt am Abzug, nicht am
      `{"ok":true}` der 20 Schreibaufrufe.
- [ ] **AK17** — `BACKUP_VERSION` steigt auf 3. Eine Sicherung mit Version 2 wird
      weiterhin angenommen (Vorarbeit aus **L82**), und der IR-Abschnitt fehlt dann
      folgenlos.
- [ ] **AK18** — Kein bestehendes Feld der Sicherungsdatei wird umgedeutet, umbenannt
      oder entfernt. Es wird ausschliesslich angefügt. Damit entfällt die Migration,
      die `L82` sonst verlangen würde.

### Querschnitt

- [ ] **AK19** — Der Round-Trip-Nachweis aus `design.md` („Prüfbarkeit ohne
      Fernbedienung") ist am Gerät durchgeführt und protokolliert.
- [ ] **AK20** — `./tools/guardrails.sh --full` läuft mit Exit 0 durch.

## Nicht Teil dieser Änderung

- **`/api/learn_ir` wird nicht angefasst.** Weder entsperrt noch abbruchfähig gemacht
  noch mit einem Fortschritt versehen. Der Lernvorgang bleibt, wie er ist.
- **`var_send_all_variables()` wird nicht repariert.** Der fehlende
  `watchdog_reload()` ist Befund **L85** und braucht einen eigenen Auftrag. F1 nutzt
  diesen Pfad bewusst **nicht** und verschlimmert ihn damit nicht — mehr leistet F1
  dort nicht.
- **`var_send_string()` und `var_send_str_variable()` bleiben unverändert.** Der
  `sprintf`-in-Stackpuffer ist Befund **L86**; F1 vermeidet ihn, indem das neue
  Kommando feste 13 Zeichen hat, behebt ihn aber nicht.
- **Kein Vollabgleich nach ESP-Neustart.** Massnahme **A6** (Befund **L42**) bleibt
  offen. Sollte sie gebaut werden, dürfen die `I`-Kommandos **nicht** Teil davon
  werden — aus demselben Grund wie bei AK2.
- **Die Legacy-Oberfläche bekommt keine IR-Seite.** Sie bleibt unverändert und damit
  Stabilitäts-Referenz.
- **Kein Löschen einzelner IR-Codes über die API.** `protocol` 0 und 255 sind als
  „leer" reserviert und werden von `/api/ir_code_set` abgewiesen (AK8). Ein gezieltes
  Löschen wäre ein eigenes Thema mit eigenem Gefahrenprofil.
- **Kein Guardrail auf die Namensliste.** Dass die 20 Namen in `app.js` der
  Reihenfolge aus `src/remote-ir/remote-ir.h` folgen, ist eine Kopplung, die eine
  Prüfung in `tools/checks/` verdiente. `tools/**` gehört dem Lead; das gehört in
  einen eigenen Auftrag und ist in `design.md` als Empfehlung vermerkt.
- **Keine Änderung an `EEPROM_VERSION` und keine Nutzung des Polsters.** Byte 104..163
  im EEPROM sind reserviert (`EEPROM_MAX_IR_CODES` 32 gegen `N_REMOTE_IR_CMDS` 20) und
  bleiben unbenutzt.

## Betroffene Laufzeiten

- [x] STM32 (`src/**`) — Neu-Flashen nötig
- [x] ESP8266 (`ESP8266/ESP-uclock/*.cpp`, `*.h`) — Neu-Flashen nötig
- [x] PWA (`data/app/**`) — LittleFS-Upload nötig (`./tools/install-app.sh`)
- [ ] Build/Release — keine Änderung an `Makefile` oder `cmake/**`

**Reihenfolge beim Ausrollen:** zuerst STM, dann ESP, dann PWA. Ein neuer ESP mit
altem STM ist definiert fehlerhaft, nicht still falsch: Der STM kennt die neue
RPC-Nummer nicht, sein `switch` hat keinen `default`-Zweig (`main.c:1525-1604`), es
kommen keine `I`-Kommandos, und `ir_codes_get` meldet `complete:false`. Der Abzug
scheitert sichtbar. Eine alte PWA mit neuem ESP ignoriert die neuen Endpunkte.
