# wordclock24h

German documentation: http://www.mikrocontroller.net/articles/WordClock_mit_WS2812

An English documentation will be available in the next days.

## PWA

Die moderne WordClock-PWA für den ESP8266 ist hier dokumentiert:

- [README-PWA.md](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/README-PWA.md)
- [APP-BUNDLE.md](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/APP-BUNDLE.md)

## Changelog

- [CHANGELOG.md](/Users/daniel/Documents/GitHub/wordclock24h/CHANGELOG.md)

## Hardware Notes

- `STM32F411CE BlackPill` mit `SK6812` verwendet in diesem Projekt `PB1` als Datenpin (`TIM3_CH4 / DMA1`).
- Ein Fehler im SK6812-Treiber für genau diese Kanalzuordnung wurde am `2026-04-08` korrigiert.
- Bei dunkler LED-Kette trotz laufender Firmware bitte nicht nur den Pull-up prüfen, sondern auch den High-Pegel der Datenleitung: `3.3V` direkt vom STM32 kann an `5V`-versorgten `SK6812` grenzwertig sein.
