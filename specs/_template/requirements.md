# Anforderungen — <Feature-Name>

**Status:** Entwurf | Zur Freigabe | Freigegeben am JJJJ-MM-TT
**Auslöser:** z. B. REVIEW.md Massnahme Nr. X, oder Nutzerwunsch vom JJJJ-MM-TT

## Problem

Was ist heute falsch oder fehlt. Mit Beleg: Datei:Zeile, Logauszug oder Beobachtung
am Gerät. Keine Vermutung ohne Kennzeichnung.

## Ziel

Was nach der Änderung gilt. Eine bis drei Sätze.

## Akzeptanzkriterien

Prüfbar formuliert. Jedes Kriterium muss jemand nachvollziehen können, ohne den Autor
zu fragen.

- [ ] AK1 —
- [ ] AK2 —
- [ ] AK3 — `./tools/guardrails.sh` läuft mit Exit 0 durch

## Nicht Teil dieser Änderung

Ausdrücklich ausgeschlossener Umfang. Verhindert, dass beim Umsetzen still
mitkorrigiert wird.

## Betroffene Laufzeiten

- [ ] STM32 (`src/**`) — Neu-Flashen nötig
- [ ] ESP8266 (`ESP8266/ESP-uclock/*.cpp`) — Neu-Flashen nötig
- [ ] PWA (`data/app/**`) — nur LittleFS-Upload
- [ ] Build/Release
