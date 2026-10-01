# Aufgaben

| # | Aufgabe | Zuständig | Stand | Verifiziert |
|---|---|---|---|---|
| 1 | `.value` beim Anlegen jedes Namens auf NULL | `esp-developer` | ☑ | Compile |
| 2 | Null-Prüfung in `normalize_http_parameters()` | `esp-developer` | ☑ | Compile |
| 3 | Forward-Deklaration des Helfers | `esp-developer` | ☑ | Compile |
| 4 | `http_send_fs_file()` prüft die Grösse | `esp-developer` | ☑ | Compile |
| 5 | Beide Fundstellen der Asset-Suche prüfen die Grösse | `esp-developer` | ☑ | Compile |
| 6 | ESP-Version auf 3.2.3 | `release-engineer` | ☑ | S4 |
| 7 | Bauen, ausrollen, flashen | `release-engineer` | ☑ | Gerät meldet 3.2.3 |
| 8 | **AK3** — `GET /?a` stürzt nicht ab | — | ☑ | 7 Varianten, alle 200 OK, ESP erreichbar |
| 9 | **AK5** — PWA-Assets unverändert ausgeliefert | — | ☑ | 6 Assets, `Content-Encoding: gzip` korrekt |

## Nachtrag: Wartungsendpunkte (ESP 3.2.4)

| # | Aufgabe | Stand | Verifiziert |
|---|---|---|---|
| 10 | `Sec-Fetch-Dest` in der GET-Header-Schleife mitlesen | ☑ | Compile |
| 11 | Zustand je Request zurücksetzen | ☑ | Compile |
| 12 | `format_fs`, `reset_eeprom`, `reset_stm32` prüfen die Herkunft | ☑ | Compile |
| 13 | **Abgewiesen** bei `Sec-Fetch-Dest: image` | ☑ | 403, kein Reset |
| 14 | **Durchgelassen** bei `Sec-Fetch-Dest: empty` (PWA) | ☑ | 200, Reset erfolgt |
| 15 | **Durchgelassen** ohne Header (alter Browser) | ☑ | 200, Reset erfolgt |

Verifiziert wurde ausschliesslich an `maintenance_reset_stm32` — der einzige der drei,
den man aufrufen kann, ohne etwas zu verlieren. Deshalb trägt er dieselbe Prüfung,
obwohl er nichts löscht.
