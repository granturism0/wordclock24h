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
- der grosse Migrations- und Entkopplungsblock ist abgeschlossen
- neue Änderungen sollen jetzt bewusst klein und gezielt bleiben

## Changelog und Befundstand

- [CHANGELOG.md](CHANGELOG.md) — was sich pro Release geändert hat
- [BEFUNDE.md](BEFUNDE.md) — der lebende Massnahmenkatalog: Stand aller Befunde aus
  den beiden Code-Reviews und aus der laufenden Arbeit, je mit nachprüfbarem Beleg

## Hardware

Die aktuelle Platine ist eine Eigenentwicklung: **WordClock USB-C / STM32F411 V2** mit
STM32F411CEU6, fest bestücktem ESP-12F, DS3231-RTC, AT24C32-EEPROM am I2C und einem
SN74AHCT1G125 als Pegelwandler für die LED-Datenleitung. Sie ist pinkompatibel zur
BlackPill, weshalb die Firmware-Konfiguration `BLACKPILL_BOARD` passt.

Pinbelegung, Signalwege und der Abgleich gegen den Firmware-Code stehen in
[HARDWARE.md](HARDWARE.md).

Für den geplanten **Umbau auf einen ESP32-S3** (ein Chip statt STM32F411 und ESP-12F, Platinenformat
und LED-Board unverändert) gibt es ein eigenes Hardware-Team, Komponentenblätter mit Datenblattbelegen
und eigene Prüfungen. Rahmen, Rollen und Stand: [knowledge/hardware-esp32s3-umbau.md](knowledge/hardware-esp32s3-umbau.md),
Einstieg in die Unterlagen: `hardware/README.md`. Wo `HARDWARE.md` und die V2-Netzliste voneinander
abweichen, gilt die Netzliste; die Berichtigungen stehen in `HARDWARE.md`.

- `PB1` ist der Datenpin der LED-Kette (`TIM3_CH4 / DMA1`).
- Ein Fehler im SK6812-Treiber für genau diese Kanalzuordnung wurde am `2026-04-08` korrigiert.
- Bei dunkler LED-Kette trotz laufender Firmware: Auf **V2** übernimmt `U3` die
  Pegelanhebung auf 5 V — der frühere Hinweis, `3.3V` direkt vom STM32 sei an
  `5V`-versorgten `SK6812` grenzwertig, gilt nur für die **Vorgängerbestückung ohne
  dieses Board**. Prüfe stattdessen `PB0`: Damit schaltet die Firmware die
  5-V-Versorgung der Kette komplett ab.

## Werkzeuge

| Befehl | Zweck |
|---|---|
| `make release-zip` | vollständiger Build aller Komponenten plus Release-ZIP |
| `./tools/guardrails.sh` | Prüfungen vor jeder Übergabe — Syntax, undefinierte Aufrufe, i18n, Versionen, `.gz`, Muster, Aktualität der Doku und die Hardware des ESP32-S3-Umbaus (Stufen S19/S20, siehe [tools/checks/README.md](tools/checks/README.md)) |
| `python3 tools/preview/server.py 8099` | PWA ohne Gerät ansehen, siehe [tools/preview/README.md](tools/preview/README.md) |
| `./tools/deploy.sh` | Release auf den Update-Server ausrollen |
| `./tools/install-app.sh` | PWA ins LittleFS des Geräts laden (`--check` zeigt nur den Stand) |
| `./tools/smoke-device.sh` | nach dem Flashen gegen das Gerät prüfen (DIR-009) |
| `./tools/logger/log.sh` | Debug-UART der Uhr abrufen, siehe [tools/logger/README.md](tools/logger/README.md) |

Arbeitsregeln, Architektur-Invarianten und Versionskonventionen stehen in
[CLAUDE.md](CLAUDE.md).
