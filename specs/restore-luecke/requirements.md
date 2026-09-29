# Anforderungen — Restore-Lücke beim Wetter-Ticker

**Status:** Zur Freigabe
**Auslöser:** `REVIEW.md`, Kernbefund 2 und Massnahme 2. Verifiziert am Code.

## Problem

`src/main.c:3699-3711` löscht `pending_weather_ticker_restore` **unbedingt**, setzt
`DISPLAY_CLOCK_FLAG_UPDATE_ALL` aber nur, wenn `display_clock_flag` gerade 0 ist:

```c
pending_weather_ticker_restore = 0;        // immer
if (! display_clock_flag)                  // nur manchmal
{
    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
}
```

Die Flags sind **disjunkte Bits** (`src/display/display.h:34-39`): `UPDATE_MINUTES 0x01`,
`UPDATE_NO_ANIMATION 0x02`, `UPDATE_ALL 0x04`. Steht bei Eintritt `0x01` oder `0x02`, ist
das Restore **verbraucht ohne Wirkung** — ohne Wiederholung, ohne Log.

`display_clock()` (`src/display/display.c:2926`) betritt den `UPDATE_ALL`-Zweig dann nie.
Nach einem Ticker existiert kein `TARGET_STATE`-Bit mehr, `display_animation_flush(FALSE)`
schreibt danach alle Display-LEDs aus. **Das Display bleibt dunkel** — bis zum nächsten
`UPDATE_ALL`: bei `WCLOCK24H == 1` bis 60 s, bei `WCLOCK24H == 0` bis 5 Minuten.

**Auslöser aus der PWA:** der Dimmkurven-Burst hält `display_clock_flag` rund 4 s auf
`0x02`. **Ohne PWA:** die LDR-Automatik setzt `0x02` bei jeder Helligkeitsänderung
(`main.c:3238`), 4×/s — das erklärt, warum das Symptom auch ohne PWA auftrat.

## Ziel

Ein ausstehendes Restore geht nicht mehr verloren. Es wird auf den nächsten Durchlauf
verschoben, bei dem es tatsächlich wirken kann, statt verworfen zu werden.

## Akzeptanzkriterien

- [ ] **AK1** — `pending_weather_ticker_restore` wird nur dann gelöscht, wenn im selben
      Durchlauf `DISPLAY_CLOCK_FLAG_UPDATE_ALL` gesetzt wird
- [ ] **AK2** — Die vier Teilbedingungen der Restore-Prüfung bleiben **unverändert**:
      Ticker inaktiv, kein Icon aktiv, kein Icon-Stop-Timer offen, kein Overlay aktiv
      (Architektur-Invariante in `CLAUDE.md`)
- [ ] **AK3** — Kein anderer Pfad, der `display_clock_flag` setzt, ändert sein Verhalten
- [ ] **AK4** — `make f103` und `make f411` übersetzen fehlerfrei
- [ ] **AK5** — `./tools/guardrails.sh` liefert keine **neuen** Findings gegenüber dem
      Stand vor der Änderung
- [ ] **AK6, am Gerät** — Reproduktion nach `REVIEW.md` F2: Wetter-Ticker starten und
      währenddessen in der PWA die Dimmkurve speichern. Vorher blieb das Display bis zu
      60 s dunkel, nachher kommt die Uhrzeit innerhalb eines Anzeigezyklus zurück

## Nicht Teil dieser Änderung

- **Kein DMA-Fix, kein Recovery-Mechanismus** (DIR-003)
- Keine Änderung an der Dimmkurven-Schleife in `app.js` — das ist eine eigene Massnahme
- Keine Änderung an der Blockadedauer von `eeprom_write`
- Keine unbedingte Logausgabe im Hauptloop
- Keine Behebung der übrigen 17 Massnahmen aus `REVIEW.md`

## Betroffene Laufzeiten

- [x] **STM32** (`src/main.c`) — **Neu-Flashen nötig**
- [ ] ESP8266 — nicht betroffen
- [ ] PWA — nicht betroffen
- [x] Build/Release — Versionsbump und Release-ZIP
