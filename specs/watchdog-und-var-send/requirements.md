# Watchdog aktivieren und die Blockade in var_send_buf schliessen

**Auslöser:** Reproduzierter Hänger vom 30.09.2026, siehe `haenger-2026-09-30.md`.
Befunde L14, L15, L16 in `BEFUNDE.md`.

## Ausgangslage — vier Glieder einer Kette

1. **`watchdog_init()` scheitert still** (`main.c:493-529`). Der LSI-Takt wird nirgends
   gestartet; `RCC_LSICmd` kommt im ganzen `src/`-Baum nicht vor. Die Warteschleife auf
   `PVU`/`RVU` läuft in den Timeout, die Funktion kehrt zurück, **`IWDG_Enable()` wird
   nie erreicht**. Am Gerät belegt: `IWDG init timeout, watchdog disabled` bei jedem Start.
2. **`var_send_buf()` wartet unbegrenzt** (`vars.c:62`):
   `while ((rtc = schedule_esp8266_messages ()) != ESP8266_OK) { ; }` — kein Timeout,
   keine Abbruchbedingung, kein `watchdog_reload()`. Die Schleife ruft den Dispatcher
   selbst auf und führt eintreffende Kommandos verschachtelt aus.
3. **`var_send_busy` wird gesetzt, aber nie geprüft.** Der gedachte Rekursionsschutz
   existiert nur als Flag.
4. **Folge:** `set_display_power()` bricht mitten drin ab und kehrt nicht zurück. Der
   zeitgesteuerte Zweig fällt aus, die Uhr steht — und mangels Watchdog **ohne Reset**,
   zweimal beobachtet über 18 Minuten beziehungsweise bis zum manuellen Eingriff.

## Ziel

Die Uhr darf durch gleichzeitige Bedienung über PWA und Legacy nicht stehenbleiben.
Bleibt sie es doch, muss der Watchdog sie innerhalb von 20 Sekunden zurücksetzen.

## Akzeptanzkriterien

- [ ] **AK1** — Die Startsequenz meldet `IWDG enabled: timeout=20000ms` statt
      `IWDG init timeout, watchdog disabled`. Am Mitschnitt prüfbar.
- [ ] **AK2** — `var_send_buf()` verlässt seine Warteschleife spätestens nach einer
      festgelegten Frist, auch wenn die Quittung ausbleibt, und protokolliert das.
- [ ] **AK3** — `var_send_busy` verhindert den rekursiven Wiedereintritt in
      `var_send_buf()` tatsächlich.
- [ ] **AK4, am Gerät** — Der Ablauf aus REPRO-3 (volles PWA-Pollingmuster plus
      `/?action=poweron` im Wechsel mit `update_path_set`) lässt die Uhr **nicht** mehr
      stehen. Mindestens fünf Durchläufe.
- [ ] **AK5, am Gerät** — Tritt dennoch eine Blockade über 20 s auf, erscheint im
      Mitschnitt eine Startsequenz mit `Reset flags: IWDGRST`.
- [ ] **AK6** — `set_display_power()` protokolliert weiterhin beide Abschlusszeilen
      (`flags=...` und `switching power on/off`); ihr Fehlen bleibt das Erkennungsmerkmal
      für einen erneuten Abbruch.

## Ausdrücklich nicht in diesem Schritt

- **Kein `watchdog_reload()` in andere blockierende Pfade** (`display_test()`,
  `remote_ir_learn()`, Ticker). Das ist Massnahme 1 aus `REVIEW.md` und braucht eine
  eigene Spec — mit aktivem Watchdog ändert sich dort das Verhalten grundlegend.
- **Kein Umbau der Kommandoverarbeitung.** Die verschachtelte Ausführung wird
  abgesichert, nicht beseitigt.
- Keine Änderung an der PWA. Der Fehler liegt im STM.

## Bekannte Nebenwirkung

**Mit aktivem Watchdog ändert sich das Verhalten bei allen langen Blockaden.**
`display_test()` blockiert 45 s und löst dann einen Reset aus — bisher hing die Uhr nur.
Das ist die in `REVIEW.md` beschriebene Erwartung, die bislang nie eintrat. Es ist die
richtige Richtung, aber eine sichtbare Änderung.

Deshalb ist die Reihenfolge festgelegt: **erst AK2/AK3, dann AK1.** Die Blockade wird
geschlossen, bevor der Watchdog scharf gestellt wird.
