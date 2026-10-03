---
name: stm-firmware
description: Belegtes Detailwissen über die STM32-Firmware — Watchdog, blockierende Pfade, EEPROM-Kosten, die Wetter-Ticker-Restore-Bedingung, verschachtelte Kommandoausführung und wirkungsloses Logging. Nutzen bei jeder Arbeit an src/**, besonders bei Display-Hängern, Freezes, Resets und unklarem Laufzeitverhalten.
when_to_use: Bei Änderungen oder Analysen an der STM32-Firmware, bei Display-Hängern, Watchdog-Resets, Timing-Fragen und bei allem, was den Hauptloop blockieren könnte.
paths: src/**, CMakeLists.txt, cmake/**
allowed-tools: Read Grep Glob
---

# STM32-Firmware — belegtes Detailwissen

Alles hier ist am Code oder am ELF nachgeprüft, nicht vermutet. Fundstellen sind
genannt, damit jede Behauptung wiederholbar ist. Der vollständige Befundstand steht
in `BEFUNDE.md`, die **Pinbelegung und die Signalwege der Platine** in `HARDWARE.md`.

## Was die Hardware vorgibt

- Das Board ist eine Eigenentwicklung, **pinkompatibel zur BlackPill** — deshalb passt
  `BLACKPILL_BOARD`. Kein Modul mit Zusatzplatine.
- **Das EEPROM hängt am I2C** (AT24C32M, zusammen mit der DS3231-RTC). Der STM32F411
  hat keins. Daher die 15 ms Wartezeit je Schreibzyklus — das ist der
  EEPROM-Schreibzyklus, nicht Flash-Programmierung.
- **`PB0` schaltet die 5-V-Versorgung der LED-Kette** über zwei MOSFETs. Der
  `delay_msec(200)` nach `power_on()` ist die Einschwingzeit, keine Willkür.
- **Der ESP hat nur einen vollwertigen UART, und das ist die Brücke zum STM.** Jede
  `Serial.print`-Debugzeile des ESP landet deshalb zwangsläufig auf der STM-UART.

## Die Quellen sind teilweise ISO-8859-1 — `grep` braucht `-a`

Acht der 125 C-Dateien sind ISO-8859-1: `base.c`, `base.h`, `display.c`, `ds18xx.c`,
`irmp.c`, `rtc.c`, `tempsensor.c`, `w25qxx.c`.

`grep` stuft sie als **binär** ein und gibt **gar nichts** aus — nicht „0 Treffer",
sondern eine leere Ausgabe. Das sieht aus wie „nicht vorhanden", ist aber „nicht
gelesen". **`LC_ALL=C` behebt das nicht, nur `grep -a`:**

```
grep -c  'display_icon' display.c  →  (leer)
grep -ac 'display_icon' display.c  →  29
```

Realer Schaden, bereits eingetreten: Eine Suche nach `display_clock()` fand nichts,
obwohl die Funktion in `display.c:2926` steht. Betroffen sind ausgerechnet die
Display-Zustandsmaschine und die Dateien des offenen DS18xx-Themas.

## Der Watchdog hat genau eine Reload-Stelle

`watchdog_reload()` wird **regulaer nur am Kopf des Hauptloops** bedient;
`WATCHDOG_TIMEOUT_MS` ist 20000 (`main.c:382`). Weitere Aufrufstellen gehoeren zu
Schleifen, die den Loop bewusst anhalten — Display-Test, Ticker, IR-Lernvorgang.
**Den gueltigen Bestand und die Zeilennummern nennt `./tools/guardrails.sh` in
Stufe S7.** Hier stand bis 03.10.2026 „genau eine Stelle, `main.c:3170`"; es sind
sechs, und 3170 stimmt auch nicht mehr. Eine abgeschriebene Zeilennummer veraltet
schon durch den naechsten Patch — bei F1 ist sie durch den eigenen Eingriff von
3214 auf 3295 gewandert.

**Jeder Pfad, der länger als 20 s blockiert, ist ein garantierter IWDG-Reset.**

| Pfad | Stelle | Dauer |
|---|---|---|
| `display_test()` | `display.c:6082-6113` | **21 s / 45 s** — RGBW: 15 × `delay_sec(3)` |
| `remote_ir_learn()` | `remote-ir.c:117` | **unbegrenzt** — `while (1)` auf Tastendruck, zwanzigmal |
| `display_set_ticker(do_wait=1)` | `display.c:4802-4809` | 0,4 s bis **144 s** |
| `var_send_buf()` Warten auf Quittung | `vars.c:60-65` | unbegrenzt |
| `sk6812_refresh()` DMA-Wait | `sk6812.c:453-474` | unbegrenzt |
| `tables.complete`-Warteschleife | `display.c:4173` | unbegrenzt, bei **jedem** Moduswechsel |
| `eeprom_waitstates()` | `eeprom.c:38-57` | ~16 ms **pro Byte** |

`delay_sec()` → `delay_msec()` ist reiner Busy-Wait auf SysTick, ohne Reload
(`delay.c:61-80`).

Beim IP-Adress-Ticker in der Startsequenz (`main.c:2705-2712`) ergeben 20 Zeichen bei
Deceleration ≥ 8 über 22 s ⇒ Reset **genau während der IP-Anzeige**. Das passt
quantitativ auf die Beobachtung „Hänger exakt bei Anzeige von IP".

## `debug_log_*` übersetzt zu nichts

`debug_log_printf` hängt an `#ifdef DEBUG`, und `DEBUG` wird in `src/**`,
`CMakeLists.txt` und `cmake/**` **nirgends** definiert. Alle 164 `debug_log_*`-Aufrufe
sind wirkungslos — am ELF gegengeprüft.

**Folge für die Instrumentierung:** Jede Empfehlung „nimm `debug_log_printf`" ist
gegenstandslos. Wer messen will, braucht `log_printf` mit Zustandswechsel-Filter (so
gelöst bei `icon_freeze` in `main.c:3171`) oder einen Zähler, der einmal pro Minute
ausgegeben wird — **nie** in der ISR.

## Logging im heissen Pfad blockiert

`sk6812.c:476` und `:495` sind **`log_printf`, nicht `debug_log_printf`** — also
unbedingt, bei jedem Refresh, zusammen ~103 Zeichen. Über `log_vprintf` mit
blockierendem `esp8266_uart_flush()` sind das gerechnet ~8,9 ms pro Refresh. Bei einem
Ticker mit 21 Hz ergibt das rund 187 ms Logblockade pro Sekunde.

Verschärfend: `esp8266_get_message()` beginnt mit `log_flush()` und
`esp8266_uart_flush()` (`esp8266.c:202-203`). **Der STM liest kein Zeichen vom ESP,
solange Logausgabe aussteht.** Damit ist die Logmenge direkt an die Empfangslatenz
gekoppelt — und der RX-Ring ist 256 Byte und verwirft bei Überlauf **still**
(`uart-driver.h:698`, `if` ohne `else`).

## Die Wetter-Ticker-Restore-Bedingung nicht vereinfachen

`main.c:3699-3711`. Restore erst wenn Ticker inaktiv, kein Icon aktiv, kein
Icon-Stop-Timer offen, kein Overlay aktiv. **Alle vier Teilbedingungen sind nötig,
keine ist redundant.**

Die Lücke lag im unbedingten Löschen des Flags: Die Display-Flags sind disjunkte Bits
(`display.h:34-39`, `UPDATE_MINUTES 0x01`, `UPDATE_NO_ANIMATION 0x02`, `UPDATE_ALL
0x04`). Stand bei Eintritt `0x01` oder `0x02`, war das Restore verbraucht ohne Wirkung
— ohne Wiederholung, ohne Log, und das Display blieb bis zu 60 s dunkel, bei
`WCLOCK24H == 0` bis zu fünf Minuten. Seit 3.2.5 wird das Flag nur verbraucht, wenn
das Restore auch greift.

## Verschachtelte Kommandoausführung

`var_send_buf()` (`vars.c:47-66`) wartet auf die ESP-Quittung mit
`while ((rtc = schedule_esp8266_messages ()) != ESP8266_OK) { ; }` — die Warteschleife
ruft **die Dispatch-Funktion selbst** auf. Jedes in dieser Zeit eintreffende `CMD` wird
vollständig und verschachtelt ausgeführt.

Folge: `main.c:1623` lautet `display_clock_flag = set_display_power (val, TRUE);`.
Setzt ein verschachteltes Kommando dort `display_clock_flag`, wird der Wert beim
Rücksprung durch die Zuweisung **überschrieben und verworfen**. Gleiches Muster an
`main.c:3363`, `:3367`, `:3798`. `display_clock_flag` ist ein `static` Global ohne
Schutz.

## EEPROM-Kosten

`eeprom_write()` schreibt **Byte für Byte** mit `EEPROM_WAITSTATES 15` Busy-Wait, rund
16 ms pro Byte. Das Speichern der Dimmkurve aus der PWA sendet 16 Kommandos am Stück,
jedes schreibt die **ganze** Kurve ⇒ rund 4,3 s, in denen `display_clock_flag`
praktisch durchgehend auf `0x02` steht.

## Was ausdrücklich kein Fehler ist

- **Keine Race in der DMA-Konfiguration.** Der nächste Transfer startet erst nach der
  Warteschleife, Pufferwechsel und Kopie gehen in getrennte Puffer, die ISR liest den
  anderen. Sauber konfiguriert. Das Problem ist allein die Warteschleife ohne Timeout.
- **Kein board-spezifischer Unterschied F411 gegen F103**, der die Hänger erklärt.
  Timer-Zuordnung, DMA-Kanal und Bit-Timing sind korrekt; F411 ist beim SK6812-Timing
  sogar genauer. Einzige Ausnahme mit Richtung „F411 schlechter": Der Watchdog rechnet
  mit angenommener LSI-Frequenz (F4 mit 32 kHz), spezifiziert sind 17 bis 47 kHz — bei
  47 kHz liefe der IWDG mit rund 13,6 s statt 20 s. Das erklärt, *dass* der Reset
  früher kommt, nicht *warum* es hängt.
- **Keine Animationsfunktion** hinterlässt einen unauflösbaren Flag-Zustand.

**Bewusste Projektentscheidung:** Beim Thema der sporadischen F411-Hänger keinen
pauschalen DMA-Fix und keinen Recovery-Mechanismus einbauen. Erst per gezielter
Instrumentierung erhärten. Mechanische Kontaktprobleme sind als Ursache noch im Rennen.
