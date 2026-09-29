# Tasks — Guardrail-Befunde und do_display_icon-Messung

| # | Task | Agent | Hängt ab von | Guardrails | Review |
|---|---|---|---|---|---|
| 1 | A1–A6: PWA-Korrekturen in `app.js` | `pwa-developer` | — | ☐ | ☐ |
| 2 | B: Instrumentierung in `src/main.c` | `stm-developer` | — | ☐ | ☐ |
| 3 | C: `tools/checks/innerhtml.mjs` plus S7-Anbindung | Lead | 1 | ☐ | ☐ |
| 4 | Versionen anheben, bauen, ausrollen | `release-engineer` | 1, 2, 3 | ☐ | ☐ |
| 5 | AK7 am Gerät verifizieren | **Nutzer** | 4 | — | — |

Task 1 und 2 laufen **parallel** — `app.js` und `src/main.c` sind disjunkt (R3).

## Die 14 fehlenden englischen Schlüssel

| Schlüssel | Deutsch |
|---|---|
| `display.ticker_delay_save_failed` | Ticker-Verzögerung konnte nicht gespeichert werden |
| `display.test_running` | Displaytest läuft |
| `display.test_start_failed` | Displaytest konnte nicht gestartet werden |
| `display.dim_curve_save_failed` | Dimmkurve konnte nicht gespeichert werden |
| `display.tft_save_failed` | TFT-Optionen konnten nicht gespeichert werden |
| `display.ambilight_brightness_save_failed` | Ambilight-Helligkeit konnte nicht gespeichert werden |
| `display.ambilight_mode_save_failed` | Ambilight-Modus konnte nicht gespeichert werden |
| `display.ambilight_profile_save_failed` | Ambilight-Profil konnte nicht gespeichert werden |
| `display.ambilight_leds_save_failed` | LED-Anzahl konnte nicht gespeichert werden |
| `display.ambilight_offset_save_failed` | Ambilight-Offset konnte nicht gespeichert werden |
| `display.default_set_failed` | Standardwert konnte nicht gesetzt werden |
| `display.ambilight_default_set_failed` | Ambilight-Standardwert konnte nicht gesetzt werden |
| `animations.profile_save_failed` | Animationsprofil konnte nicht gespeichert werden |
| `animations.color_profile_save_failed` | Farbanimationsprofil konnte nicht gespeichert werden |

## Abschluss

- [ ] AK1 bis AK6 erfüllt, `./tools/guardrails.sh` mit **Exit 0**
- [ ] Release gebaut und auf die Synology ausgerollt
- [ ] Flash-Umfang benannt: **STM32 und LittleFS**, ESP unverändert
- [ ] AK7 am Gerät offen — Icon-Overlay starten, Display ausschalten, Logbuch prüfen
