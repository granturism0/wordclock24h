# Tasks — Paket 2026-10-05

**Erstellt:** 2026-10-04, fortgeschrieben am 2026-10-05 (Entscheidungen des Nutzers,
A35-Nachweis L266, neue Runde H). Momentaufnahme (DIR-006).

**Rundenfolge:** V → W → E1 → E2 → **H** → S → P → U → F.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wenn die Dateien disjunkt sind**
(R3b). Der Besitz-Hook lässt zwei Tasks durch, zwischen denen eine Abhängigkeit
steht — er prüft, **wer** schreibt, nicht **wann**. Am 03.10.2026 sind so zwei Tasks
parallel gestartet, zwischen denen eine Flash-Runde stand, die eine Änderung isoliert
nachweisen sollte; diese Zusage war danach nicht mehr einlösbar.

**Lesehinweis zu den Abnahmen:** Jede nennt ihr Instrument **und den Weg, auf dem die
Meldung ankommt**. Das ist kein Stil, sondern die Lehre aus L177/L185 — eine Zeile,
die am API-Ring vorbeiläuft, ist über die Oberfläche nicht abnehmbar, auch wenn sie
im seriellen Mitschnitt steht.

---

## Runde V — Nachzählen

**Warum zuerst, und warum als eigene Runde:** Sie kann den Umfang jeder folgenden
Runde verkleinern. Beim Schreiben dieser Spec haben drei Einträge der Arbeitsliste
nicht mehr gestimmt, und einer davon hätte eine ganze Runde erzeugt. Kein Flash, kein
Produktcode — die Runde kostet nur Lesezeit und spart im besten Fall eine Flash-Runde.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| V.1 | Welle 1, STM und Brücke: L232/A33, L42/L103/A6, L205/A30 nachzählen; dazu die **Gewichtsprüfung** zu A22/A24/A26 | `firmware-analyst` | — | Bericht nennt je Befund **ein** Verdikt aus `design.md` §1.1 mit Datei:Zeile **dieses** Commits. „nicht reproduziert" kommt nicht vor. Jedes `geschlossen` zitiert den Code, nicht einen anderen Befund | — | — |
| V.2 | Welle 1, ESP: L174, L178, L179, L180/C13, L125/L140/C9, L175/C9c2, L187/C16 | `code-reviewer` | — | wie V.1 | — | — |
| V.3 | Welle 1, Gerät: die Befunde aus V.1/V.2, die am Quelltext nicht entscheidbar sind (mindestens L205, L178, L180, L175) | Lead | V.1, V.2 | Je Befund ein Messwert mit Instrument und Zeitstempel. **L175 wird unter EINER Bedingung gemessen** — L262 und L254 widersprechen sich nur, weil zwei Bedingungen verglichen wurden. `./tools/watch-log.sh` lief mit (DIR-013); ein Neustart im Messfenster macht die Messung ungültig | — | — |
| V.4 | Welle 2: Abschnitt „Stand des grossen Pakets" Task für Task gegen den Quelltext — 1.3, 1.5, 0.11, 2.11, 3.4, 3.5, 3.6, 3.10, Runde 4a, Runde 4b | `code-reviewer` | — | Je Task ein Verdikt. **Runde 4a ist der Testfall dieses Durchgangs**: Der Abschnitt führt sie als „noch nicht begonnen", `src/main.c:2974`/`:3655` und L261 sagen etwas anderes | — | — |
| V.5 | Welle 3, Filter: je Eintrag der Arbeitsliste den jüngsten Commit auf die genannten Dateien gegen das Belegdatum stellen; Ergebnis ist die Liste der nachzuzählenden Einträge | Lead | — | Die Liste nennt je Eintrag Datei, Belegdatum und jüngsten Commit. Einträge ohne Änderung seit dem Beleg stehen als **ausdrücklich nicht nachgezählt** darin, mit dieser Begründung | — | — |
| V.6a | Welle 3 abarbeiten, Anteil `src/**` | `firmware-analyst` | V.5 | wie V.1 | — | — |
| V.6b | Welle 3 abarbeiten, Anteil ESP und PWA | `code-reviewer` | V.5 | wie V.1 | — | — |
| V.7 | Ergebnisse in `BEFUNDE.md` eintragen: je nachgezählter Zeile `nachgezaehlt: JJJJ-MM-TT · <Verdikt> · <Beleg>` — **auch bei Bestätigung** | `doc-writer` | V.1, V.2, V.3, V.4, V.6a, V.6b | **AKV.1 bis AKV.3.** Stichprobe von fünf `geschlossen`-Zeilen durch den Lead, jede am Quelltext nachvollzogen. Fällt eine durch, geht die ganze Welle zurück | ☐ | ☐ |
| V.8 | Arbeitsliste bereinigen; Standabschnitt des alten Pakets fortschreiben oder auflösen | `doc-writer` | V.7 | **AKV.4, AKV.5, AKV.6.** Guardrail S10 in beiden Richtungen; kein Eintrag verweist ausschliesslich auf Erledigtes, und jeder offene Befund steht in der Liste | ☐ | ☐ |
| V.9 | Bericht an den Nutzer: was geschlossen wurde, was bestätigt, und **welche Runde dieses Pakets dadurch entfällt oder schrumpft** | Lead | V.8 | Der Bericht nennt je entfallener Runde den Beleg. Entfällt nichts, steht auch das darin | — | — |

**Zwischenhalt.** Nach V.9 entscheidet der Nutzer, ob die geplanten Runden unverändert
gefahren werden. Das ist keine Formalie: Wenn Welle 1 etwa L174 als geschlossen
belegt, verliert Runde E1 ihren Hauptgrund.

---

## Runde W — Werkzeug und Verfahren

**Warum parallel zu V erlaubt:** Disjunkte Dateien (`tools/**`, `.claude/**`,
`knowledge/**` gegen `BEFUNDE.md`), und keine inhaltliche Abhängigkeit — **mit einer
Ausnahme**: W.7 (Stufe S11) braucht das Format der Nachzählvermerke aus V.7. Das steht
in der Abhängigkeitsspalte und ist nicht zu umgehen, auch wenn der Hook beide
durchliesse.

**Warum vor den Code-Runden:** U.1 und U.2 sind **ohne** das Messmodul nicht abnehmbar,
und die Prüfstandsheimat aus W.6 trägt die Abnahmen von E2.1 **und** H.1.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| W.1 | Messmodul: Kachelgeometrie je Panel, Ankreuzfeld-Geometrie, Farbe **am Bildpunkt**, Scrollen zum Element, echter `:focus-visible` über Tabulator. Als eigenständiges Skriptstück, das zwei Wirte einbinden | Lead | — | **AKW.1 (Vorschauteil).** Ein Lauf gegen die Vorschau legt eine Ergebnisdatei an, die je Panel Höhe und Breite und je Ankreuzfeld Geometrie und Farben nennt | ☐ | ☐ |
| W.2 | `tools/check-pwa.mjs` führt dasselbe Modul **gegen das Gerät** aus | Lead | W.1 | **AKW.1 (Geräteteil).** Eine Ergebnisdatei vom Gerät, mit Modulnamen und Viewport. Damit ist L229 geschlossen: Vorschau- und Gerätezahlen stehen nebeneinander | ☐ | ☐ |
| W.3 | `tools/preview/shot.sh`: ganzseitige Aufnahme und Scrollen | Lead | W.1 | Eine Aufnahme zeigt die TFT-Felder, die heute unterhalb des Seitenkopfs liegen und auf keiner Aufnahme je zu sehen waren | ☐ | ☐ |
| W.4 | Eichung: die Zahlen aus L227 mit dem Werkzeug reproduzieren | Lead | W.2 | **AKW.2** — 302 / 406 / 594 px über die Breiten 390, 600, 768, 900, 1240, 1512 px, Abweichung ≤ 2 px. **Reproduziert es die alten Zahlen nicht, misst es etwas anderes** und die Runde geht zurück zu W.1 | ☐ | ☐ |
| W.5 | `file-ownership.py` auf Schreibpositionen; `no-danger.py` auf Gerätebezug | Lead | — | **AKW.3 und AKW.4**, je zwei Gegenproben (DIR-014): Fehlalarm läuft durch, echter Zugriff wird abgewiesen. **Jedes Erkennungsmuster enthält den Pfad** — der erste Entwurf von `file-ownership.py` blockierte sonst die eigene Dokumentation | ☐ | ☐ |
| W.6 | Prüfstands-Heimat unter `tools/checks/` anlegen, mit der Pflicht zur Typangleichung (L256) als Teil der Ablage, nicht als Merksatz | Lead | — | **AKW.5-Vorbedingung.** Ein Beispiel liegt lauffähig vor (`var-crc.c` als Vorbild), und der Aufruf steht in `tools/guardrails.sh` oder ist dort begründet ausgelassen | ☐ | ☐ |
| W.7 | Guardrail-Stufe S11 „Nachzählpflicht" | Lead | V.7, W.6 | **AKW.6.** Die Stufe ist **einmal fehlgeschlagen** (DIR-014) **und** es ist nachgewiesen, dass sie ihren ganzen Gegenstand sieht: Sie meldet die Zahl der geprüften Befunde, und die wird von Hand gegen die Tabelle abgezählt (L235) | ☐ | ☐ |
| W.8 | Messauftrag B31: Trägt der Umriss des unangekreuzten Ankreuzfelds auch in **WebKit** die 3:1? | Lead | W.2 | Ein Messwert aus Safari, am Bildpunkt. Das ist die eine Messung, die die Priorität von B31 entscheidet — sie kostet fast nichts und braucht nur ein Gerät mit Safari. Ergebnis geht als Befundzeile an V.7/V.8 | — | — |
| W.9 | `knowledge/directives.md` und `knowledge/quick-reference.md` nachführen: E18 (DIR-010 bis DIR-014), B21 (`grep -a`), L256 — **und die Warnung aus `design.md` §6.5** | `doc-writer` | W.5, W.7 | **E18 verlangt zuerst eine Entscheidung, nicht ein Nachtragen:** Wer führt den Bestand? Empfehlung — der Katalog führt, `CLAUDE.md` behält die Kurzregeln und verweist. **Die Warnung im Wortlaut:** „Ein Mechanismus, dessen Sicherheit an einem Hardware-Nebeneffekt hängt, ist nicht abgesichert — er hat bisher Glück gehabt." Mit den beiden Belegen: GPIO-Puls (`esp8266.c:738-740`) und Einschaltstrom über `PB0` (L183). Abnahme: Keine Direktive steht inhaltlich an zwei Orten | ☐ | ☐ |
| W.10 | Agentenanweisungen: „Der Besitz-Hook ist eine Erinnerung, kein Zwang" (B22/L171) | Lead | W.5 | Die Aussage steht in jeder schreibenden Agentendefinition, und sie sagt, was daraus folgt: Fremde Dateien bleiben tabu, auch wo der Hook sie durchliesse | ☐ | ☐ |

---

## Runde E1 — ESP: Absturz und Beobachtbarkeit

**Warum eine eigene Runde:** Sie behebt den **einzigen reproduzierten** Absturz, den
der Nutzer heute durch Vermeiden eines Reiters umgeht. Alles andere kann warten; das
hier nicht. Und sie geht **vor** E2, weil E2 den Pfad anfasst, den jeder Request
durchläuft — eine Änderung dort auf einem Gerät zu messen, das sporadisch abstürzt,
vermischt zwei Ursachen.

**Warum keine zusätzliche Beobachtungsrunde danach:** Die Abnahme ist eine
**Reproduktion**, kein Ausbleiben. Es gibt ein Verfahren, das den Absturz zweimal
ausgelöst hat (voller Seitenaufbau mit geöffnetem Netzwerk-Modul). Ein Verfahren, das
auslöst, braucht keine Wartezeit.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E1.1 | `http_api_network_scan()`: Scan **vor** die erste Kopfzeile; Netzliste strömen statt `String` je Eintrag | `esp-developer` | V.2 | **AKE1.1**, per `grep` am Quelltext: `scanNetworks` steht vor dem ersten Sendeaufruf | ☐ | ☐ |
| E1.2 | `esp_heap_log()` und `- http write lost N` zusätzlich durch `stm32_log_append()` (C14/L185) | `esp-developer` | E1.1 | Quelltext; die Abnahme am Gerät folgt in E1.7 **über den Endpunkt**, nicht über den seriellen Mitschnitt | ☐ | ☐ |
| E1.3 | Review | `code-reviewer` | E1.2 | Ausdrücklich gegen `knowledge/architecture-checklist.md`, alle vier Punkte beantwortet. Besonders: Hat das Strömen der Netzliste einen Pfad ohne Fehlerauswertung erzeugt? | — | ☐ |
| E1.4 | ESP-Version anheben | `release-engineer` | E1.3 | S4 zeigt den neuen Stand | ☐ | — |
| E1.5 | Build, Release-ZIP, Rollout, Einspielzeile, Commit + Tag + Push | Lead | E1.4 | `guardrails.sh --full` Exit 0; Tag `release/<stm>-<esp>-<app>` mit den Versionen **dieses** Commits (DIR-011) | ☐ | — |
| E1.6 | ESP einspielen | **Nutzer** | E1.5 | neue ESP-Version in `/api/update_status` | — | — |
| E1.7 | Abnahme am Gerät | Lead | E1.6 | **AKE1.2 bis AKE1.4.** `check-pwa.sh` mit geöffnetem Netzwerk-Modul, zehn Durchläufe, `watch-log.sh` mitgelesen. Heap- und Verlustzeile in `/api/stm32_log`. Heapwerte unter **derselben** Bedingung wie die Vormessung | — | ☐ |
| E1.8 | `BEFUNDE.md` nachführen | `doc-writer` | E1.7 | S10 läuft durch; L174 und L185 tragen Status und Beleg | ☐ | — |

**Einspielreihenfolge:** nur ESP. Keine PWA-Änderung setzt diese Firmware voraus.
**Was sich für den Nutzer sichtbar ändert:** Der Netzwerk-Reiter ist danach wieder
gefahrlos. **Der Logring bleibt unverändert** — die geplante Verkleinerung ist aus dem
Paket genommen (L262).

---

## Runde E2 — ESP: Eingang der Brücke und Parametervertrag

**Warum eine eigene Runde und nicht mit E1 zusammen:** Beide sind ESP, aber E2 fasst
die Parameterauswertung an, die **jeder** Request durchläuft. Scheitert nach einem
gemeinsamen OTA irgendetwas, ist die Ursache nicht zuzuordnen. Das ist dieselbe
Überlegung, die im alten Paket 4a und 4b getrennt hat — und hier trägt sie, weil es
**keine** Fähigkeitsmeldung und keine Strommarke gibt, die die beiden Änderungen
voneinander entkoppelt.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E2.1 | **Eine** Längenprüfung der Kommandozeile vor dem Zerlegen (A37/L237); Prüfstand mit Schutzseite unter `tools/checks/` | `esp-developer` | E1.8, W.6 | **AKE2.1.** Die **alte** Fassung erzeugt im Prüfstand `SIGBUS`, die neue nicht; gültige Eingaben liefern in beiden byte-identische Ausgabe. **Die Ausgabe allein genügt nicht** — L249 zeigt den Fall, der harmlos *aussieht*, weil die gelesene Null im Ziel landet | ☐ | ☐ |
| E2.2 | `parse_json()` auf die Zeichengrenze zurücknehmen (C24/L249) | `esp-developer` | E2.1 | **AKE2.2**, im selben Prüfstand: Beschreibung, deren 31. Byte mitten in einem Zeichen liegt | ☐ | ☐ |
| E2.3 | Sieben Zeichenkettenfelder: abweisen statt kürzen (C22/L206) | `esp-developer` | E2.2 | Quelltext; die Gerätemessung folgt in E2.11 und **erst nach dem Einspielen** | ☐ | ☐ |
| E2.4 | `http_get_param()`-Vertrag; `fs_remove` und `dfplayer_play` melden keinen Erfolg beim Nichtstun (C20/L199) | `esp-developer` | E2.3 | Quelltext: es gibt keine toten `if (! value)`-Zweige mehr, oder sie sind ausdrücklich als tot entfernt | ☐ | ☐ |
| E2.5 | `dfplayer_alarm_set` absichern (C17/L197) — **abschreiben** aus `http_api_timer_set_common()`, nicht neu erfinden | `esp-developer` | E2.4 | Quelltext: dieselben drei Prüfungen, dieselbe Form | ☐ | ☐ |
| E2.6 | 19 Abweisungen auf `<param> out of range (<min>..<max>)`; die zwei Laufzeitgrenzen per `snprintf` in einen Stackpuffer, **kein `String`** | `esp-developer` | E2.5 | **AKE2.6**, `grep` über die Fehlertexte. **Dieser Task fällt als erster heraus, wenn die Runde zu lang wird** | ☐ | ☐ |
| E2.7 | Review, dazu AKE2.7: Verschluckt die PWA die neuen Abweisungen? | `code-reviewer` | E2.6 | Findet er eine Stelle, die nur auf `ok` prüft, wird daraus ein Task in Runde P — **keine stille Mitkorrektur in dieser Runde** | — | ☐ |
| E2.8 | ESP-Version anheben | `release-engineer` | E2.7 | S4 zeigt den neuen Stand | ☐ | — |
| E2.9 | Build, Release-ZIP, Rollout, Einspielzeile, Commit + Tag + Push | Lead | E2.8 | wie E1.5 | ☐ | — |
| E2.10 | ESP einspielen | **Nutzer** | E2.9 | neue ESP-Version in `/api/update_status` | — | — |
| E2.11 | Abnahme am Gerät; die drei schreibenden Proben erst **nach ausdrücklicher Freigabe des Nutzers im selben Gespräch** | Lead | E2.10 | **AKE2.3 bis AKE2.6.** Zu langer Wert auf `update_host_set` ⇒ `error=2`, Host unverändert, gegengeprüft mit `./tools/check-update-source.sh`. `dfplayer_alarm_set` mit ungültigem `idx` ⇒ abgewiesen, gegengeprüft mit `./tools/diff-snapshot.sh --soll`. **Vor dem Einspielen darf keine dieser Proben laufen** — sie würden heute einen unerreichbaren Alarm anlegen bzw. die Update-Quelle verstellen | — | ☐ |
| E2.12 | `BEFUNDE.md` nachführen | `doc-writer` | E2.11 | S10 läuft durch | ☐ | — |

**Einspielreihenfolge:** nur ESP. **Die PWA bleibt unverändert** — wenn E2.7 eine
Lücke in der Auswertung findet, geht sie nach Runde P und **nach** diesem OTA, nicht
davor (L241: Wenn eine PWA-Änderung eine Firmware-Änderung voraussetzt, kommt die
Firmware zuerst).

---

## Runde H — STM: Eingang härten (A41 / L265)

**Warum eine eigene Runde:** Runde S ändert das **Protokoll**, A41 den **Parser jedes
eingehenden Kommandos**. Beides in einem Flash macht einen Fehlschlag unzuordenbar.

**Warum vor S:** Die neuen Zeilenarten aus Runde S (Eröffnungszeile, Abschlussmarke,
Zuordnungszeile) landen dann auf einem gehärteten Parser und nicht umgekehrt.

**Keine Fähigkeitsmeldung im Spiel:** A41 ändert nichts, was der ESP sieht. Eine
Reihenfolgefrage gegenüber dem ESP stellt sich nicht.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| H.1 | **Eine** Längenprüfung der Kommandozeile vor dem Zerlegen, abgeleitet aus der Grösse des Empfangspuffers — nicht 66 Einzelprüfungen. Dazu die beiden exponierten Stellen `tftled.c:122` und `esp-spiffs.c:206-231`, wo hinter dem Vorrücken eine `while (*p)`-Schleife läuft. **Abgewiesene Zeile wird gezählt und einmal protokolliert** | `stm-developer` | E2.12, W.6 | **AKH.1 und AKH.4.** Prüfstand mit Schutzseite unter `tools/checks/`, **Typen angeglichen** (L256): alte Fassung `SIGBUS`, neue nicht, gültige Eingaben byte-identisch. Die beiden exponierten Stellen sind als eigene Fälle aufgeführt | ☐ | ☐ |
| H.2 | Review | `firmware-analyst` | H.1 | Vier Punkte der Checkliste; besonders: Erzeugt die Härtung eine stille Verwerfung? Ist die Längengrenze aus dem Puffer abgeleitet und nicht zweitgeschrieben? | — | ☐ |
| H.3 | STM-Version anheben | `release-engineer` | H.2 | S4 zeigt den neuen Stand | ☐ | — |
| H.4 | Build, Release-ZIP, Rollout, Einspielzeile, Commit + Tag + Push | Lead | H.3 | `guardrails.sh --full` Exit 0 | ☐ | — |
| H.5 | STM flashen über `./tools/flash-stm.sh` | **Nutzer** | H.4 | `--check` vorher ohne Beanstandung; danach die erwartete STM-Version. **Nicht** `/api/remote_stm32_flash` von Hand (DIR-010) | — | — |
| H.6 | Abnahme am Gerät | Lead | H.5 | **AKH.2 und AKH.3.** ESP-Neustart auslösen ⇒ vollständiger Vollabgleich durch den neuen Parser; Erfolgszeile im Logring, `d=` und Verlustzähler vorher/nachher, Diagnosefolge lückenlos, Hauptloop-Zähler gegen die Referenz aus L226. Dazu `./tools/diff-snapshot.sh --soll` und **ein Blick auf die Uhr** — `tftled.c` ist ein Anzeigepfad, ein Fehler dort steht nicht in der API | — | ☐ |
| H.7 | `BEFUNDE.md` nachführen | `doc-writer` | H.6 | S10 läuft durch; A41/L265 trägt Status und Beleg | ☐ | — |

**Einspielreihenfolge:** nur STM. Keine ESP- oder PWA-Änderung setzt diesen Stand
voraus. **Was sich für den Nutzer sichtbar ändert:** nichts — und genau das ist die
Abnahme (AKH.3).

---

## Runde S — die Brücke fertigbauen

**Warum ESP und STM in einer Runde, aber mit zwei Einspielschritten:** Alle drei
Neuerungen tragen ihre Voraussetzung **im Strom** mit, statt sich auf eine
Fähigkeitsmeldung zu stützen (`design.md` §6.0 und §6.5) — die zeigt ohnehin in die
falsche Richtung (`CAP`/`FIRMWARE` laufen ESP → STM, diese drei laufen STM → ESP).
Ein alter ESP verwirft eine unbekannte Kommandoart stillschweigend, ein alter STM eine
unbekannte Antwortzeile. **Beide Rückfalllagen verhalten sich wie heute, und das ist
am Code belegt, nicht angenommen** (L266). Deshalb braucht es zwischen den beiden
Teilen **keine eigene Beobachtungsrunde**. Zwei Einspielschritte bleiben, weil jede
Runde **eine** Laufzeit aufspielt (Risiko aus C13/L180).

**Reihenfolge: ESP zuerst, dann STM.** Begründung: Die neue Wirkung entsteht erst mit
dem STM-Teil. Steht der ESP schon bereit, ist der STM-Flash der Moment, in dem alles
zusammen wirkt — und er ist mit `flash-stm.sh` jederzeit wiederholbar, während ein OTA
es nicht immer ist (L180). **Zusätzlich liefert diese Reihenfolge den Nachweis für
AKS.6 gratis:** Zwischen S.7 und S.16 läuft ein neuer ESP gegen einen alten STM — die
Rückfalllage selbst, am Gerät.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S.1 | ESP: Eröffnungszeile und **Abschlussmarke** auswerten; `var_sync_check()` verlangt die Marke, **wenn** die Eröffnungszeile kam, sonst heutiges Kriterium (L260) | `esp-developer` | H.7 | **AKS.3.** Quelltext; der Rückfall ohne Eröffnungszeile ist ausdrücklich ausgewiesen | ☐ | ☐ |
| S.2 | ESP: Zuordnung als **eigene Zeile vor** der Quittung senden — `ACK <xy>`, dann `.` bzw. `!v`. **Der Punkt bleibt unverändert ein nackter Punkt** | `esp-developer` | S.1 | **AKS.4.** Quelltext. Die Zeile geht **jeder** Quittung voraus, dem Punkt wie dem `!v` — sonst bliebe genau der Fall unzugeordnet, in dem Nachsendungen entstehen | ☐ | ☐ |
| S.3 | ESP: Zähler „unmarkiert nach erster Marke"; Markenpflicht für die **drei benannten** Kommandoarten, Abweisung mit **`!v`** (C26) | `esp-developer` | S.2 | **AKS.7 und AKS.8.** Der Zähler ist über den Logring **oder** `/api/device_ready` ablesbar, nicht nur seriell. Die Liste ist **aufgezählt, nicht gemustert**. `!v` statt Schweigen ist Pflicht (L266) | ☐ | ☐ |
| S.4 | Review ESP-Seite | `code-reviewer` | S.3 | Vier Punkte der Checkliste; besonders: Kann die Markenpflicht einen Dauerzustand erzeugen? Wird die Markenerwartung mit dem Abgleich beendet, zu dem sie gehört? Die Antwort muss am Code stehen, nicht in der Spec | — | ☐ |
| S.5 | ESP-Version anheben | `release-engineer` | S.4 | S4 zeigt den neuen Stand | ☐ | — |
| S.6 | Build, Release-ZIP, Rollout, Einspielzeile, Commit + Tag + Push | Lead | S.5 | `guardrails.sh --full` Exit 0 | ☐ | — |
| S.7 | ESP einspielen | **Nutzer** | S.6 | neue ESP-Version in `/api/update_status` | — | — |
| S.8 | **Zwischenabnahme: neuer ESP gegen alten STM** — das ist der Rückfallnachweis | Lead | S.7 | **AKS.6.** ESP-Neustart auslösen ⇒ ein Vollabgleich läuft durch, **ohne Timeout und ohne Reset**; `var_send_timeout_cnt` steigt nicht, Diagnosefolge lückenlos, Smoketest 30/0, `watch-log.sh` mitgelesen. **Schlägt das fehl, geht Runde S zurück und wird nicht am STM fortgesetzt** | — | ☐ |
| S.9 | STM: Eröffnungszeile und Abschlussmarke senden | `stm-developer` | S.8 | Quelltext; die Eröffnungszeile sagt, was **dieser** Abgleich tut (markiert / schliesst ab), und gilt nur für ihn — kein Sitzungszustand | ☐ | ☐ |
| S.10 | STM: A39 — `IPADDRESS`-Zweig merkt vor, Ticker **nach** dem Abgleich (Variante b) | `stm-developer` | S.9 | **AKS.1.** Dazu ausdrücklich: `pending_weather_ticker_restore` wandert mit dem Ticker mit, zu dem es gehört. Der Umsetzer weist das im Bericht nach — keine der vier Teilbedingungen des Restores wird vereinfacht | ☐ | ☐ |
| S.11 | STM: Zuordnungszeile lesen, dem folgenden Punkt bzw. `!v` zuordnen, unpassende Quittung verwerfen | `stm-developer` | S.10 | **AKS.5** über einen Prüfstand unter `tools/checks/` mit **angeglichenen Typen** (L256): verspätete Quittung einspielen, das Kommando wird **einmal** nachgesendet, nicht endlos. Eine gemerkte Zuordnung ohne folgende Quittung wird **verbraucht**, nicht übertragen | ☐ | ☐ |
| S.12 | STM: `watchdog_reload()` in Tetris und Snake (A16/L106) | `stm-developer` | S.11 | Quelltext: in beiden Spielschleifen; die Gerätemessung folgt in S.18 | ☐ | ☐ |
| S.13 | Review STM-Seite | `firmware-analyst` | S.12 | Vier Punkte der Checkliste; besonders die drei neuen Zustände (`ip_ticker_pending`, gemerkte Zuordnung, Markenerwartung): Wird jeder auf **jedem** Pfad wieder aufgelöst? | — | ☐ |
| S.14 | STM-Version anheben | `release-engineer` | S.13 | S4 zeigt den neuen Stand | ☐ | — |
| S.15 | Build, Release-ZIP, Rollout, Einspielzeile | Lead | S.14 | `guardrails.sh --full` Exit 0 | ☐ | — |
| S.16 | STM flashen über `./tools/flash-stm.sh` | **Nutzer** | S.15 | `--check` vorher ohne Beanstandung; danach die erwartete STM-Version. **Nicht** `/api/remote_stm32_flash` von Hand (DIR-010) | — | — |
| S.17 | Abnahme am Gerät, ohne den Spiellauf | Lead | S.16 | **AKS.2, AKS.3, AKS.5, AKS.7, AKS.8.** Hauptloop-Zähler während eines ESP-Neustarts mit IP-Meldung gegen die Referenz aus L226; **der IP-Lauftext muss vollständig durchlaufen** — das nimmt nur das Auge ab; Erfolgszeile des Vollabgleichs im Logring **erst nach der Abschlussmarke**; Zähler ablesbar; keine `!v`-Abweisung im gesunden Zustand; `watch-log.sh` mitgelesen | — | ☐ |
| S.18 | Spiellauf Tetris oder Snake, mindestens 60 s | **Nutzer** | S.16 | **AKS.9.** Kein Watchdog-Reset im Mitschnitt. Fällt der Lauf aus, gilt AKS.9 als **nicht erfüllt** und wird so berichtet — nicht als „vermutlich in Ordnung" | — | — |
| S.19 | Commit + Tag + Push für den STM-Stand | Lead | S.17 | DIR-011, je Einspielschritt einzeln getaggt und gepusht | — | — |
| S.20 | `BEFUNDE.md` nachführen | `doc-writer` | S.18, S.19 | S10 läuft durch; A39, A35 Teil 1, C26, L260, L266 und A16 tragen Status und Beleg | ☐ | — |

**Einspielreihenfolge: erst ESP (S.7), dann STM (S.16).** Dazwischen die
Zwischenabnahme S.8, kein eigener Gerätelauf. **Was sich für den Nutzer sichtbar
ändert:** Der IP-Lauftext erscheint nach einem ESP-Neustart unverändert und läuft
vollständig durch — dass er das tut, ist Teil der Abnahme.

---

## Runde P — PWA

**Warum nach den Firmware-Runden:** Beide Änderungen sind firmwareunabhängig, aber die
Reihenfolge kostet nichts und hält die Regel aus L241 ein, ohne dass man sie je
Änderung neu prüfen muss. **Warum nicht mit U zusammen:** `make app-gz` braucht ein
stilles Arbeitsverzeichnis (R2); zwei Schreiber an der PWA gleichzeitig sind der
belegte Weisschirm-Fall.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P.1 | Binärsperre für `fs_show` (B33/L246): `.gz` und andere Binärendungen werden nicht angezeigt, sondern als binär gemeldet, mit Grösse aus `fs_list` | `pwa-developer` | S.20 | **AKP.1.** Browserlauf über `./tools/check-pwa.sh` mit geöffnetem Dateimodul: Konsole ohne Fehler, kein Binärinhalt im DOM | ☐ | ☐ |
| P.2 | `getDisplayModeName()` an die Layout-Modi binden (B24/L208) | `pwa-developer` | P.1 | **AKP.2.** Übersicht und Auswahlfeld nennen denselben Modus; die fünf fest verdrahteten Namen kommen im Quelltext nicht mehr vor | ☐ | ☐ |
| P.3 | Falls aus E2.7 hervorgegangen: Auswertung der neuen Abweisungen nachziehen | `pwa-developer` | P.2 | Nur wenn E2.7 eine Lücke gefunden hat; sonst entfällt der Task **ausdrücklich**, mit Vermerk | ☐ | ☐ |
| P.4 | Review | `code-reviewer` | P.3 | Vier Punkte; besonders: Hängt die Erfolgsmeldung vom tatsächlichen Ergebnis ab? | — | ☐ |
| P.5 | `APP_VERSION` **und** `CACHE_NAME` anheben | `release-engineer` | P.4 | Beide zusammen (R4), S4 zeigt den neuen Stand | ☐ | — |
| P.6 | `git status` prüfen, dann Build inkl. `app-gz`, Release-ZIP, Rollout, Commit + Tag + Push | Lead | P.5 | **R2:** Vor `app-gz` ist das Arbeitsverzeichnis still, alle Editier-Tasks abgeschlossen | ☐ | — |
| P.7 | PWA aufs Gerät: `./tools/install-app.sh --check`, dann hochladen | Lead | P.6 | `--check` nennt keine fehlende Datei; danach meldet die Seite die neue App-Version. **Der Rollout bringt die PWA nicht aufs Gerät** — das ist ein eigener Schritt | — | — |
| P.8 | Abnahme | Lead | P.7 | `check-pwa.sh` ohne Fehler, `watch-log.sh` mitgelesen | — | ☐ |

---

## Runde U — UI

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| U.1 | 44-px-Reset auf **alle** Ankreuzfelder heben (B30/L240) — gemeinsamer Selektor statt `.chip-toggle` allein | `ui-developer` | P.8, W.4 | **AKU.1**, gemessen mit dem Modul aus W.1/W.2: **einmal Vorschau, einmal Gerät**. Die drei TFT-Felder und die Favoritenfelder sind einzeln aufgeführt | ☐ | ☐ |
| U.2 | Inhalt der Sicherungskachel verdichten (B28/L227) — **über den Inhalt, nicht über das Raster** | `ui-developer` | U.1 | **AKU.2**: grösste Höhendifferenz ≤ 400 px über 390, 600, 768, 900, 1240, 1512 px. **Die Breite 600 px ist Pflicht** — dort lag der schlechteste Wert. **Wird es verfehlt, gilt es als verfehlt** und wird so berichtet (L227) | ☐ | ☐ |
| U.3 | Review | `ui-reviewer` | U.2 | **Dieser Agent hat kein `Bash`** (L238). Er bewertet die **Zahlen aus U.1/U.2** und das Markup; er misst nicht. Ein Messauftrag an ihn wäre derselbe Koordinationsfehler wie am 04.10.2026 | — | ☐ |
| U.4 | `APP_VERSION` und `CACHE_NAME` anheben | `release-engineer` | U.3 | zusammen (R4) | ☐ | — |
| U.5 | `git status`, Build, Rollout, Commit + Tag + Push | Lead | U.4 | R2 beachtet | ☐ | — |
| U.6 | Hochladen und Abnahme | Lead | U.5 | `install-app.sh --check`, dann Upload; danach Messlauf gegen das Gerät (W.2) und `check-pwa.sh` | — | ☐ |
| U.7 | `BEFUNDE.md` nachführen (B30, B28, dazu das WebKit-Ergebnis aus W.8 zu B31) | `doc-writer` | U.6 | S10 läuft durch | ☐ | — |

---

## Runde F — Flash-Überwachung

**Warum zuletzt:** Ihre Abnahme ist ein STM-Flash, und sie prüft genau den Pfad, über
den geflasht wird. Nach den Runden H und S liegt ein frischer STM-Stand vor, dessen
Version bekannt ist — ein Flash, dessen Ergebnis man vorhersagen kann, ist die
ehrlichste Probe für einen veränderten Flashpfad.

**Warum keine Fähigkeitsmeldung hilft:** Hier gibt es keine zwei Laufzeiten, die sich
abstimmen müssten — die Änderung liegt vollständig im ESP, der STM-Flash ist nur die
Probe. Eine Reihenfolgefrage stellt sich nicht.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| F.1 | Legacy-Flashpfad ruft `http_remote_stm32_filename_matches()` — **dieselbe** Funktion wie die API, keine zweite Prüfung | `esp-developer` | U.7 | **AKF.1**, `grep` findet den Aufruf an beiden Stellen; im Legacy-Zweig steht kein ungeprüftes `strncpy` des Dateinamens mehr. **Nicht scharf fahren** | ☐ | ☐ |
| F.2 | Auswahlliste bietet bei `HARDWARE_CONFIGURATION` = 65535 **keine** Datei an; Hinweistext nennt `./tools/flash-stm.sh` als Rückweg | `esp-developer` | F.1 | **AKF.2**, lesend über die Legacy-Seite. **Die Länge des Hinweistextes ist mitzuprüfen** (L257): Eine zu lange Meldung würde genau am Rückweg abgeschnitten | ☐ | ☐ |
| F.3 | `tools/smoke-device.sh`: `HARDWARE_CONFIGURATION` ≠ 65535 als eigene Stufe | Lead | F.2 | **AKF.3**, Gegenprobe gegen einen **festen Testwert**, nicht gegen einen am Gerät erzeugten Zustand. Im gesunden Zustand schlägt sie nicht an (DIR-014: einmal fehlgeschlagen) | ☐ | ☐ |
| F.4 | Review | `code-reviewer` | F.3 | Vier Punkte; besonders: Bleibt der Rückweg offen? | — | ☐ |
| F.5 | ESP-Version anheben | `release-engineer` | F.4 | S4 zeigt den neuen Stand | ☐ | — |
| F.6 | Build, Release-ZIP, Rollout, Einspielzeile, Commit + Tag + Push | Lead | F.5 | `guardrails.sh --full` Exit 0 | ☐ | — |
| F.7 | ESP einspielen | **Nutzer** | F.6 | neue ESP-Version in `/api/update_status`; Smoketest mit der neuen Stufe | — | — |
| F.8 | STM-Flash als Abnahme über `./tools/flash-stm.sh` | **Nutzer** | F.7 | **AKF.4**: `--check` vorher ohne Beanstandung, danach die erwartete STM-Version in `/api/update_status` | — | — |
| F.9 | Abschlussbilanz des Pakets in `BEFUNDE.md` | `doc-writer` | F.8 | Jeder im Paket berührte Befund trägt Status, Beleg und Nachzählvermerk; S10 und S11 laufen durch | ☐ | ☐ |

---

## Was gleichzeitig laufen darf — und was nicht

**Erlaubt:**

- V.1, V.2, V.4, V.5 gleichzeitig — alle rein lesend, verschiedene Agenten.
- Runde V und Runde W gleichzeitig, **ausser** W.7 (hängt an V.7).
- W.1 bis W.6 sind alle Lead und laufen deshalb ohnehin seriell.

**Nicht erlaubt:**

- **Kein Teammate führt `make` aus** (R1). Der Lead baut, einmal je Runde. Der Hook
  `tools/hooks/no-build.py` weist es ab; harmlose Ziele wie `make stm-version-file`
  bleiben erlaubt.
- **Zwei Schreiber in derselben Datei.** `http.cpp` wird in E1, E2 und F angefasst,
  `src/main.c` in H und S — **nie gleichzeitig**. Zwischen H und S liegt eine
  Flash-Runde; der Besitz-Hook liesse beide durch, die Spec tut es nicht (R3b).
- **Versionsnummern und `CACHE_NAME`** bumpt nur der `release-engineer` (R4).
- **Hardware ist exklusiv** (R5). Flashen und Live-Test macht der Nutzer bzw. genau
  ein Agent. Alles Schreibende am Gerät braucht die ausdrückliche Freigabe des
  Nutzers **im selben Gespräch** — es ist seine Uhr im Dauerbetrieb.

---

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um.
2. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings.
3. Review durch den zuständigen Review-Agenten, ausdrücklich gegen
   `knowledge/architecture-checklist.md`.
4. Erst dann nächster Task oder Schreibübergabe.

**Vor jedem Patch an einer ESP-Quelle: Kodierung *und* Zeilenende feststellen, nicht
annehmen.** `http.cpp` und `stm32flash.cpp` sind UTF-8, die übrigen überwiegend ASCII
oder ISO-8859-1; `ESP-uclock.ino` hat gemischte Zeilenenden. Ein Patch, der sie still
vereinheitlicht, macht aus neun geänderten Zeilen 432.

**Bei jeder Suche in den STM-Quellen: `grep -a`.** Ohne das übersieht die Suche acht
Quelldateien (B21/L166).

---

## Abschluss

- [ ] Alle Tasks erledigt oder **ausdrücklich mit Begründung ausgelassen**
- [ ] `./tools/guardrails.sh --full` mit Exit 0
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt — **oder als verfehlt
      berichtet**, nicht stillschweigend übergangen (L227, DIR-012)
- [ ] Je Runde ein Release-ZIP, der Flash-Umfang benannt, sofort committet, getaggt
      und **gepusht** (DIR-011)
- [ ] `BEFUNDE.md` führt für jeden berührten Befund Status, Beleg und
      Nachzählvermerk; S10 und S11 laufen durch
