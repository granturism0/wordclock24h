# wordclock24h

German documentation: http://www.mikrocontroller.net/articles/WordClock_mit_WS2812

An English documentation will be available in the next days.

## PWA

Die moderne WordClock-PWA für den ESP8266 ist hier dokumentiert:

- [README-PWA.md](ESP8266/ESP-uclock/README-PWA.md)
- [APP-BUNDLE.md](ESP8266/ESP-uclock/APP-BUNDLE.md)

Aktueller Betriebsstand:

- Legacy bleibt unter `/` und `/legacy`
- die moderne Oberfläche läuft unter `/app`
- der große Migrations- und Entkopplungsblock ist abgeschlossen
- neue Änderungen sollen jetzt bewusst klein und gezielt bleiben

## Changelog

- [CHANGELOG.md](CHANGELOG.md)

## Hardware Notes

- `STM32F411CE BlackPill` mit `SK6812` verwendet in diesem Projekt `PB1` als Datenpin (`TIM3_CH4 / DMA1`).
- Ein Fehler im SK6812-Treiber für genau diese Kanalzuordnung wurde am `2026-04-08` korrigiert.
- Bei dunkler LED-Kette trotz laufender Firmware bitte nicht nur den Pull-up prüfen, sondern auch den High-Pegel der Datenleitung: `3.3V` direkt vom STM32 kann an `5V`-versorgten `SK6812` grenzwertig sein.

## Werkzeuge

| Befehl | Zweck |
|---|---|
| `make release-zip` | vollständiger Build aller Komponenten plus Release-ZIP |
| `./tools/guardrails.sh` | Prüfungen vor jeder Übergabe — Syntax, undefinierte Aufrufe, i18n, Versionen, `.gz`, Muster |
| `python3 tools/preview/server.py 8099` | PWA ohne Gerät ansehen, siehe [tools/preview/README.md](tools/preview/README.md) |
| `./tools/deploy.sh` | Release auf den Update-Server ausrollen |

Arbeitsregeln, Architektur-Invarianten und Versionskonventionen stehen in
[CLAUDE.md](CLAUDE.md).
