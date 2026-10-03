# Design — Den Auslieferungspfad des ESP entlasten

**Datum:** 2026-10-03 (Momentaufnahme, DIR-006)
Gehört zu `requirements.md` im selben Verzeichnis.

## Was für diese Spec am Quelltext nachgeprüft wurde

Damit der Umsetzer weiss, was belegt ist und was er selbst nachsehen muss:

| Nachgesehen | Ergebnis |
|---|---|
| `http.cpp:1151-1203` (`http_send_fs_file`) | Schleifenform, 256-Byte-Puffer, fehlender `break`, fehlendes `yield()` — wie in L128/L129 beschrieben |
| `http.cpp:1531-1562` (`http_find_stored_app_asset_filename`) | Drei Rückkehrwege bei `:1543`, `:1557`, `:1561`, keiner mit `LittleFS.end()` |
| `http.cpp` gesamt, `LittleFS.begin`/`end` | **28 `begin`, 27 `end`** — nachgezählt, deckt sich mit L129 |
| `http.cpp:1592-1708` (`http_app`) | `app_complete` wird bei `:1647` immer berechnet, aber nur bei `:1695` und `:1700` gelesen, beide Male unter `is_pwa_index`. Einziger Aufrufer von `http_send_fs_file` ist `:1702` |
| `http.cpp:11872` | **Einzige** Aufrufstelle von `http_app` |
| `http.cpp:112-113`, `:1016-1056` | `MAX_HTTP_RESPONSE_LEN` ist **1024**; `http_response[1025]` ist ein statischer Puffer, `http_flush()` setzt `http_response_len` auf 0 |
| `http.cpp:743-772` | `http_json_send_uint_field (key, unsigned long)` existiert und setzt das führende Komma selbst |
| `http.cpp:10984-10990` (`http_api_device_ready`) | Antwortet `{"ok":true,"ready":true}`, **ohne** Dateisystem-Mount und **ohne** STM-Kommando |
| `http.cpp:12417-12430` | `setNoDelay(1)` steht nach der 250-ms-Wartezeit auf den ersten Request, vor dem Lesen der Requestzeile |
| `tools/`-Bestand | `device_ready` wird von `smoke-device.sh:39,114,134`, `snapshot-device.sh:78,83` und `flash-stm.sh:115` benutzt — alle prüfen auf `200` und gültiges JSON, keiner auf eine feste Feldmenge |

**Nicht nachgesehen und deshalb Auftrag an den Umsetzer:** das Verhalten von
`File::read` im Core 3.1.2 (`LittleFS.h:411-422`). L129 sagt, es liefert bei
`lfs_file_read < 0` eine **0**. Das ist zu **prüfen**, nicht zu glauben — genau diese
Sorte Annahme hat in diesem Projekt schon zweimal eine Prüfung wertlos gemacht (L123,
L100).

---

## Lösungsweg

### Teil 1 — Blockgrösse 1024 und Nagle nur für die Dateiauslieferung

**Blockgrösse.** `http.cpp:1184` von 256 auf 1024. Die Rechnung:

| Variante | Schreibvorgänge | Frames bei `TCP_MSS = 536` |
|---|---|---|
| heute: 256 Byte, Nagle aus | 505 | **505** (jeder Schreibvorgang ein Paket) |
| 1024 Byte, Nagle aus | 127 | 126 × (536 + 488) + 1 = **253** |
| 1024 Byte, Nagle an | 127 | **241 bis 242** (Rest wird mit dem nächsten Block zu vollen Segmenten verschmolzen) |

Der grosse Schritt ist die Blockgrösse: 505 → 253. Nagle holt weitere zwölf Frames.
**Beide Varianten liegen über dem Boden von 241**, den `TCP_MSS = 536` setzt
(129'313 / 536 = 241,3).

**Woher der Puffer kommt — und warum nicht vom Stack.** Ein `uint8_t buf[1024]` an
dieser Stelle kostet 768 Byte mehr Stack auf dem tiefsten Pfad des Programms, und der
`cont`-Stack des ESP8266 ist 4 KB gross. Ausgerechnet bei einem Befund, dessen
Verdacht auf Speichermangel lautet, 768 Byte Stack nachzulegen, wäre der falsche Weg.
Ein neuer statischer 1-KB-Puffer wiederum verkleinert den freien Heap dauerhaft um
1 KB — bei rund 12'400 Byte ist das 8 %.

**Deshalb:** Der bereits vorhandene statische Puffer `http_response` wird als
Lesepuffer mitbenutzt. Er ist 1024 Byte gross (`MAX_HTTP_RESPONSE_LEN`, `:112`), und
`http_send_fs_file` ruft bei `:1180` **vor** der Schleife `http_flush()` auf — danach
ist `http_response_len` gleich 0 und der Puffer frei. Im Schleifenrumpf wird kein
`http_send` gerufen, und nach der Schleife auch nicht mehr (die Verbindung wird
geschlossen). Kosten: **null Byte**, weder Stack noch Heap.

Das ist eine Aliasnutzung und gehört deshalb kommentiert — im Code, nicht nur hier.
Der Kommentar nennt die Bedingung, unter der sie gilt: *kein `http_send` zwischen
`http_flush()` und dem Ende der Schleife*. Wer das später bricht, muss es sehen.

Vorgesehene Form (der Umsetzer darf abweichen, solange AK12 und AK15 gelten):

```c
/* Lesepuffer ist bewusst der bereits vorhandene statische Antwortpuffer:
 * 1024 Byte, bei :1180 geflusht und bis zum Verbindungsende ungenutzt.
 * Ein eigener Puffer auf dem Stack kostete 768 Byte vom 4-KB-cont-Stack,
 * ein eigener statischer Puffer 1 KB vom ohnehin knappen Heap.
 * BEDINGUNG: zwischen http_flush() und dem Schleifenende darf kein
 * http_send() stehen, sonst ueberschreiben sich beide Nutzungen.
 */
http_client.setNoDelay (0);                     /* Nagle an: Restsegmente verschmelzen */

while (fp.available ())
{
    int len = fp.read ((uint8_t *) http_response, MAX_HTTP_RESPONSE_LEN);

    if (len <= 0)                               /* Lesefehler ODER unerwartetes Dateiende */
    {
        Serial.printf ("- fs read error: %s\r\n", filename);
        Serial.flush ();
        rtc = 2;                                /* Kopfzeilen sind raus: kein Fallback mehr */
        break;
    }

    if (http_client.write ((const uint8_t *) http_response, (size_t) len) != (size_t) len)
    {
        Serial.printf ("- fs write short: %s\r\n", filename);
        Serial.flush ();
        rtc = 2;
        break;
    }

    yield ();
}
```

**`setNoDelay` — die Antwort auf „ist das für die Dateiauslieferung nötig".** Nein, und
es schadet dort. Aber die Zeile bei `:12430` bleibt **unverändert**: Sie wird für jeden
Request gesetzt, bevor der Pfad überhaupt bekannt ist, und für die kleinen API-Antworten
ist „sofort raus" richtig — die PWA pollt, und 200 ms Verzögerung durch das Zusammenspiel
von Nagle und Delayed-ACK wären dort spürbar. Umgeschaltet wird deshalb **lokal in
`http_send_fs_file`**, direkt vor dem Senden der Kopfzeilen.

Kein Zurücksetzen nötig: `:12430` setzt den Wert bei jeder neuen Verbindung explizit,
und die Verbindung wird am Ende der Auslieferung mit `http_client.stop()` geschlossen.
Der Zustand kann also nicht in den nächsten Request hinüberlecken — **das ist der
Grund, warum die pauschale Zeile stehenbleiben darf.**

Nagle blockiert hier nichts: Solange noch ein voller Block ansteht, hat lwIP immer ein
volles MSS zu senden; zurückgehalten wird höchstens das letzte Teilsegment, und das
schiebt `http_client.flush()` nach der Schleife hinaus.

### Teil 2 — Die drei Defekte

**(a) Leseabbruch statt Endlosschleife.** Im Entwurf oben enthalten. Drei Punkte sind
tragend:

1. **`int` statt `size_t`.** Liefert der Core im Fehlerfall −1 statt 0, würde `size_t`
   daraus 4'294'967'295 machen und `len > 0` wäre wahr — aus der Endlosschleife würde
   ein Schreibvorgang über 4 GB aus einem 1-KB-Puffer. Der vorzeichenbehaftete Typ
   fängt **beide** Core-Verhalten ab, und damit hängt die Korrektheit nicht an der
   ungeprüften Aussage aus L129.
2. **`break` statt `continue`.** Nach einem Lesefehler ist die Datei nicht
   wiederherstellbar; Weiterprobieren ist die Endlosschleife unter anderem Namen.
3. **`yield()` je Durchlauf.** Kostet nichts und sichert den Soft-Watchdog auch dann,
   wenn `write()` einmal ohne Blockieren zurückkehrt.

**Der Abbruch bricht eine Zusage** — die Kopfzeilen haben `Content-Length` genannt, die
Nutzlast bleibt kürzer. Mehr ist nicht möglich: Eine Fehlerseite lässt sich nach den
Kopfzeilen nicht mehr senden. Der Client sieht einen abgebrochenen Download, und im
STM-Logring steht eine Zeile mit dem Dateinamen. Genau **eine** Zeile, nur im
Fehlerfall — die STM-UART ist die einzige des ESP.

**`rtc = 2` und warum es nötig ist.** Heute kann dieser Pfad nicht auftreten (statt
abzubrechen läuft die Schleife endlos), nach der Änderung schon. Bliebe `rtc` auf 0,
liefe `http_app` bei `:1710` in den `! sent`-Zweig und schriebe eine **zweite** HTTP-
Antwort in dieselbe, bereits mit `stop()` geschlossene Verbindung. Die bestehende
Prüfung `if (sent) { return 0; }` bei `:1705` fängt jeden Wert ungleich 0 ab — es
genügt also, `rtc` zu setzen. Die drei Bedeutungen gehören an die Funktion kommentiert:
**0 = nichts gesendet, 1 = vollständig, 2 = abgebrochen, Verbindung verbraucht.**
`http_send_fs_file` hat genau einen Aufrufer (`:1702`), also ist die Erweiterung
vollständig überblickbar.

**(b) Mounts paarig machen.** `http_find_stored_app_asset_filename` bekommt einen
einzigen Ausstieg: Ergebnis in eine lokale Variable, am Ende **ein** `LittleFS.end()`,
**ein** `return`. Kein neues Hilfskonstrukt, keine Zählmechanik (Begründung unter
„Verworfene Alternativen"). Danach stimmen die Zahlen über die Datei: 28 zu 28.

**(c) Den folgenlosen Mount entfernen.** Der Aufruf bei `:1647` wandert in den Zweig,
der ihn braucht. `app_complete` wird nur unter `is_pwa_index` gelesen; für
`/app/app.js` entfallen damit ein Mount, zwölf Dateiprüfungen und ein Unmount —
unmittelbar vor der Übertragung. Vorgesehene Form:

```c
bool app_complete = false;

if (is_pwa_index)
{
    app_complete = http_app_installation_complete ();
}
```

Die beiden Lesestellen bleiben unverändert: `:1695` prüft `is_pwa_index && ! app_complete`,
`:1700` prüft `! is_pwa_index || app_complete`. Bei `is_pwa_index == 0` entscheidet in
beiden Fällen bereits der erste Operand, der neue Startwert `false` ändert also nichts.
**Das ist der Nachweis, dass die Verschiebung verhaltensneutral ist** — er gehört in den
Bericht des Umsetzers, nicht nur in diese Spec.

**Eine unbequeme Nebenwirkung, die genannt sein will.** Heute bleibt das Dateisystem
wegen (b) zwischendurch gemountet; nach der Reparatur wird in `http_send_fs_file`
wieder **wirklich** gemountet, unmittelbar vor der Übertragung. Ob die Heap-Lage am
Startpunkt der Übertragung dadurch besser oder nur anders wird, ist **nicht
vorhersagbar** — und genau deshalb steht M4 in dieser Spec und wird **vor** dem Umbau
geflasht. Ohne Vorher-Wert wäre der Nachher-Wert bedeutungslos.

### Teil 3 — M4: `free_heap` und `max_free_block`

**Wohin.** In `/api/device_ready` (`http.cpp:10984`). Begründung:

- Es ist die **billigste** lesende Antwort im ganzen Gerät: zwei Literale, kein
  Dateisystem-Mount, kein STM-Kommando, keine Abfrage beim Update-Server.
- Eine Messung, die selbst Speicher bewegt, misst sich selbst. `/api/fs_info` mountet
  LittleFS, `/api/update_status` holt vier Dateien vom Update-Server und baut einen
  3-KB-`String` — beide verfälschen genau die Grösse, um die es geht.
- Der Endpunkt wird von `smoke-device.sh`, `snapshot-device.sh` und `flash-stm.sh`
  bereits benutzt; alle drei prüfen auf `200` und gültiges JSON, keiner auf eine feste
  Feldmenge. Zusätzliche Felder brechen nichts.
- Die PWA liest ihn für die Wiederverbindung und ignoriert unbekannte Felder.

**Wie.**

```c
static int
http_api_device_ready ()
{
    unsigned long free_heap      = ESP.getFreeHeap ();
    unsigned long max_free_block = ESP.getMaxFreeBlockSize ();

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ready\":true"));
    http_json_send_uint_field (FS("free_heap"), free_heap);
    http_json_send_uint_field (FS("max_free_block"), max_free_block);
    http_send (FS("}"));
    http_flush ();
    return 0;
}
```

**Beide Werte werden zuerst eingelesen, dann ausgegeben.** `http_json_send_uint_field`
baut intern ein `String`-Objekt; würde erst gesendet und dann gemessen, läge die
Messung hinter einer eigenen Heap-Anforderung. Die Reihenfolge ist kein Schönheitsfehler
— sie ist der Unterschied zwischen Messung und Messrauschen.

`http_json_send_uint_field` setzt das führende Komma selbst (`:743-749`), deshalb endet
das Literal ohne Komma und ohne `}`.

**Warum zwei Felder und nicht eines.** `free_heap` allein beantwortet die Frage nicht.
Die Vermutung aus L125 lautet „eine Pufferanforderung scheitert"; die scheitert auch
bei reichlich freiem Speicher, wenn er zerstückelt ist. **`max_free_block` ist die
Zahl, an der das sichtbar wird.** Fällt sie deutlich unter `free_heap`, ist der Heap
fragmentiert — und das ist die einzige Beobachtung, die die Vermutung stützen oder
schwächen kann, ohne SDK-Quelltext.

**Grenze des Messmittels, die jeder kennen muss, bevor er misst.** Der ESP bedient
**eine** Verbindung nach der anderen — `http_client` ist ein einzelnes globales Objekt,
und der Auslieferungspfad läuft am Stück. Eine parallele Abfrage von `device_ready`
**während** der Übertragung von `app.js` ist nicht möglich: Sie wird erst bedient, wenn
die Datei durch ist. Wer es trotzdem versucht, misst eine Wartezeit und hält sie für
einen Hänger. **Gemessen wird vorher und nachher, nicht dazwischen.**

### Wie die Framezahl nachgewiesen wird

Direkt, auf der Leitung, mit Bordmitteln von macOS. Auf dem Rechner, der auch `curl`
ausführt — sonst sieht die Aufzeichnung den Verkehr nicht.

```sh
# 1. Schnittstelle bestimmen (meist en0)
route get "$DEVICE_HOST" | awk '/interface:/ {print $2}'

# 2. Mitschnitt starten (braucht sudo; nur Kopfdaten, keine Nutzlast)
sudo tcpdump -i en0 -n -s 96 -w /tmp/appjs.pcap "host $DEVICE_IP and tcp port 80"

# 3. In einem zweiten Fenster: genau EIN Abruf
curl -s -o /dev/null -w '%{http_code} %{size_download} %{time_total}\n' \
     "http://$DEVICE_HOST/app/app.js"

# 4. Mitschnitt beenden, Frames MIT NUTZLAST vom Geraet zaehlen
tcpdump -r /tmp/appjs.pcap -n \
  "src host $DEVICE_IP and tcp port 80 and (((ip[2:2] - ((ip[0]&0x0f)<<2)) - ((tcp[12]&0xf0)>>2)) != 0)" \
  | wc -l
```

Der Filter in Schritt 4 zählt nur Pakete, die tatsächlich Daten tragen — reine
Quittungen, Verbindungsauf- und -abbau fallen heraus. **Erwartet: 241 bis 253.**
Vorher-Wert ist 505 aus L128 und wird nicht neu erhoben (`requirements.md`, AK7).

Die Zahl kann durch Wiederholungen leicht über 253 liegen; das ist der Grund für die
Schwelle 280 statt 253. Liegt sie dagegen bei 500, hat die Blockgrösse nicht gewirkt —
dann ist entweder die falsche Firmware auf dem Gerät (`/api/update_status` nennt die
laufende ESP-Version) oder der Umbau ist nicht dort angekommen, wo er hingehört.

**Ersatzweg ohne `sudo`, falls die Aufzeichnung nicht möglich ist:** fünf Abrufe mit
`-w '%{size_download} %{time_total} %{speed_download}\n'` und Vergleich mit dem in
Runde A notierten Vorher-Wert. Das ist ein **Indiz, kein Nachweis** — die Dauer hängt
auch an der Funkqualität. Wird dieser Weg genommen, steht im Bericht, dass AK7 nicht
erfüllt, sondern ersetzt wurde.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `ESP8266/ESP-uclock/http.cpp` — `http_api_device_ready` (`:10984`) | Zwei Zahlenfelder `free_heap`, `max_free_block` | `esp-developer` |
| `ESP8266/ESP-uclock/http.cpp` — `http_send_fs_file` (`:1151-1203`) | Blockgrösse 1024 über `http_response`, Leseabbruch, Kurzschreibabbruch, `yield()`, `setNoDelay(0)`, `rtc = 2` | `esp-developer` |
| `ESP8266/ESP-uclock/http.cpp` — `http_find_stored_app_asset_filename` (`:1531-1562`) | Ein Ausstieg, ein `LittleFS.end()` | `esp-developer` |
| `ESP8266/ESP-uclock/http.cpp` — `http_app` (`:1647`) | `http_app_installation_complete()` nur noch unter `is_pwa_index` | `esp-developer` |
| `ESP8266/ESP-uclock/version.h` | `ESP_VERSION` je Runde einmal anheben | `release-engineer` (R4) |

**`http.cpp` ist UTF-8** (`CLAUDE.md`, 37 Nicht-ASCII-Bytes), nicht ISO-8859-1. Wer sie
mit einem `latin-1`-Patcher anfasst, beschädigt sie — in diesem Projekt ist das in
beide Richtungen schon passiert. Neue Kommentare in Umschrift (`Geraet`, `waehrend`)
schaden in keiner der beiden Welten; gefährlich ist die **Kodierungsannahme beim
Patchen**, nicht die Sprache.

---

## Prüfung gegen die Architektur-Checkliste

Siehe `knowledge/architecture-checklist.md`. Beantwortet, nicht abgehakt.

**Proper architecture**

> Die PWA bleibt parallel zur Legacy-Oberfläche: Beide laufen über denselben
> Auslieferungspfad, geändert wird die Blockgrösse, nicht die Zuordnung. Die
> Legacy-Oberfläche ist in AK6 als Prüfpunkt benannt, weil sie die Stabilitäts-Referenz
> ist. Die Wetter-Endpunkte werden nicht berührt; der entfernte Legacy-Bypass kommt
> nicht zurück. Die Restore-Bedingung um `pending_weather_ticker_restore` liegt im STM
> und wird nicht angefasst. Assets werden weiterhin **ausschliesslich** als `.gz`
> ausgeliefert — es kommt kein Plain-Fallback dazu, und die Prüfung „existiert **und**
> Grösse > 0" (`http_fs_file_exists_and_nonempty`, `:1163`) bleibt genau dort stehen,
> wo sie steht; eine leere `.gz` führt zum Weissbild, das ist real passiert.
> **Zur richtigen Schicht:** Der Absturz liegt auf dem ESP, und die Änderung liegt auf
> dem ESP. Das ist ausdrücklich der Unterschied zu B15/L127, das dasselbe Symptom in
> der PWA angeht — **Faktor 1,12 gegen 2,10**, nachgemessen am 03.10.2026 (506 Frames
> heute, 453 nach der Auslagerung, 241 bis 253 nach dieser Spec). Die Sprachauslagerung
> ist aus eigenen Gründen richtig, nicht als Umgehung eines ESP-Problems.

**Scalable systems**

> **STM-Kommandos: null.** Kein geänderter Pfad erzeugt ein Kommando an den STM;
> `device_ready` hat auch vorher keines erzeugt.
> **Byte je Minute auf der UART: unverändert im Normalbetrieb.** Hinzu kommt genau eine
> Zeile von rund 40 Byte, und zwar ausschliesslich im neuen Fehlerpfad (Leseabbruch).
> Der RX-Ring ist 256 Byte und verwirft still; deshalb steht in AK11, dass ein
> `app.js`-Abruf im Logring nicht mehr Zeilen erzeugen darf als vorher.
> **Watchdog:** Der Pfad wird kürzer, nicht länger. Heute ist der Fehlerfall eine
> Endlosschleife ohne `yield()` — also der sichere Soft-Watchdog-Reset; nachher bricht
> er nach einem Durchlauf ab. Im Normalfall sinkt die Zahl der Schleifendurchläufe von
> 505 auf 127, jeder mit `yield()`.
> **Hauptloop:** Die Auslieferung blockiert ihn weiterhin für die Dauer der
> Übertragung (rund 1,4 s für `app.js`) — das ändert diese Spec nicht und behauptet es
> auch nicht. Sie wird eher kürzer, weil 378 Schreibvorgänge entfallen. Keine
> EEPROM-Zugriffe, also keine 16-ms-Blöcke.
> **Polling:** Die PWA fragt nicht öfter; M4 fügt kein Polling hinzu, die Felder werden
> von Hand abgefragt.
> **Harte Grenzen:** Die Blockgrösse ist an `MAX_HTTP_RESPONSE_LEN` gebunden und nicht
> getrennt hartkodiert — wächst dieser Puffer einmal, wächst der Block mit, und die
> Aliasnutzung bleibt korrekt.

**Secure by design**

> Kein `innerHTML`, keine PWA-Änderung — der Weg für Fremdinhalt in eine Seite wird
> nicht berührt. `free_heap` und `max_free_block` sind Betriebszahlen, keine
> Geheimnisse; `device_ready` ist bereits ohne Anmeldung aus dem LAN lesbar und meldet
> dort heute schon Betriebsbereitschaft. Die neue Logzeile enthält **nur** den
> Dateinamen aus dem Asset-Nachschlag, keine Requestdaten, keine Zugangsdaten und
> keinen Update-Host — die Falle aus L115 (Geheimnisse im Logbuch) wird hier nicht neu
> aufgemacht. Es entsteht kein Endpunkt, der Konfiguration schreibt.
> **Ein Nebenaspekt, der ehrlich genannt sein will:** Wer Heap-Zahlen aus dem LAN lesen
> kann, kann einen Speichermangel beobachten. Das ist in einem Gerät, dessen
> destruktive Endpunkte ohnehin ohne Anmeldung erreichbar sind, kein neuer Hebel — aber
> es ist eine bewusste Entscheidung und keine Unachtsamkeit.

**Stable & reliable**

> **Jeder Fehler wird ausgewertet.** Heute wird der Rückgabewert von `fp.read()` nur
> gegen 0 geprüft und der von `http_client.write()` gar nicht; nachher führen beide zu
> einem Abbruch mit Logzeile. Keine stille Verwerfung, kein leerer `catch` (C hat
> keinen).
> **Definierter Zustand nach Abbruch:** Datei geschlossen (`fp.close()` läuft auf
> jedem Pfad, `File` ist ein `shared_ptr`-Objekt — L129 stellt ausdrücklich fest, dass
> `max_open_files: 5` hier kein Problem ist), Dateisystem per `LittleFS.end()`
> entladen, Verbindung gestoppt, Rückgabewert 2 statt 0, damit der Aufrufer **keine**
> zweite Antwort in dieselbe Verbindung schreibt. Genau dieser Punkt ist AK15.
> **Gesetztes Flag auf jedem Pfad aufgelöst:** Das ist in dieser Änderung der Mount.
> `http_find_stored_app_asset_filename` bekommt einen Ausstieg, damit `begin` und `end`
> auf allen drei Wegen paarig sind — vorher 28 zu 27 über die Datei.
> **Keine Erfolgsmeldung ohne Ergebnis:** Eine abgebrochene Auslieferung meldet nicht
> 1. Der abbrechende Client sieht weniger Bytes als `Content-Length` — das ist die
> einzige verbleibende Unschärfe, und sie ist nach gesendeten Kopfzeilen technisch
> nicht auflösbar.

---

## Verworfene Alternativen

| Verworfen | Warum |
|---|---|
| **`ip=hb2f` im `ESP_FQBN`** (MSS 1460 → 89 Frames) | Braucht mehr lwIP-Speicher; frei sind rund 12'400 Byte für Heap und Stack zusammen. L128 nennt es ausdrücklich „nicht empfohlen". Bei einem Befund, dessen Verdacht Speichermangel ist, mehr Speicher zu verbrauchen, ist die falsche Richtung |
| **`TCP_WRITE_FLAG_MORE` setzen** | Über die öffentliche `WiFiClient`-Schnittstelle nicht erreichbar; es hinge an einem Eingriff in den Core. Nagle erreicht fast dasselbe (241 gegen 253 Frames) ohne Fremdeingriff |
| **Eigener Stack-Puffer `uint8_t buf[1024]`** | 768 Byte mehr auf dem 4-KB-`cont`-Stack, auf dem tiefsten Pfad des Programms, bei einem Speicherbefund. Falsche Richtung |
| **Neuer statischer 1-KB-Puffer** | Verkleinert den freien Heap dauerhaft um rund 8 % der 12'400 Byte. Dieselbe falsche Richtung, nur permanent |
| **Blockgrösse 2048 oder 4096** | Bringt nichts: `TCP_MSS = 536` ist der Boden, und 1024 erreicht ihn bereits auf zwölf Frames genau. Mehr Puffer wäre Kosten ohne Gegenwert |
| **`setNoDelay(1)` bei `:12430` ersatzlos entfernen** | Beträfe auch die kleinen API-Antworten, die die PWA pollt. Nagle und Delayed-ACK können dort 200 ms kosten. Das lokale Umschalten in der Auslieferungsfunktion ist genauer und reversibel |
| **Zählende Mount-Klammer (`depth`-Zähler) um `LittleFS.begin`/`end`** | Es gibt in `http.cpp` 28 rohe `begin`- und 27 rohe `end`-Stellen. Eine zählende Klammer, die mit rohen Aufrufen gemischt wird, hängt das Dateisystem unter einem Halter aus — ein stiller Fehler in genau der Datei, durch die jeder Request läuft. Die drei Stellen des Auslieferungspfads sind heute **nicht** verschachtelt; sie brauchen keine Zählung, sondern einen sauberen Ausstieg |
| **Eine Mount-Klammer um den einzigen `http_app`-Aufruf (`:11872`)** | Wäre dreimal Mount auf einmal reduziert statt zweimal, und ist eine Dreizeilenänderung. Verworfen, weil `http_app` im Installationszweig `http_try_auto_install_app_files()` und `http_remote_app_files_available()` ruft, und die arbeiten mit **rohen** `LittleFS.end()`-Aufrufen (`:1319/:1368`, `:5815/:5904` u. a.). Ein rohes `end()` innerhalb der Klammer würde unter dem Halter aushängen. Das ist auflösbar, aber nicht nebenbei — und nicht in derselben Runde wie der Auslieferungsumbau |
| **Ein drittes Feld „Zahl der Schreibvorgänge der letzten Datei"** | Verlockend, weil es die Blockgrösse geräteseitig ohne `sudo` belegen würde. Es belegt aber nur das, was ohnehin im Quelltext steht; die Grösse, auf die es ankommt, sind die **Frames auf der Leitung**, und die misst nur der Mitschnitt. Dazu käme ein Feld mehr, als der Auftrag vorsieht |
| **Ein eigener Endpunkt `/api/heap`** | Der Auftrag schliesst es aus, und zu Recht: ein weiterer Pfad durch den Dispatcher, der vom Smoketest, von `snapshot-device.sh` und von der Dokumentation mitgezogen werden müsste — für zwei Zahlen |
| **`free_heap` in `/api/fs_info` oder `/api/update_status`** | Beide verfälschen die Messung durch ihre eigene Arbeit: `fs_info` mountet LittleFS, `update_status` holt vier Dateien vom Update-Server und baut einen 3-KB-`String` |
| **Messung des freien Heaps *während* der Auslieferung** | Technisch unmöglich: Der ESP bedient eine Verbindung nach der anderen; die zweite Abfrage wird erst nach dem Dateiende bedient. Wer es versucht, misst eine Wartezeit und hält sie für einen Hänger |
| **Einen Recovery-Mechanismus für den Absturz bauen** | Dieselbe bewusste Entscheidung wie bei den F411-Hängern: erst erhärten, dann bauen. Ein Mechanismus, der einen nicht verstandenen Absturz „abfängt", verdeckt ihn |
| **`B15`/i18n als L125-Fix führen** | **Faktor 1,12 gegen 2,10** (nachgemessen 03.10.2026: 506 → 453 Frames gegen 506 → 241 bis 253), und die Änderung läge in der PWA, während die Ursache im ESP liegt. L127 sagt das selbst. Dazu kommt eine zweite Messgrösse: Die drei Stillstandsstellen aus L125 — 25'344, 76'800 und 83'712 Byte — liegen **alle unterhalb** der 115'843 Byte, die `app.js.gz` nach der Auslagerung misst. Die Datei verlässt den Gefahrenbereich also gar nicht |

---

## Versionsfolgen

- [ ] STM `src/main.h` anheben — **nein**, `src/**` ist nicht berührt
- [x] ESP `version.h` anheben — **ja, je Runde einmal** (Runde A und Runde B sind zwei
      Releases, zwei Rollouts, zwei Tags)
- [ ] App `APP_VERSION` anheben — **nein**, die PWA ist nicht berührt
- [ ] `CACHE_NAME` in `sw.js` anheben — **nein**, gehört untrennbar zu `APP_VERSION`

Nur der `release-engineer` führt das aus (R4). Kein Gleichschritt: Ändert ein Release
nur den ESP-Code, steigt nur dessen Version (DIR-004).
