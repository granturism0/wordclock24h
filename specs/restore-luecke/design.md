# Design — Restore-Lücke beim Wetter-Ticker

## Lösungsweg

`pending_weather_ticker_restore = 0;` wird **in den `if`-Block hinein** verschoben, sodass
Flag und Wirkung zusammenfallen:

```c
if (! display_clock_flag)
{
    pending_weather_ticker_restore = 0;
    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
}
```

Ist `display_clock_flag` belegt, bleibt das Restore ausstehend und wird im nächsten
Durchlauf erneut geprüft.

**Warum das terminiert [BELEGT]:** `main.c:3766` setzt `display_clock_flag` nach jeder
Verarbeitung auf `DISPLAY_CLOCK_FLAG_NONE`. Die Restore-Prüfung (3699) läuft **vor** dem
Verarbeitungsblock (3713). Ein belegtes Flag wird also im selben Durchlauf abgearbeitet
und zurückgesetzt; im nächsten Durchlauf ist es 0 und das Restore greift. Der Hauptloop
läuft um Grössenordnungen schneller als die Ereignisse, die das Flag setzen.

## Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `src/main.c:3705-3710` | eine Zeile verschoben | `stm-developer` |
| `src/main.h` | `VERSION` anheben | `release-engineer` |

## Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Die vier Teilbedingungen bleiben unangetastet; die
Architektur-Invariante aus `CLAUDE.md` („Diese Bedingung nicht vereinfachen") ist
eingehalten. PWA und Legacy sind nicht betroffen, die Wetter-Endpunkte ebenso wenig. Die
Änderung greift dort an, wo die Ursache liegt — im STM, nicht als Umgehung in der App.

**Scalable systems** — Keine zusätzlichen STM-Kommandos, keine zusätzliche UART-Last, kein
zusätzlicher Code im blockierenden Pfad. Die Änderung kann einen `UPDATE_ALL` um wenige
Durchläufe verzögern, niemals verlängern. Watchdog-Verhalten unverändert.

**Secure by design** — Kein Netzwerkpfad, keine Fremddaten, keine Logausgabe mit
Gerätedaten. Nicht anwendbar.

**Stable & reliable** — Genau hier liegt der Gewinn: ein gesetztes Flag wird nicht mehr
still verworfen, sondern auf **jedem** Pfad aufgelöst. Beseitigt einen stillen
Fehlschlag, ohne einen neuen einzuführen. Das theoretische Restrisiko — Flag bleibt
hängen, weil `display_clock_flag` nie 0 wird — ist durch `main.c:3766` ausgeschlossen.

## Verworfene Alternativen

**Variante B, verodern (`display_clock_flag |= UPDATE_ALL`)** — technisch wirksam, weil
`display_clock()` die Bits mit `&` prüft und `UPDATE_ALL` in einem eigenen `if` steht
(`display.c:2962`). **Verworfen:** Es verwandelt ein bewusstes `UPDATE_NO_ANIMATION`
(„ohne Animation") in einen vollen Update **mit** Animation. Jede LDR-Helligkeitsänderung
während eines ausstehenden Restores würde sichtbar animieren.

**Variante C, überschreiben (`display_clock_flag = UPDATE_ALL`)** — verwirft die Absicht
des ursprünglichen Setzers vollständig, mit demselben Animationsproblem wie B, und
zusätzlich dem Verlust von `UPDATE_MINUTES`.

**Variante D, Timeout oder Zähler auf dem Flag** — löst das Problem nicht, sondern
kaschiert es. Kommt nur in Frage, falls sich am Gerät zeigt, dass A nicht terminiert.

## Versionsfolgen

- [x] STM `src/main.h` von `3.2.4` auf `3.2.5`
- [ ] ESP — unverändert
- [ ] App — unverändert
- [ ] `CACHE_NAME` — unverändert

Nur der `release-engineer` führt das aus (R4).
