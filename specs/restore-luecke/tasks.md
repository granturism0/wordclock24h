# Tasks — Restore-Lücke beim Wetter-Ticker

| # | Task | Agent | Hängt ab von | Guardrails | Review |
|---|---|---|---|---|---|
| 1 | `pending_weather_ticker_restore = 0;` in den `if`-Block verschieben | `stm-developer` | — | ☐ | ☐ |
| 2 | `VERSION` auf 3.2.5 anheben | `release-engineer` | 1 | ☐ | ☐ |
| 3 | `make f103` und `make f411` übersetzen | `release-engineer` | 2 | ☐ | ☐ |
| 4 | Verifikation am Gerät nach AK6 | **Nutzer** | 3 | — | — |

## Abschluss

- [ ] AK1 bis AK5 erfüllt
- [ ] `./tools/guardrails.sh --full` ohne neue Findings
- [ ] AK6 am Gerät bestätigt
- [ ] Flash-Umfang benannt: **nur STM32**, ESP und LittleFS unverändert
