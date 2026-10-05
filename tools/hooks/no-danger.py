#!/usr/bin/env python3
"""PreToolUse-Hook: haelt den Testagenten von den gefaehrlichen Endpunkten fern.

Setzt Phase 8 aus TESTPLAN-PWA.md durch. Dort steht, welche sieben Funktionen NICHT
scharf ausgefuehrt werden. Bisher stuende das nur in einem Dokument -- und ein
Dokument verhindert nichts.

Die Uhr ist ein Einzelstueck im Dauerbetrieb. Ein Agent, der eine Teststrategie
abarbeitet, trifft frueher oder spaeter auf eine Zeile wie "Netzwerkeinstellungen
pruefen" und hat dann die Wahl zwischen Auslassen und Ausprobieren. Diese Wahl soll
er gar nicht erst haben.

Blockiert werden drei Klassen:

  1. Zugangsdaten der AKTIVEN Verbindung (network_client_set, network_wps,
     boot_as_ap). Sie zu schreiben kappt die Verbindung, ueber die geprueft wird --
     auch beim Zurueckschreiben unveraenderter Werte, weil eine Neuanmeldung folgt.

  2. Datenverlust (maintenance_format_fs, maintenance_reset_eeprom, fs_remove).
     Angelernte IR-Codes stehen in KEINER Sicherung; ein EEPROM-Reset kostet sie
     unwiederbringlich.

  3. Garantierte Blockade (test_display zieht die Versorgung bis zum Brownout,
     learn_ir blockiert unbegrenzt).

Ausserdem der Request, der den ESP frueher zum Absturz brachte: ein Parameter ohne
'='. Der Smoketest sendet den bewusst und prueft, dass die Uhr weiterlebt -- von Hand
hat er nichts zu suchen.

Erlaubt bleibt alles Lesende und alles, was sich folgenlos zuruecknehmen laesst.
Registriert wird der Hook im Frontmatter des Testagenten, nicht global: Der Nutzer
selbst und die Hauptsitzung duerfen diese Endpunkte nach ausdruecklicher Freigabe
ansprechen.
"""

import json
import re
import sys

# Endpunktnamen, nicht ganze URLs: Der Treffer soll unabhaengig davon greifen, ob der
# Aufruf ueber curl, ueber ein Skript oder mit anderem Host erfolgt.
FORBIDDEN = {
    "network_client_set": "schreibt SSID und WLAN-Schluessel und loest eine Neuanmeldung aus — das kappt genau die Verbindung, ueber die geprueft wird",
    "network_wps": "kann die gespeicherten Zugangsdaten ersetzen, ohne dass das Ergebnis kontrollierbar ist",
    # Nachgetragen am 02.10.2026: Der erste Testdurchlauf hat gezeigt, dass der
    # Testplan diesen Endpunkt falsch eingestuft hatte. http_api_network_ap_set
    # setzt EEPROM_FLAG_BOOT_AS_AP, schreibt das EEPROM und ruft sofort wifi_ap() --
    # die Uhr ist dann aus dem WLAN und kommt auch beim naechsten Start als
    # Zugangspunkt hoch. Der Agent hat die Pruefung von sich aus verweigert; dass
    # er das musste, war die Luecke.
    "network_ap_set": "setzt EEPROM_FLAG_BOOT_AS_AP und schaltet sofort in den Zugangspunkt-Modus — die Uhr ist dann aus dem WLAN, auch nach dem naechsten Start",
    "maintenance_format_fs": "loescht die PWA vom Geraet — danach fehlt die Oberflaeche, mit der man sie wieder hochladen wuerde",
    "maintenance_reset_eeprom": "setzt alle Geraeteeinstellungen zurueck; die angelernten IR-Codes stehen in KEINER Sicherung",
    "fs_remove": "loescht einzelne Dateien aus dem LittleFS, darunter die PWA selbst",
    "test_display": "zieht bei voller Last so viel Strom, dass die Versorgung einbricht — am Geraet beobachtet: Brownout nach 44 s",
    "learn_ir": "blockiert unbegrenzt, bis ein IR-Code eintrifft",
    # Nachgetragen am 03.10.2026 mit der Spec zu F1, also BEVOR der Endpunkt
    # existiert -- die bisherigen Eintraege kamen alle erst, nachdem jemand in die
    # Falle gelaufen war. Der Endpunkt ist der einzige Schreibweg zu den IR-Codes,
    # und die sind die einzige Konfiguration, von der es bis heute keine Sicherung
    # gibt: Ein falsch geschriebener Code macht die betroffene Taste unbrauchbar,
    # und der einzige Rueckweg ist erneutes Anlernen ueber learn_ir -- das eine
    # Zeile darueber gesperrt ist, weil es unbegrenzt blockiert.
    "ir_code_set": "ueberschreibt einen angelernten IR-Code; der einzige Rueckweg ist erneutes Anlernen ueber learn_ir, und das blockiert unbegrenzt",
    "local_esp_update": "spielt Firmware ein — gehoert in einen Release, nicht in einen Testdurchlauf",
    "remote_esp_update": "dito, per OTA",
    "local_stm32_flash": "flasht den STM — dito",
    "remote_stm32_flash": "dito, per OTA",
}

# eeprom_settings_set mit boot_as_ap: die Uhr waere nach dem naechsten Start nicht
# mehr im WLAN, sondern spannte einen eigenen Zugangspunkt auf.
BOOT_AS_AP = re.compile(r"eeprom_settings_set[^\s'\"]*[?&]boot_as_ap=(1|true|on)", re.I)

# Parameter ohne '=' in einem Request an das Geraet.
NO_EQUALS = re.compile(r"https?://[^\s'\"]*\?[A-Za-z_][A-Za-z0-9_]*(?![=A-Za-z0-9_])")


def refuse(what, why):
    print(
        f"Blockiert durch Phase 8 (TESTPLAN-PWA.md): {what}\n\n"
        f"Grund: {why}\n\n"
        "Die Uhr ist ein Einzelstueck im Dauerbetrieb, kein Testgeraet. Fuer diese "
        "Funktion sieht der Testplan eine Ersatzpruefung vor — siehe Phase 8. "
        "Soll sie wirklich scharf ausgefuehrt werden, macht das der Nutzer selbst "
        "oder die Hauptsitzung nach ausdruecklicher Freigabe.",
        file=sys.stderr,
    )
    return 2


def main():
    raw = sys.stdin.read()
    payload = json.loads(raw) if raw.strip() else {}
    if payload.get("tool_name") != "Bash":
        return 0

    command = payload.get("tool_input", {}).get("command", "")

    for name, why in FORBIDDEN.items():
        # Der Name muss als ENDPUNKT auftreten, nicht bloss als Wort.
        #
        # Die Wortgrenze allein reichte nicht, und das ist am 05.10.2026 bei der
        # Gegenprobe aufgefallen: `echo "test_display ausgelassen"` wurde abgewiesen.
        # Ausgerechnet der pwa-tester, der diesen Hook traegt, SOLL ausgelassene
        # Schritte in seinen Bericht schreiben (DIR-012) -- der Hook blockierte also
        # genau das Verhalten, das die Direktive verlangt.
        #
        # Das ist dieselbe Falle wie bei file-ownership.py, in CLAUDE.md unter R3
        # beschrieben: Ein Hook, der das SCHREIBEN UEBER eine Gefahr blockiert statt
        # ihres Aufrufs, wird umgangen statt befolgt.
        #
        # Verlangt wird deshalb der Pfadbezug: /api/<name> oder <name>? bzw. <name>&.
        # Das deckt jede Aufrufform ab, die ueber HTTP geht -- und laesst jede
        # Erwaehnung in Prosa durch.
        if re.search(rf"/api/{re.escape(name)}\b|\b{re.escape(name)}[?&]", command):
            return refuse(f"Aufruf von /api/{name}", why)

    if BOOT_AS_AP.search(command):
        return refuse(
            "eeprom_settings_set mit boot_as_ap=1",
            "beim naechsten Neustart waere die Uhr nicht mehr im WLAN, sondern "
            "spannte einen eigenen Zugangspunkt auf",
        )

    if NO_EQUALS.search(command):
        return refuse(
            "Request mit einem Parameter ohne '='",
            "genau dieser Request hat den ESP frueher zum Absturz gebracht "
            "(behoben in 3.2.3). Der Smoketest sendet ihn bewusst und prueft, dass "
            "die Uhr weiterlebt — von Hand hat er nichts zu suchen",
        )

    return 0


if __name__ == "__main__":
    sys.exit(main())
