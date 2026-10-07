# Design — Paket 2026-10-06 (Runden P, P2, S, P3, F, C4, verteilte Runde K)

**Erstellt:** 2026-10-06, Stand `401aae2`. **Nachgeführt** am 06.10.2026 nach der
Auslieferung von P und dem Gerätetest (L324), am **07.10.2026** mit den Entscheidungen des
Nutzers. **Freigegeben am 07.10.2026** (Ent-1). Momentaufnahme (DIR-006).

**Rundenfolge:** **P (ausgeliefert) → P2 → S → P3 → F**, dazu **C4** ohne Flash nach S.
Runde K fährt mit (§6). Kennungen: „N1"/„N2" sind Befunde der Nachzählung; **B40/L321**,
**C38/L323**, **L324** aus dem Gerätetest von 1.4.91; „Ent-n" sind Entscheidungen
(`requirements.md`). **`vars.h` ohne Pfad meint `ESP8266/ESP-uclock/vars.h`.**

---

## 0. Was sich gegenüber `specs/paket-2026-10-05/` geändert hat

| Punkt | Alte Spec | Diese Spec | Grund |
|---|---|---|---|
| Runden V, W, E1, E2, H | geplant | **weg** | ausgeliefert bzw. E1 entfallen |
| Runde U | eigene Runde | **aufgelöst**: B27, B32 nach P; B28, B31 warten | Nutzer |
| Reihenfolge | … → S → P → U → F | **P → P2 → S → P3 → F**, C4 ohne Flash | Nutzer; P2 aus dem Rest von P; P3 für A5 |
| Runde K | — | **neu**, ohne eigenen Flash | Nutzer, Nachzählung |
| Runde P | B33, B24 | **ausgeliefert als 1.4.91** | — |
| Runde P2 | — | Overlay-Text, **B40**, **SSID-Auswahl**, **Massnahme 4**, B17, Probe L303/L306 | L324 |
| Runde S | A39, L260, A35 T1, C26, A16 | dasselbe **plus C31, A46, N1, E17, A5** und neun STM-K-Punkte; **zweistufiges Flash-Gate**, Reserve 1'024 Byte | L298, L291, Nachzählung, L296, Entscheidungen vom 07.10. |
| Runde P3 | — | **A5 in der PWA** | Firmware vor PWA (L241) |
| Runde F | C9c6, 65535-Liste, Smoketest | dasselbe **plus C28, C38, Ent-4, Ent-5, C23, C6u** und K-Punkte | §3, §6 |
| Schritt C4 | — | Umschrift ASCII, **`.hex`-Gleichheit, kein Flash** | Nutzer, §8 |
| Testdurchlauf | eine Abnahme je Runde | **je Runde ein eigener Task**; P2 und P3 begründet ohne; **M2 nach B19 durch den Agenten** | L302, §5 |
| Zähler C26 | „Logring **oder** `/api/device_ready`" | **nur Logring** | §4.2 |
| Prüfstände, Proben | vom Umsetzer abgelegt | Umsetzer baut, **Lead legt ab** | `tools/**` |
| STM-Guards | Gerätetest | **nur Prüfstand** (S15) | über HTTP nicht herstellbar |
| Builds | „einmal am Ende" | **serielle Testbauten** (S), **Gleichheitsbauten** (C4) | Ent-7 |
| Commit/Tag | teils nach der Abnahme | **im Rollout-Task** | DIR-011 |

**Was bleibt, wörtlich oder sinngemäss:** §2.1 (A39, Variante b), §2.2 (Eröffnungszeile und
Abschlussmarke samt Prämisse aus L269), §2.3 (C26), §2.4 (A35), §2.5 (die beiden Warnungen).

---

## 1. Runde P — ausgeliefert, und P2

**Stand:** P ist als PWA 1.4.91 ausgeliefert, die Abnahme steht in L324. §1.1 bis §1.5
beschreiben den Entwurf, nach dem gebaut wurde.

### 1.1 B35 — Byte statt Zeichen, ohne Markup-Änderung

`http_check_strvar_len()` (`http.cpp:10351`) vergleicht `strlen (value)` — die
URL-dekodierten UTF-8-Byte. **`new TextEncoder().encode(text).length` liefert exakt die
Zahl, die das Gerät prüft.**

| Endpunkt | Byte | Quelle (`ESP8266/ESP-uclock/vars.h`) | Gerät heute | PWA 1.4.91 |
|---|---|---|---|---|
| Ticker | 32 | `:183` | weist ab | prüft |
| Datumsformat | 5 | `:194` | weist ab | prüft |
| Wetter-APPID | 32 | `:188` | weist ab | prüft |
| Wetter-Ort | 32 | `:189` | weist ab | prüft |
| Koordinaten | je 8 | `:190-191` | weist ab | prüft |
| Zeitserver | 16 | `:187` | weist ab | prüft — **ein später Scan kann den Wert vorher ersetzen** (B40) |
| Update-Host / -Pfad | je 63 | `:192-193` | weist ab | prüft |
| **Overlay-Text** | 32 | `:414` | **kürzt still** — K-Punkt F | **fehlt — P2.1** |

**Der Overlay-Text gehört in dieselbe Tabelle**, obwohl das Gerät ihn heute noch kürzt —
bis zum F-OTA ist die PWA der einzige Schutz. **Die Prüfung sitzt an der Endpunktgrenze,
nicht am Eingabefeld**, weil Eingabefeld, Kartenauswahl, Overlay-Editor und Backup-Import
dieselben Endpunkte beschreiben (L179 in der PWA). `index.html` bleibt unverändert.

### 1.2 R1 und R2 — Kennung 6 aus `fs_remove` ist doppelt belegt

`http_api_fs_remove()` meldet Kennung 6 für „file not found" (`:11536`) **und** „remove
failed" (`:11545`). **Regel (ausgeliefert):** Kennung 6 erst nach erneutem Listenabruf
deuten. **Ent-4 = ja:** F.2 gibt „remove failed" eine eigene Kennung; die PWA-Regel bleibt
richtig, und bis zu einer Übersetzung zeigt `describeApiError()` den Rückfalltext mit Detail.

### 1.3 B36, 1.4 B34, 1.5 R3, R4, B33, B27, B26, B32, B17

Ausgeliefert wie entworfen: B36 über den Rückgabewert von `runButtonRequest()`; B34 über
den Schutz gefüllter Flächen in `applyTranslations()`; R3 einmal je Sitzung sichtbar (löst
C32 nicht); R4 überspringt und nennt; B33 zeigt „binär"; B27 mit „Sekunden am
Ambilight-Ring weich ausblenden"; B26 entfernt, die Kennungen `files-panel` und
`local-update-panel` bleiben als Panelnamen von `tools/ui-mess`; B32 gefahren. **B17** ist
nicht gemacht und steht in P2 (nur wenn klein).

### 1.6 E31 ist erledigt — mit einem Vorbehalt

Ob die STM-Indexguards greifen, ist über HTTP nicht zeigbar, weil der ESP jeden Index vorher
abweist. **Abnahme von STM-Guards über den Prüfstand** (AKK.6).

### 1.7 P2: Einspielen, und warum P2 keinen eigenen Testdurchlauf hat

**P2 vor S**, weil jeder S-Build `app-gz` mitbaut (§4.1). **Kein eigener
`pwa-tester`-Durchlauf**, aber nicht mehr wegen der Grösse:

- **B40 und die SSID-Auswahl sind Wettläufe.** Ein Gerätelauf träfe sie zufällig und meldete
  bei Verfehlen grün. In der Vorschau mit verzögerter `network_scan`-Antwort sind sie gezielt
  herstellbar — **Probe P2.1c**, einmal gegen 1.4.91 fehlgeschlagen.
- **L303 und L306** brauchen eine Fehlerantwort, die an einer produktiven Uhr nicht folgenlos
  entsteht — **Probe P2.1e** mit nachgebildeter Antwort (§1.10).
- **Massnahme 4** ist in der Vorschau vollständig prüfbar.
- **Am Gerät lesend, ohne Speichern** (P2.8); **S.26** prüft B35 am Zeitserver mit
  Speichern.

**Offen bleibt ausdrücklich:** Zwischen P2-Upload und S.26 sind B40 und die SSID-Auswahl am
Gerät nur lesend belegt; L303 und L306 nie mit echtem Fehlschlag.

### 1.8 B40 / L321 und die SSID-Auswahl — eine Regel, drei Stellen

**B40:** `updateNetworkControlsFromMeta()` setzt Zeitserver und Zeitzone nach **jeder**
`network_scan`-Antwort ohne Bedingung; „Speichern" schickt danach still den alten Wert.
**SSID-Auswahl** (P2.1f): `network-ssid-select` wird bei jedem Scan per `innerHTML` neu
aufgebaut, mit `selected` auf der Geräte-SSID — eine ungespeicherte Auswahl geht verloren.

**Lösung für beide:** **dieselbe** `prefillDeviceValue()`-Regel wie bei der AP-SSID (L34):
Ein Gerätewert setzt nur, was weder fokussiert noch vom Nutzer geändert ist. Bei der
Auswahlliste heisst das: Die Liste darf neu aufgebaut werden, aber eine vom Gerätewert
abweichende, ungespeicherte Auswahl wird danach wiederhergestellt. **Keine zweite und keine
dritte Fassung** (L179, L289). SSID-Namen sind Fremddaten aus der Luft — sie gehen weiter
über `escapeHtml` oder DOM-Knoten ins DOM, nie roh per `innerHTML`.

### 1.9 Massnahme 4 in engerer Form — ändern heisst abweichen

`handleDirtyFormInteraction()` setzt `hasUnsavedEdits` bei jeder Eingabe. Künftig: geändert
ist, was **vom Ausgangswert abweicht** — und der Ausgangswert ist **derselbe**, den
`prefillDeviceValue()` aus §1.8 kennt. Zwei Antworten auf „was ist der Gerätewert" liefen
auseinander, sobald eine geändert wird.

### 1.10 L303 und L306 — Nachweis über eine nachgebildete Fehlerantwort

Hinter `runConfirmedButtonAction` stehen `downloadUpdateAssets`, `resetStm32` und
`formatLittleFsFromFiles` — nichts davon lässt man an einer produktiven Uhr scheitern; ein
Flag-Umschalten (L306) scheitert nicht über einen zu langen Text. **Probe P2.1e** bildet die
Fehlerantwort nach. **Einmal fehlgeschlagen gegen einen künstlich zurückgebauten Stand**, im
Scratchpad (L268), weil 1.4.91 die Korrekturen schon enthält. **Warum das genügt:** Beide
Befunde betreffen die **Darstellung** einer Fehlerantwort, nicht ihr Zustandekommen.

---

## 2. Runde S — die Brücke fertigbauen

### 2.0 Zwei Richtungen

`CAP var-crc` und `FIRMWARE` laufen **ESP → STM**; Prüfsummen-Marke **STM → ESP**; Quittung
`.` und `!v` **ESP → STM**. **Alle Neuerungen dieser Runde laufen STM → ESP**, und dafür gibt
es keinen Fähigkeitskanal. **Die Information reist im Strom mit.** Das gilt auch für A5
(§7): Die neue Variable trägt ihren eigenen „unbekannt"-Wert, statt dass eine Fähigkeit
gemeldet wird.

### 2.1 A39 / L259 — kein verschachtelter 194er-Stoss mehr

✔ `src/main.c:3217` ruft im `IPADDRESS`-Zweig direkt `var_send_all_variables()`; der
`SYNCVARS`-Zweig (`:3243-3266`) merkt vor. **Variante (b):** vormerken, im Hauptloop senden
(`:3946-3949`), **danach** den IP-Lauftext setzen. **`pending_weather_ticker_restore`
wandert mit dem Ticker mit.** A39 schützt zugleich den Empfangsring für §2.4.

### 2.1b E17 — die Umbenennung, als eigener Diff nach A39

**Entscheidung des Nutzers (07.10.2026): mit S.** Die frühere Empfehlung dieser Spec war
„nicht mit S", weil eine Umbenennung im selben Flash den Diff von A39 unlesbar macht. **Die
Entscheidung lässt sich so umsetzen, dass dieser Einwand entfällt:**

- **S.15 (A39) bleibt beim alten Namen.** Sein Diff zeigt nur die Verschiebung.
- **S.15b ist die Umbenennung allein** — `pending_weather_ticker_restore` →
  `pending_ticker_restore` (Vorschlag; der Name sagt, was L217 belegt: Das Flag trägt alle
  Ticker). **Keine andere Änderung.** Der Lead prüft mit `git diff --word-diff`; jede
  Zeile, die mehr als den Bezeichner ändert, ist ein Fehlschlag.
- **S.15c/S.15d ziehen die Dokumente nach:** `CLAUDE.md` (Lead), `knowledge/quick-reference.md`
  und `knowledge/architecture-checklist.md` (`doc-writer`), dazu `tools/**` (Lead), falls
  ein Werkzeug den Namen sucht. **Commitet mit S.23**, damit Code und Invariante nie
  auseinanderstehen.
- **Kein Flash:** Bezeichner erscheinen im Fabrikat nicht. Der Testbau S.18 dient zugleich
  als Gegenprobe — ein Zuwachs durch S.15b wäre ein Hinweis, dass mehr geändert wurde.

### 2.2 L260 / A42 — ein Erfolgskriterium, das den ganzen Satz prüft

`var_sync_check()` (`ESP-uclock.ino:753`) prüft nur das dritte von rund 194 Kommandos.
**Eröffnungszeile und Abschlussmarke im Strom**; ohne Eröffnungszeile gilt das heutige
Kriterium. Kosten: zwei Kommandos.

#### Die Prämisse — drei Punkte (L269)

1. **`var_cmd_min_len()` liefert für einen unbekannten Buchstaben `0`** (`vars.cpp:914`),
   von E2.1 ausdrücklich erhalten (L265).
2. **`var_set_parameter()` hat keinen `default:`-Zweig.**
3. **Die gefährliche Zeigerarithmetik steht nur in den `case`-Rümpfen** — versionsrobust.

#### Auflagen und Verpackung

Kommandobuchstabe ausserhalb `N n S T D A C M O t a l I`; Nutzlast endet nicht auf `*` +
vier Hexziffern; Nutzlast unter 72 Zeichen. **Verpackung `var <Buchstabe>…` über
`var_send_buf()`, verbindlich:**

> **Ein eigenes Top-Level-Präfix wäre gefährlich.** Die ESP-Kette hat **kein
> abschliessendes `else`** — auf ein unbekanntes Präfix antwortet der ESP **gar nicht**,
> `var_send_buf()` läuft je Zeile 3 Sekunden leer, **ohne `watchdog_reload()`**. **Das ist
> exakt die Mechanik aus L266.**

### 2.3 C26 / L253 — Zähler **und** Markenpflicht

Zähler über den Logring; Markenpflicht für `HARDWARE_CONFIGURATION`, Update-Host,
Update-Pfad, Abweisung mit **`!v`**, nicht mit Schweigen. Liste **aufgezählt**.

### 2.4 A35 / L233 — Zuordnung der Quittung

`ACK <xy>` als **eigene Zeile vor** jeder Quittung; ein alter STM verwirft sie und liest den
Punkt unverändert. Herkunft der zwei Zeichen in S.2 festzulegen. Kosten rund 1,5 kB je
Vollabgleich. **Teil 2 nicht in diesem Paket.**

### 2.5 Zwei Warnungen (aus der alten Spec §6.5)

> **Ein Mechanismus, dessen Sicherheit an einem Hardware-Nebeneffekt hängt, ist nicht
> abgesichert — er hat bisher Glück gehabt.**

> **Die `var `-Verpackung ist nicht stumm, sondern regulär quittiert — und gerade das
> rettet den Entwurf.**

### 2.6 A16 / L106 — Tetris und Snake

Vom Nutzer bestätigt (Ent-6), samt 60 Sekunden Spiellauf. S7 führt zwei Stellen mehr.

### 2.7 Das Flash-Gate, zweistufig (Ent-2 entschieden, Ent-7 entschieden)

**Lage:** 1'924 Byte frei auf dem F103; **Reserve 1'024 Byte** (Ent-2). Verfügbar für S
also **rund 900 Byte**. Darin: der Kern, **A5**, und neun STM-K-Punkte (mit A9).

**Stufe 1 — Vorabschätzung (S.12).** Zweck: erkennen, ob **schon der Kern** nicht passt.

**Stufe 2 — Testbauten (S.18, dann nach S.18b und jedem S.19x).** Nur F103, ohne ZIP, durch
den Lead, seriell, zwischen abgeschlossenen Tasks. **Ent-7:** R1 soll paralleles Bauen
verhindern; das bleibt gewahrt. „Einmal am Ende" stammt aus einer Zeit ohne Flash-Gate.

**Reihenfolge und Regel (Ent-2):**

| # | Teil | Bei Unterschreitung der Reserve |
|---|---|---|
| S.18b | **A5** | **anhalten und vorlegen** — Festlegung dieser Spec (§7.6) |
| S.19a | A20 | anhalten und vorlegen |
| S.19b | A13+A47 | anhalten und vorlegen |
| S.19c | A3+A48 | anhalten und vorlegen |
| S.19d | A24 | zurückstellen, weiter |
| S.19e | A45 | zurückstellen, weiter |
| S.19f | A14 | zurückstellen, weiter |
| S.19g | A46 | zurückstellen, weiter |
| S.19h | A49 | zurückstellen, weiter |
| S.19i | A9 | zurückstellen |

Ein zurückgestellter Punkt wird **ganz** zurückgenommen, **erneut gemessen** und steht mit
seiner Zahl in der K-Tabelle. **Gegenprobe S.23:** S8b neben Schätzung und Testbauten.

### 2.8 C31 / L298 — keine Schlüssel auf der UART

„gesetzt"/„leer", kein Wert, keine Länge. Zweitweg über die STM-Logausgabe in S.7.

### 2.9 A46; A47 mit A13

A46 schliesst AKH.4 für die Datenpfade; A47 fährt mit A13 (L96: je Stelle den Verbraucher
prüfen). Beide ändern nichts an angenommenen Zeilen.

### 2.10 C32 — bewusst nicht in S

Anderer Gegenstand, offene Entwurfsfrage. **Aber die Lehre aus C32 trägt A5** (§7.2).

### 2.11 Einspielen

**Erst ESP (S.10), dann STM (S.24);** dazwischen der Rückfallnachweis S.11. Nach dem ESP-OTA
`install-app.sh --check` und Update-Quelle (DIR-009).

### 2.12 N1 — der Legacy-Overlayindex, und warum er mit S fährt

✔ `http_overlays()` (`http.cpp:4689-4717`): `oidx` aus `atoi`, nur gegen `0xff` geprüft,
Schreibzugriff über 32 Plätze, auch per `<img>` von einer fremden Seite. **Typbreite** (L256):
`uint_fast8_t` ist auf Xtensa breiter als ein Byte. **Fix:** `oidx < MAX_OVERLAYS && oidx <=
n_overlays` vor jedem Schreibzugriff und vor `n_overlays++`. **Mit S**, weil S-ESP der
nächste ESP-Flash ist. STM-Seite über A3+A48 (S.19c), abgenommen über den Prüfstand.
**C6u** (F.5j) fasst später dieselbe Funktion an — nacheinander, nie gleichzeitig.

---

## 3. Runde F — Flash-Überwachung

### 3.1 Eine Funktion für drei Stellen

`http_remote_stm32_filename_matches()` gibt ohne Filter und bei kurzen Namen `0` zurück.
Legacy-Flashzweig, Legacy-Liste und `fname + 6` (C28) über dieselbe Funktion (F.1). **Ent-5
= ja:** auch die API-Liste (F.3) — bei 65535 **leer**. F.6 prüft, wie die PWA eine leere
Liste darstellt; braucht sie eine Anpassung, kommt sie **nach** dem F-OTA (L241).

### 3.2 Smoketest-Stufe — 65535 als Fehlschlag; E11 schreibt dieselbe Datei.

### 3.3 Warum F zuletzt — ein STM-Flash mit vorhersagbarem Ergebnis ist die ehrlichste Probe.

### 3.4 Einspielen — nur ESP, danach der STM-Flash als Probe.

### 3.5 C38 / L323 — abweisen statt still kürzen

Über `http_check_strvar_len()` oder dieselbe Byte-Regel. **Abnahme nur am Quelltext und am
Prüfstand** — `network_client_set` bleibt gesperrt.

### 3.6 C23 — `/api/stm32_log` in UTF-8 mit Escapen

**Entscheidung (07.10.2026): wandeln und escapen**, nicht ersetzen. Die STM-Zeilen kommen in
ISO-8859-1; der Endpunkt wandelt je Byte über `0x7F` nach UTF-8 und escapet `"`, `\` und
Steuerzeichen nach JSON. **Zeichenweise in den Ausgabestrom**, nicht über einen `String`
(L175). **Wechselwirkung:** Jedes Werkzeug, das den Logring liest (`watch-log.sh`,
`check-pwa.mjs`), wird in F.5f nachgezogen. **Die PWA profitiert ohne Änderung** — sie
parst JSON, und gültiges JSON war genau das, was fehlte.

### 3.7 C6u — Legacy-Overlay-Formular ohne blankes `atoi`

**Entscheidung (07.10.2026): ja, in F.** `type`, `date_code` und `days` werden geprüft und
mit derselben Abweisungsform wie die API (C18) abgewiesen. **Dieselbe Funktion wie N1**, das
in S schon einen Indexguard bekommt — deshalb erst in F, und der Review prüft, dass beide
Eingriffe zusammenpassen. **C19 als Ganzes bleibt offen** — `saveuphost` wird nicht
mitgezogen.

---

## 4. Abhängigkeiten und Parallelität (R3b)

### 4.1 Inhaltlich unabhängig heisst nicht baulich unabhängig

`release-zip` baut STM, ESP und `app-gz` aus dem **gemeinsamen** Arbeitsbaum. **Kein
Release-Build, solange irgendein Produktcode-Task offen ist.** Daraus die Kette:
**P2.6 → S.1**, **S.23 → P3.1**, **P3.4 → F.1**, **C4.3 → F.8**.

### 4.2 S und F

S fasst `http.cpp` nur für N1 an; F danach in seriellen Tasks. Nicht parallel: S.11
verspricht einen isolierten ESP-Stand.

### 4.3 `src/**`

S.14 bis S.19i nacheinander durch den `stm-developer`, Testbauten dazwischen. **C4.1 erst
nach S.23** — dann ist `src/**` für S abgeschlossen, und C4 darf neben der F-Entwicklung
laufen, weil F nur ESP-Dateien schreibt.

---

## 5. Testdurchlauf und M2

**Bis B19 umgesetzt ist:** M2 legt der Nutzer im eigenen Terminal an, in derselben Sitzung
wie das Einspielen. **Nach B19 (Entscheidung vom 07.10.2026):** `snapshot-device.sh` liest
das Passwort aus einer **Datei ausserhalb des Repos**, deren Pfad in `tools/device.conf`
steht (gitignored); der `pwa-tester` legt M2 in Phase 0 selbst an. **Was dabei gelten muss:**
Das Passwort erscheint in keiner Prozessliste (nicht als Argument, sondern über Datei oder
Umgebung an `openssl`), in keinem Mitschnitt und in keiner Logzeile; fehlt die Datei, bricht
das Skript **vor** dem ersten Abruf ab (L300 bleibt). Die alte Sicherung
`vor-durchlauf-3.2.15.tar.gz.enc` **bleibt**.

**Je Runde:** Einspielen (Nutzer), Gerätabnahme (Lead), `pwa-tester` Phasen 0–4 und 9.
**Unvollständig ist nicht abgenommen.** **P2 und P3** ohne eigenen Durchlauf, begründet
(§1.7, §7.5).

**S.26 trägt zusätzlich:** B35 am Zeitserver (Phase 2) und **B36** über einen zu langen
Update-Host, den der ESP mit `error=2` abweist — Phase 9 belegt den unveränderten
Gerätewert. **L303 und L306 nicht** (§1.10).

**Was ein Durchlauf nicht leisten kann:** STM-Guards (§1.6), Wettläufe (§1.7),
Fehlerantworten nicht scheiterbarer Aktionen (§1.10), schreibende WLAN-Aufrufe (§3.5) und
**Minusgrade** (§7.5). Dafür stehen Prüfstände und Proben.

---

## 6. Runde K — Kleinkram, verteilt auf die vorhandenen Flashes

### 6.1 Warum keine eigene Runde

Jeder Flash kostet eine Sitzung, ein M2, einen Durchlauf und beim ESP ein OTA-Risiko.
Kleinkram fährt mit. **Der Preis ist Zuordenbarkeit**; §6.2 hält K-Punkte von den Stellen
fern, deren Isolation die Spec zusagt.

### 6.2 Welcher Punkt mit welchem Flash fährt

| Träger | Dateien | Fährt mit | Tasks |
|---|---|---|---|
| **P2** | `app.js`, `sw.js`, `i18n/*.json`, Markup | nächster PWA-Upload | P2.1 |
| **S-ESP** | ESP-Quellen **mit Brückenbezug** | ESP-OTA von S | — (N1 als Ausnahme, S.5) |
| **S-STM** | `src/**` | STM-Flash von S, **gemessen** | S.19a–S.19i |
| **F** | ESP-Quellen **ohne Brückenbezug** | ESP-OTA von F | F.5a–F.5e, F.5i, F.5j |
| **W** | `tools/**`, `.claude/**`, `CLAUDE.md` | kein Flash | K.W |

**Warum ESP-Punkte ohne Brückenbezug mit F:** S.11 sagt einen isolierten Rückfallnachweis
zu. **N1 ist die eine Ausnahme** (kritisch, aus dem LAN). **STM-K-Punkte im Brückenpfad**
markiert S.21; **im Anzeigepfad** (A24, A45) bekommt S.25 einen Blick auf die Uhr.

**Wo es nicht passt:** schreibender Gerätezugriff zur Abnahme (R5); STM-Guards (Prüfstand);
nach S.18 entschiedene S-STM-Punkte (nächste STM-Runde); **C4** — jetzt eigener Schritt (§8).

### 6.3 Was ein K-Punkt ist — höchstens rund zehn Zeilen **und** keine Entwurfsfrage.

**A5 war als K-Punkt gemeldet und ist keiner mehr:** Mit „echte Minusgrade" ist er eine
Protokolländerung über drei Laufzeiten (§7). **A9** bleibt K: Nur der **gesendete** Wert
wird ungeklammert; die Automatik rechnet mit ihrem eigenen Wert weiter.

### 6.4 Die Abnahme je Punkt ist Pflicht

Vorher „erledigt, wenn …"; nachher Beleg dieses Commits oder Messwert; **geschlossen durch
Entscheidung** (C2, C9c4) mit Datum und Wortlaut der Entscheidung — sonst ist in einer Woche
nicht mehr zu unterscheiden, ob er bewusst stehen bleibt oder vergessen wurde.

### 6.5 Testdurchlauf und Versionen — im Durchlauf des Trägers; keine eigenen Versionen.

---

## 7. A5 — echte Minusgrade über die Brücke

### 7.1 Ausgangslage, am Code

| Stelle | Heute | Fundstelle |
|---|---|---|
| Messung | `buffer[0]` als `uint8_t`, beim DS3231 vorzeichenbehaftet; Ergebnis auf **0..250** halbe Grad begrenzt, 255 = Fehler | `src/rtc/rtc.c:352-384` |
| Brücke | `RTC_TEMP_INDEX_NUM_VAR` = **Index 21**, 16 Bit (`N<idx><lo><hi>`), gesendet bei Änderung und im Vollabgleich | `src/vars/vars.h:63`, `src/vars/vars.c:836`, `:1356-1358`, `src/main.c:4304-4320` |
| ESP | speichert den Wert ungeprüft in `numvars[21]`; prüft den **Index** gegen `MAX_NUM_VARIABLES` | `ESP8266/ESP-uclock/vars.cpp:1061-1072` |
| Legacy-Seite | `temp_index / 2` „,5" — **und rechnet bei einer Korrekturänderung den Index selbst nach**, begrenzt auf 0..255 | `http.cpp:3684-3747` |
| PWA | `formatHalfDegreeValue()`: `Math.floor (value / 2)`, `value % 2`, 255 = ungültig | `app.js`, Funktion `formatHalfDegreeValue()` |
| Anzeige an der Uhr | Wortanzeige nur für Index 20..79 (10–40 °C); Ziffernanzeige `"%02u"` | `src/display/display.c:6358-6425`, `:6437-6477` |

**Der Index 21 hat vier Leser** — Legacy-Seite, PWA, Wortanzeige, Ziffernanzeige —, und
jeder nimmt an, dass er vorzeichenlos ist und 255 „Fehler" heisst.

### 7.2 Entwurf: eine neue Variable am Ende, die alte bleibt

**Nicht** Index 21 umdeuten. Eine alte PWA auf neuer Firmware läse dann einen
Zweierkomplement-Wert als vorzeichenlos — −5 °C wären −10 halbe Grad = `0xFFF6`, und die PWA
zeigte „32763 °C" oder „ungültig". **Das wäre schlechter als heute**, und genau das schliesst
die Bedingung des Nutzers aus.

**Auch keinen obsoleten Index wiederverwenden** (`OBSOLETE_3_NUMVAR` usw.). **Das ist die
Lehre aus C32/L299:** Index 9 bedeutet auf beiden Seiten etwas anderes, und das kostet seit
Monaten. Ein wiederverwendeter Index trägt diese Gefahr, sobald eine Seite älter ist als die
andere.

**Stattdessen:**

- **Neue Variable `RTC_TEMP_HALF_DEG_NUM_VAR` am Ende** beider Enums, unmittelbar vor
  `MAX_NUM_VARIABLES` — **Index 49** (heute ist `UPTIME_SECONDS_HI_NUM_VAR` Index 48, in
  beiden Dateien gleich; ✔ abgezählt).
- **Format:** `int16` im Zweierkomplement, **halbe Grad**, über das bestehende `N`-Kommando
  mit 16 Bit. Bereich des DS3231: −40..+85 °C ⇒ −80..+170, weit innerhalb von `int16`.
- **„Kein Messwert": `0x8000`** (`INT16_MIN`). Nie ein gültiger Messwert, und nicht `0`,
  weil `0` eine echte Temperatur ist.
- **Der Index 21 bleibt bitgleich** wie heute: begrenzt auf 0..250, 255 bei Fehler. Er
  speist weiterhin Legacy-Seite, alte PWA und die Anzeige an der Uhr.

### 7.3 Wer was ändert

| Laufzeit | Änderung | Task | Datei |
|---|---|---|---|
| **ESP** | Enum um `RTC_TEMP_HALF_DEG_NUM_VAR` erweitern (Index 49); in `vars_init()` mit `0x8000` vorbelegen. **Sonst nichts** — der ESP reicht `numvars` an die API weiter | S.4b | `ESP8266/ESP-uclock/vars.h`, `vars.cpp` |
| **STM** | Vorzeichenbehaftet rechnen (`(int8_t) buffer[0]`); den Wert in halben Grad in einem neuen Feld halten; Index 21 **daraus** begrenzt ableiten; neue Variable senden — im Vollabgleich und **bei Änderung des vorzeichenbehafteten Werts** | S.18b | `src/rtc/rtc.c`, `rtc.h`, `src/vars/vars.c`, `vars.h`, `src/main.c` |
| **PWA** | Index 49 lesen, vorzeichenrichtig formatieren; **Rückfall auf Index 21**, wenn Index 49 fehlt oder `0x8000` trägt | P3.1 | `app.js` |

**Die Änderungserkennung ist der eine Fallstrick im STM-Teil.** Heute sendet `main.c:4315`
nur bei geändertem **Index**. Unter null ist der Index immer 0 — von −1 °C auf −8 °C änderte
sich nichts, und die neue Variable bliebe stehen. **Verglichen wird künftig der
vorzeichenbehaftete Wert**, und bei einer Änderung gehen **beide** Variablen hinaus.

**Was der ESP bewusst nicht tut:** Die Legacy-Seite und der Korrektur-Endpunkt rechnen den
Index bei einer Korrekturänderung selbst nach (`http.cpp:3686-3704`, `:9549`). Für die neue
Variable unterbleibt das: Der STM liest bei jeder Korrekturänderung sofort neu
(`main.c:1893`) und sendet beide Werte, die PWA zeigt also nach einem Bruchteil einer
Sekunde den richtigen. **Das hält A5 aus `http.cpp` heraus** und S damit bei der einen
Ausnahme N1.

### 7.4 Rückfalllagen — keine Oberfläche wird schlechter

| Lage | Index 21 | Index 49 | Alte PWA | Neue PWA (P3) | Legacy |
|---|---|---|---|---|---|
| alter ESP, alter STM (heute) | 0..250 | gibt es nicht | wie heute | Rückfall auf 21 | wie heute |
| **neuer ESP, alter STM** (S.10–S.24) | 0..250 | `0x8000` | wie heute | Rückfall auf 21 | wie heute |
| neuer ESP, neuer STM | 0..250, begrenzt | mit Vorzeichen | **wie heute** (liest 21) | **Minusgrade** | wie heute |
| alter ESP, neuer STM (darf nicht vorkommen, ESP zuerst) | 0..250 | vom ESP **verworfen** — `vars.cpp:1069` prüft den Index gegen `MAX_NUM_VARIABLES` | wie heute | Rückfall auf 21 | wie heute |

**Die vierte Zeile ist die wichtigste Sicherheit:** Selbst wenn die Reihenfolge einmal
verletzt würde, schreibt der alte ESP nicht über sein Array hinaus — ✔ am Code
(`vars.cpp:1069`). Und der Vollabgleich wird um ein Kommando länger (rund 197), was C26/A35
nicht berührt.

**Der Nachkommabit-Verdacht (S.7 (4)).** `(buffer[1] & 0x02) >> 1` liest Bit 1; beim DS3231
stehen die Nachkommabits nach Datenblatt in Bit 7 und 6 — ● ungeprüft. Bestätigt sich das,
hat die Uhr **nie** „,5 °C" gemeldet. **Die Korrektur änderte angezeigte Werte**, deshalb ist
sie ein **eigener Befund** und geht nur mit Freigabe des Nutzers in S.18b. Die neue Variable
wird so gebaut, dass sie das richtige Bit nimmt, **sobald** es geklärt ist — sie erbt den
Fehler nicht stillschweigend.

### 7.5 Abnahme — Minusgrade sind am Gerät nicht herstellbar

Die Uhr steht drinnen. **Tragend sind deshalb zwei Proben:**

- **STM-Prüfstand** (S.18b, abgelegt in S.20): `rtc_get_temperature_index()` mit
  Registerbytes für −10,0 °C, −0,5 °C, 0,0 °C, +24,5 °C, mit Korrektur ±20 und mit
  Lesefehler; geprüft werden **beide** Ausgänge — Index 21 bleibt bei allen negativen Werten
  0 —, und dass eine Änderung von −1 °C auf −8 °C einen Versand auslöst.
- **PWA-Probe** (P3.2): nachgebildete `numvars` mit −20, −1, 0, 49, `0x8000`, fehlend. Die
  heutige Formatierung rechnet mit `Math.floor` und `%` und rundet negative Werte falsch
  (−3 halbe Grad ⇒ „−2.5" statt „−1.5") — **die Probe muss genau das einmal fangen**.

**Am Gerät lesend:** zwischen S.10 und S.24 steht Index 49 auf `0x8000` (32768); danach ist
Index 49 bei Raumtemperatur = Index 21; die PWA zeigt vor und nach P3 dieselbe Temperatur.

### 7.6 Passt A5 in S? — Ja, unter drei Bedingungen

**Ja.** A5 ist inhaltlich dieselbe Art Änderung wie S — eine neue Zeile im Vollabgleich,
getragen vom Strom statt von einer Fähigkeit —, und S flasht ESP und STM ohnehin. Eine
eigene Runde nach F kostete zwei zusätzliche Flashes samt Sitzungen und Durchläufen.

**Die Bedingungen:**

1. **Der STM-Teil steht im Flash-Gate**, direkt nach dem Kern (S.18b), mit der Regel
   **anhalten und vorlegen**. A5 ist eine ausdrückliche Bestellung des Nutzers; ihn still
   zurückzustellen hiesse, für den Nutzer zu entscheiden. **Erwarteter Bedarf**, ● nicht
   gemessen: ein `int16`-Feld, eine Vorzeichenerweiterung, ein zweiter Vergleich, ein
   weiterer `var_send_num_variable()`-Aufruf und kein neuer Text — **eher Dutzende als
   Hunderte Byte**. Das entscheidet S.12 und der Testbau, nicht diese Schätzung.
2. **Der PWA-Teil fährt nicht mit S**, sondern als **P3** danach — Firmware vor PWA (L241).
   Er gehört nicht in P2 (das vor S gebaut wird) und nicht in F (eine reine ESP-Runde).
3. **Die Anzeige an der Uhr bleibt, wie sie ist** (Ent-8). Minusgrade auf der Wort- oder
   Ziffernanzeige bräuchten ein Minuszeichen im Lauftextzeichensatz und mehr Flash — damit
   würde A5 S sprengen.

**Würde A5 S sprengen, wenn Bedingung 3 fällt?** Wahrscheinlich ja — dann wäre es eine
Änderung am Anzeigepfad mit eigener Abnahme an der Uhr und unbekanntem Flashbedarf. **Dann
gehört der Anzeigeteil in eine eigene Runde nach F**; der Transport (dieser Entwurf) bliebe
in S.

**DS18xx** (A2) nutzt dieselbe Index-Logik und kann den Entwurf mit einer weiteren
angehängten Variable übernehmen. Nicht Teil dieses Pakets.

---

## 8. Schritt C4 — Umschrift ohne Flash

**Entscheidung (07.10.2026):** ASCII-Umschrift der acht ISO-8859-1-Dateien unter `src/**`
(L166), **eigener Schritt**, Nachweis über **gleiche `.hex`**, **nicht mit S**.

**Warum nicht mit S:** Der Nachweis „das Fabrikat ist dasselbe" gelingt nur, wenn **sonst
nichts** geändert ist. In S ändert sich das Fabrikat absichtlich.

**Warum ohne Flash:** Sind die `.hex` vor und nach C4 gleich, gibt es nichts einzuspielen und
nichts zu versionieren (DIR-004 verlangt eine Erhöhung nur für geänderte Komponenten). Ein
Flash wäre ein Risiko ohne Gegenstand.

**Wie:** byteweise ersetzen, Zeilenenden je Datei unverändert (DIR-015). F103 **und** F411
bauen, beide Fabrikate vergleichen. **Weicht eines ab, stand Nicht-ASCII doch ausserhalb
eines Kommentars** — dann Halt, zurück, Befund. Der Bestand aus C4 („62 Zeilen, alle in
Kommentaren") wird damit nicht angenommen, sondern geprüft.

**Wann:** nach S.23 — `src/**` ist dann für S abgeschlossen —, parallel zur F-Entwicklung,
**vor F.8**, weil der F-Release-Build auch den STM aus dem Baum baut. **Gewinn:** B21/L166
(`grep` überspringt diese Dateien) verliert seinen Grund.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Runde |
|---|---|---|---|
| `BEFUNDE.md` | je Runde nachführen; C2, C9c4 geschlossen; E3 offen | `doc-writer` | P, P2, S, P3, F |
| `app.js` | **P2:** Overlay-Text, B40, SSID-Auswahl, Massnahme 4, B17. **P3:** A5 | `pwa-developer` | P2, P3 |
| `tools/ui-mess/proben/` | Proben B40+SSID (P2.1c), L303/L306 (P2.1e), A5 (P3.2) | Lead | P2, P3 |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | Eröffnung/Abschluss, Zuordnung, Markenpflicht, Zähler | `esp-developer` | S |
| `ESP8266/ESP-uclock/eepromdata.cpp` | C31 | `esp-developer` | S |
| `ESP8266/ESP-uclock/vars.h`, `vars.cpp` | **A5:** Index 49, Vorgabe `0x8000` | `esp-developer` | S |
| `ESP8266/ESP-uclock/http.cpp` | **S:** N1. **F:** F.1–F.3, C27+N2, E12, C25, Overlay-Text, C38, C23, C6u | `esp-developer` | S, F |
| `ESP8266/ESP-uclock/httpclient.cpp` | C9k | `esp-developer` | F |
| `src/vars/vars.c`, `vars.h` | Eröffnung/Abschluss; A20; A14; **A5** (Index 49, Senden) | `stm-developer` | S |
| `src/rtc/rtc.c`, `rtc.h` | **A5** vorzeichenbehaftet | `stm-developer` | S |
| `src/main.c` | A39; **E17**; A3+A48; A49; A46-Anteil; **A5** (Änderungserkennung) | `stm-developer` | S |
| `src/esp8266/esp8266.c` | Zuordnung; A13+A47 | `stm-developer` | S |
| `src/tables/tables.c`, `src/display/display.c`, `src/tftled/`, `src/esp-spiffs/`, `src/tetris/`, `src/ldr/ldr.c` | A24, A45, A46, A16, **A9** | `stm-developer` | S |
| acht Dateien aus L166 | **C4** Umschrift | `stm-developer` | C4 |
| `CLAUDE.md`, `tools/**` | **E17** | Lead | S |
| `knowledge/quick-reference.md`, `knowledge/architecture-checklist.md` | **E17** | `doc-writer` | S |
| `tools/checks/` | Prüfstände AKS.5, AKS.14, **AKS.15**, C38; S13, S15, S7 | Lead | S, F |
| `tools/smoke-device.sh`, `tools/watch-log.sh`, `tools/check-pwa.mjs` | Stufe 65535, E11; C25- und C23-Format | Lead | F, K |
| `tools/snapshot-device.sh`, `TESTPLAN-PWA.md`, `tools/device.conf.example` | **B19** | Lead | K |
| die vier Bundle-Dateien aus L1 | **E2** löschen — **vorher Besitz nach R3 prüfen** | Lead bzw. Besitzer | K |
| `tools/hooks/file-ownership.py`, `tools/ui-mess/`, Skill `/release` | E20, E14-Rest, E24, B20 | Lead | K |
| Versionsdateien | je Einspielschritt | `release-engineer` | je Runde |

**Kodierung und Zeilenenden vor jedem Patch feststellen** (DIR-015). **`grep -a` in
`src/**`** (B21) — bis C4 erledigt ist.

---

## Prüfung gegen die Architektur-Checkliste

### Proper architecture

**PWA parallel zu Legacy?** Ja. F stärkt den Legacy-Flashpfad; N1 und C6u härten das
Legacy-Overlayformular; **A5 lässt die Legacy-Seite bitgleich** (sie liest Index 21 wie
heute). Legacy bleibt die Stabilitäts-Referenz.

**Wetter-Endpunkte?** Unberührt.

**Restore-Bedingung um `pending_weather_ticker_restore`?** **A39 und E17 berühren sie.** A39
verschiebt den Ort des Setzens, das Flag wandert mit; E17 benennt es danach um, **in einem
eigenen Diff ohne jede andere Änderung** (§2.1b). Keine der vier Teilbedingungen wird
vereinfacht; `CLAUDE.md`, Kurzreferenz und Checkliste werden **im selben Commit** wie der
Code nachgezogen, damit die Invariante nie einen Namen nennt, den es nicht gibt.

**Nur `.gz`?** Ja. C27 stärkt „Grösse > 0".

**Richtige Schicht?** B35, B40, SSID-Auswahl, Massnahme 4, L303/L306 in der Oberfläche; C38,
C23, C6u, N1 auf dem ESP; **A5 an der Quelle** — der STM rechnet mit Vorzeichen, weil nur er
das Register kennt; ESP reicht durch, PWA formatiert. **Eine Umrechnung im ESP oder in der
PWA aus dem begrenzten Index wäre die falsche Schicht** — die Information ist dort schon
verloren.

### Scalable systems

**STM-Kommandos?** Vollabgleich rund **197** (Eröffnung, Abschluss, A5). A5 sendet
zusätzlich nur bei Änderung, also selten. N1 und B35/R4 **verhindern** Kommandos.

**UART-Last?**

| Posten | Richtung | Kosten |
|---|---|---|
| Eröffnung + Abschluss | STM → ESP | rund 22 Byte je Vollabgleich |
| Zuordnungszeile | ESP → STM | **rund 1,5 kB je Vollabgleich** |
| A5 | STM → ESP | ein `N`-Kommando (rund 10 Byte) je Temperaturänderung |
| C31, A20 | — | weniger |
| A46, A49 | STM → ESP | gedrosselt |

**Unter 20 s?** Gefährlichster Pfad: die ausbleibende Quittung — daher eigene Zeile, `!v`,
`var …`. A5 nutzt das bestehende quittierte `N`-Kommando. A16 beseitigt zwei Pfade über 20 s.

**Polling?** Unverändert. **Hauptloop?** AKS.2.

**Hartkodierte Grenzen?** `TEXT_FIELD_LIMITS` einmal; STM-Guards aus `sizeof`; A5 hängt am
Enum-Ende statt an einer festen Zahl, und **beide Enums müssen gleich lang bleiben** — S.21
prüft das. **Härteste Grenze: F103-Flash** (§2.7).

### Secure by design

**`innerHTML`?** B33; **P2.1f** baut die SSID-Liste neu — SSID-Namen sind Fremddaten aus der
Luft und gehen nie ungeschützt ins DOM.

**Fremddaten?** **N1** ist der schärfste Fall. **C23** escapet Logzeilen nach JSON — eine
STM-Zeile mit `"` oder Steuerzeichen bräche heute das JSON, und das Escapen schliesst
zugleich eine Injektion in die Antwort.

**Credentials?** **C31, A20 — und B19:** Das Passwort für M2 liegt in einer Datei ausserhalb
des Repos und darf in keiner Prozessliste, keinem Mitschnitt und keiner Logzeile erscheinen
(§5). **Dass ein Agent M2 anlegen darf, ändert, wer das Passwort benutzt, nicht wer es
kennt** — der Agent liest es nie.

**Konfiguration schreibende Endpunkte?** Keine neuen. E12, C38, C6u, F.2, F.3 ändern
Antworten bzw. Abweisungen.

### Stable & reliable

**Fehler ausgewertet?** P: B36, R1–R4, L303/L306. Brücke: A35. F: E12, Overlay-Text, C38,
C6u, Ent-4. **A5:** „kein Messwert" ist `0x8000`, nicht `0` — ein fehlender Wert darf nie
als 0 °C erscheinen.

**Leere `catch`?** R3 beseitigt einen; zwei bleiben bewusst (`localStorage`).

**Stille Verwerfungen?** C26-Zähler, gezählte unpassende Quittung, A46, A49, C25. Der alte
ESP verwirft Index 49 über die Bereichsprüfung — beabsichtigt und ohne Folgen (§7.4).

**Still zurechtgebogen?** **A5 beseitigt genau das:** Unter 0 °C meldete die Uhr 0 °C. B40
und die SSID-Auswahl tauschten Eingaben still gegen alte Werte; Overlay-Text und C38 kürzten
still. Massnahme 4 ist die Gegenrichtung — eine Warnung ohne Grund.

**Zustand nach Abbruch?** Import, Layout-Restore, L306 (Knopf zurück auf seinen Zustand),
Vollabgleich, Zuordnung, N1, C38, zurückgestellte K-Punkte — wie in den Abschnitten oben.
**C4:** Weicht ein `.hex` ab, wird die Umschrift ganz zurückgenommen.

**Flags aufgelöst?** IP-Ticker-Vormerker, Zuordnung, Markenerwartung im selben Durchlauf;
`prefillDeviceValue()`-Zustand bis Speichern oder Neuladen; `hasUnsavedEdits` fällt auch bei
wiederhergestelltem Ausgangswert; Busy-Zustand fällt im Fehlerpfad; A49-Maske bewusst nie.

---

## Verworfene Alternativen

**A5: Index 21 umdeuten.** Eine alte PWA zeigte Unsinn bei Minusgraden — schlechter als
heute (§7.2).

**A5: einen obsoleten Index wiederverwenden.** Die Gattung aus C32/L299 (§7.2).

**A5: Vorzeichen als eigenes Flag neben Index 21.** Zwei Variablen, die zusammen gelesen
werden müssen, kommen über die Brücke nicht atomar — ein Leser sähe den neuen Betrag mit dem
alten Vorzeichen. Ein `int16` ist eine Zeile.

**A5: Offset-Kodierung (z. B. +128).** Ebenfalls lesbar, aber jeder Leser müsste den Offset
kennen; Zweierkomplement in 16 Bit ist das, was der Wert ist.

**A5: ESP rechnet die neue Variable bei Korrekturänderung nach.** Zöge `http.cpp` in S; der
STM liefert den richtigen Wert ohnehin sofort (§7.3).

**A5 mit P2 ausliefern.** P2 wird vor S gebaut; die PWA läse eine Variable, die es noch
nicht gibt. Der Rückfall wäre harmlos, aber die Probe gegen echte Firmware fehlte.

**A5 als eigene Runde nach F.** Zwei Flashes mehr für eine Zeile im Vollabgleich (§7.6) —
**es sei denn, die Anzeige an der Uhr soll mit** (Ent-8).

**E17 im selben Diff wie A39.** Unlesbar; daher S.15b als reiner Umbenennungsdiff.

**C4 mit S.** Der `.hex`-Gleichheitsnachweis gelingt nur ohne andere Änderung (§8).

**C4 mit Flash.** Ein Flash ohne geändertes Fabrikat ist Risiko ohne Gegenstand.

**B19: Passwort als Kommandozeilenargument oder Umgebung im Repo.** Prozessliste bzw.
Repo; beides schliesst die Entscheidung aus.

**P2 mit eigenem Durchlauf; L303/L306 in S.26; C38 am Gerät; Flash-Gate nur als Schätzung
oder nur am Ende; K als eigene Runde; F parallel zu S** — wie in der vorigen Fassung
begründet.

Aus der alten Spec weiter gültig: eigenes Top-Level-Präfix, Zuordnung an den Punkt (`.c3`),
Fähigkeitsmeldung für die Marke, Markenpflicht für alle Kommandoarten, Schweigen statt `!v`,
Folgenummer, A35 Teil 2, A39 Variante (a).

---

## Versionsfolgen

Je Einspielschritt, **nur die geänderte Komponente** (DIR-004). K-Punkte fahren unter der
Version ihres Trägers; Testbauten und C4 heben keine Version an.

| Schritt | STM `src/main.h` | ESP `version.h` | `APP_VERSION` | `CACHE_NAME` |
|---|---|---|---|---|
| P — **ausgeliefert** (1.4.91) | — | — | ☑ | ☑ |
| P2 | — | — | ☐ | ☐ |
| S, ESP-Teil (N1, A5-ESP) | — | ☐ | — | — |
| S, STM-Teil (E17, A5, gemessene K-Punkte) | ☐ | — | — | — |
| P3 (A5-PWA) | — | — | ☐ | ☐ |
| C4 | — | — | — | — |
| F | — | ☐ | — | — |

`APP_VERSION` und `CACHE_NAME` **immer zusammen**. Nur der `release-engineer` (R4). Der Tag
`release/<stm>-<esp>-<app>` wird **je Einspielschritt** gesetzt und gepusht (DIR-011).
