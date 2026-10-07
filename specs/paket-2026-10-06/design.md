# Design — Paket 2026-10-06 (Runden P, P2, S, P3, F, C4, verteilte Runde K)

**Erstellt:** 2026-10-06, Stand `401aae2`. **Nachgeführt** am 06.10.2026 nach der
Auslieferung von P und dem Gerätetest (L324), am **07.10.2026** in zwei Durchgängen mit den
Entscheidungen des Nutzers. **Freigegeben am 07.10.2026** (Ent-1). Momentaufnahme (DIR-006).

**Rundenfolge:** **P (ausgeliefert) → P2 → S → P3 → F**, dazu **C4** ohne Flash nach S.
Runde K fährt mit (§6). Kennungen: „N1"/„N2" ohne Zusatz sind Befunde der Nachzählung,
„P2-Review N1/N2" die zwei Befunde aus dem Review von P2; **B40/L321**, **C38/L323**, **L324**
aus dem Gerätetest von 1.4.91; „L-Befund Nachkommabit" (Nummer vergibt der Lead); „Ent-n"
sind Entscheidungen (`requirements.md`). **`vars.h` ohne Pfad meint
`ESP8266/ESP-uclock/vars.h`.**

---

## 0. Was sich gegenüber `specs/paket-2026-10-05/` geändert hat

| Punkt | Alte Spec | Diese Spec | Grund |
|---|---|---|---|
| Runden V, W, E1, E2, H | geplant | **weg** | ausgeliefert bzw. E1 entfallen |
| Runde U | eigene Runde | **aufgelöst**: B27, B32 nach P; B28, B31 warten | Nutzer |
| Reihenfolge | … → S → P → U → F | **P → P2 → S → P3 → F**, C4 ohne Flash | Nutzer; P2 aus dem Rest von P; P3 für A5 |
| Runde K | — | **neu**, ohne eigenen Flash | Nutzer, Nachzählung |
| Runde P | B33, B24 | **ausgeliefert als 1.4.91** | — |
| Runde P2 | — | Overlay-Text, **B40**, **SSID-Auswahl**, **Massnahme 4 (nur vier Netzwerkfelder)**, **P2-Review N1/N2**, Probe L303/L306 | L324, Review |
| Runde S | A39, L260, A35 T1, C26, A16 | dasselbe **plus C31, A46, N1, E17, A5 (mit Legacy) samt Nachkommabit** und neun STM-K-Punkte; **zweistufiges Flash-Gate**, Reserve 1'024 Byte; **B17 Spur 3** als Beobachtung | L298, L291, Nachzählung, L296, Entscheidungen vom 07.10. |
| Runde P3 | — | **A5 in der PWA** | Firmware vor PWA (L241) |
| Runde F | C9c6, 65535-Liste, Smoketest | dasselbe **plus C28, C38, Ent-4, Ent-5, C23, C6u** und K-Punkte | §3, §6 |
| Schritt C4 | — | Umschrift ASCII, **`.hex`-Gleichheit, kein Flash** | Nutzer, §8 |
| Testdurchlauf | eine Abnahme je Runde | **je Runde ein eigener Task**; P2 und P3 begründet ohne; **M2 durch den Agenten, sobald die Passwortdatei aus B19 liegt** | L302, §5 |
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

Die Prüfung sitzt **an der Endpunktgrenze**, nicht am Eingabefeld (L179 in der PWA).
`index.html` bleibt unverändert.

### 1.2 R1 und R2 — Kennung 6 aus `fs_remove`

Ausgeliefert: Kennung 6 erst nach erneutem Listenabruf deuten. **Ent-4 = ja:** F.2 gibt
„remove failed" eine eigene Kennung; bis zu einer Übersetzung zeigt `describeApiError()` den
Rückfalltext mit Detail.

### 1.3 bis 1.5 — B36, B34, R3, R4, B33, B27, B26, B32

Ausgeliefert wie entworfen. **B17** ist nicht in P und **nicht in P2** — siehe §1.11.

### 1.6 E31 ist erledigt — mit einem Vorbehalt

STM-Indexguards sind über HTTP nicht zeigbar; Abnahme über den Prüfstand (AKK.6).

### 1.7 P2: warum vor S, und warum ohne eigenen Testdurchlauf

**P2 vor S**, weil jeder S-Build `app-gz` mitbaut (§4.1). **Kein eigener
`pwa-tester`-Durchlauf:** B40 und die SSID-Auswahl sind **Wettläufe** — ein Gerätelauf
träfe sie zufällig und meldete bei Verfehlen grün; die **Probe P2.1c** stellt sie in der
Vorschau gezielt her. L303 und L306 brauchen eine nachgebildete Fehlerantwort (**P2.1e**).
Massnahme 4 ist in der Vorschau prüfbar. Am Gerät lesend ohne Speichern (P2.8); **S.26**
prüft B35 am Zeitserver mit Speichern. **Offen bleibt ausdrücklich:** Zwischen P2-Upload und
S.26 sind B40 und die SSID-Auswahl am Gerät nur lesend belegt; L303/L306 nie mit echtem
Fehlschlag.

### 1.8 B40 / L321 und die SSID-Auswahl — eine Regel, drei Stellen

Zeitserver und Zeitzone (B40) und die SSID-Auswahl (P2.1f) werden nach jeder
`network_scan`-Antwort neu gesetzt bzw. neu aufgebaut. **Lösung für alle drei:** **dieselbe**
`prefillDeviceValue()`-Regel wie bei der AP-SSID (L34) — ein Gerätewert setzt nur, was weder
fokussiert noch vom Nutzer geändert ist; bei der Auswahlliste wird eine abweichende,
ungespeicherte Auswahl nach dem Neuaufbau wiederhergestellt. **Keine zweite und keine dritte
Fassung** (L179, L289). SSID-Namen gehen nie roh per `innerHTML` ins DOM.

### 1.9 Massnahme 4 in engerer Form — nur die vier vorbefüllten Netzwerkfelder

**Befund (L324):** `handleDirtyFormInteraction()` setzt `hasUnsavedEdits` bei jeder Eingabe.
**Entscheidung des Leads (07.10.2026): Die Vergleichsregel gilt nur für die vier
vorbefüllten Netzwerkfelder.** Dort ist geändert, was **vom Ausgangswert abweicht**, mit
**demselben** Ausgangswert, den `prefillDeviceValue()` aus §1.8 kennt. **An allen anderen
Feldern bleibt das Verhalten, wie es ist** — eine Eingabe gilt dort weiter als Änderung.

**Warum so eng:** Genau an diesen vier Feldern gibt es einen Gerätewert, den die Oberfläche
vorbefüllt und gegen den sich „unverändert" sauber entscheiden lässt. Bei allen anderen
Feldern müsste der Ausgangswert erst eingeführt werden — das wäre ein Umbau der
Dirty-Erkennung, nicht ihre Korrektur.

**Was daraus für die Dokumentation folgt:** `BEFUNDE.md` führt Massnahme 4 als
**eingeschränkt** umgesetzt (P2.9). Stünde sie als „erledigt", glaubte der nächste Leser,
der Dialog sei überall genau — und das ist er nicht.

**Die zwei Befunde aus dem P2-Review (P2.1h)** betreffen genau diese Regel: **P2-Review N1**
(Ersatz-Ausgangswert `networks[0]`) und **P2-Review N2** (`unsavedEditFields` nach dem
Vorbefüllen). Sie werden vor dem Review P2.4 behoben; die Probe P2.1c muss danach erneut grün
laufen.

### 1.10 L303 und L306 — Nachweis über eine nachgebildete Fehlerantwort

Hinter `runConfirmedButtonAction` stehen `downloadUpdateAssets`, `resetStm32`,
`formatLittleFsFromFiles` — nichts davon lässt man an einer produktiven Uhr scheitern; ein
Flag-Umschalten (L306) scheitert nicht über einen zu langen Text. **Probe P2.1e**, einmal
gegen einen künstlich zurückgebauten Stand fehlgeschlagen (Scratchpad, L268). Beide Befunde
betreffen die **Darstellung** einer Fehlerantwort, nicht ihr Zustandekommen.

### 1.11 B17 — aus P2 heraus, Spur 3 wird in S beobachtet

**Entscheidung des Leads (07.10.2026):** Die Spuren 1 und 2 aus L142 — verschluckter Fehler,
hängendes `stm32LogRefreshInFlight` — sind im Code abgedeckt. **P2.2 und P2.3 entfallen.**
Offen ist **Spur 3:** ein leerer Logring nach einem ESP-Neustart, den das Logfenster nicht
als solchen erkennbar macht. **Der ESP-OTA von S ist genau dieser Auslöser** — deshalb wird
Spur 3 in **S.11** lesend beobachtet: PWA unmittelbar nach dem Neustart laden, Modul
`system`, Logfenster ohne Reload, mit Zeitstempeln. Ergebnis an S.27: B17 geschlossen oder
mit Beleg offen. **Kein zusätzlicher Gerätezugriff** — der Neustart passiert ohnehin.

---

## 2. Runde S — die Brücke fertigbauen

### 2.0 Zwei Richtungen

`CAP var-crc` und `FIRMWARE` laufen **ESP → STM**; die Neuerungen dieser Runde **STM → ESP**,
ohne Fähigkeitskanal. **Die Information reist im Strom mit** — auch bei A5: Die neue
Variable trägt ihren eigenen „unbekannt"-Wert (§7).

### 2.1 A39 / L259 — kein verschachtelter 194er-Stoss mehr

✔ `src/main.c:3217`. **Variante (b):** vormerken, im Hauptloop senden, **danach** den
IP-Lauftext setzen; **`pending_weather_ticker_restore` wandert mit**. A39 schützt zugleich
den Empfangsring für §2.4.

### 2.1b E17 — die Umbenennung, als eigener Diff nach A39

**Entscheidung des Nutzers: mit S.** So eingeordnet, dass der Diff von A39 lesbar bleibt:
**S.15** bleibt beim alten Namen; **S.15b** ist die Umbenennung allein
(`pending_weather_ticker_restore` → `pending_ticker_restore`, Vorschlag), geprüft mit
`git diff --word-diff`; **S.15c/S.15d** ziehen `CLAUDE.md`, `tools/**`,
`knowledge/quick-reference.md` und `knowledge/architecture-checklist.md` nach, **commitet mit
S.23**. Kein Flash — der Testbau S.18 dient als Gegenprobe.

### 2.2 L260 / A42 — ein Erfolgskriterium, das den ganzen Satz prüft

Eröffnungszeile und Abschlussmarke im Strom; ohne Eröffnungszeile gilt das heutige
Kriterium. **Prämisse in drei Punkten** (L269): `var_cmd_min_len()` liefert für einen
unbekannten Buchstaben `0` (von E2.1 erhalten), `var_set_parameter()` hat keinen
`default:`-Zweig, die Zeigerarithmetik steht nur in den `case`-Rümpfen. **Auflagen:**
Buchstabe ausserhalb `N n S T D A C M O t a l I`; Nutzlast nicht auf `*` + vier Hexziffern;
unter 72 Zeichen; **Verpackung `var <Buchstabe>…`**:

> **Ein eigenes Top-Level-Präfix wäre gefährlich.** Die ESP-Kette hat **kein
> abschliessendes `else`**, `var_send_buf()` liefe je Zeile 3 Sekunden leer, **ohne
> `watchdog_reload()`**. **Das ist exakt die Mechanik aus L266.**

### 2.3 C26, 2.4 A35, 2.5 die beiden Warnungen, 2.6 A16

Wie in der vorigen Fassung: Zähler über den Logring und Markenpflicht mit `!v`; `ACK <xy>` als
eigene Zeile vor jeder Quittung; *„Ein Mechanismus, dessen Sicherheit an einem
Hardware-Nebeneffekt hängt, ist nicht abgesichert"* und *„Die `var `-Verpackung ist nicht
stumm, sondern regulär quittiert"*; A16 mit Spiellauf des Nutzers (Ent-6).

### 2.7 Das Flash-Gate, zweistufig (Ent-2, Ent-7)

**Lage:** 1'924 Byte frei, **Reserve 1'024 Byte** — für S also **rund 900 Byte**. Darin: der
Kern, **A5 samt Nachkommabit**, neun STM-K-Punkte.

**Stufe 1 (S.12):** Vorabschätzung — passt **schon der Kern**? **Stufe 2:** Testbauten nach
dem Kern und nach jedem weiteren Teil; nur F103, ohne ZIP, durch den Lead, seriell.
**Ent-7:** R1 soll paralleles Bauen verhindern; das bleibt gewahrt.

| # | Teil | Bei Unterschreitung der Reserve |
|---|---|---|
| S.18b | **A5** (mit Nachkommabit, falls bestätigt) | **anhalten und vorlegen** — Festlegung dieser Spec (§7.6) |
| S.19a | A20 | anhalten und vorlegen |
| S.19b | A13+A47 | anhalten und vorlegen |
| S.19c | A3+A48 | anhalten und vorlegen |
| S.19d–S.19h | A24, A45, A14, A46, A49 | zurückstellen, weiter |
| S.19i | A9 | zurückstellen |

**Gegenprobe S.23:** S8b neben Schätzung und Testbauten.

### 2.8 bis 2.10 — C31, A46/A47, C32

C31 „gesetzt"/„leer"; A46 schliesst AKH.4 für die Datenpfade; A47 mit A13 (L96); C32 nicht
in S — **die Lehre aus C32 trägt A5** (§7.2).

### 2.11 Einspielen

**Erst ESP (S.10), dann STM (S.24);** dazwischen der Rückfallnachweis S.11 — und dort auch die
Beobachtung zu B17 Spur 3 (§1.11).

### 2.12 N1 — der Legacy-Overlayindex, und warum er mit S fährt

`http_overlays()` (`http.cpp:4689-4717`): `oidx` aus `atoi`, nur gegen `0xff` geprüft,
Schreibzugriff über 32 Plätze, auch per `<img>`. **Fix:** `oidx < MAX_OVERLAYS && oidx <=
n_overlays` vor jedem Schreibzugriff und vor `n_overlays++`. **Mit S**, weil S-ESP der
nächste ESP-Flash ist. STM-Seite über A3+A48, abgenommen über den Prüfstand. **C6u** (F.5j)
fasst später dieselbe Funktion an.

---

## 3. Runde F — Flash-Überwachung

**3.1** Legacy-Flashzweig, Legacy-Liste und `fname + 6` über
`http_remote_stm32_filename_matches()` (F.1); **Ent-5 = ja:** auch die API-Liste (F.3), bei
65535 leer. **3.2** Smoketest-Stufe 65535; E11 schreibt dieselbe Datei. **3.3** F zuletzt —
ein STM-Flash mit vorhersagbarem Ergebnis ist die ehrlichste Probe; der Legacy-Pfad wird
nicht scharf gefahren. **3.4** Nur ESP, danach der STM-Flash als Probe.

**3.5 C38** — abweisen statt kürzen; Abnahme nur am Quelltext und am Prüfstand.

**3.6 C23** — `/api/stm32_log` in UTF-8 mit JSON-Escapen, zeichenweise in den Ausgabestrom
(L175). Leser in F.5f nachziehen; die PWA profitiert ohne Änderung.

**3.7 C6u** — `type`, `date_code`, `days` im Legacy-Overlayformular geprüft, Abweisungsform
wie C18; dieselbe Funktion wie N1, deshalb in F; C19 als Ganzes bleibt offen.

---

## 4. Abhängigkeiten und Parallelität (R3b)

### 4.1 Inhaltlich unabhängig heisst nicht baulich unabhängig

`release-zip` baut STM, ESP und `app-gz` aus dem gemeinsamen Arbeitsbaum. **Kein
Release-Build, solange irgendein Produktcode-Task offen ist.** Kette: **P2.6 → S.1**,
**S.23 → P3.1**, **P3.4 → F.1**, **C4.3 → F.8**.

### 4.2 S und F

**S fasst `http.cpp` an zwei Stellen an:** N1 (`http_overlays()`) und **der Legacy-Teil von
A5** (RTC-Anzeige und das Nachrechnen bei einer Korrekturänderung, §7.3). Beides in einer
Funktion bzw. einem Abschnitt, den F nicht berührt; F folgt danach seriell. **Nicht
parallel:** S.11 verspricht einen isolierten ESP-Stand.

### 4.3 `src/**`

S.14 bis S.19i nacheinander, Testbauten dazwischen. **C4.1 erst nach S.23**, dann parallel
zur F-Entwicklung.

---

## 5. Testdurchlauf und M2

**B19 ist umgesetzt** (`e121058`, Meldung des Leads): `snapshot-device.sh` liest das
Passwort aus `SNAPSHOT_PASS`, dann aus `~/.config/wordclock/snapshot.pass`
(`SNAPSHOT_PASS_FILE`), dann vom Terminal; es bricht ab bei einer Datei im Repo, bei einem
Modus ungleich `x00` und bei einer leeren Datei; das Passwort läuft über `env:`/`file:` an
`openssl`, nicht über die Prozessliste. **Die Datei legt der Nutzer an.** Bis dahin bleibt
M2 ein Nutzerschritt in derselben Sitzung wie das Einspielen; danach fährt der
`pwa-tester` Phase 0 selbst. Die alte Sicherung bleibt.

**Je Runde:** Einspielen (Nutzer), Gerätabnahme (Lead), `pwa-tester` Phasen 0–4 und 9.
**Unvollständig ist nicht abgenommen.** **P2 und P3** ohne eigenen Durchlauf (§1.7, §7.5).

**S.26 trägt zusätzlich:** B35 am Zeitserver (Phase 2) und **B36** über einen zu langen
Update-Host mit `error=2` — Phase 9 belegt den unveränderten Gerätewert. **L303 und L306
nicht** (§1.10).

**Was ein Durchlauf nicht leisten kann:** STM-Guards, Wettläufe, Fehlerantworten nicht
scheiterbarer Aktionen, schreibende WLAN-Aufrufe, **Minusgrade**. Dafür stehen Prüfstände
und Proben.

---

## 6. Runde K — Kleinkram, verteilt auf die vorhandenen Flashes

**6.1** Kleinkram fährt mit; der Preis ist Zuordenbarkeit. **6.2 Träger:** P2 (PWA), S-ESP
(ESP mit Brückenbezug; **N1** als Ausnahme), S-STM (gemessen), F (ESP ohne Brückenbezug), W
(Werkzeug). STM-K-Punkte im Brückenpfad markiert S.21, im Anzeigepfad (A24, A45) ein Blick
auf die Uhr in S.25. **6.3** K heisst höchstens rund zehn Zeilen und keine Entwurfsfrage;
**A5** ist deshalb kein K-Punkt mehr. **6.4** Abnahme je Punkt Pflicht; **geschlossen durch
Entscheidung** (C2, C9c4) mit Datum; **erledigt gemeldete Werkzeugpunkte** mit Commit — B19
`e121058`, **E2 `8dfba36`** (die Dateien lagen unter `ESP8266/ESP-uclock/` — `APP-BUNDLE.md`,
`data/app-bundle.txt`, `tools/*.py|sh` —, nicht unter `data/app/`). **6.5** Im Durchlauf des
Trägers; keine eigenen Versionen.

---

## 7. A5 — echte Minusgrade über die Brücke

### 7.1 Ausgangslage, am Code

| Stelle | Heute | Fundstelle |
|---|---|---|
| Messung | `buffer[0]` als `uint8_t`, beim DS3231 vorzeichenbehaftet; Ergebnis auf **0..250** halbe Grad begrenzt, 255 = Fehler; **halbes Grad aus Bit 1** von Register `0x12` (● vermutlich falsch, §7.4) | `src/rtc/rtc.c:352-384` |
| Brücke | `RTC_TEMP_INDEX_NUM_VAR` = **Index 21**, 16 Bit, bei Änderung und im Vollabgleich | `src/vars/vars.h:63`, `src/vars/vars.c:836`, `:1356-1358`, `src/main.c:4304-4320` |
| ESP | speichert den Wert in `numvars[21]`; prüft den **Index** gegen `MAX_NUM_VARIABLES` | `ESP8266/ESP-uclock/vars.cpp:1061-1072` |
| Legacy-Seite | `temp_index / 2` „,5"; **rechnet bei einer Korrekturänderung den Index selbst nach** | `http.cpp:3684-3747`, Korrektur-Endpunkt `:9549` ff. |
| PWA | `formatHalfDegreeValue()`: `Math.floor (value / 2)`, `value % 2`, 255 = ungültig | `app.js` |
| Anzeige an der Uhr | Wortanzeige nur für Index 20..79 (10–40 °C); Ziffern `"%02u"` | `src/display/display.c:6358-6477` |

### 7.2 Entwurf: eine neue Variable am Ende, die alte bleibt

**Nicht** Index 21 umdeuten — eine alte PWA zeigte dann Unsinn. **Keinen obsoleten Index
wiederverwenden** — die Lehre aus C32/L299. **Stattdessen:**

- **Neue Variable `RTC_TEMP_HALF_DEG_NUM_VAR` am Ende** beider Enums — **Index 49** (heute ist
  `UPTIME_SECONDS_HI_NUM_VAR` Index 48; ✔ in beiden Dateien abgezählt).
- **Format:** `int16` im Zweierkomplement, halbe Grad, über das bestehende `N`-Kommando.
- **„Kein Messwert": `0x8000`.** Nie gültig, und nicht `0`, weil `0` eine echte Temperatur ist.
- **Index 21 bleibt begrenzt auf 0..250** und speist weiter die alte PWA und die Anzeige an
  der Uhr. **Ent-8:** Die Uhr zeigt bei Minusgraden 0 °C, wie heute.

### 7.3 Wer was ändert

| Laufzeit | Änderung | Task | Datei |
|---|---|---|---|
| **ESP** | Enum um Index 49 erweitern, mit `0x8000` vorbelegen. **Legacy-Seite zeigt Minusgrade** (Ent-8): Die RTC-Anzeige liest Index 49 vorzeichenrichtig, Rückfall auf Index 21 bei `0x8000`. **An beiden Stellen, die den Index bei einer Korrekturänderung selbst nachrechnen** (Legacy `savetcorrrtc` und der Korrektur-Endpunkt), wird Index 49 **im selben Schritt und über dieselbe Hilfsfunktion** nachgerechnet — sonst zeigten Legacy und PWA nach einer Korrektur kurz verschiedene Werte | S.4b | `ESP8266/ESP-uclock/vars.h`, `vars.cpp`, `http.cpp` |
| **STM** | Vorzeichenbehaftet rechnen (`(int8_t) buffer[0]`); **halbes Grad aus Bit 7**, falls S.7 es bestätigt (§7.4); Wert in halben Grad in einem neuen Feld; Index 21 daraus begrenzt ableiten; neue Variable senden — im Vollabgleich und **bei Änderung des vorzeichenbehafteten Werts** | S.18b | `src/rtc/rtc.c`, `rtc.h`, `src/vars/vars.c`, `vars.h`, `src/main.c` |
| **PWA** | Index 49 lesen, vorzeichenrichtig formatieren, **gleich wie die Legacy-Seite**; Rückfall auf Index 21, wenn Index 49 fehlt oder `0x8000` trägt | P3.1 | `app.js` |

**Der Fallstrick im STM-Teil:** Heute sendet `main.c` um Zeile 4315 nur bei geändertem
**Index**. Unter null ist der Index immer 0 — von −1 °C auf −8 °C änderte sich nichts.
**Verglichen wird künftig der vorzeichenbehaftete Wert.**

**Gegenüber der vorigen Fassung geändert:** Dort blieb A5 aus `http.cpp` heraus, und die
Legacy-Seite zeigte weiter Index 21. **Ent-8 verlangt Minusgrade auch in Legacy** — damit
gehört die Legacy-Anzeige in den ESP-Teil, und mit ihr das Nachrechnen bei einer Korrektur.
Wer die Legacy-Anzeige umstellt und das Nachrechnen nicht, erzeugt zwei Wahrheiten.

### 7.4 Rückfalllagen — und das Nachkommabit

| Lage | Index 21 | Index 49 | Alte PWA | Neue PWA (P3) | Legacy |
|---|---|---|---|---|---|
| alter ESP, alter STM (heute) | 0..250 | gibt es nicht | wie heute | Rückfall auf 21 | wie heute |
| **neuer ESP, alter STM** (S.10–S.24) | 0..250 | `0x8000` | wie heute | Rückfall auf 21 | **Rückfall auf 21**, wie heute |
| neuer ESP, neuer STM | 0..250, begrenzt | mit Vorzeichen | wie heute (liest 21) | **Minusgrade** | **Minusgrade** |
| alter ESP, neuer STM (darf nicht vorkommen) | 0..250 | vom ESP **verworfen** (`vars.cpp:1069`) | wie heute | Rückfall | wie heute |

**L-Befund Nachkommabit.** `src/rtc/rtc.c:364` liest das halbe Grad als
`(buffer[1] & 0x02) >> 1` — Bit 1. Beim DS3231 trägt Register `0x12` die Nachkommastelle
nach Datenblatt in **Bit 7 (0,5 °C) und Bit 6 (0,25 °C)**; Bit 1 ist dort immer 0. Stimmt
das, war der Index **nie ungerade**. **Entscheidung des Nutzers (07.10.2026): Bestätigt S.7
den Fehler, wird er in S.18b mit A5 korrigiert.** Die Korrektur ändert **beide** Ausgänge —
Index 21 und Index 49 zeigen danach auch halbe Grad. **Das ist die einzige gewollte Änderung,
die auch eine alte PWA sieht** (AKP3.2): Sie zeigt dann „,5", wo vorher nie eines stand —
richtiger, nicht schlechter. Das 0,25-°C-Bit (Bit 6) bleibt unberücksichtigt; der Index hat
halbe Grad als Auflösung.

**Abnahme des Nachkommabits** (AKS.17): **Prüfstand mit Registerbytes**, der den Fall Bit 7
ausdrücklich enthält und **gegen das alte `rtc.c` fehlschlägt**: +24,0 °C mit Bit 7 ⇒
Index 49; mit Bit 6 allein ⇒ Index 48; −0,5 °C ⇒ −1. Am Gerät **nur als Beobachtung** — ein
ungerader Index tritt auf, wenn die Raumtemperatur ihn hergibt; das ist kein Nachweis, weil
es von der Temperatur abhängt.

### 7.5 Abnahme — Minusgrade sind am Gerät nicht herstellbar

**Tragend sind Proben:** STM-Prüfstand (S.18b), Legacy-Formatierung (S.4b) und PWA-Probe
(P3.2) — jede mit −20, −1, 0, +49 halben Grad, `0x8000` und (PWA) fehlendem Index. Die
heutige PWA-Formatierung rundet negative Werte falsch (−3 ⇒ „−2.5" statt „−1.5"), **die Probe
muss genau das einmal fangen**. **Am Gerät lesend:** S.11 (Index 49 = `0x8000`, Legacy und PWA
unverändert), S.25 (Index 49 = Index 21, Legacy = PWA), P3.6.

### 7.6 A5 in S — bestätigt

**Ent-8 hält A5 in S:** Die Uhranzeige bleibt unverändert, also gibt es keinen Eingriff in
den Anzeigepfad und kein Minuszeichen im Zeichensatz. **Was dazukommt** — Legacy-Anzeige und
Nachrechnen auf dem ESP, das Nachkommabit auf dem STM — sind wenige Zeilen in Dateien, die S
ohnehin anfasst.

**Flash:** A5 steht als S.18b direkt nach dem Kern, mit der Regel **anhalten und vorlegen** —
A5 ist eine ausdrückliche Bestellung des Nutzers, sie still zurückzustellen hiesse, für ihn
zu entscheiden. **Erwarteter Bedarf** (● nicht gemessen): ein `int16`-Feld, eine
Vorzeichenerweiterung, eine geänderte Bitmaske, ein zweiter Vergleich, ein weiterer
Sendeaufruf, kein neuer Text — eher Dutzende als Hunderte Byte. Das entscheiden S.12 und der
Testbau.

**PWA-Teil als P3** nach S (L241). **DS18xx** (A2) kann den Entwurf mit einer weiteren
angehängten Variable übernehmen.

---

## 8. Schritt C4 — Umschrift ohne Flash

ASCII-Umschrift der acht ISO-8859-1-Dateien unter `src/**` (L166), **eigener Schritt**,
Nachweis über **gleiche `.hex`** von F103 und F411, **nicht mit S** (dort ändert sich das
Fabrikat absichtlich), **ohne Flash und ohne Versionserhöhung** (das Fabrikat ist dasselbe).
Byteweise, Zeilenenden unverändert (DIR-015). Weicht ein `.hex` ab, stand Nicht-ASCII doch
ausserhalb eines Kommentars — Halt, zurück, Befund. **Wann:** nach S.23, parallel zu F, vor
F.8. **Achtung:** C4 fasst auch `rtc.c` an, das S.18b ändert — deshalb strikt nach S.23.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Runde |
|---|---|---|---|
| `BEFUNDE.md` | je Runde nachführen; C2, C9c4 geschlossen; B19, E2 erledigt; Massnahme 4 eingeschränkt; E3 offen | `doc-writer` | P, P2, S, P3, F |
| `app.js` | **P2:** Overlay-Text, B40, SSID-Auswahl, Massnahme 4 (vier Felder), P2-Review N1/N2. **P3:** A5 | `pwa-developer` | P2, P3 |
| `tools/ui-mess/proben/` | Proben B40+SSID (P2.1c), L303/L306 (P2.1e), A5 (P3.2) | Lead | P2, P3 |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | Eröffnung/Abschluss, Zuordnung, Markenpflicht, Zähler | `esp-developer` | S |
| `ESP8266/ESP-uclock/eepromdata.cpp` | C31 | `esp-developer` | S |
| `ESP8266/ESP-uclock/vars.h`, `vars.cpp` | **A5:** Index 49, Vorgabe `0x8000` | `esp-developer` | S |
| `ESP8266/ESP-uclock/http.cpp` | **S:** N1; **A5-Legacy** (RTC-Anzeige, Nachrechnen bei Korrektur, eine Hilfsfunktion). **F:** F.1–F.3, C27+N2, E12, C25, Overlay-Text, C38, C23, C6u | `esp-developer` | S, F |
| `ESP8266/ESP-uclock/httpclient.cpp` | C9k | `esp-developer` | F |
| `src/vars/vars.c`, `vars.h` | Eröffnung/Abschluss; A20; A14; **A5** | `stm-developer` | S |
| `src/rtc/rtc.c`, `rtc.h` | **A5** vorzeichenbehaftet; **Nachkommabit** (falls bestätigt) | `stm-developer` | S |
| `src/main.c` | A39; **E17**; A3+A48; A49; A46-Anteil; **A5** (Änderungserkennung) | `stm-developer` | S |
| weitere `src/**` | A13+A47, A24, A45, A46, A16, **A9** | `stm-developer` | S |
| acht Dateien aus L166 | **C4** Umschrift | `stm-developer` | C4 |
| `CLAUDE.md`, `tools/**` | **E17** | Lead | S |
| `knowledge/quick-reference.md`, `knowledge/architecture-checklist.md` | **E17** | `doc-writer` | S |
| `tools/checks/` | Prüfstände AKS.5, AKS.14, **AKS.15** (STM und Legacy), **AKS.17**, C38; S13, S15, S7 | Lead | S, F |
| `tools/smoke-device.sh`, `tools/watch-log.sh`, `tools/check-pwa.mjs` | Stufe 65535, E11; C25- und C23-Format | Lead | F, K |
| Versionsdateien | je Einspielschritt | `release-engineer` | je Runde |

**Erledigt und nicht mehr Teil der Planung:** `tools/snapshot-device.sh` (B19, `e121058`);
`APP-BUNDLE.md`, `data/app-bundle.txt`, `tools/*.py|sh` unter `ESP8266/ESP-uclock/` (E2,
`8dfba36`).

**Kodierung und Zeilenenden vor jedem Patch feststellen** (DIR-015). **`grep -a` in
`src/**`** (B21) — bis C4 erledigt ist.

---

## Prüfung gegen die Architektur-Checkliste

### Proper architecture

**PWA parallel zu Legacy?** Ja. F stärkt den Legacy-Flashpfad; N1 und C6u härten das
Legacy-Overlayformular; **A5 bringt die Minusgrade in beide Oberflächen gleich** (Ent-8) —
Legacy zeigt nicht weniger als die PWA, und beide formatieren denselben Wert gleich (AKP3.3).
Legacy bleibt die Stabilitäts-Referenz.

**Wetter-Endpunkte?** Unberührt.

**Restore-Bedingung?** A39 verschiebt den Ort des Setzens, das Flag wandert mit; E17 benennt
es danach um, in einem eigenen Diff; Code und Dokumente im selben Commit.

**Nur `.gz`?** Ja. C27 stärkt „Grösse > 0".

**Richtige Schicht?** B35, B40, SSID-Auswahl, Massnahme 4, L303/L306 in der Oberfläche; C38,
C23, C6u, N1 auf dem ESP; **A5 und das Nachkommabit an der Quelle** — nur der STM kennt das
Register. Eine Umrechnung im ESP oder in der PWA aus dem begrenzten Index wäre die falsche
Schicht; die Information ist dort schon verloren. **Das Nachrechnen bei einer
Korrekturänderung bleibt auf dem ESP**, wo es heute schon für Index 21 geschieht — eine
zweite, abweichende Stelle dafür wäre schlechter als die bestehende.

### Scalable systems

**STM-Kommandos?** Vollabgleich rund **197**. A5 sendet zusätzlich nur bei Änderung. Die
Nachkommabit-Korrektur kann die Änderungsrate leicht erhöhen — Wechsel im halben Grad statt
im ganzen —, also wenige zusätzliche `N`-Kommandos je Stunde.

**UART-Last?** Zuordnungszeile rund 1,5 kB je Vollabgleich; A5 rund 10 Byte je
Temperaturänderung (zwei Variablen); C31 und A20 weniger; A46/A49 gedrosselt.

**Unter 20 s?** Ausbleibende Quittung bleibt der gefährlichste Pfad — eigene Zeile, `!v`,
`var …`. A16 beseitigt zwei Pfade über 20 s.

**Hartkodierte Grenzen?** A5 hängt am Enum-Ende; **beide Enums müssen gleich lang bleiben** —
S.21 prüft das. **Härteste Grenze: F103-Flash** (§2.7).

### Secure by design

**`innerHTML`?** SSID-Namen nie roh (P2.1f). **Fremddaten?** N1 als schärfster Fall; C23
escapet Logzeilen. **Credentials?** C31, A20 — **und B19**: Das Passwort für M2 liegt
ausserhalb des Repos, läuft über `env:`/`file:` statt über die Prozessliste, und der Agent
liest es nie. **Neue schreibende Endpunkte?** Keine.

### Stable & reliable

**Fehler ausgewertet?** P: B36, R1–R4, L303/L306. Brücke: A35. F: E12, Overlay-Text, C38,
C6u, Ent-4. **A5:** „kein Messwert" ist `0x8000`, nicht `0`.

**Leere `catch`?** R3 beseitigt einen; zwei bleiben bewusst (`localStorage`).

**Stille Verwerfungen?** C26, A35, A46, A49, C25; Index 49 auf einem alten ESP verworfen —
beabsichtigt.

**Still zurechtgebogen?** **A5 beseitigt die Klemmung auf 0 °C** in PWA und Legacy; **das
Nachkommabit** beseitigt ein stilles Abrunden auf ganze Grad. B40 und die SSID-Auswahl
tauschten Eingaben still; Overlay-Text und C38 kürzten still. **Massnahme 4** ist die
Gegenrichtung — und bewusst nur dort korrigiert, wo es einen Ausgangswert gibt.

**Zustand nach Abbruch?** Wie in den Abschnitten oben; **C4:** Weicht ein `.hex` ab, wird die
Umschrift ganz zurückgenommen.

**Flags aufgelöst?** Wie zuvor; der „geändert"-Zustand der vier Netzwerkfelder fällt bei
wiederhergestelltem Ausgangswert.

---

## Verworfene Alternativen

**A5: Index 21 umdeuten; einen obsoleten Index wiederverwenden; Vorzeichen als eigenes Flag;
Offset-Kodierung.** §7.2 bzw. vorige Fassung.

**A5: Legacy weiter auf Index 21 lassen.** Durch **Ent-8** überholt — der Nutzer will
Minusgrade auch in Legacy.

**A5: ESP rechnet Index 49 bei einer Korrekturänderung nicht nach.** Verworfen, seit Legacy
Index 49 zeigt: Legacy und PWA zeigten nach einer Korrektur kurz verschiedene Werte (§7.3).

**Nachkommabit: Bit 6 (0,25 °C) mit auswerten.** Der Index hat halbe Grad als Auflösung; ein
Viertelgrad bräuchte ein anderes Format. Nicht Teil.

**Nachkommabit am Gerät abnehmen.** Hängt von der Raumtemperatur ab; Prüfstand.

**Massnahme 4 allgemein.** Entscheidung des Leads: nur die vier vorbefüllten Netzwerkfelder
(§1.9).

**B17 in P2 umsetzen.** Spuren 1 und 2 sind abgedeckt; Spur 3 braucht einen ESP-Neustart, und
den liefert S ohnehin (§1.11).

**E17 im selben Diff wie A39; C4 mit S oder mit Flash; P2 mit eigenem Durchlauf; L303/L306 in
S.26; C38 am Gerät; Flash-Gate nur als Schätzung oder nur am Ende; K als eigene Runde; F
parallel zu S** — wie in der vorigen Fassung begründet.

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
| S, ESP-Teil (N1, A5 mit Legacy) | — | ☐ | — | — |
| S, STM-Teil (E17, A5 samt Nachkommabit, gemessene K-Punkte) | ☐ | — | — | — |
| P3 (A5-PWA) | — | — | ☐ | ☐ |
| C4 | — | — | — | — |
| F | — | ☐ | — | — |

`APP_VERSION` und `CACHE_NAME` **immer zusammen**. Nur der `release-engineer` (R4). Der Tag
`release/<stm>-<esp>-<app>` wird **je Einspielschritt** gesetzt und gepusht (DIR-011).
