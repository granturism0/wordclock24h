# Debug-UART der WordClock dauerhaft mitschneiden

Ein Raspberry hängt über einen USB-TTL-Adapter am Stecker `H6` der WordClock und
schneidet alles mit, was der STM32 sendet. Der Mitschnitt ist über SSH abrufbar.

**Warum das mehr ist als `/api/stm32_log`:** Der Endpunkt der PWA versagt genau dann,
wenn man ihn braucht. Hängt die Uhr, antwortet der ESP nicht mehr und das Logbuch ist
weg. Der Mitschnitt läuft durch — auch durch einen Watchdog-Reset hindurch, und fängt
danach die Reset-Ursache aus der Startsequenz. Vier der offenen Messungen in
`BEFUNDE.md` sind ohne ihn gar nicht zu beantworten.

---

## 1. Was du brauchst

- **Raspberry** — jedes Modell ab Pi Zero 2 W oder Pi 3 reicht locker. Netzwerk per
  WLAN oder Kabel.
- **SD-Karte** ab 16 GB. Der Mitschnitt ist umfangreich, siehe Schritt 7.
- **USB-TTL-Adapter**, zwingend auf **3,3 V** stellbar. Viele haben einen Jumper
  zwischen 5 V und 3,3 V — **auf 3,3 V**.
- **Zwei Dupont-Kabel** (Buchse/Buchse), mehr nicht.

> **Alternative ohne Adapter:** Der Raspberry hat selbst einen UART auf GPIO 14/15,
> ebenfalls 3,3 V. Das spart den Adapter, kollidiert aber mit der seriellen Konsole des
> Pi und muss erst freigeschaltet werden. Mit vorhandenem Adapter ist der USB-Weg der
> kürzere.

---

## 2. Verkabelung — hier bitte genau lesen

`H6` auf dem Controller-Board ist vierpolig:

| Pin | Signal | Anschliessen? |
|---|---|---|
| **1** | 3,3 V (über Schottky-Diode `D3`) | **NEIN — nicht anschliessen** |
| **2** | STM **RX** (die Uhr empfängt) | nein, siehe unten |
| **3** | STM **TX** (die Uhr sendet) | **ja** → Adapter **RX** |
| **4** | GND | **ja** → Adapter **GND** |

**Es sind genau zwei Drähte:**

```
Adapter GND  ──────  H6 Pin 4
Adapter RX   ──────  H6 Pin 3
```

### ⚠️ Pin 1 niemals mit 5 V verbinden

Die Schottky-Diode `D3` zeigt vom Stecker **zur Platine**. Legst du dort 5 V an,
landen rund 4,7 V auf `VCC_3.3V` — das zerstört STM32, ESP, RTC und EEPROM in einem
Zug. Die Uhr hat ihre eigene Versorgung. **Lass Pin 1 frei.**

### Warum Pin 2 nicht angeschlossen wird

Ohne die TX-Leitung des Adapters kann versehentlich nichts in die Uhr geschrieben
werden. Ausserdem entfällt damit eine Stolperstelle: Der Schiebeschalter `SW4` legt den
Empfang des STM wahlweise auf den ESP oder auf `H6`. Zum reinen Mitlesen ist er egal.

### Kontrolle vor dem Einstecken

1. Adapter-Jumper steht auf **3,3 V**.
2. Am Adapter liegen nur **GND** und **RX** an, sonst nichts.
3. Messe mit dem Multimeter zwischen `H6` Pin 4 und Pin 1: Dort müssen **3,3 V**
   stehen, nicht 5 V. Stimmt das nicht, stopp — dann ist die Zählrichtung des Steckers
   anders als angenommen.

**Pin 1 ist die Seite mit dem quadratischen Lötpad** — auf der Platine prüfen, bevor
gesteckt wird.

---

## 3. Raspberry aufsetzen

**Raspberry Pi Imager**, Betriebssystem **Raspberry Pi OS Lite (64-bit)**. Vor dem
Schreiben das Zahnrad für die Vorkonfiguration öffnen:

| Feld | Wert |
|---|---|
| Hostname | `wordclock-pi` |
| Benutzer | `pi` (oder deiner — merken, wird gebraucht) |
| WLAN | deine SSID und Passwort, Land `CH` |
| SSH | **aktivieren**, Passwort-Anmeldung |

Karte schreiben, in den Pi, einschalten, ein bis zwei Minuten warten.

```bash
ssh pi@wordclock-pi.local
```

Meldet sich der Pi nicht unter diesem Namen, schau im Router nach seiner IP-Adresse und
nimm die. **Vergib im Router eine feste Adresse** — sonst ändert sie sich irgendwann
und der Mitschnitt ist unauffindbar.

Dann einmal aktualisieren und die Pakete holen:

```bash
sudo apt update && sudo apt full-upgrade -y
sudo apt install -y python3-serial git
```

---

## 4. Adapter erkennen und fest benennen

Adapter einstecken, dann:

```bash
lsusb
dmesg | tail -5
```

`lsusb` zeigt eine Zeile wie `ID 10c4:ea60 Silicon Labs CP210x`. **Notiere die beiden
Werte** vor und nach dem Doppelpunkt. `dmesg` sagt, als was der Adapter erkannt wurde,
meist `ttyUSB0`.

Ohne feste Benennung heisst der Adapter mal `ttyUSB0` und mal `ttyUSB1`. Deshalb:

```bash
sudo nano /etc/udev/rules.d/99-wordclock-uart.rules
```

Inhalt, mit **deinen** Werten anstelle von `HIER`:

```
SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", SYMLINK+="wordclock", MODE="0660", GROUP="dialout"
```

Übernehmen und prüfen:

```bash
sudo udevadm control --reload-rules && sudo udevadm trigger
ls -l /dev/wordclock
```

**Richtig sieht so aus:** `/dev/wordclock -> ttyUSB0`. Kommt „No such file", stimmen die
IDs nicht.

### Erster Blick auf die Daten

```bash
sudo apt install -y minicom
minicom -D /dev/wordclock -b 115200
```

Es müssen laufend Zeilen erscheinen wie `sk6812_refresh: start leds=114`. Beenden mit
`Strg-A`, dann `X`.

**Kommt nichts:** RX und TX vertauscht — Adapter-RX gehört an `H6` Pin 3.
**Kommt Zeichensalat:** falsche Baudrate, es müssen 115200 sein.

---

## 5. Den Mitschnitt einrichten

Die Dateien liegen im Projekt unter `tools/logger/`. Kopiere sie **von deinem Mac**
aus — der Branch `pwa-decoupling` ist noch nicht auf GitHub, ein `git clone` auf dem Pi
holt sie also nicht:

```bash
cd ~/Documents/GitHub/wordclock24h
scp tools/logger/serial-logger.py \
    tools/logger/wordclock-logger.service \
    tools/logger/wordclock-logger.logrotate \
    pi@wordclock-pi.local:/tmp/
```

Dann auf dem Pi an ihren Platz bringen:

```bash
sudo mkdir -p /opt/wordclock /var/log/wordclock
sudo mv /tmp/serial-logger.py /opt/wordclock/
sudo mv /tmp/wordclock-logger.service /etc/systemd/system/
sudo mv /tmp/wordclock-logger.logrotate /etc/logrotate.d/wordclock
sudo chmod +x /opt/wordclock/serial-logger.py
```

Dienst starten:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now wordclock-logger
systemctl status wordclock-logger
```

**Richtig sieht so aus:** `active (running)`. Und:

```bash
tail -f /var/log/wordclock/serial.log
```

Zeilen mit Zeitstempel davor, etwa:

```
2026-09-30T22:14:07.123+02:00   sk6812_refresh: start leds=114 nextbuf=1
```

Mit `Strg-C` beenden.

---

## 6. Zugang für mich einrichten

Auf **deinem Mac**, einmalig:

```bash
ssh-keygen -t ed25519 -f ~/.ssh/wordclock_pi -N "" -C "wordclock-logger"
ssh-copy-id -i ~/.ssh/wordclock_pi.pub pi@wordclock-pi.local
```

Prüfen, dass es ohne Passwort geht:

```bash
ssh -i ~/.ssh/wordclock_pi -o BatchMode=yes pi@wordclock-pi.local "echo OK"
```

Der Logger läuft als `root`, die Logdatei gehört `root`. Damit ich sie ohne Passwort
lesen kann, gib der Gruppe Leserechte:

```bash
sudo chmod 0644 /var/log/wordclock/serial.log
sudo sed -i 's/^    notifempty/    notifempty\n    create 0644 root root/' /etc/logrotate.d/wordclock
```

Danach funktioniert vom Mac aus:

```bash
./tools/logger/log.sh tail 50
./tools/logger/log.sh stats
```

### Damit ich den Logger selbst nachziehen kann

Das Skript gehört `pi` und lässt sich per `scp` ersetzen. Für den Neustart des Dienstes
brauche ich aber Rechte. Eine eng begrenzte Regel — **nur dieser eine Dienst**, keine
anderen Befehle:

```bash
sudo tee /etc/sudoers.d/wordclock-logger >/dev/null <<'EOT'
pi ALL=(root) NOPASSWD: /usr/bin/systemctl restart wordclock-logger, \
                        /usr/bin/systemctl start wordclock-logger, \
                        /usr/bin/systemctl stop wordclock-logger, \
                        /usr/bin/systemctl kill -s HUP wordclock-logger
EOT
sudo chmod 440 /etc/sudoers.d/wordclock-logger
sudo visudo -c
```

Die letzte Zeile muss `parsed OK` melden. Heisst dein Benutzer anders, ersetze `pi`.

Ohne diese Regel musst du nach jeder Änderung am Logger selbst
`sudo systemctl restart wordclock-logger` ausführen.

Heisst dein Benutzer anders oder hat der Pi eine feste IP, setze das einmal:

```bash
export LOG_HOST=192.168.1.xxx
export LOG_USER=deinname
```

---

## 7. Was auf dich zukommt — Datenmenge

**Gemessen am 30.09.2026 im Ruhezustand: rund 31 Byte/s, also etwa 2,7 MB pro Tag.**
Das ist unkritisch. Die Rotation (14 Tage, komprimiert, maximal 200 MB je Datei) hat
reichlich Luft.

```bash
./tools/logger/log.sh stats
```

Die Ausgabe nennt Byte pro Sekunde und hochgerechnet MB pro Tag.

> **Korrektur einer früheren Annahme:** Hier stand zunächst, es seien mehrere hundert MB
> pro Tag zu erwarten. Diese Schätzung stützte sich auf die Angabe „Minuten-LEDs mit
> 64 Hz" aus `REVIEW.md` und lag um **Faktor 225** daneben. Im Ruhezustand erscheinen
> rund 0,1 `sk6812_refresh`-Paare je Sekunde, nicht 64. Die Firmware protokolliert nur
> bei Ereignissen. Während Tickern und Animationen kann die Rate deutlich höher liegen
> — das ist noch nicht gemessen.

---

## 8. Die Tests, Schritt für Schritt

**Setze vor und nach jedem Schritt eine Marke.** Sie erscheint im Mitschnitt mit einem
`#` und macht hinterher auffindbar, wann du was getan hast:

```bash
./tools/logger/log.sh mark "Test M2 -- Uhr wird gleich neu gestartet"
```

### Erst prüfen: welche Firmware läuft?

Auf der Uhr läuft derzeit **STM 3.2.4**. Unser Stand ist **3.2.6**. Das entscheidet,
welche Tests heute schon gehen:

| Test | Braucht 3.2.6? |
|---|---|
| T1 Temperatursensor | nein |
| T2 Reset-Ursache (M2) | nein |
| T3 DMA-Stillstand (M3) | nein |
| T4 Blockaden finden | nein |
| T5 Restore-Lücke (AK6) | nein — prüft gerade den **alten** Stand |
| T6 Icon-Freeze (AK7) | **ja**, die Instrumentierung kam erst mit 3.2.6 |

---

### T1 — Der Temperatursensor (sofort, ohne Eingriff)

Schon jetzt sichtbar: Die Uhr meldet durchgehend `DS18xxx temperature: 127.5`. Das ist
**kein Messwert, sondern der Fehlercode** — intern 255, geteilt durch 2 ergibt 127,5.
Der DS18B20 liefert also gar nichts.

```bash
./tools/logger/log.sh grep "DS18xxx temperature" 40
./tools/logger/log.sh grep "RTC temperature" 10
```

**Erwartung:** DS18xxx immer `127.5`, RTC ein plausibler Wert. Dann ist der I2C-Bus in
Ordnung und nur der 1-Wire-Pfad gestört.

**Was das eingrenzt:** Ein konstanter Fehlerwert spricht für „Sensor nicht gefunden",
nicht für die sporadischen Lesefehler, die wegen der fehlenden CRC-Prüfung zu erwarten
wären. Prüfe die Lötstelle von `U1` (TO-92, dreibeinig) und ob der 4,7-kΩ-Pull-up `R5`
bestückt ist. Schwankt der Wert dagegen, ist es das bekannte offene Thema 1.

---

### T2 — Reset-Ursache (M2)

Der beste Einzelschritt, weil er die ganze Hänger-Frage in drei Richtungen aufteilt.

```bash
./tools/logger/log.sh mark "T2: Reset-Ursache, Uhr wird neu gestartet"
```

Uhr kurz stromlos machen, wieder einschalten, eine Minute warten:

```bash
./tools/logger/log.sh grep "reset" 60
```

**Auswertung:**

| Im Log | Bedeutung |
|---|---|
| `Watchdog reset` | Ein blockierender Pfad. Bestätigt die Software-Spur |
| `Software reset` mit `fatal fault detected` davor | HardFault — stützt die Rekursions-These |
| `Software reset` ohne Fault | Reset über die Web-Oberfläche, harmlos |
| kein Flag | Versorgung oder Mechanik — **dann ist Befund L9 dran** |

Der eigentliche Wert kommt später: Wenn die Uhr **von selbst** hängt, steht die Ursache
danach im Mitschnitt, ohne dass du dabei sein musst.

---

### T3 — DMA-Stillstand ausschliessen (M3)

Ein Lauf über den gesamten Bestand, kein Eingriff:

```bash
./tools/logger/log.sh grep "sk6812_refresh: waiting" 50
```

**Kein Treffer** über mehrere Tage ⇒ ein DMA-Stillstand ist **widerlegt**, und die
Entscheidung gegen einen pauschalen DMA-Fix ist belegt statt vermutet.
**Treffer** ⇒ der Stillstand ist bewiesen, samt `pos`, `off` und `pause`.

Lass das ein paar Tage laufen, bevor du es auswertest. Ein einzelner Tag sagt wenig.

---

### T4 — Blockaden sichtbar machen

Der Test, den es ohne Mitschnitt gar nicht geben kann.

```bash
./tools/logger/log.sh gaps 500
```

Zeigt jede Pause über 500 ms, mit der Zeile davor und danach.

**Eine Lücke allein beweist noch keine Blockade.** Die Firmware protokolliert nur bei
Ereignissen — im Ruhezustand vergehen regelmässig zehn Sekunden und mehr ohne eine
einzige Zeile. Aussagekräftig ist eine Lücke erst dort, wo eine Fortsetzung erwartet
wird:

- zwischen `sk6812_refresh: start` und dem zugehörigen `dma started` — die beiden
  gehören unmittelbar zusammen
- zwischen `main: call display_clock` und `main: display_clock returned`
- über einer Minutengrenze hinweg, wo `show_time` fällig wäre

Der Watchdog schlägt bei 20 s zu. Eine Lücke darüber mit anschliessender Startsequenz
ist ein Reset; eine Lücke zwischen zusammengehörigen Zeilen ist eine überstandene
Blockade.

Gezielt provozieren, jeweils mit Marke:

```bash
./tools/logger/log.sh mark "T4a: Dimmkurve speichern"
# in der PWA: Anzeige -> Dimmkurve -> speichern
./tools/logger/log.sh gaps 200 3000
```

**Erwartung nach Befund L7:** rund 240 ms je Kurvenschreibvorgang, und die PWA sendet
sechzehn davon. Findest du dort mehrere Sekunden am Stück, ist Kernbefund 3 aus
`REVIEW.md` gemessen statt gerechnet.

---

### T5 — Die Restore-Lücke (AK6)

Prüft den **jetzigen** Stand 3.2.4, also den Zustand **vor** dem Fix:

```bash
./tools/logger/log.sh mark "T5: Wetter-Ticker laeuft, jetzt Dimmkurve speichern"
```

In der PWA den Wetter-Ticker starten und **währenddessen** die Dimmkurve speichern.

**Erwartung auf 3.2.4:** Das Display bleibt danach bis zu 60 Sekunden dunkel. Genau das
behebt 3.2.5. Nach dem Flashen auf 3.2.6 denselben Ablauf wiederholen — bleibt das
Display an, ist der Fix am Gerät bestätigt.

---

### T6 — Icon-Freeze (AK7) — **erst nach dem Flashen auf 3.2.6**

```bash
./tools/logger/log.sh mark "T6: Icon-Overlay laeuft, jetzt Display ausschalten"
```

Ein Icon-Overlay starten und **während es läuft** das Display ausschalten.

```bash
./tools/logger/log.sh grep "icon_freeze" 20
```

| Im Log | Bedeutung |
|---|---|
| `icon_freeze: ENTER do_icon=1 power=0` | **Der Freeze ist belegt.** Dann folgt ein Fix |
| nichts | **Widerlegt.** Die Instrumentierung wird wieder ausgebaut |

Das ist der einzige Zweck, zu dem 3.2.6 gebaut wurde.

---

## 9. Wenn etwas nicht geht

| Symptom | Ursache |
|---|---|
| `/dev/wordclock` fehlt | udev-Regel: IDs aus `lsusb` stimmen nicht |
| Dienst läuft, Datei bleibt leer | RX/TX vertauscht — Adapter-RX an `H6` Pin 3 |
| Zeichensalat | falsche Baudrate, es sind 115200 |
| `Permission denied` beim Abruf | SSH-Key nicht hinterlegt, Schritt 6 |
| Log wächst nicht mehr | `systemctl status wordclock-logger`, dann `journalctl -u wordclock-logger -n 50` |
| SD-Karte voll | Rotation greift nicht: `sudo logrotate -f /etc/logrotate.d/wordclock` |

Nach jedem Neustart des Pi läuft der Dienst von selbst wieder an. Wird der Adapter
abgezogen, wartet der Logger und öffnet den Port neu, sobald er wieder da ist — der
Dienst muss nicht angefasst werden.
