# Umsetzung

## 1. `var_send_buf()` — `src/vars/vars.c`

Zwei Sicherungen, beide neu:

```c
if (var_send_nested)          // Wiedereintritt aus der eigenen Warteschleife
{
    return;                   // Kommando ist raus, nur die Quittungspruefung entfaellt
}

var_send_nested = 1;
start_uptime = uptime;

while (schedule_esp8266_messages () != ESP8266_OK)
{
    if (uptime - start_uptime >= VAR_SEND_TIMEOUT_SEC)   // 3 s
    {
        log_printf ("var_send_buf: keine Quittung nach %ds, weiter ohne: %s\r\n", ...);
        break;
    }
}
```

**Warum ein eigenes `var_send_nested` und nicht `var_send_busy`?** Letzteres wird in
`main.c:2827` bei **jedem** `ESP8266_OK` zurückgesetzt, auch bei dem eines
verschachtelten Kommandos. Als Wiedereintrittssperre ist es damit unbrauchbar.

**Warum 3 Sekunden?** Deutlich unter den 20 s des Watchdogs, damit eine ausbleibende
Quittung keinen Reset auslöst. Die normale Quittung kommt in Millisekunden.
`uptime` hat Sekundenauflösung, die Frist liegt real bei 2 bis 3 Sekunden.

## 2. `watchdog_init()` — `src/main.c`

Der LSI wird jetzt **vor** der IWDG-Konfiguration gestartet:

```c
RCC_LSICmd (ENABLE);

while (RCC_GetFlagStatus (RCC_FLAG_LSIRDY) == RESET && timeout > 0)
{
    timeout--;
}

if (timeout == 0) { log_message ("LSI startet nicht, watchdog disabled"); return; }

timeout = 1000000UL;                    // fuer die Flag-Schleifen darunter
```

Ohne das blieben `PVU`/`RVU` gesetzt, die Warteschleife lief in den Timeout, und
`IWDG_Enable()` wurde nie erreicht.

## 3. `display_test()` — `src/display/display.c`

Der Display-Test **muss** erhalten bleiben, er ist das Werkzeug für den LED-Test. Mit
aktivem Watchdog wäre er bei 45 s (RGBW) beziehungsweise 21 s (RGB) ein garantierter
Reset. Deshalb bedient er den Watchdog jetzt selbst:

```c
for (w = 0; w < 30; w++)        // statt delay_sec (3)
{
    delay_msec (100);
    watchdog_reload ();
}
```

An **beiden** Stellen (RGBW- und RGB-Zweig). Dafür ist `watchdog_reload()` in `main.c`
nicht mehr `static` und in `main.h` deklariert.

**Abgrenzung:** Das ist bewusst nur `display_test()`. `remote_ir_learn()` und die
blockierenden Ticker bleiben unangetastet — dort ist die Blockade **nicht** gewollt, und
ein Reset ist die richtige Antwort. Ein pauschales `watchdog_reload()` in `delay_msec()`
würde genau diese Unterscheidung einebnen.

## Versionierung

Nur `src/**` geändert ⇒ nach DIR-004 steigt **nur** die STM-Version: 3.2.6 → **3.2.7**.
ESP und PWA bleiben stehen. Erster Praxisfall der komponentenweisen Regel.
