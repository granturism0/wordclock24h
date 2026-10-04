# Anforderungen — Wiederholung auf der Kommandobrücke (A32 / L230)

**Status:** Entwurf — zur Freigabe durch den Nutzer
**Momentaufnahme vom 2026-10-04** (DIR-006). `specs/**` wird nicht fortgeschrieben.
**Auslöser:** Frage des Nutzers vom 04.10.2026 — „Und sollten wir das Thema mit der
Flusskontrolle nicht als Task aufnehmen, denn dies wäre ja eigentlich ein sauberes
Protokollhandling?" Daraus Befund **L230**, Massnahme **A32**.

---

## Problem

### Der Befund, nachgeprüft statt übernommen

`var_send_buf()` (`src/vars/vars.c:142-255`) sendet ein Kommando, wartet auf die
Quittung und bricht nach `VAR_SEND_TIMEOUT_SEC = 3` ab:

```c
if (uptime - start_uptime >= VAR_SEND_TIMEOUT_SEC)
{
    ...
    log_printf ("var_send_buf: keine Quittung nach %ds, weiter ohne: %s\r\n", ...);
    break;                                              // got_ack bleibt 0
}
```

Nach dem `break` passiert mit dem Kommando **nichts mehr**. Es ist weg. Die Zeile
`var_send_buf: keine Quittung nach 3s, weiter ohne: …` stand im Testdurchlauf vom
04.10.2026 im Mitschnitt und schon am 02.10.2026 im Mitschnitt zu L42 — es ist kein
theoretischer Fall.

Das Protokoll quittiert also bereits, es wiederholt nur nicht. **Es fehlt nicht die
Flusskontrolle, es fehlt die Nachsendung.**

### Was auf dieser Brücke tatsächlich läuft — abgezählt, nicht vermutet

| Richtung | Träger | Quittung | Wiederholung |
|---|---|---|---|
| STM → ESP | `var <kommando>` (`vars.c`) | Punkt `.` (`ESP-uclock.ino:532`) | **keine** ← A32 |
| ESP → STM | `CMD …` (RPC, Setter, `IPADDRESS`, `TIME`, …) | **keine** | keine |

Beide Richtungen teilen sich **eine** UART (der ESP hat nur einen vollwertigen,
`CLAUDE.md`, Hardware). Der Empfangsring des STM verwirft bei Überlauf still; am
04.10.2026 gemessen: `d=5010` verworfene Zeichen, in Sprüngen von rund 77 Byte
(L188, L141).

### Die These des Auftrags — geprüft, und sie trägt nur zur Hälfte

Die These lautete: A6, L42, L103 und L205 sind Folgen **desselben** Lochs.

**Für L42 und L103 (Werte fehlen) trägt sie — aber nicht so, wie es naheliegt.**
Der statische Variablensatz geht nicht verloren, weil einzelne Kommandos im Timeout
verschwinden, sondern weil **der Empfänger seinen Zustand verliert**: Der ESP startet
neu, sein RAM ist leer. Die Kommandos von vorgestern waren längst erfolgreich
zugestellt. Keine Wiederholung eines laufenden Kommandos stellt Zustand wieder her,
den die Gegenstelle weggeworfen hat.

Der Zusammenhang ist mittelbar und dadurch nicht schwächer: Nach jedem ESP-Neustart
meldet der ESP `IPADDRESS` (`wifi.cpp:101`, `:164`, `:186`, `:202`), und der STM
antwortet darauf mit `var_send_all_variables()` — rund 194 Kommandos am Stück
(`main.c:2925`). **Genau in diesem Stoss gehen Kommandos verloren**, und was dort
verlorengeht, kommt nie wieder, weil es einmalig angekündigte Werte sind.
`HARDWARE_CONFIGURATION` bleibt dann auf 65535 (L42), Helligkeit auf 0, Zeitzone auf
UTC (L103). **A32 wirkt also nicht gegen den ESP-Neustart, sondern gegen die
Verluste in dem Nachsendestoss, den der Neustart auslöst.**

**Für L205 (Wert kam verfälscht an) trägt sie nicht.** Das ist der wichtigste
Einzelbefund dieser Spec und steht ausführlich in `design.md`, Abschnitt 1. Kurz: Der
ESP quittiert **unbedingt**, direkt nach `var_set_parameter()`
(`ESP-uclock.ino:531-533`) — ohne zu prüfen, ob die Zeile überhaupt vollständig war.
Eine verfälschte Zeile wird angewandt **und** quittiert. Der Sender sieht Erfolg.
**Eine Wiederholung, die am ausbleibenden Ack hängt, wird in diesem Fall nie
ausgelöst.** Gegen Verfälschung hilft nur eine Integritätsprüfung der Zeile.

### Warum es drängt

Der Pfad trägt **jede** Einstellung der Uhr. Die drei bekannten Folgen:

- `HARDWARE_CONFIGURATION` = 65535 ⇒ jeder STM-Flash schlägt still fehl (DIR-010)
- `UPDATE_HOST`/`UPDATE_PATH` leer ⇒ der ESP holt Firmware vom Server des
  Ursprungsprojekts, ohne Fehlermeldung (L42, am 03.10.2026 eingetreten)
- `overlay[0].type` = 14 statt 2 ⇒ die PWA schreibt beim nächsten Speichern still 0
  und löscht die Einstellung des Nutzers (L205)

---

## Ziel

Ein Kommando, dessen Quittung ausbleibt, wird **nachgesendet** statt verworfen — ohne
den Hauptloop länger anzuhalten, als er heute schon angehalten wird. Ein Kommando,
das **verfälscht** ankommt, wird vom Empfänger erkannt, **nicht angewandt** und
abgelehnt, statt stillschweigend einen falschen Wert zu setzen.

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein Messmittel. Mittel, die es heute gibt:
`./tools/watch-log.sh` (Mitschnitt), `./tools/measure-log.sh`,
`./tools/snapshot-device.sh` + `diff-snapshot.sh`, `./tools/smoke-device.sh`,
`/api/stm32_log`, das Feld `v=<timeouts>/<verschachtelt>` der Diagnosezeile und der
Referenzwert **148'000 Hauptloop-Durchläufe je Sekunde** (L226).

**Die L177-Falle ist bei jedem Kriterium geprüft worden**: Dort war die Abnahme „die
Zeile aus `/api/stm32_log` auslesen" grundsätzlich unerfüllbar, weil die Zeile dort nie
ankam. Jedes AK unten nennt deshalb den Weg, auf dem die Information **nachweislich**
ankommt, und AK9/AK10 benennen offen, was mit den heutigen Mitteln **nicht** am Gerät
beweisbar ist.

### Runde 1 — Nachsendeliste im STM

- [ ] **AK1 — Keine neue Blockade.** In vier aufeinanderfolgenden Ruhefenstern zu je
      10 s liegt die Zahl der Hauptloop-Durchläufe zwischen zwei Diagnosezeilen bei
      mindestens **1'400'000** (Referenz 1'482'480, L226). Messmittel:
      `./tools/measure-log.sh`, Feld `l=`. **Dieses AK ist das wichtigste der Spec** —
      A29/A31 haben diesen Pfad am 04.10.2026 von Blockaden befreit, und eine
      Wiederholung, die wartet, wäre ein Rückfall dahinter.
- [ ] **AK2 — Ruhebetrieb unverändert.** Über 10 Minuten ohne Eingriff: `v=0/0`,
      keine `var retry:`-Zeile im Mitschnitt. Steigt dort etwas, ist die Nachsendung
      falsch angeschlossen und nicht „fast richtig".
- [ ] **AK3 — Die Nachsendung greift nachweislich.** Nach einem ESP-Neustart (OTA
      oder Reset durch den Nutzer) steht im Mitschnitt mindestens eine Zeile
      `var retry: ok <kennung> nach <n> Versuchen`. Messmittel: `./tools/watch-log.sh`
      **parallel** zum Neustart (DIR-013), zusätzlich `/api/stm32_log`.
      **Warum das erfüllbar ist:** Die Zeile entsteht über `log_printf()`, wird wie
      jede STM-Logzeile als `LOG …` auf die Brücke gespiegelt (`src/log/log.c:33`) und
      landet damit sowohl im seriellen Mitschnitt als auch im Ring des ESP. Der
      Mitschnitt ist das **führende** Mittel; der API-Ring kann 32 Zeilen halten und
      überläuft bei hoher Diagnoserate.
- [ ] **AK4 — Die Obergrenze greift.** Bei totem ESP (Nutzer hält ihn im
      Flash-Zustand) erscheint höchstens **eine** `var retry: Liste voll`-Zeile je
      60 s, und die Uhr läuft weiter oder wird vom Watchdog zurückgesetzt — sie bleibt
      nicht stehen. Nachweis: Diagnosefolge `seq` lückenlos oder mit erkennbarem
      Neuanfang bei 1.
- [ ] **AK5 — Kein Rückschritt im Abzug.** `./tools/snapshot-device.sh` vor und nach
      einem ESP-Neustart, `diff-snapshot.sh`: Die Zahl der abweichenden Felder ist
      **nicht grösser** als im Referenzlauf vor der Änderung. Der Referenzlauf wird in
      `tasks.md` **vor** dem ersten Flash genommen; ohne ihn ist das AK wertlos.
- [ ] **AK6 — `./tools/guardrails.sh` mit Exit 0**, `WD_EXPECTED` unverändert (die
      Nachsendung bringt **keine** neue `watchdog_reload()`-Aufrufstelle).
- [ ] **AK7 — Speicherbedarf belegt.** `arm-none-eabi-size` vor und nach, für
      **beide** Ziele (F103 **und** F411): Zuwachs in `.bss` höchstens **512 Byte**.
      Der F103 hat 20 KB RAM; eine Nachsendeliste, die dort nicht mehr passt, ist
      keine. Misst der Lead beim Bauen (R1).

### Runde 2 — der ESP erkennt verfälschte Zeilen

- [ ] **AK8 — Inert bis Runde 3.** Nach dem ESP-Flash allein ändert sich am
      Verhalten nichts: Smoketest grün, Abzug unverändert, Zähler der abgewiesenen
      Zeilen auf 0.
- [ ] **AK9 — Die Fähigkeitsmeldung kommt an.** Beim ESP-Start steht `CAP var-crc`
      im Mitschnitt. Das ist der einzige am Gerät **deterministisch** nachweisbare
      Teil von Runde 2 — und er ist der, an dem Runde 3 hängt.
- [ ] **AK10 — Dass die Prüfung greift, wird am Schreibtisch nachgewiesen, nicht am
      Gerät.** Es gibt heute **keinen** Weg, eine verstümmelte `var`-Zeile gezielt
      einzuspeisen; der einzige Sender ist der STM. Nachweis deshalb über eine
      eigenständige Tischprüfung (`tools/checks/`, Lead) gegen **dieselbe**
      Prüfsummenfunktion. **Das ist eine benannte Lücke, kein Versehen** — ein AK
      „eine abgewiesene Zeile im Mitschnitt" wäre die L177-Falle ein zweites Mal.

### Runde 3 — der STM hängt die Prüfsumme an

- [ ] **AK11 — Die Prüfsumme steht auf der Leitung.** Im Mitschnitt enden die
      `var`-Zeilen auf `*<2 Hexziffern>`. Direkt ablesbar.
- [ ] **AK12 — Kein verfälschter Wert mehr im Abzug.** Nach drei ESP-Neustarts
      enthält kein Abzug einen Wert ausserhalb seines Bereichs. Geprüft wird
      ausdrücklich `overlay[*].type` gegen 0..10 (L205) und
      `HARDWARE_CONFIGURATION != 65535`. Messmittel: `snapshot-device.sh`.
      **Einschränkung, die ins Protokoll gehört:** Drei saubere Durchgänge belegen
      keine Zustellgarantie. Sie belegen, dass der bekannte Schadensfall nicht mehr
      auftritt.
- [ ] **AK13 — Die Rückroll-Falle ist geprüft.** `./tools/smoke-device.sh` meldet
      einen Zeichenkettenwert, der auf `*<2 Hexziffern>` endet, als Fehler. Gegenprobe
      gefahren (künstlicher Wert im Prüfskript), sonst ist die Prüfung unbelegt.
- [ ] **AK14 — Keine neue Blockade**, gemessen wie AK1.

---

## Nicht Teil dieser Änderung

Dieser Abschnitt ist die Grenze. Was hier steht, wird **nicht** mitkorrigiert, auch
wenn die Datei ohnehin offen ist.

1. **Die Gegenrichtung ESP → STM.** Sie hat **gar keine** Quittung, und sie trägt die
   Kommandos, die **nicht** idempotent sind (`design.md`, Abschnitt 2). Ein
   Wiederholmechanismus dort wäre gefährlich, kein Fortschritt. Der Verlust einer
   `IPADDRESS`-Zeile in dieser Richtung bleibt damit ungeheilt — und er ist einer der
   beiden Wege, auf denen A6 entsteht. **Gehört als eigener Befund aufgenommen**, siehe
   „Was diese Spec offenlässt".
2. **Runde 4a / A6 (Weg B).** Bleibt bestehen. Die Begründung steht in `design.md`,
   Abschnitt 5 — die Vermutung, A32 mache 4a überflüssig, hat sich **nicht** bestätigt.
3. **Den Vollabgleich vertagen** (`specs/bruecke`, Design §4, Tasks 10/11). Dort
   bereits entworfen, nie gebaut. Wäre der grösste Hebel auf den Hauptloop und macht
   die Nachsendeliste kleiner — aber es ist ein eigener Eingriff mit eigener
   Abnahme. **Nicht hier mit hineinziehen.**
4. **A20 / L115 — Geheimnisse in der Timeout-Meldung.** Die bestehende Zeile gibt
   weiterhin `buf` aus, also auch Zeichenkettenwerte. Diese Spec ändert an ihr
   **nur den Text** („weiter ohne" wäre nach der Änderung eine Falschaussage) und
   **nicht** den Wert. Die neuen `var retry:`-Zeilen tragen von vornherein keine
   Werte.
5. **A1 / L85** — `watchdog_reload()` in `var_send_all_variables()`. Unverändert.
6. **Die PWA.** Keine Datei unter `data/app/**` wird angefasst. Kein
   `APP_VERSION`-Bump, kein `CACHE_NAME`-Bump.
7. **Eine Folgenummer in der Quittung.** Der Punkt `.` ist heute nicht zuordenbar;
   eine verspätete Quittung quittiert deshalb das **nächste** Kommando
   (`design.md`, Abschnitt 6). Das bleibt offen und wird als Befund aufgenommen.

---

## Betroffene Laufzeiten

- [x] **STM32 (`src/**`)** — Neu-Flashen nötig (Runde 1 und Runde 3)
- [x] **ESP8266 (`ESP8266/ESP-uclock/*`)** — Neu-Flashen nötig (Runde 2)
- [ ] PWA (`data/app/**`) — nicht betroffen
- [x] **Build/Release** — drei Rollouts, drei Tags (DIR-011)

**Versionen (R4, DIR-004):** STM steigt in Runde 1 und Runde 3, ESP in Runde 2.
Kein Gleichschritt, kein PWA-Bump. Bumpt ausschliesslich der `release-engineer`.

---

## Risiken — was diese Änderung kaputtmachen kann, das heute funktioniert

Die Uhr läuft produktiv. Fünf Risiken, nach Schwere:

| # | Risiko | Warum es real ist | Gegenmittel in dieser Spec |
|---|---|---|---|
| **R-1** | **Eine Nachsendung überschreibt einen neueren Wert.** Kommando läuft in den Timeout, wird vorgemerkt; zwei Sekunden später setzt der Nutzer denselben Wert neu, das gelingt; danach schreibt die Nachsendung den **alten** Wert | Genau die Schadensform, gegen die wir antreten — ein falscher Wert, der gültig aussieht (L205) | Jede Nachsendeliste wird beim Senden **und** beim Quittieren nach derselben Kennung durchsucht und der Eintrag entfernt. `design.md` Abschnitt 3.4. **Ohne diesen Teil ist die Spec abzulehnen** |
| **R-2** | **Zusätzliche Last auf genau der überlasteten Leitung.** Jede Nachsendung legt Bytes auf die Leitung, deren Überlastung die Quittung gekostet hat — die Mitkopplung aus L109 | Belegt: 4'215 verworfene Zeichen in 20 Minuten (L141), Sprünge von rund 77 Byte | Höchstens **ein** Nachsendeversuch je Sekunde, höchstens 4 vorgemerkte Kommandos, höchstens 2 zusätzliche Versuche je Kommando. Obergrenze der Zusatzlast: rund **16 Byte/s**, und nur im Störfall |
| **R-3** | **Rückrollen des ESP nach Runde 3.** Ein ESP ohne Prüfsummenverständnis bekommt `var S03meinhost*4a` und speichert den Hostnamen **mit** der Prüfsumme | OTA-Rückrollen ist in diesem Projekt Routine | Der STM hängt die Prüfsumme **nur** an, nachdem er `CAP var-crc` gesehen hat. Ein STM-Reset nach dem Rückrollen räumt den Rest. AK13 lässt den Smoketest danach suchen |
| **R-4** | **RAM auf dem F103.** 20 KB gesamt | Die Liste kostet rund 330 Byte | AK7 misst beide Ziele. Reicht es nicht, fallen die Plätze von 4 auf 2 |
| **R-5** | **Doppelte Ausführung nach verlorener Quittung.** Das Kommando kam an, nur der Punkt ging verloren — die Nachsendung wendet es ein zweites Mal an | Der Rückweg verwirft nachweislich Zeichen | **Unschädlich, abgezählt:** Jedes Kommando dieser Richtung ist eine reine Zuweisung in den ESP-Speicher. `design.md` Abschnitt 2 geht alle 14 Kommandoarten durch |

---

## Was diese Spec offenlässt — ausdrücklich, damit es niemand für geschlossen hält

1. **Die unquittierte Gegenrichtung.** Geht die `IPADDRESS`-Zeile des ESP im
   Empfangsring des STM verloren, startet **kein** Nachsendestoss, und der gesamte
   statische Satz bleibt weg. A32 ändert daran nichts. Das ist der zweite Weg zu A6 —
   und der, gegen den Weg B hilft.
2. **Die nicht zuordenbare Quittung.** Siehe „Nicht Teil dieser Änderung", Punkt 7.
3. **Der verschachtelte Sendefall.** `var_send_buf()` kehrt bei `var_send_nested`
   sofort zurück, ohne jede Quittungsprüfung (`vars.c:165-173`). Diese Kommandos
   werden **nicht** nachgesendet, weil niemand bemerkt, dass sie fehlen. Gezählt
   werden sie (`v=…/<verschachtelt>`). Aufgelöst würde das durch die Vertagung des
   Vollabgleichs (`specs/bruecke` §4), nicht durch A32.
4. **Verfälschung innerhalb eines Zeichenkettenwerts** bleibt nach Runde 3 in dem
   Mass erkennbar, in dem die Prüfsumme sie erkennt — das ist gut, aber keine
   Fehlerkorrektur. Erkannt heisst: nachgesendet.
