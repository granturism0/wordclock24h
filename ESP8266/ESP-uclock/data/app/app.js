/*----------------------------------------------------------------------------------------------------------------------------------------
 * app.js - WordClock progressive web app logic
 *
 * Copyright (c) 2026 Daniel Kocher - danny(at)ewanet.ch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
const APP_VERSION = "1.4.87";
const DEFAULT_LANGUAGE = "de";
const LANGUAGE_STORAGE_KEY = "wordclock-language";
// Deutsch bleibt fest im Bundle, und das ist eine Zusicherung, keine Bequemlichkeit:
// Die Oberfläche muss vollständig bedienbar sein, auch wenn kein Nachladen gelingt.
// app.js kommt vom Gerät, und genau dessen Auslieferung ist der kritische Befund.
// Jede weitere Sprache liegt als eigene Datei unter data/app/i18n/<code>.json und
// wird erst beim Umschalten geholt -- sie kostet damit nichts am Startgewicht.
const I18N_DE = {
  "app.title": "WordClock",
  "app.version_label": "App-Version",
  "app.insecure_context": "Ohne HTTPS gibt es keinen Service Worker: Offline-Betrieb und Installation als App stehen über diese Adresse nicht zur Verfügung. Die Bedienung funktioniert trotzdem vollständig.",
  "language.load_failed": "Die Texte für {language} konnten nicht geladen werden. Die Oberfläche bleibt auf Deutsch — prüf die Verbindung zur Uhr und versuch es noch einmal.",
  "language.load_missing": "Die Sprachdatei für {language} liegt nicht auf der Uhr. Die Oberfläche bleibt auf Deutsch — lad die App-Dateien neu auf die Uhr, dann steht die Sprache wieder zur Verfügung.",
  "hero.eyebrow": "Parallel zur Legacy-Seite",
  "hero.text": "Erste echte PWA neben der bestehenden ESP-Weboberfläche. Diese Ansicht liest reale Daten vom Gerät und bleibt bewusst schlank, während die Legacy-Seite weiter verfügbar ist.",
  "hero.language_label": "Sprache",
  "hero.legacy_button": "Legacy-Seite",
  "hero.reload_button": "Neu laden",
  "nav.aria_label": "Bereiche",
  "nav.main": "Hauptseite",
  "nav.system": "System",
  "nav.network": "Netzwerk",
  "nav.climate": "Umwelt",
  "nav.display": "Display",
  "nav.animations": "Animationen",
  "nav.overlays": "Overlays",
  "nav.ambilight": "Ambilight",
  "nav.timers": "Timer",
  "nav.dfplayer": "DFPlayer",
  "nav.maintenance": "Wartung",
  "nav.hint": "Wische seitlich, um weitere Bereiche zu sehen.",
  "section.eyebrow": "Bereich",
  "section.main.title": "Hauptseite",
  "section.main.hint": "Live-Status, Sofortaktionen und die zentrale Gerätezeit auf einen Blick.",
  "section.system.title": "System",
  "section.system.hint": "Hardwaredaten, Gerätezeit, erkannte Subsysteme und Debug-Ansichten für die Oberfläche.",
  "section.network.title": "Netzwerk",
  "section.network.hint": "WLAN-Client, eigener Access Point und Zeitdienste klar getrennt für Verbindung und Inbetriebnahme.",
  "section.climate.title": "Umwelt",
  "section.climate.hint": "Wetter, Temperatursensoren und Helligkeitssensor als gemeinsamer Umweltbereich.",
  "section.display.title": "Display",
  "section.display.hint": "Anzeige, Helligkeit, Ticker, Dimmkurven und TFT-spezifische Optionen.",
  "section.animations.title": "Animationen",
  "section.animations.hint": "Anzeige- und Farbanimationen inklusive Profilen und Verzögerungen.",
  "section.overlays.title": "Overlays",
  "section.overlays.hint": "Einblendungen für Icons, Ticker, Datum, Wetter und DFPlayer-Inhalte.",
  "section.ambilight.title": "Ambilight",
  "section.ambilight.hint": "Ambilight-Steuerung, Farben, Profile und die eigene Dimmkurve.",
  "section.timers.title": "Timer",
  "section.timers.hint": "Zeitpläne für Display und Ambilight in einem gemeinsamen Modul.",
  "section.dfplayer.title": "DFPlayer",
  "section.dfplayer.hint": "Audio, Glocke, Sprache und die gespeicherten Titelblöcke des DFPlayers.",
  "section.maintenance.title": "Wartung",
  "section.maintenance.hint": "Update-Quelle, LittleFS, lokale Dateien und Service-Aktionen für das Gerät.",
  "main.live_eyebrow": "Live",
  "main.status_title": "Aktueller Status",
  "main.waiting": "wartet auf Daten",
  "main.label.display": "Display",
  "main.label.ambilight": "Ambilight",
  "main.label.firmware": "Firmware",
  "main.label.last_start": "Letzter Start",
  "main.display_toggle": "Display umschalten",
  "main.ambilight_toggle": "Ambilight umschalten",
  "main.display_turn_on": "Display einschalten",
  "main.display_turn_off": "Display ausschalten",
  "main.ambilight_turn_on": "Ambilight einschalten",
  "main.ambilight_turn_off": "Ambilight ausschalten",
  "main.display_toggle_failed": "Display konnte nicht geschaltet werden",
  "main.ambilight_toggle_failed": "Ambilight konnte nicht geschaltet werden",
  "common.auto": "Auto",
  "system.hardware_eyebrow": "System",
  "system.hardware_title": "Hardware",
  "system.subsystems_eyebrow": "Subsysteme",
  "system.subsystems_title": "Verfügbarkeit",
  "system.time_eyebrow": "Zeit",
  "system.time_title": "Datum, Uhrzeit und IR",
  "system.time_loading": "Gerätezeit wird geladen...",
  "system.date_label": "Datum",
  "system.day_label": "Tag",
  "system.month_label": "Monat",
  "system.year_label": "Jahr",
  "system.clock_label": "Uhrzeit",
  "system.hour_label": "Stunde",
  "system.minute_label": "Minute",
  "system.datetime_save": "Datum und Uhrzeit speichern",
  "system.learn_ir": "IR-Fernbedienung lernen",
  "system.debug_eyebrow": "Debug",
  "system.debug_overrides_title": "Ansichts-Overrides",
  "system.debug_overrides_hint": "Nur für lokale Tests in der App. Die Gerätemeldungen selbst werden nicht geändert.",
  "system.debug_ambilight_ui": "Ambilight UI",
  "system.debug_dfplayer_ui": "DFPlayer UI",
  "system.debug_color_ui": "Farb-LED UI",
  "system.debug_tft_ui": "TFT UI",
  "system.force_online": "Online erzwingen",
  "system.force_rgb": "RGB erzwingen",
  "system.force_rgbw": "RGBW erzwingen",
  "system.force_visible": "Sichtbar erzwingen",
  "system.apply_overrides": "Overrides anwenden",
  "system.reset_overrides": "Overrides zurücksetzen",
  "system.preview_hint": "Zeigt gespeicherte Display-Farbe, aktuelle Live-Farbe vom Gerät und das verwendete Vorschau-Layout.",
  "system.logs_hint": "Zeigt bewusst markierte STM32-Logzeilen an, die über die ESP-UART in die Weboberfläche gespiegelt werden.",
  "system.preview_colors": "Vorschau-Farben",
  "system.logbook_title": "STM32-Logbuch",
  "network.client_eyebrow": "Client",
  "network.client_title": "Mit WLAN verbinden",
  "network.status_loading": "Netzwerkstatus wird geladen...",
  "network.found_ssids": "Gefundene WLANs",
  "network.wifi_password": "WLAN-Passwort",
  "network.wifi_password_placeholder": "Passwort für WLAN-Client",
  "network.scan": "WLANs neu laden",
  "network.connect_client": "Als WLAN-Client verbinden",
  "network.connect_client_error": "WLAN-Client konnte nicht gesetzt werden",
  "network.connect_client_started": "WLAN-Client-Verbindung wurde angestossen",
  "network.manual_ssid": "Oder SSID von Hand (verstecktes Netz)",
  "network.manual_ssid_placeholder": "leer lassen, wenn oben gewählt",
  "network.ssid_missing": "Bitte zuerst ein WLAN auswählen oder die SSID von Hand eintragen.",
  "network.ap_eyebrow": "Access Point",
  "network.ap_title": "Eigenes WLAN bereitstellen",
  "network.ap_hint": "Hilfreich für Erstinbetriebnahme oder wenn kein vorhandenes WLAN genutzt werden soll.",
  "network.ap_ssid": "AP SSID",
  "network.ap_ssid_placeholder": "WordClock Zugangspunkt",
  "network.ap_password": "AP-Passwort",
  "network.ap_password_placeholder": "Mindestens 10 Zeichen",
  "network.start_ap": "Zugangspunkt starten",
  "network.start_ap_error": "Zugangspunkt konnte nicht gesetzt werden",
  "network.start_ap_started": "Start des Zugangspunkts wurde angestossen",
  "network.time_eyebrow": "Zeit",
  "network.time_title": "Zeitserver und Uhrzeit",
  "network.timeserver": "Zeitserver",
  "network.save_timeserver": "Zeitserver speichern",
  "network.timezone": "Zeitzone (GMT +/-)",
  "network.save_timezone": "Zeitzone speichern",
  "network.summertime": "Sommerzeit berücksichtigen",
  "network.summertime_disable": "Sommerzeit-Berücksichtigung deaktivieren",
  "network.fetch_network_time": "Netzzeit abrufen",
  "climate.weather_eyebrow": "Wetter",
  "climate.weather_title": "Wetter und Standort",
  "climate.api_key": "API-Schlüssel",
  "climate.save_api_key": "API-Schlüssel speichern",
  "climate.choose_location": "Standort wählen",
  "climate.city": "Ort",
  "climate.save_city": "Ort speichern",
  "climate.longitude": "Längengrad",
  "climate.latitude": "Breitengrad",
  "climate.save_coordinates": "Koordinaten speichern",
  "climate.pick_on_map": "Ort auf Karte wählen",
  "climate.location_ready": "Karte und Suche stehen für die Standortwahl bereit.",
  "climate.load_weather": "Wetter laden",
  "climate.weather_hint": "Nutze den gewählten Standort und rufe aktuelle Werte oder die Vorhersage ab.",
  "climate.fetch_weather": "Wetter abrufen",
  "climate.fetch_forecast": "Wettervorhersage abrufen",
  "climate.temperature_eyebrow": "Temperatur",
  "climate.temperature_title": "Sensoren und Korrektur",
  "climate.ds18xx_correction": "DS18xx-Korrektur (-20 bis +20 Schritte zu je 0,5 °C, also höchstens ±10 °C)",
  "climate.save_ds18xx_correction": "DS18xx-Korrektur speichern",
  "climate.rtc_correction": "RTC-Korrektur (-20 bis +20 Schritte zu je 0,5 °C, also höchstens ±10 °C)",
  "climate.save_rtc_correction": "RTC-Korrektur speichern",
  "climate.show_temperature": "Temperatur anzeigen",
  "climate.ldr_title": "Helligkeitssensor",
  "climate.toggle_auto_brightness": "Automatische Helligkeit umschalten",
  "climate.set_min_ldr": "Aktuellen Wert als Minimum setzen",
  "climate.set_max_ldr": "Aktuellen Wert als Maximum setzen",
  "climate.enable_auto_brightness": "Automatische Helligkeit aktivieren",
  "climate.disable_auto_brightness": "Automatische Helligkeit deaktivieren",
  "climate.auto_brightness": "Automatische Helligkeit",
  "climate.current_ldr_value": "Aktueller LDR-Wert",
  "climate.minimum": "Minimum",
  "climate.maximum": "Maximum",
  "climate.status_on": "ein",
  "climate.status_off": "aus",
  "display.config_eyebrow": "Konfiguration",
  "display.config_title": "Wichtige Einstellungen",
  "display.control_eyebrow": "Display",
  "display.control_title": "Weitere Steuerung",
  "display.brightness": "Display-Helligkeit",
  "display.save_brightness": "Helligkeit speichern",
  "display.color": "Display-Farbe",
  "display.color_hint": "Display-Farbe ist wählbar, wenn keine Farbanimation aktiv ist.",
  "display.white_channel": "Weisskanal",
  "display.use_rgbw": "Weisskanal einschalten",
  "display.use_rgbw_disable": "Weisskanal ausschalten",
  "display.save_color": "Display-Farbe speichern",
  "display.color_save_failed": "Farbe konnte nicht gespeichert werden",
  "display.mode": "Display-Modus",
  "display.save_mode": "Display-Modus speichern",
  "display.mode_normal": "Normal",
  "display.mode_seconds": "Sekunden",
  "display.mode_date": "Datum",
  "display.mode_temperature": "Temperatur",
  "display.mode_ticker": "Ticker",
  "display.keep_it_is": "„ES IST“ dauerhaft anzeigen",
  "display.keep_it_is_disable": "„ES IST“ deaktivieren",
  "display.ticker_text": "Ticker-Text",
  "display.ticker_placeholder": "WordClock bereit",
  "display.save_ticker": "Ticker speichern",
  "display.date_format": "Datumsformat im Ticker",
  "display.save_date_format": "Datumsformat speichern",
  "display.ticker_delay": "Ticker-Verzögerung",
  "display.save_ticker_delay": "Ticker-Verzögerung speichern",
  "display.ticker_delay_save_failed": "Ticker-Verzögerung konnte nicht gespeichert werden",
  "display.diagnostics": "Diagnose",
  "display.diagnostics_hint": "Temporärer LED- und Farbtest für die reine Funktionsprüfung des Displays.",
  "display.run_test": "Displaytest starten",
  "display.test_running": "Displaytest läuft",
  "display.test_start_failed": "Displaytest konnte nicht gestartet werden",
  "display.dim_curve_eyebrow": "Helligkeitskurve",
  "display.dim_curve_title": "Display",
  "display.dim_curve_hint": "Dimmwerte für die 16 Helligkeitsstufen.",
  "display.dim_level": "Stufe {idx}",
  "display.dim_curve_ambilight_title": "Ambilight",
  "display.dim_curve_ambilight_hint": "Dimmwerte für die 16 Ambilight-Stufen.",
  "display.presets": "Voreinstellungen",
  "display.curve_preset": "Kurvenvorgabe",
  "display.apply_preset_save": "Voreinstellung anwenden und speichern",
  "display.dim_curve_save": "Display-Dimmkurve speichern",
  "display.save_ambilight_dim_curve": "Ambilight-Dimmkurve speichern",
  "display.dim_curve_save_failed": "Dimmkurve konnte nicht gespeichert werden",
  "display.tft_eyebrow": "TFT",
  "display.tft_panel_title": "Panel-Optionen",
  "display.tft_save_failed": "TFT-Optionen konnten nicht gespeichert werden",
  "display.ambilight_control_eyebrow": "Ambilight",
  "display.ambilight_control_title": "Weitere Steuerung",
  "display.ambilight_brightness": "Ambilight-Helligkeit",
  "display.save_ambilight_brightness": "Ambilight-Helligkeit speichern",
  "display.ambilight_brightness_save_failed": "Ambilight-Helligkeit konnte nicht gespeichert werden",
  "display.ambilight_mode": "Ambilight-Modus",
  "display.save_ambilight_mode": "Ambilight-Modus speichern",
  "display.ambilight_mode_save_failed": "Ambilight-Modus konnte nicht gespeichert werden",
  "display.ambilight_mode_profiles": "Modus-Profile",
  "display.ambilight_profile_save_failed": "Ambilight-Profil konnte nicht gespeichert werden",
  "display.ambilight_leds": "Ambilight-LEDs",
  "display.save_ambilight_leds": "LED-Anzahl speichern",
  "display.ambilight_leds_save_failed": "LED-Anzahl konnte nicht gespeichert werden",
  "display.ambilight_offset": "Offset bei Sekunde 0",
  "display.save_ambilight_offset": "Offset speichern",
  "display.ambilight_offset_save_failed": "Ambilight-Offset konnte nicht gespeichert werden",
  "display.colors_eyebrow": "Farben",
  "display.colors_title": "RGBW und Synchronisierung",
  "display.color_detecting": "Hardware wird erkannt...",
  "display.marker_color": "Marker-Farbe",
  "display.save_ambilight_color": "Ambilight-Farbe speichern",
  "display.save_marker_color": "Marker-Farbe speichern",
  "display.manual_adjustment": "Manuelle Anpassung",
  "display.sync_ambilight": "Ambilight synchronisieren",
  "display.unsync_ambilight": "Ambilight-Synchronisierung deaktivieren",
  "display.sync_markers": "Marker synchronisieren",
  "display.unsync_markers": "Marker-Synchronisierung deaktivieren",
  "display.fade_clock_seconds": "Sekunden weich ausblenden",
  "display.fade_clock_seconds_disable": "Weiches Ausblenden deaktivieren",
  "display.five_second_markers": "5-Sekunden Marker",
  "display.ambilight_markers": "5-Sekunden-Marker aktivieren",
  "display.ambilight_markers_disable": "5-Sekunden-Marker deaktivieren",
  "display.ambilight_modes_unavailable": "Für die erkannte Hardware gibt es keine konfigurierbaren Ambilight-Modi.",
  "display.default_set_failed": "Standardwert konnte nicht gesetzt werden",
  "display.ambilight_default_set_failed": "Ambilight-Standardwert konnte nicht gesetzt werden",
  "display.tft_rgb_order": "RGB-Reihenfolge",
  "display.tft_flip_horizontal": "Horizontal spiegeln",
  "display.tft_flip_vertical": "Vertikal spiegeln",
  "display.save_tft_options": "TFT-Optionen speichern",
  "animations.current_eyebrow": "Animationen",
  "animations.current_title": "Aktuelle Auswahl",
  "animations.current_hint": "Grundmodus für Anzeige und Farben direkt wählen und speichern.",
  "animations.display_animation": "Anzeigeanimation",
  "animations.save_display_animation": "Anzeigeanimation speichern",
  "animations.color_animation": "Farbanimation",
  "animations.save_color_animation": "Farbanimation speichern",
  "animations.display_profiles_eyebrow": "Anzeigeanimationen",
  "animations.profiles_title": "Profile",
  "animations.display_profiles_hint": "Verzögerung und Favoriten pro Profil anpassen.",
  "animations.color_profiles_eyebrow": "Farbanimationen",
  "animations.color_profiles_hint": "Verzögerung pro Profil anpassen.",
  "animations.favorite": "Favorit",
  "animations.delay": "Verzögerung",
  "animations.profile_save": "Profil speichern",
  "animations.profile_save_failed": "Animationsprofil konnte nicht gespeichert werden",
  "animations.color_profile_save_failed": "Farbanimationsprofil konnte nicht gespeichert werden",
  "animations.default": "Standard",
  "animations.name_fade": "Einblenden",
  "animations.name_roll": "Rollen",
  "animations.name_explode": "Explosion",
  "animations.name_snake": "Schlange",
  "animations.name_cube": "Würfel",
  "animations.name_teletype": "Fernschreiber",
  "animations.name_none": "Keine",
  "animations.name_normal": "Normal",
  "animations.name_clock": "Uhr",
  "animations.name_rainbow": "Regenbogen",
  "animations.name_temperature": "Temperatur",
  "animations.name_ticker": "Ticker",
  "animations.name_date": "Datum",
  "animations.name_seconds": "Sekunden",
  "overlays.eyebrow": "Overlays",
  "overlays.title": "Einblendungen",
  "overlays.hint": "Bestehende Overlays bearbeiten oder eine neue Overlay-Zeile anlegen.",
  "overlays.new_title": "Neues Overlay",
  "overlays.new_badge": "Neu",
  "overlays.content": "Inhalt",
  "overlays.type": "Typ",
  "overlays.icon": "Icon",
  "overlays.value": "Wert",
  "overlays.folder": "Ordner",
  "overlays.track": "Track",
  "overlays.time_and_date": "Zeit und Datum",
  "overlays.interval": "Intervall (Min.)",
  "overlays.duration": "Dauer (Sek.)",
  "overlays.date_code": "Datums-Code",
  "overlays.day": "Tag",
  "overlays.month": "Monat",
  "overlays.days": "Tage",
  "overlays.create": "Overlay anlegen",
  "overlays.save": "Overlay speichern",
  "overlays.cancel": "Abbrechen",
  "overlays.display": "Anzeigen",
  "overlays.delete": "Löschen",
  "overlays.type_none": "Keins",
  "overlays.type_icon": "Icon",
  "overlays.type_date": "Datum",
  "overlays.type_temperature": "Temperatur",
  "overlays.type_weather_icon": "Wetter-Icon",
  "overlays.type_weather_ticker": "Wetter-Ticker",
  "overlays.type_ticker": "Ticker",
  "overlays.type_dfplayer": "DFPlayer",
  "overlays.type_forecast_icon": "Wettervorhersage-Icon",
  "overlays.type_forecast_ticker": "Wettervorhersage-Ticker",
  "overlays.type_temperature_digits": "Temperatur als Ziffern",
  "overlays.datecode_none": "----",
  "overlays.datecode_carnival": "Karneval",
  "overlays.datecode_easter": "Ostersonntag",
  "overlays.datecode_advent1": "1. Advent",
  "overlays.datecode_advent2": "2. Advent",
  "overlays.datecode_advent3": "3. Advent",
  "overlays.datecode_advent4": "4. Advent",
  "overlays.save_failed": "Overlay konnte nicht gespeichert werden",
  "overlays.start_date_incomplete": "Das Startdatum ist unvollständig. Wähle Tag und Monat zusammen aus — oder lass beide leer, wenn das Overlay kein Startdatum haben soll.",
  "overlays.type_unknown_save": "Dieses Overlay trägt den unbekannten Typ {value}. Erlaubt sind {min} bis {max}. Wähle zuerst einen gültigen Typ aus — es wurde nichts gespeichert und der Gerätewert bleibt unangetastet.",
  "timers.save_all": "Alle Timer speichern",
  "timers.ambilight_eyebrow": "Ambilight-Timer",
  "timers.ambilight_title": "Zeiten für Ambilight",
  "timers.save_all_ambilight": "Alle Ambilight-Timer speichern",
  "timers.slot": "Slot",
  "timers.slot_subline": "Timer",
  "timers.ambilight_slot_subline": "Ambilight-Timer",
  "timers.period": "Zeitraum",
  "timers.action": "Aktion",
  "timers.switch_on": "Einschalten",
  "timers.switch_off": "Ausschalten",
  "timers.time": "Zeit",
  "timers.from_day": "Von Tag",
  "timers.to_day": "Bis Tag",
  "timers.clear_slot": "Slot leeren",
  "timers.main_eyebrow": "Timer",
  "timers.main_title": "Zeiten",
  "timers.conflict_hint": "Gleiche Zeit wie {others}, an mindestens einem gemeinsamen Wochentag. Die Uhr prüft die Slots aufsteigend und hält beim ersten Treffer an — Slot {winner} greift zuerst.",
  "dfplayer.volume": "Lautstärke",
  "dfplayer.volume_save": "Lautstärke speichern",
  "dfplayer.volume_save_failed": "DFPlayer-Lautstärke konnte nicht gespeichert werden",
  "dfplayer.panel_eyebrow": "DFPlayer",
  "dfplayer.panel_title": "Audio und Klingel",
  "dfplayer.note_online": "DFPlayer ist online.",
  "dfplayer.note_offline": "DFPlayer ist offline und wird ausgeblendet.",
  "dfplayer.mode": "Modus",
  "dfplayer.mode_none": "Keiner",
  "dfplayer.mode_bell": "Glocke",
  "dfplayer.mode_speech": "Sprache",
  "dfplayer.mode_save": "Modus speichern",
  "dfplayer.mode_save_failed": "DFPlayer-Modus konnte nicht gespeichert werden",
  "dfplayer.bell_times": "Glockenzeiten",
  "dfplayer.bell_save": "Glockenzeiten speichern",
  "dfplayer.bell_save_failed": "Glockenzeiten konnten nicht gespeichert werden",
  "dfplayer.speech": "Sprache",
  "dfplayer.speak_cycle": "Sprachzyklus",
  "dfplayer.speak_cycle_save": "Sprachzyklus speichern",
  "dfplayer.speak_cycle_save_failed": "Sprachzyklus konnte nicht gespeichert werden",
  "dfplayer.silence_start": "Ruhezeit Beginn",
  "dfplayer.silence_stop": "Ruhezeit Ende",
  "dfplayer.silence_start_save": "Ruhezeit Beginn speichern",
  "dfplayer.silence_stop_save": "Ruhezeit Ende speichern",
  "dfplayer.silence_start_save_failed": "Ruhezeit Beginn konnte nicht gespeichert werden",
  "dfplayer.silence_stop_save_failed": "Ruhezeit Ende konnte nicht gespeichert werden",
  "dfplayer.test_folder": "Test-Ordner",
  "dfplayer.test_track": "Test-Titel",
  "dfplayer.play_track": "Titel abspielen",
  "dfplayer.play_started": "DFPlayer-Titel gestartet",
  "dfplayer.play_failed": "DFPlayer-Titel konnte nicht gestartet werden",
  "dfplayer.saved_titles": "Titel 001-008",
  "dfplayer.alarm_title": "Titel",
  "dfplayer.alarm_subline": "Zeitplan",
  "dfplayer.from": "Von",
  "dfplayer.to": "Bis",
  "dfplayer.time": "Zeit",
  "dfplayer.alarm_save_failed": "DFPlayer-Titel konnte nicht gespeichert werden",
  "maintenance.update_eyebrow": "Update",
  "maintenance.source_versions_title": "Quelle und Versionen",
  "maintenance.source_versions_hint": "Update-Quelle, erkannte Versionen und verfügbare Server-Dateien im Überblick.",
  "maintenance.server": "Server",
  "maintenance.update_host": "Update-Host",
  "maintenance.save_update_host": "Update-Host speichern",
  "maintenance.update_path": "Update-Pfad",
  "maintenance.save_update_path": "Update-Pfad speichern",
  "maintenance.server_selection": "Server-Auswahl",
  "maintenance.stm32_server_firmware": "STM32-Firmware vom Server",
  "maintenance.layout_server_table": "Layout-Tabelle vom Server",
  "maintenance.versions": "Versionen",
  "maintenance.update_progress": "Update-Fortschritt",
  "maintenance.waiting_for_action": "Wartet auf Aktion...",
  "maintenance.progress_prepare_title": "Vorbereiten",
  "maintenance.progress_prepare_note": "Update wird gestartet.",
  "maintenance.progress_bootloader_title": "Bootloader",
  "maintenance.progress_bootloader_note": "STM32-Bootloader wird angesprochen.",
  "maintenance.progress_hex_check_title": "HEX prüfen",
  "maintenance.progress_hex_check_note": "Firmwaredatei wird geprüft.",
  "maintenance.progress_flash_erase_title": "Flash löschen",
  "maintenance.progress_flash_erase_note": "STM32-Flash wird gelöscht.",
  "maintenance.progress_flash_write_title": "Flash schreiben",
  "maintenance.progress_flash_write_note": "Firmware wird geschrieben und verifiziert.",
  "maintenance.progress_reset_title": "Zurücksetzen",
  "maintenance.progress_reset_note": "STM32 wird automatisch neu gestartet.",
  "maintenance.progress_finish_title": "Abschliessen",
  "maintenance.progress_finish_note": "Daten werden neu geladen.",
  "maintenance.stm32_prepare_note": "STM32-Update wird vorbereitet...",
  "maintenance.stm32_wait_bootloader": "Warte auf Rückmeldung vom STM32-Bootloader...",
  "maintenance.stm32_bootloader_reached": "STM32-Bootloader wurde erreicht.",
  "maintenance.stm32_hex_check_running": "Firmwaredatei wird geprüft.",
  "maintenance.stm32_flash_erasing": "STM32-Flash wird gelöscht.",
  "maintenance.stm32_flash_writing": "STM32-Firmware wird geschrieben und verifiziert...",
  "maintenance.esp_update_waiting": "ESP aktualisiert. Es wird gewartet, bis das Gerät wieder bereit ist.",
  "maintenance.stm32_flash_failed": "STM32-Flash ist fehlgeschlagen.",
  "maintenance.stm32_flash_done_reset": "STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.",
  "maintenance.stm32_local_flash_starting": "Lokaler STM32-Flash wird gestartet.",
  "maintenance.stm32_local_flash_start_failed": "Lokaler STM32-Flash konnte nicht gestartet werden.",
  "maintenance.stm32_local_upload_failed": "Lokaler STM32-Upload ist fehlgeschlagen.",
  "maintenance.stm32_local_upload_progress": "STM32-Firmware wird hochgeladen: {percent}%",
  "maintenance.stm32_local_upload_done": "STM32-Firmware wurde hochgeladen. Flash startet...",
  "maintenance.stm32_flash_auto_reset_failed": "STM32-Flash fertig, automatischer Reset ist fehlgeschlagen.",
  "maintenance.stm32_update_response_received": "Update-Antwort empfangen.",
  "maintenance.server_recheck_running": "Server-Dateien werden mit den neuen Update-Angaben neu geprüft...",
  "maintenance.server_recheck_done": "Server-Verfügbarkeit wurde neu geprüft",
  "maintenance.server_recheck_failed": "Server-Verfügbarkeit konnte nicht neu geprüft werden",
  "maintenance.weather_file": "Wetter-Datei",
  "maintenance.icon_file": "Icon-Datei",
  "maintenance.layout_table_file": "Layout-Tabelle",
  "maintenance.tft_display_file": "TFT-Display-Datei",
  "maintenance.install_app_files_direct": "App-Dateien direkt installieren",
  "maintenance.remote_eyebrow": "Remote",
  "maintenance.remote_title": "Vom Server laden und installieren",
  "maintenance.remote_hint": "Verfügbare Dateien direkt vom Update-Server laden. Fortschritt und Ergebnis werden darunter angezeigt.",
  "maintenance.update_esp": "ESP-Firmware aktualisieren",
  "maintenance.update_esp_confirm": "ESP-Firmware jetzt vom Update-Server aktualisieren? Das Gerät startet dabei neu.",
  "maintenance.update_esp_start": "ESP-Update wird gestartet...",
  "maintenance.flash_stm32": "STM32 flashen",
  "maintenance.flash_stm32_confirm": "STM32 jetzt mit „{file}“ flashen?",
  "maintenance.flash_stm32_started": "STM32-Flash wurde gestartet.",
  "maintenance.server_files": "Dateien vom Server",
  "maintenance.load_layout_table": "Layout-Tabelle laden",
  "maintenance.load_icon_files": "Icon-Dateien laden",
  "maintenance.load_app_files": "App-Dateien laden",
  "maintenance.service_eyebrow": "Wartung",
  "maintenance.service_title": "Service-Aktionen",
  "maintenance.service_hint": "Nur für gezielte Servicefälle. Diese Aktionen lösen keinen normalen Update- oder Backup-Workflow aus.",
  "maintenance.reset_stm32": "STM32 zurücksetzen",
  "maintenance.reset_stm32_failed": "STM32 konnte nicht zurückgesetzt werden",
  "maintenance.reset_eeprom": "EEPROM zurücksetzen",
  "maintenance.release_notes": "Release Notes",
  "maintenance.files_eyebrow": "Dateien",
  "maintenance.files_title": "LittleFS",
  "maintenance.files_hint": "Dateien anzeigen, löschen und die bekannten Upload-Ziele direkt beschicken.",
  "maintenance.file_list": "Dateiliste",
  "maintenance.file_preview": "Dateivorschau",
  "maintenance.file_actions_status": "Aktionen und Status",
  "maintenance.preview_placeholder": "Mit „Anzeigen“ aus der Dateiliste wird hier der Inhalt der gewählten Datei eingeblendet.",
  "maintenance.file_action_placeholder": "Noch keine Datei-Aktion ausgeführt.",
  "maintenance.no_files": "Noch keine Dateien im LittleFS gefunden.",
  "maintenance.special_display_target": "TFT-Sonderfall",
  "maintenance.no_stm32_files": "keine STM32-Dateien gefunden",
  "maintenance.no_release_notes": "Keine Release Notes vom Server gelesen.",
  "maintenance.fs_total": "Gesamt",
  "maintenance.fs_used": "Belegt",
  "maintenance.fs_block_size": "Blockgrösse",
  "maintenance.fs_page_size": "Seitengrösse",
  "maintenance.fs_max_open_files": "Max. offene Dateien",
  "maintenance.fs_max_path_length": "Max. Pfadlänge",
  "maintenance.boot_mode": "Bootmodus",
  "maintenance.boot_mode_ap": "Zugangspunkt",
  "maintenance.boot_mode_client": "WLAN-Client",
  "maintenance.no_data": "keine Daten",
  "maintenance.version_flash": "ESP-Flash",
  "maintenance.version_ota": "OTA-Update",
  "maintenance.version_wc": "WordClock-Version",
  "maintenance.version_wc_available": "WordClock verfügbar",
  "maintenance.version_esp": "ESP-Version",
  "maintenance.version_esp_available": "ESP verfügbar",
  "maintenance.version_app": "App-Version",
  "maintenance.version_app_available": "App verfügbar",
  "maintenance.version_stm32_default": "Standard STM32",
  "maintenance.version_available_yes": "möglich",
  "maintenance.version_available_no": "nicht möglich",
  "maintenance.local_update_unavailable": "Lokales Update ist bei dieser ESP-Flashgrösse nicht verfügbar.",
  "maintenance.choose_file_for_target": "Bitte zuerst eine Datei für {target} auswählen.",
  "maintenance.file_empty": "{file} ist 0 Byte gross und wird nicht hochgeladen — eine leere Datei würde die vorhandene ersetzen.",
  "maintenance.file_empty_status": "Leere Datei wird nicht hochgeladen",
  "maintenance.file_expected_pattern": "Falsche Datei ausgewählt. Erwartet wird ein passendes Tabellenmuster wie {pattern} für {target}.",
  "maintenance.file_expected_exact": "Falsche Datei ausgewählt. Erwartet wird {target}.",
  "maintenance.txt_required": "{target} muss eine .txt-Datei sein.",
  "maintenance.file_upload_failed": "{target} konnte nicht hochgeladen werden",
  "maintenance.file_upload_failed_detail": "{target} konnte nicht hochgeladen werden: {error}",
  "maintenance.local_uploads": "Lokale Dateien hochladen",
  "maintenance.local_app_assets": "Lokale App-Dateien (.gz)",
  "maintenance.choose_local_app_folder": "Lokalen App-Ordner wählen",
  "maintenance.local_update_eyebrow": "Lokales Update",
  "maintenance.local_update_title": "ESP- und STM32-Firmware hochladen",
  "maintenance.local_update_hint": "Lokale Firmware-Dateien direkt hochladen, ohne den Update-Server zu verwenden.",
  "maintenance.local_update_partial_support": "Einige lokale PWA-Updatepfade werden von dieser Firmware noch nicht unterstützt.",
  "maintenance.local_update_select_file": "ESP- oder STM32-Datei auswählen und direkt lokal hochladen.",
  "maintenance.local_update_preparing": "Lokal-Update wird vorbereitet...",
  "maintenance.local_esp_file": "ESP-Firmwaredatei (.bin)",
  "maintenance.local_esp_update": "ESP lokal aktualisieren",
  "maintenance.local_esp_choose_first": "Bitte zuerst eine ESP-Firmwaredatei auswählen.",
  "maintenance.local_esp_expected": "ESP-.bin-Datei erwartet",
  "maintenance.local_esp_uploading": "ESP-Firmware wird lokal hochgeladen...",
  "maintenance.local_esp_uploaded_wait": "ESP-Firmware wurde übertragen. Es wird auf den Neustart gewartet.",
  "maintenance.local_stm32_file": "STM32-Firmwaredatei (.hex)",
  "maintenance.local_stm32_update": "STM32 lokal aktualisieren",
  "maintenance.local_stm32_choose_first": "Bitte zuerst eine STM32-Firmwaredatei auswählen.",
  "maintenance.local_stm32_expected": "Passende STM32-Datei erwartet",
  "maintenance.local_stm32_uploading": "STM32-Firmware wird lokal hochgeladen...",
  "maintenance.local_stm32_update_failed": "STM32-Firmware konnte nicht aktualisiert werden.",
  "maintenance.backup_eyebrow": "Sicherung",
  "maintenance.backup_title": "Einstellungen exportieren und importieren",
  "maintenance.backup_import_file": "Sicherungsdatei importieren",
  "maintenance.backup_hint": "Sichert die konfigurierbaren Einstellungen als JSON-Datei und spielt sie auf Wunsch wieder ein. Die Datei enthält auch WLAN- und AP-Daten im Klartext.",
  "maintenance.backup_idle": "Noch keine Sicherungsaktion ausgeführt.",
  "weather.map_modal_title": "Standort auf Karte wählen",
  "weather.close": "Schliessen",
  "weather.modal_eyebrow": "Wetter",
  "weather.modal_hint": "Suche nach einem Ort, tippe auf die Karte oder nutze deinen aktuellen Standort.",
  "weather.search_place": "Ort suchen",
  "weather.search_placeholder": "Zürich, Schweiz",
  "weather.use_current_location": "Aktuellen Standort verwenden",
  "weather.map_aria": "Standortkarte",
  "weather.waiting_for_map": "Warte auf Karte...",
  "weather.place": "Ort",
  "weather.place_placeholder": "Wird aus Suche oder Karte übernommen",
  "weather.current_location_prefix": "Aktuell:",
  "status.auto_refresh_paused": "Automatische Aktualisierung pausiert, bis ungespeicherte Änderungen gespeichert sind",
  "status.waiting_for_data": "Wartet auf Daten...",
  "status.updated_at": "Aktualisiert {time}",
  "status.data_load_failed": "Daten konnten nicht geladen werden",
  "input.number_required": "Bitte einen Wert zwischen {min} und {max} eintragen. Ein leeres Feld wird nicht gespeichert.",
  "input.number_range": "Der Wert {value} liegt ausserhalb des erlaubten Bereichs {min} bis {max}. Es wurde nichts gespeichert — bitte korrigiere die Eingabe.",
  "status.settings_parse_error": "Die Konfiguration des Geräts ist beschädigt und konnte nicht gelesen werden. Ein Anführungszeichen in Ort, Tickertext, AppID oder Update-Host ist die häufigste Ursache — korrigiere es über die Legacy-Oberfläche.",
  "overview.display_mode": "Display-Modus",
  "overview.brightness": "Helligkeit",
  "overview.auto_brightness": "Automatische Helligkeit",
  "overview.led_capabilities": "LED-Fähigkeiten",
  "overview.timeserver": "Zeitserver",
  "overview.ticker_delay": "Ticker-Verzögerung",
  "overview.last_stm32_restart": "Letzter STM32-Neustart",
  "overview.last_start": "Letzter Start",
  "overview.weather_location": "Wetter-Ort",
  "overview.ticker": "Ticker",
  "overview.date_format": "Datumsformat",
  "overview.ambilight_mode": "Ambilight-Modus",
  "overview.ambilight_brightness": "Ambilight-Helligkeit",
  "overview.ambilight_leds": "Ambilight LEDs",
  "overview.ambilight_offset": "Ambilight Offset",
  "overview.dfplayer_mode": "DFPlayer-Modus",
  "overview.dfplayer_volume": "DFPlayer-Lautstärke",
  "overview.speak_cycle": "Sprechintervall",
  "preview.color_animation": "Farbanimation",
  "preview.persisted_display_color": "Gespeicherte Display-Farbe",
  "preview.live_device_color": "Live-Farbe vom Gerät",
  "preview.layout_file": "Vorschau-Layout",
  "display.white_channel_inactive": "RGBW-Hardware erkannt, aber der White-Channel ist aktuell firmwareseitig nicht aktiv.",
  "display.color_direct_available": "Die Display-Farbe kann direkt gesetzt werden, solange keine Farbanimation aktiv ist.",
  "display.color_direct_unavailable": "Die Display-Farbe ist nur direkt wählbar, wenn Farbanimation = Keine ist.",
  "system.logs_empty": "Noch keine STM32-Logs vorhanden.",
  "system.logs_buffer": "{count} Log-Zeile{suffix} im Puffer.",
  "system.logs_reload": "Logs neu laden",
  "system.logs_clear": "Logs leeren",
  "system.logs_jump_end": "Ans Ende springen",
  "system.logs_reload_busy": "lädt...",
  "system.logs_clear_busy": "leert...",
  "system.logs_jump_done": "unten",
  "system.logs_loaded": "geladen",
  "system.logs_cleared": "geleert",
  "system.logs_load_failed": "STM32-Logs konnten nicht geladen werden",
  "system.logs_load_error_hint": "STM32-Logs konnten nicht geladen werden — der Ring ist deshalb nicht zwingend leer. Versuch es mit „Logs neu laden“ noch einmal.",
  "system.logs_reload_in_flight": "Ein Abruf der STM32-Logs läuft bereits. Es wurde nichts neu geladen.",
  "system.logs_clear_failed": "STM32-Logbuch konnte nicht geleert werden",
  "system.logs_cleared_status": "STM32-Logbuch wurde geleert",
  "system.logs_clear_confirm": "STM32-Logbuch wirklich leeren?",
  "backup.export_button": "Einstellungen exportieren",
  "backup.import_button": "Einstellungen importieren",
  "backup.exported": "exportiert",
  "backup.imported": "importiert",
  "backup.export_success": "Einstellungen wurden exportiert.",
  "backup.export_failed": "Einstellungen konnten nicht exportiert werden.",
  "backup.export_eeprom_unavailable": "Die Netzwerkeinstellungen liessen sich nicht lesen. Der Export wurde abgebrochen, damit die Sicherung keine leeren WLAN-Felder enthält.",
  "backup.ir_collecting": "Lese die IR-Codes der Fernbedienung aus...",
  "backup.ir_included": "Alle {expected} IR-Tasten sind enthalten, {learned} davon angelernt.",
  "backup.ir_skipped_incomplete": "Die IR-Codes fehlen: Der Abzug blieb auch im zweiten Anlauf unvollständig ({received} von {expected} Tasten). Ein halber Tastensatz wäre schlimmer als gar keiner, deshalb fehlt der Abschnitt ganz. Versuch den Export gleich noch einmal.",
  "backup.ir_skipped_stale": "Die IR-Codes fehlen: Der ESP hat den laufenden Abzug verworfen, meist nach einem Neustart. Starte den Export noch einmal.",
  "backup.ir_skipped_unavailable": "Die IR-Codes fehlen: Die Uhr kennt die dafür nötigen Abfragen nicht — dann ist ihre ESP-Firmware älter als diese App.",
  "backup.ir_skipped_invalid": "Die IR-Codes fehlen: Die Uhr hat unbrauchbare Werte geliefert, deshalb fehlt der Abschnitt ganz.",
  "backup.import_ir": "Schreibe die IR-Codes der Fernbedienung zurück...",
  "backup.ir_restore_snapshot": "Sichere die aktuellen IR-Codes als Rückfalldatei...",
  "backup.ir_restore_fallback_ok": "Dein bisheriger Stand wurde als {file} heruntergeladen.",
  "backup.ir_restore_fallback_failed": "Es konnte KEINE Rückfalldatei angelegt werden, weil {reason}.",
  "backup.ir_restore_fallback_hint": "Dein bisheriger Stand liegt als {file} im Download-Ordner — damit kommst du zurück.",
  "backup.ir_restore_fallback_none": "Eine Rückfalldatei gibt es nicht: Der alte Stand liess sich vorher nicht lesen.",
  "backup.ir_restore_confirm_1": "{count} von {total} IR-Tasten werden mit den Werten aus der Sicherung überschrieben:\n\n{names}\n\n{fallback}\n\nDer einzige andere Rückweg ist erneutes Anlernen mit der Fernbedienung — dabei blockiert die Uhr, bis du alle Tasten gedrückt hast.\n\nJetzt überschreiben?",
  "backup.ir_restore_confirm_2": "Ohne Rückfalldatei gibt es keinen Rückweg: Eine falsch geschriebene Taste bekommst du nur durch erneutes Anlernen wieder hin. Im Zweifel brich jetzt ab.\n\nTrotzdem überschreiben?",
  "backup.ir_restore_cancelled": "Die IR-Codes wurden nicht überschrieben — du hast abgebrochen.",
  "backup.ir_restore_writing": "Schreibe IR-Taste {done} von {count}: {name}...",
  "backup.ir_restore_verifying": "Lese die IR-Codes zur Gegenprobe wieder aus...",
  "backup.ir_restore_ok": "{written} von {total} IR-Tasten geschrieben und nachgelesen, keine Abweichung.",
  "backup.ir_restore_skipped": "{skipped} von {total} Tasten stehen in der Sicherung als nie angelernt und blieben unberührt.",
  "backup.ir_restore_invalid_entries": "Unbrauchbare Einträge in der Sicherung ({count} von {total}), nicht geschrieben: {names}.",
  "backup.ir_restore_write_failed": "{failed} von {count} IR-Tasten liessen sich nicht schreiben: {names}.",
  "backup.ir_restore_mismatch": "Achtung: Die Gegenprobe meldet {mismatches} von {written} IR-Tasten abweichend ({names}). Die Uhr hat diese Codes nicht so übernommen, wie sie in der Sicherung stehen. {fallback}",
  "backup.ir_restore_unverified": "Achtung: {written} von {total} IR-Tasten wurden geschrieben, die Gegenprobe liess sich aber nicht durchführen, weil {reason}. Ob die Codes angekommen sind, ist damit offen. {fallback}",
  "backup.ir_restore_section_invalid": "Die IR-Codes wurden übersprungen: Der Abschnitt in der Sicherung hat nicht genau {expected} Einträge mit den erwarteten Namen.",
  "backup.ir_restore_more_names": " und {rest} weitere",
  "backup.ir_reason_stale": "der ESP den laufenden Abzug verworfen hat",
  "backup.ir_reason_incomplete": "der Abzug unvollständig blieb",
  "backup.ir_reason_unavailable": "die Uhr die dafür nötigen Abfragen nicht kennt",
  "backup.ir_reason_invalid": "die Uhr unbrauchbare Werte geliefert hat",
  "backup.ir_reason_download": "sich die Datei nicht herunterladen liess",
  "backup.network_skipped_empty_ssid": "WLAN-Zugangsdaten übersprungen: In der Sicherung steht keine SSID. Die bestehenden Einstellungen bleiben unverändert.",
  "backup.choose_file_first": "Bitte zuerst eine Sicherungsdatei auswählen.",
  "backup.invalid_format": "Ungültiges Dateiformat – keine gültige WordClock-Sicherungsdatei.",
  "backup.incompatible_version": "Inkompatible Backup-Version – Datei mit einer neueren App erstellt.",
  "backup.import_older_version": "Die Sicherung stammt aus einer älteren App-Version ({version} statt {current}). Einstellungen, die es damals noch nicht gab, bleiben unverändert.",
  "backup.import_failed": "Einstellungen konnten nicht importiert werden.",
  "backup.import_confirm": "Einstellungen aus „{file}“ jetzt importieren?",
  "backup.import_start": "Starte Wiederherstellung der Sicherung...",
  "backup.import_validate": "Prüfe importierte Einstellungen...",
  "backup.import_verify_sections": "Prüfe Display, Klima, Overlays und Timer...",
  "backup.import_reload_data": "Lade aktualisierte Gerätedaten neu...",
  "backup.import_network_final": "Übernehme Netzwerk-Einstellungen abschliessend...",
  "backup.import_network_verify": "Prüfe Netzwerk-Einstellungen erneut...",
  "backup.import_network_wait": "Warte auf die Übernahme der Netzwerkeinstellungen...",
  "backup.import_sensor_final": "Übernehme Sensor-Korrekturen abschliessend...",
  "backup.import_sensor_verify": "Prüfe Sensor-Korrekturen erneut...",
  "backup.import_persist_critical": "Schreibe kritische Einstellungen dauerhaft...",
  "backup.import_persist_temperature": "Schreibe Temperatur-Korrekturen endgültig...",
  "backup.import_temperature_final": "Schreibe Temperatur-Korrekturen abschliessend...",
  "backup.import_wait_persist": "Warte, bis Einstellungen dauerhaft gespeichert sind...",
  "backup.import_restart_now": "Import abgeschlossen. STM32 wird jetzt automatisch neu gestartet...",
  "backup.import_restart_reload_data": "STM32 wurde neu gestartet. Lade Daten neu...",
  "backup.import_restart_refreshing": "Import abgeschlossen. Uhr startet neu, App wird aktualisiert...",
  "backup.import_service": "Übernehme Wartungs-Einstellungen...",
  "backup.import_assets": "Stelle Dateien und Assets wieder her...",
  "backup.import_display": "Übernehme Display-Einstellungen...",
  "backup.import_climate": "Übernehme Klima- und Wetter-Einstellungen...",
  "backup.import_animations": "Übernehme Animations-Einstellungen...",
  "backup.import_tft": "Übernehme TFT-Einstellungen...",
  "backup.import_ambilight": "Übernehme Ambilight-Einstellungen...",
  "backup.import_dfplayer": "Übernehme DFPlayer-Einstellungen...",
  "backup.import_overlays": "Übernehme Overlays...",
  "backup.overlay_clear_failed": "{count} von 32 Overlay-Plätzen konnten nicht geleert werden. Dort können alte Einträge stehen bleiben – prüfe die Overlays nach dem Import.",
  "backup.import_timers": "Übernehme Timer...",
  "backup.import_display_retry": "Übernehme Display-Einstellungen erneut...",
  "backup.import_climate_retry": "Übernehme Klima- und Wetter-Einstellungen erneut...",
  "backup.import_overlays_retry": "Übernehme Overlays erneut...",
  "backup.import_timers_retry": "Übernehme Timer erneut...",
  "backup.import_temperature_retry": "Übernehme Temperatur-Korrekturen erneut...",
  "backup.import_rtc_final": "Übernehme RTC-Korrektur abschliessend...",
  "backup.import_rtc_retry": "Übernehme RTC-Korrektur erneut...",
  "backup.import_network_retry": "Übernehme Netzwerk- und Zeiteinstellungen erneut...",
  "backup.import_network_time_retry": "Übernehme Zeitserver- und Uhrzeit-Einstellungen erneut...",
  "backup.import_maintenance_retry": "Übernehme Update-Host und Update-Pfad erneut...",
  "backup.import_network_time_final": "Übernehme Zeitserver- und Uhrzeit-Einstellungen abschliessend...",
  "backup.import_maintenance_final": "Übernehme Update-Host und Update-Pfad abschliessend...",
  "backup.import_sensor_persist_final": "Übernehme Sensor-Korrekturen als letzten Persistenzschritt...",
  "backup.import_restart": "Import abgeschlossen. STM32 wird automatisch neu gestartet",
  "backup.import_reload": "Import abgeschlossen. App wird neu geladen",
  "backup.import_reconnect": "Import abgeschlossen. Verbindung wird nach dem Neustart erneut aufgebaut",
  "backup.import_done_reload": "Import abgeschlossen. App wird neu geladen...",
  "backup.import_skipped_fields": "Übersprungen, weil in der Sicherung leer: {fields}. Diese Werte sind auf der Uhr unverändert geblieben.",
  "backup.import_stage_failed": "Abgebrochene Importschritte: {stages}. Was in ihnen noch folgen sollte, ist nicht geschrieben worden — sieh dir diese Bereiche an.",
  "backup.field.timeserver": "Zeitserver",
  "backup.field.weather_appid": "Wetter-API-Schlüssel",
  "backup.field.weather_location": "Ort und Koordinaten für das Wetter",
  "backup.field.date_ticker_format": "Datumsformat des Tickers",
  "backup.field.update_host": "Update-Host",
  "backup.field.update_path": "Update-Pfad",
  "backup.field.rtc_temp_correction": "Temperaturkorrektur der RTC",
  "backup.field.ds18xx_temp_correction": "Temperaturkorrektur des DS18xx",
  "backup.field.display_mode": "Anzeigemodus",
  "backup.field.display_brightness": "Helligkeit der Anzeige",
  "backup.field.ticker_deceleration": "Verzögerung des Tickers",
  "backup.field.animation_mode": "Anzeigeanimation",
  "backup.field.color_animation_mode": "Farbanimation",
  "backup.field.animation_deceleration": "Verzögerung einer Anzeigeanimation",
  "backup.field.color_animation_deceleration": "Verzögerung einer Farbanimation",
  "backup.field.ambilight_mode": "Ambilight-Modus",
  "backup.field.ambilight_leds": "Anzahl der Ambilight-LEDs",
  "backup.field.ambilight_offset": "Versatz des Ambilights",
  "backup.field.ambilight_brightness": "Helligkeit des Ambilights",
  "backup.field.ambilight_deceleration": "Verzögerung eines Ambilight-Profils",
  "backup.field.dfplayer_volume": "Lautstärke des DFPlayers",
  "backup.field.dfplayer_mode": "Betriebsart des DFPlayers",
  "backup.field.dfplayer_speak_cycle": "Sprechintervall des DFPlayers",
  "backup.field.dfplayer_alarm": "Weckzeit des DFPlayers",
  "backup.field.ldr_min": "Minimalwert des Helligkeitssensors",
  "backup.field.ldr_max": "Maximalwert des Helligkeitssensors",
  "backup.field.color": "Farbanteil",
  "backup.field.dim_curve": "Dimmkurve",
  "backup.field.timer": "Schaltzeit",
  "backup.field.overlay_interval": "Intervall eines Overlays",
  "backup.field.overlay_duration": "Anzeigedauer eines Overlays",
  "backup.field.overlay_days": "Anzahl Tage eines Overlays",
  "backup.field.overlay_start_date": "Startdatum eines Overlays",
  "backup.import_adjusted_fields": "Aus der Sicherung übernommen, aber in den erlaubten Bereich gebracht: {fields}. Die Sicherung enthielt diese Werte ausserhalb der Grenzen — sieh sie dir an.",
  "weather.map_loading": "Kartendienst wird geladen...",
  "weather.map_load_failed": "Kartendienst konnte nicht geladen werden.",
  "weather.map_hint": "Tippe auf die Karte oder suche einen Ort.",
  "weather.enter_location_first": "Bitte zuerst einen Ort eingeben.",
  "weather.search_busy": "sucht...",
  "weather.searching": "Ort wird gesucht...",
  "weather.no_result": "Kein Treffer für diesen Ort gefunden.",
  "weather.search_button": "Suchen",
  "weather.found": "gefunden",
  "weather.location_found": "Ort gefunden und auf der Karte gesetzt.",
  "weather.search_failed": "Ortssuche konnte nicht geladen werden.",
  "weather.current_location_busy": "liest...",
  "weather.current_location_label": "Aktuellen Standort verwenden",
  "weather.current_location_reading": "Aktueller Standort wird gelesen...",
  "weather.current_location_set": "Aktueller Standort gesetzt.",
  "weather.approx_location_start": "Näherungsstandort über Internetverbindung wird ermittelt...",
  "weather.approx_location_city": "Näherungsstandort gesetzt: {city}.",
  "weather.approx_location_set": "Näherungsstandort wurde gesetzt.",
  "weather.location_unavailable": "Standort konnte auch näherungsweise nicht ermittelt werden.",
  "weather.map_applied": "Standort aus Karte übernommen.",
  "weather.reverse_failed": "Koordinaten gesetzt. Ortsname konnte nicht aufgelöst werden.",
  "weather.city_shortened": "Der Ortsname ist länger, als die Uhr speichern kann. Gekürzt auf: {city}",
  "weather.map_preview": "Aus Karte gewählt: {city} | {lon} / {lat}",
  "weather.apply_map_busy": "übernimmt...",
  "weather.apply_map_idle": "In Wetter übernehmen",
  "weather.apply_map_done": "übernommen",
  "weather.apply_map_failed": "Wetter-Ort und Koordinaten konnten nicht übernommen werden",
  "weather.apply_map_success": "Wetter-Ort und Koordinaten wurden übernommen",
  "network.scan_busy": "lädt...",
  "network.scan_success": "WLAN-Liste wurde aktualisiert",
  "network.scan_failed": "WLAN-Liste konnte nicht aktualisiert werden",
  "local_app.folder_read_failed": "Der App-Ordner konnte nicht gelesen werden",
  "local_app.folder_browser_failed": "Der lokale App-Ordner konnte über den Browser nicht geöffnet werden.",
  "local_app.folder_checked": "Lokaler App-Ordner geprüft: {found}/{total} Pflichtdateien erkannt.",
  "local_app.folder_none": "Lokaler App-Ordner wurde gewählt, aber der Browser hat keine passenden App-Dateien unter app/... geliefert.",
  "local_app.note_empty": "Noch kein App-Ordner gewählt. Bitte den Ordner wählen, der die komprimierten Dateien (.gz) unter app/... enthält.",
  "local_app.note_complete": "App-Ordner vollständig erkannt. {found}/{total} Dateien sind bereit und können direkt installiert werden.",
  "local_app.note_missing": "App-Ordner geprüft. {found}/{total} Dateien gefunden. Es fehlen: {missing}",
  "local_app.incomplete": "Der App-Ordner ist noch nicht vollständig.",
  "local_app.empty_assets": "Leere App-Dateien gefunden — es wird nichts hochgeladen",
  "local_app.empty_assets_detail": "Diese Dateien sind 0 Byte gross: {assets}. Eine leere .gz führt auf dem Gerät zum weissen Bildschirm. Baue die App neu und wähle den Ordner erneut.",
  "local_app.missing_required": "Lokale App-Dateien können noch nicht installiert werden. Es fehlen Pflichtdateien.",
  "local_app.upload_unsupported": "Diese Firmware unterstützt noch keinen lokalen App-Datei-Upload.",
  "local_app.install_confirm": "Die lokalen App-Dateien jetzt direkt auf das Gerät schreiben?",
  "local_app.installing": "Lokale App-Dateien werden installiert...",
  "local_app.installing_fs": "Lokale App-Dateien werden direkt in das LittleFS geschrieben...",
  "local_app.progress": "Lokale App-Dateien: {step}/{total} {asset}",
  "local_app.installed_reload": "Lokale App-Dateien installiert. App wird neu geladen...",
  "local_app.installed_status": "Lokale App-Dateien installiert. Seite wird neu geladen.",
  "local_app.installed_fs": "Lokale App-Dateien wurden erfolgreich installiert.",
  "local_app.install_button": "Lokale App-Dateien installieren",
  "local_app.installed": "installiert",
  "local_app.install_failed": "Lokale App-Dateien konnten nicht installiert werden",
  "local_app.install_failed_detail": "Lokale App-Dateien konnten nicht installiert werden: {error}",
  "maintenance.app_install_confirm": "App-Dateien jetzt direkt vom Server laden und installieren?",
  "maintenance.app_install_button": "App-Dateien laden",
  "maintenance.app_install_loading": "App-Dateien werden vom Server geladen...",
  "maintenance.app_install_running": "App-Dateien werden installiert...",
  "maintenance.app_install_loaded": "App-Dateien wurden geladen und werden installiert...",
  "maintenance.app_install_reload": "App-Dateien installiert. App wird neu geladen...",
  "maintenance.app_install_success": "App-Dateien installiert. Seite wird neu geladen.",
  "maintenance.app_install_failed": "App-Dateien konnten nicht geladen werden",
  "maintenance.app_install_failed_detail": "App-Dateien konnten nicht geladen oder installiert werden.",
  "maintenance.layout_choose_first": "Bitte zuerst eine Layout-Tabelle auswählen",
  "maintenance.layout_confirm": "Layout-Tabelle „{file}“ jetzt laden?",
  "maintenance.layout_loading": "Layout-Tabelle wird geladen...",
  "maintenance.layout_loaded": "Layout-Tabelle wurde geladen.",
  "maintenance.layout_load_failed": "Layout-Tabelle konnte nicht geladen werden",
  "maintenance.layout_button": "Layout-Tabelle laden",
  "maintenance.reset_stm32_ok": "STM32 wurde zurückgesetzt",
  "maintenance.reset_stm32_started_wait": "STM32-Reset wurde ausgelöst. Warte auf Abschluss...",
  "maintenance.reset_stm32_reconnect": "STM32 wieder bereit. App wird neu geladen...",
  "maintenance.reset_stm32_reconnect_unclear": "STM32-Reconnect nicht sicher erkannt. App wird vorsorglich neu geladen...",
  "maintenance.esp_ready_reload": "ESP wieder erreichbar. Seite wird neu geladen.",
  "maintenance.reset_eeprom_confirm_1": "EEPROM wirklich auf Werkseinstellungen zurücksetzen?",
  "maintenance.reset_eeprom_confirm_2": "Wirklich alle EEPROM-Werte auf Werkseinstellungen zurücksetzen?",
  "maintenance.reset_eeprom_busy": "setzt zurück...",
  "maintenance.reset_eeprom_wait": "wartet...",
  "maintenance.reset_eeprom_start": "EEPROM-Reset wird ausgelöst...",
  "maintenance.reset_eeprom_restart": "EEPROM-Reset ausgelöst. STM32 wird neu gestartet...",
  "maintenance.reset_eeprom_reload": "Warte auf STM32-Neustart. Danach wird die App neu geladen...",
  "maintenance.reset_eeprom_failed": "EEPROM konnte nicht zurückgesetzt werden",
  "maintenance.device_ready_reload": "Gerät sollte wieder bereit sein. App wird vorsorglich neu geladen.",
  "maintenance.no_reconnect_reload": "Kein sicheres Reconnect-Signal erhalten. App wird vorsorglich neu geladen.",
  "maintenance.esp_not_ready": "ESP ist noch nicht wieder erreichbar. Bitte Seite bei Bedarf manuell neu laden.",
  "maintenance.unsaved_reload_confirm": "Es gibt ungespeicherte Änderungen. App trotzdem neu laden?",
  "modules.unsaved_switch_confirm": "Es gibt ungespeicherte Änderungen. Modul trotzdem wechseln? Deine Bearbeitung geht dabei verloren.",
  "maintenance.reloading": "App wird neu geladen...",
  "maintenance.forced_reload": "Neuladen wird erzwungen, damit die aktualisierte App wieder angezeigt wird.",
  "maintenance.assets_confirm": "Icon-Dateien jetzt wirklich vom Server laden?",
  "maintenance.assets_loaded": "Icon-Dateien geladen",
  "maintenance.assets_load_failed": "Icon-Dateien konnten nicht geladen werden",
  "maintenance.stm32_file_choose_first": "Bitte zuerst eine STM32-Datei auswählen",
  "maintenance.reset_stm32_confirm": "STM32 jetzt wirklich resetten?",
  "maintenance.format_fs_confirm": "LittleFS wirklich formatieren?",
  "maintenance.format_fs_button": "LittleFS formatieren",
  "maintenance.format_fs_done": "LittleFS wurde formatiert.",
  "maintenance.local_esp_waiting": "Lokales ESP-Update läuft. Warte auf Neustart und Reconnect...",
  "maintenance.remote_esp_waiting": "ESP aktualisiert sich gerade. Warte auf Neustart und Reconnect...",
  "maintenance.stm32_auto_reset_wait": "STM32 wurde nach dem Flash automatisch zurückgesetzt. Warte auf Abschluss...",
  "maintenance.stm32_auto_reset_running": "STM32 wird automatisch zurückgesetzt. Daten werden danach neu geladen.",
  "maintenance.stm32_flash_success": "STM32-Update erfolgreich abgeschlossen.",
  "maintenance.fs_showing": "Datei „{file}“ wird angezeigt.",
  "maintenance.fs_deleted": "Datei „{file}“ wurde gelöscht.",
  "maintenance.fs_delete_confirm": "Datei „{file}“ wirklich löschen?",
  "maintenance.file_load_failed": "Datei konnte nicht geladen werden",
  "maintenance.file_delete_failed": "Datei konnte nicht gelöscht werden",
  "overlays.discarded": "Verworfen",
  "overlays.new_discarded": "Neues Overlay verworfen",
  "overlays.show_failed": "Overlay konnte nicht angezeigt werden",
  "overlays.show_status": "Overlay {idx} wird angezeigt",
  "overlays.delete_failed": "Overlay konnte nicht gelöscht werden",
  "overlays.deleted": "Overlay wurde gelöscht",
  "overlays.icon_list_failed": "Icon-Liste konnte nicht geladen werden",
  "timers.saved_all": "Alle Timer wurden gespeichert",
  "timers.save_all_failed": "Timer konnten nicht vollständig gespeichert werden",
  "timers.save_failed": "Timer konnte nicht gespeichert werden",
  "timers.cleared": "Slot geleert",
  "timers.clear_failed": "Timer konnte nicht geleert werden",
  "flags.toggle_failed": "Schalter konnte nicht gesetzt werden",
  "flags.enabled": "aktiviert",
  "flags.disabled": "deaktiviert",
  "flags.ambilight_offline_hint": "Das Ambilight meldet sich gerade nicht. Der angezeigte Zustand stammt aus dem Gerätespeicher.",
  "debug.apply_busy": "übernimmt...",
  "debug.active": "Overrides aktiv",
  "debug.active_short": "aktiv",
  "debug.apply_failed": "Overrides konnten nicht angewendet werden",
  "debug.reset_busy": "setzt zurück...",
  "debug.reset_done": "Overrides zurückgesetzt",
  "debug.reset_short": "zurückgesetzt",
  "debug.reset_failed": "Overrides konnten nicht zurückgesetzt werden",
  "common.error": "Fehler",
  "api.error.1": "Das Feld darf nicht leer sein. Zahlenfelder brauchen zusätzlich eine gültige Zahl. Es wurde nichts gespeichert.",
  "api.error.2": "Der Wert liegt ausserhalb des erlaubten Bereichs. Es wurde nichts gespeichert.",
  "api.error.3": "Der Schlüssel ist zu kurz — mindestens 10 Zeichen. Es wurde nichts gespeichert.",
  "api.error.4": "Datum oder Uhrzeit sind ungültig. Die Uhr wurde nicht gestellt.",
  "api.warning.ldr_min_max": "Der Minimalwert der automatischen Helligkeit liegt nicht unter dem Maximalwert. Solange das so bleibt, regelt die Uhr die Helligkeit gar nicht — und meldet dazu nichts weiter. Setz das Minimum unter das Maximum.",
  "api.error.5": "Dafür fehlen noch Angaben: Trage unter Klima den Wetter-API-Schlüssel ein und dazu entweder einen Ort oder ein vollständiges Koordinatenpaar. Im eigenen Accesspoint hat die Uhr keinen Weg ins Internet.",
  "common.saving": "speichert...",
  "common.loading": "lädt...",
  "common.running": "läuft...",
  "common.connecting": "verbindet...",
  "common.starting": "startet...",
  "common.uploading": "lädt hoch...",
  "common.deleting": "löscht...",
  "common.clearing": "leert...",
  "common.showing": "zeigt...",
  "common.setting": "setzt...",
  "common.saved": "gespeichert",
  "common.loaded": "geladen",
  "common.started": "gestartet",
  "common.switched_on": "eingeschaltet",
  "common.switched_off": "ausgeschaltet",
  "common.applied": "gesetzt",
  "common.set": "gesetzt",
  "common.uploaded": "hochgeladen",
  "common.canceled": "verworfen",
  "common.active": "Aktiv",
  "common.save": "Speichern",
  "common.deleted": "gelöscht",
  "common.cleared": "geleert",
  "common.none": "Keins",
  "common.unknown_device_value": "Unbekannter Gerätewert ({value})",
  "common.offline": "offline",
  "common.invalid_value": "ungültig",
  "common.file": "Datei",
  "common.display": "Anzeigen",
  "common.delete": "Löschen",
  "common.file_upload": "Datei hochladen",
  "common.loading_short": "wird geladen...",
  "common.reloading": "lädt neu...",
  "common.ready": "fertig",
  "common.invalid_file_extension": "Ungültige Dateiendung",
  "display.ambilight_state_saved": "Ambilight-Status gespeichert",
  "display.ambilight_state_set_failed": "Ambilight-Status konnte nicht gesetzt werden",
  "display.brightness_save_failed": "Helligkeit konnte nicht gespeichert werden",
  "display.it_is_set_failed": "„ES IST“ konnte nicht gesetzt werden",
  "display.preset_apply_failed": "Preset konnte nicht angewendet werden",
  "display.mode_save_failed": "Display-Modus konnte nicht gespeichert werden",
  "display.ticker_save_failed": "Ticker konnte nicht gespeichert werden",
  "display.date_format_save_failed": "Datumsformat konnte nicht gespeichert werden",
  "climate.auto_brightness_toggle_failed": "Automatische Helligkeit konnte nicht geschaltet werden",
  "climate.api_key_save_failed": "API-Schlüssel konnte nicht gespeichert werden",
  "climate.city_save_failed": "Ort konnte nicht gespeichert werden",
  "climate.coordinates_save_failed": "Koordinaten konnten nicht gespeichert werden",
  "climate.weather_request_failed": "Wetter konnte nicht angefordert werden",
  "climate.forecast_request_failed": "Wettervorhersage konnte nicht angefordert werden",
  "climate.rtc_correction_save_failed": "RTC-Korrektur konnte nicht gespeichert werden",
  "climate.ds18xx_correction_save_failed": "DS18xx-Korrektur konnte nicht gespeichert werden",
  "climate.temperature_display_failed": "Temperatur konnte nicht angezeigt werden",
  "climate.temperature_display_started": "Temperaturanzeige ausgelöst",
  "climate.ldr_min_set_failed": "LDR-Minimum konnte nicht gesetzt werden",
  "climate.ldr_min_saved": "LDR-Minimum gespeichert",
  "climate.ldr_max_set_failed": "LDR-Maximum konnte nicht gesetzt werden",
  "climate.ldr_max_saved": "LDR-Maximum gespeichert",
  "network.timeserver_save_failed": "Zeitserver konnte nicht gespeichert werden",
  "network.timezone_save_failed": "Zeitzone konnte nicht gespeichert werden",
  "network.summertime_set_failed": "Sommerzeit konnte nicht gesetzt werden",
  "network.nettime_failed": "Netzzeit konnte nicht angefordert werden",
  "network.nettime_requested": "Netzzeit angefordert",
  "network.wps_failed": "WPS konnte nicht gestartet werden",
  "network.wps_started": "WPS wurde gestartet",
  "system.datetime_save_failed": "Datum und Uhrzeit konnten nicht gespeichert werden",
  "system.learn_ir_failed": "IR-Lernmodus konnte nicht gestartet werden",
  "system.learn_ir_started": "IR-Lernmodus gestartet",
  "maintenance.update_host_save_failed": "Update-Host konnte nicht gespeichert werden",
  "maintenance.update_path_save_failed": "Update-Pfad konnte nicht gespeichert werden",
  "maintenance.format_fs_failed": "LittleFS konnte nicht formatiert werden",
  "maintenance.preview_empty": "(leer)",
  "maintenance.target_uploads_unsupported": "PWA-Zieluploads werden von dieser Firmware noch nicht unterstützt.",
  "maintenance.upload_file_done": "Datei wurde hochgeladen.",
  "maintenance.upload_table_done": "Layout-Tabelle wurde hochgeladen.",
  "maintenance.upload_display_done": "TFT-Display-Datei wurde hochgeladen.",
  "animations.display_animation_save_failed": "Anzeigeanimation konnte nicht gespeichert werden",
  "animations.color_animation_save_failed": "Farbanimation konnte nicht gespeichert werden",
  "backup.file_selected": "Ausgewählt: {name}",
  "backup.no_file_selected": "Noch keine Sicherungsdatei ausgewählt.",
  "backup.exporting": "exportiert...",
  "backup.importing": "importiert...",
  "backup.checking_files": "Prüfe benötigte Dateien...",
  "backup.restoring_layout_table": "Stelle Layout-Tabelle wieder her...",
  "backup.restoring_assets": "Stelle Icon- und Overlay-Dateien wieder her...",
  "backup.importing_timers": "Importiere Timer...",
  "weather.locating_short": "ermittelt...",
  "weather.geo_denied": "Standortfreigabe wurde abgelehnt. Näherungsstandort wird ermittelt...",
  "weather.geo_unavailable": "Standort ist derzeit nicht verfügbar. Näherungsstandort wird ermittelt...",
  "weather.geo_timeout": "Standortabfrage lief in ein Zeitlimit. Näherungsstandort wird ermittelt...",
  "weather.geo_failed": "Standort konnte nicht gelesen werden. Näherungsstandort wird ermittelt...",
  "display.led_note_debug_rgbw": "Debug Override aktiv. Farb-LED UI wird als RGBW angezeigt.",
  "display.led_note_debug_rgb": "Debug Override aktiv. Farb-LED UI wird als RGB angezeigt.",
  "display.led_note_rgbw": "RGBW-Hardware erkannt. RGB- und Weisskanal sind verfügbar.",
  "display.led_note_tft": "TFT-Hardware erkannt. TFT-Optionen sind verfügbar, der Weisskanal bleibt ausgeblendet.",
  "display.led_note_rgb": "RGB-Hardware erkannt. Der Weisskanal ist deshalb ausgeblendet.",
  "display.led_note_none": "Keine unterstützte Farb-LED-Hardware erkannt. Farbsteuerung ist deshalb ausgeblendet.",
  "display.led_label_none": "keine Farb-LEDs erkannt",
  "maintenance.no_layout_tables": "keine Layout-Tabellen gefunden",
  "common.unknown_error": "unbekannter Fehler",
  "maintenance.local_esp_wrong_file": "Falsche ESP-Datei ausgewählt. Erwartet wird eine .bin-Datei.",
  "maintenance.local_esp_upload_progress": "ESP-Firmware wird hochgeladen: {percent}%",
  "maintenance.local_esp_upload_failed": "ESP-Firmware konnte nicht hochgeladen werden: {error}",
  "maintenance.local_stm32_expected_fallback": "passende STM32-.hex-Datei",
  "maintenance.local_stm32_wrong_file": "Falsche STM32-Datei ausgewählt. Erwartet wird {expected}.",
  "maintenance.target_expected": "{target} erwartet",
  "maintenance.target_uploading": "{target} wird hochgeladen: {name}",
  "maintenance.target_uploaded_saving": "{target} wurde hochgeladen und wird jetzt gespeichert...",
  "maintenance.target_upload_progress": "{target} wird hochgeladen: {percent}%",
  "maintenance.remote_stm32_start_failed": "Remote STM32-Flash konnte nicht gestartet werden.",
  "maintenance.stm32_flash_end_unclear": "STM32-Flash-Ende konnte nicht sicher erkannt werden.",
  "maintenance.stm32_reset_after_flash": "STM32 wurde nach dem Flash automatisch zurückgesetzt",
  "maintenance.reload_after_stm32_failed": "Daten konnten nach dem STM32-Update nicht neu geladen werden",
  "maintenance.stm32_flash_done_confirming": "STM32-Flash abgeschlossen. Abschluss wird bestätigt...",
  "common.toggle_failed": "Schalter konnte nicht gesetzt werden",
  "common.action_failed": "Aktion konnte nicht ausgeführt werden",
  "common.installing": "wird installiert...",
  "common.uploading_percent": "lädt hoch... {percent}%"
};
// Eine Zeile je Sprache. Der Dateiname folgt dem Code: /app/i18n/<code>.json.
// Der Eigenname steht bewusst in der Sprache selbst -- wer Englisch sucht, findet
// "English", auch wenn die Oberfläche gerade deutsch ist. Eine vierte Sprache ist
// damit diese eine Datenzeile, die Datei und ein Eintrag in APP_INSTALL_ASSETS
// (http.cpp). Keine Logikänderung, keine Änderung an den Werkzeugen.
const LANGUAGES = [
  { code: "de", label: "Deutsch", table: I18N_DE },
  { code: "en", label: "English" }
];
const LANGUAGE_FETCH_TIMEOUT_MS = 4000;
const LOCAL_APP_REQUIRED_ASSETS = [
  "app/index.html.gz",
  "app/styles.css.gz",
  "app/app.js.gz",
  "app/sw.js.gz",
  "app/layout-previews.json.gz",
  "app/manifest.webmanifest.gz",
  "app/icons/icon-192.svg.gz",
  "app/icons/icon-512.svg.gz"
];
const CONNECTION_STABILITY = {
  fastReadAttempts: 1,
  slowReadAttempts: 2,
  retryDelayMs: 220,
  apiWriteTimeoutMs: 6500,
  coreSettingsTimeoutMs: 3200,
  corePowerTimeoutMs: 2200,
  updateStatusTimeoutMs: 4500,
  updateTableInfoTimeoutMs: 4500,
  networkScanTimeoutMs: 3800,
  overlayIconsTimeoutMs: 2200,
  fsInfoTimeoutMs: 5500,
  fsListTimeoutMs: 6500,
  eepromSettingsTimeoutMs: 5500,
  stm32LogTimeoutMs: 3800,
  progressPollTimeoutMs: 1800,
  progressPollIntervalMs: 900,
  frameProbeTimeoutMs: 2500
};
const DIM_CURVE_PRESETS = {
  linear: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
  sanft: [0, 0, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15],
  kontrast: [0, 0, 0, 1, 1, 2, 3, 5, 7, 9, 11, 12, 13, 14, 15, 15],
  nacht: [0, 0, 0, 0, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12]
};
const DIM_CURVE_PRESET_NAMES = {
  custom: "Individuell",
  linear: "Linear",
  sanft: "Sanft",
  kontrast: "Kontrast",
  nacht: "Nacht"
};
const DAYLIGHT_RED =   [0, 0, 0, 15, 31, 47, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 47, 31, 15, 0, 0];
const DAYLIGHT_GREEN = [0, 0, 0,  0,  0,  0,  0,  0,  0, 15, 31, 47, 63, 47, 31, 15,  0,  0,  0,  0,  0,  0, 0, 0];
const DAYLIGHT_BLUE =  [63, 47, 31, 15, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 15, 31, 47, 63, 63, 63, 63, 63, 63];
const RAINBOW_PREVIEW_COLOR = { red: 0, green: 0, blue: 63, white: 0 };

let espReloadWatchdogId = 0;
const TABLES_VERSION_MAGIC = 0xff;
const WP_IF_HOUR_IS_0 = 0xfe;
const WP_IF_MINUTE_IS_0 = 0xff;
const MDF_IT_IS_1 = 0x01;
const MDF_HOUR_OFFSET_1 = 0x02;
const MDF_HOUR_OFFSET_2 = 0x04;
const ILLUMINATION_LEN_MASK = 0x1f;
const ILLUMINATION_FLAG_IT_IS = 0x80;
const ILLUMINATION_FLAG_AM = 0x40;
const ILLUMINATION_FLAG_PM = 0x20;

const NUM = {
  DISPLAY_USE_RGBW: 0,
  EEPROM_IS_UP: 1,
  RTC_IS_UP: 2,
  DISPLAY_POWER: 3,
  DISPLAY_MODE: 4,
  SSD1963_FLAGS: 5,
  DISPLAY_BRIGHTNESS: 6,
  DISPLAY_FLAGS: 7,
  DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE: 8,
  AMBILIGHT_IS_UP: 9,
  ANIMATION_MODE: 10,
  AMBILIGHT_MODE: 11,
  AMBILIGHT_LEDS: 12,
  AMBILIGHT_OFFSET: 13,
  AMBILIGHT_BRIGHTNESS: 14,
  COLOR_ANIMATION_MODE: 15,
  LDR_RAW_VALUE: 16,
  LDR_MIN_VALUE: 17,
  LDR_MAX_VALUE: 18,
  TIMEZONE: 19,
  DS18XX_IS_UP: 20,
  RTC_TEMP_INDEX: 21,
  RTC_TEMP_CORRECTION: 22,
  DS18XX_TEMP_INDEX: 23,
  DS18XX_TEMP_CORRECTION: 24,
  HARDWARE_CONFIGURATION: 29,
  DISPLAY_AMBILIGHT_POWER: 30,
  TICKER_DECELERATION: 31,
  DFPLAYER_IS_UP: 32,
  DFPLAYER_VERSION: 33,
  DFPLAYER_VOLUME: 34,
  DFPLAYER_SILENCE_START: 35,
  DFPLAYER_SILENCE_STOP: 36,
  DFPLAYER_MODE: 37,
  DFPLAYER_BELL_FLAGS: 38,
  DFPLAYER_SPEAK_CYCLE: 39,
  DISPLAY_OVERLAY: 45,
  OVERLAY_N_OVERLAYS: 46,
  UPTIME_SECONDS_LO: 47,
  UPTIME_SECONDS_HI: 48
};

const STR = {
  TICKER_TEXT: 0,
  VERSION: 1,
  EEPROM_VERSION: 2,
  ESP8266_VERSION: 3,
  TIMESERVER: 4,
  WEATHER_APPID: 5,
  WEATHER_CITY: 6,
  WEATHER_LON: 7,
  WEATHER_LAT: 8,
  UPDATE_HOST: 9,
  UPDATE_PATH: 10,
  DATE_TICKER_FORMAT: 11,
  RESET_CAUSE: 12
};

const BACKUP_FORMAT = "wordclock-settings-backup";
const BACKUP_VERSION = 3;

// Reihenfolge und Schreibweise 1:1 aus src/remote-ir/remote-ir.h:38-67 --
// REMOTE_IR_CMD_* in Kleinschrift ohne Präfix.
//
// Der NAME ist der Schlüssel beim Import, der Index ist informativ. In
// remote-ir.h:19-34 steht ein auskommentierter Block "New Modes (future use)".
// Wird er je aktiviert, verschiebt sich die Nummerierung vollständig -- eine
// namensbasierte Zuordnung bleibt dann richtig, eine indexbasierte würde still
// auf die falschen Tasten schreiben.
const IR_BACKUP_KEY_NAMES = [
  "power",
  "ok",
  "decrement_display_mode",
  "increment_display_mode",
  "decrement_animation_mode",
  "increment_animation_mode",
  "decrement_hour",
  "increment_hour",
  "decrement_minute",
  "increment_minute",
  "decrement_brightness_red",
  "increment_brightness_red",
  "decrement_brightness_green",
  "increment_brightness_green",
  "decrement_brightness_blue",
  "increment_brightness_blue",
  "decrement_brightness",
  "increment_brightness",
  "auto_brightness_control",
  "get_temperature"
];

const HW = {
  STM32_MASK: 0x07,
  WC_MASK: 0x07 << 3,
  LED_MASK: 0x07 << 6,
  OSC_MASK: 0x07 << 9,
  WC_24H: 0x00 << 3,
  WC_12H: 0x01 << 3,
  UCLOCK: 0x02 << 3,
  LED_WS2812_GRB: 0x00 << 6,
  LED_WS2812_RGB: 0x01 << 6,
  LED_APA102_RGB: 0x02 << 6,
  LED_SK6812_RGB: 0x03 << 6,
  LED_SK6812_RGBW: 0x04 << 6,
  LED_TFT_RGB: 0x05 << 6
};

const fallbackWordclockRows = [
  "ESKISTAFUNF",
  "ZEHNZWANZIG",
  "DREIVIERTEL",
  "VORFUNKNACH",
  "HALBAELFUNF",
  "EINSXAMZWEI",
  "DREIPMJVIER",
  "SECHSNLACHT",
  "SIEBENZWOLF",
  "ZEHNEUNKUHR"
];

const DEFAULT_LAYOUT_PREVIEW_ROWS = {
  "wc12h-tables-de.txt": [
    "ESKISTLFÜNF",
    "ZEHNZWANZIG",
    "DREIVIERTEL",
    "TGNACHVORJM",
    "HALBQZWÖLFP",
    "ZWEINSIEBEN",
    "KDREIRHFÜNF",
    "ELFNEUNVIER",
    "WACHTZEHNRS",
    "BSECHSFMUHR"
  ],
  "wc24h-tables-de.txt": [
    "ES#IST#VIERTELEINS",
    "DREINERSECHSIEBEN#",
    "ELFÜNFNEUNVIERACHT",
    "NULLZWEI#ZWÖLFZEHN",
    "UND#ZWANZIGVIERZIG",
    "DREISSIGFÜNFZIGUHR",
    "MINUTEN#VORUNDNACH",
    "EINDREIVIERTELHALB",
    "SIEBENEUNULLZWEINE",
    "FÜNFSECHSNACHTVIER",
    "DREINSUND#ELF#ZEHN",
    "ZWANZIGGRADREISSIG",
    "VIERZIGZWÖLFÜNFZIG",
    "MINUTENUHR#FRÜHVOR",
    "ABENDSMITTERNACHTS",
    "MORGENSWARMMITTAGS"
  ]
};
const LAYOUT_PREVIEW_ROWS_URL = "/app/layout-previews.json";

function getOverlayTypeNames() {
  return [
    translate("overlays.type_none"),
    translate("overlays.type_icon"),
    translate("overlays.type_date"),
    translate("overlays.type_temperature"),
    translate("overlays.type_weather_icon"),
    translate("overlays.type_weather_ticker"),
    translate("overlays.type_ticker"),
    translate("overlays.type_dfplayer"),
    translate("overlays.type_forecast_icon"),
    translate("overlays.type_forecast_ticker"),
    translate("overlays.type_temperature_digits")
  ];
}

function getOverlayDateCodeNames() {
  return [
    translate("overlays.datecode_none"),
    translate("overlays.datecode_carnival"),
    translate("overlays.datecode_easter"),
    translate("overlays.datecode_advent1"),
    translate("overlays.datecode_advent2"),
    translate("overlays.datecode_advent3"),
    translate("overlays.datecode_advent4")
  ];
}

const MONTH_OPTIONS = [
  "",
  "01",
  "02",
  "03",
  "04",
  "05",
  "06",
  "07",
  "08",
  "09",
  "10",
  "11",
  "12"
];

let overlayIconsCache = [];
let weatherMap = null;
let weatherMarker = null;
let selectedWeatherLocation = null;
let currentFsFiles = [];
let currentLayoutPreview = null;
let currentSettingsSnapshot = null;
let currentEepromSettings = null;
let currentUpdateStatus = {};
let currentUpdateTableInfo = {};
let currentUpdateTableInfoLoaded = false;
let currentNetworkInfo = {};
const layoutPreviewCache = {};
let layoutPreviewRowsMap = null;
let layoutPreviewRowsMapPromise = null;
let leafletAssetsPromise = null;
let activeLoadCount = 0;
let loadRequestSerial = 0;
let lastWordclockRenderSignature = "";
let lastWordclockThemeSignature = "";
let pendingProgressAction = "";
let pendingProgressButtonId = "";
let stm32ProgressTimer = 0;
let stm32ProgressStage = 0;
let stm32ProgressAdvanceTimer = 0;
let updateProgressPollTimer = 0;
let stm32AutoResetStarted = false;
let stm32RemoteStreamOffset = 0;
let stm32RemoteRequestInFlight = false;
let stm32RemoteResultOkSeen = false;
let hasUnsavedEdits = false;
let wordclockSizingFrame = 0;
let wordclockSizingTimeout = 0;
let wordclockResizeObserver = null;
let liveDisplayColorTimer = 0;
let currentLiveDisplayColor = null;
let lastLiveDisplayColorMode = 0;
const LIVE_DISPLAY_COLOR_POLL_INTERVAL_MS = 5000;
let localAppSelectedFiles = new Map();
let overlayEditorState = null;
let stm32LogTimer = 0;
let stm32LogRefreshInFlight = false;
// Ob im Logfeld gerade echte Zeilen stehen. Ohne diese Angabe muesste eine
// Fehlermeldung entweder den bereits gezeigten Inhalt ueberschreiben oder gar nicht
// erscheinen -- beides falsch. B17.
let stm32LogShowsLines = false;
let settingsImportInProgress = false;
let progressReturnScrollY = null;
let appServiceWorkerRegistration = null;
let appServiceWorkerUpdateApplied = false;
let initialLoadRetryTimer = 0;
let initialLoadAttemptCount = 0;
let reloadBootstrapPending = false;
let reloadBootstrapTimers = [];
let startupLoadScheduled = true;
let startupLoadIssued = false;
let startupLoadIssuedAt = 0;
let lastSuccessfulLoadAt = 0;
let backgroundPauseUntil = 0;
let backgroundPauseFinishTimer = 0;

const INITIAL_LOAD_RETRY_DELAYS_MS = [1800, 3200, 5000];
const RELOAD_BOOTSTRAP_RETRY_DELAYS_MS = [300, 700, 1400, 2400, 3600];
const RELOAD_BOOTSTRAP_LOAD_DELAYS_MS = [250, 900, 1800, 3200];

const DEBUG_STORAGE_KEY = "wordclock-app-debug-overrides";
const MODULE_STORAGE_KEY = "wordclock-app-active-module";
const AMBILIGHT_STORAGE_KEY = "wordclock-app-ambilight-online";
const LAYOUT_PREVIEW_STORAGE_KEY = "wordclock-app-layout-preview";
const LIVE_DISPLAY_COLOR_STORAGE_KEY = "wordclock-app-live-display-color";
const INSECURE_CONTEXT_HINT_KEY = "wordclock-app-insecure-context-hint";
const PROGRESS_SCROLL_RESTORE_KEY = "wordclock-progress-scroll-restore";
const LEAFLET_CSS_URL = "https://unpkg.com/leaflet@1.9.4/dist/leaflet.css";
const LEAFLET_JS_URL = "https://unpkg.com/leaflet@1.9.4/dist/leaflet.js";
let currentLanguage = DEFAULT_LANGUAGE;
// Einmal geladene Tabellen bleiben im Speicher: Zweimal hin und zurück schalten
// erzeugt keinen zweiten Abruf. Ohne Prototyp, damit ein Schlüssel "__proto__"
// oder "constructor" aus einer Fremddatei folgenlos bleibt.
const loadedLanguageTables = Object.create(null);
loadedLanguageTables[DEFAULT_LANGUAGE] = I18N_DE;
// Genau ein Ladevorgang zählt. Schaltet der Nutzer währenddessen weiter, wird die
// überholte Antwort verworfen statt angewendet -- weder Anzeige noch Auswahlfeld
// noch localStorage dürfen von einer veralteten Anfrage verändert werden.
let pendingLanguage = null;

function languageEntry(code) {
  return LANGUAGES.find((entry) => entry.code === code) || null;
}

function languageLabel(code) {
  const entry = languageEntry(code);
  return entry ? entry.label : String(code || "");
}

function normalizeLanguage(language) {
  return languageEntry(language) ? language : DEFAULT_LANGUAGE;
}

function getStoredLanguage() {
  try {
    return normalizeLanguage(window.localStorage.getItem(LANGUAGE_STORAGE_KEY));
  } catch (_) {
    return DEFAULT_LANGUAGE;
  }
}

// Bleibt bewusst synchron -- die rund 900 Aufrufstellen werden nicht auf await
// umgestellt. Fehlt die Tabelle oder fehlt ein einzelner Schlüssel darin, steht
// deutscher Text da und nie ein roher Schlüssel.
function translate(key) {
  const active = loadedLanguageTables[currentLanguage] || I18N_DE;
  return active[key] || I18N_DE[key] || key;
}

function translateFormat(key, values) {
  return translate(key).replace(/\{(\w+)\}/g, (_, name) => {
    if (!values || values[name] === undefined || values[name] === null) {
      return "";
    }
    return String(values[name]);
  });
}

function applyStaticTranslations() {
  document.documentElement.lang = currentLanguage;
  document.title = translate("app.title");

  document.querySelectorAll("[data-i18n]").forEach((element) => {
    const key = element.getAttribute("data-i18n");
    if (key) {
      const translated = translate(key);
      element.textContent = translated;
      if (element.tagName === "BUTTON") {
        element.dataset.restoreText = translated;
      }
    }
  });

  document.querySelectorAll("[data-i18n-aria-label]").forEach((element) => {
    const key = element.getAttribute("data-i18n-aria-label");
    if (key) {
      element.setAttribute("aria-label", translate(key));
    }
  });

  document.querySelectorAll("[data-i18n-placeholder]").forEach((element) => {
    const key = element.getAttribute("data-i18n-placeholder");
    if (key) {
      element.setAttribute("placeholder", translate(key));
    }
  });

  const languageSelect = document.getElementById("language-select");
  if (languageSelect && languageSelect.value !== currentLanguage) {
    languageSelect.value = currentLanguage;
  }

  if (appVersionLabel) {
    appVersionLabel.textContent = translate("app.version_label") + " " + APP_VERSION;
  }
}

// Die Optionen entstehen aus LANGUAGES, nicht aus dem Markup: Eine neue Sprache
// soll in genau einer Datei eingetragen werden. textContent statt innerHTML --
// die Eigennamen gehen damit ohne escapeHtml sicher in die Seite.
function renderLanguageOptions() {
  const languageSelect = document.getElementById("language-select");

  if (!languageSelect) {
    return;
  }

  languageSelect.textContent = "";
  LANGUAGES.forEach((entry) => {
    const option = document.createElement("option");
    option.value = entry.code;
    option.textContent = entry.label;
    languageSelect.appendChild(option);
  });
  languageSelect.value = currentLanguage;
}

function setLanguageSelectBusy(busy) {
  const languageSelect = document.getElementById("language-select");

  if (!languageSelect) {
    return;
  }

  languageSelect.disabled = busy === true;
  languageSelect.setAttribute("aria-busy", busy === true ? "true" : "false");
}

// Grenze zwischen Fremddaten und Anwendung: Die Datei kommt über unverschlüsseltes
// HTTP von einem konfigurierbaren Host. Übernommen werden ausschliesslich eigene
// Schlüssel mit Zeichenkettenwert, und das Ziel hat keinen Prototyp.
function sanitizeLanguageTable(payload) {
  if (!payload || typeof payload !== "object" || Array.isArray(payload)) {
    throw { kind: "failed", reason: "shape" };
  }

  const table = Object.create(null);

  Object.keys(payload).forEach((key) => {
    if (typeof payload[key] === "string") {
      table[key] = payload[key];
    }
  });

  if (!Object.keys(table).length) {
    throw { kind: "failed", reason: "empty" };
  }

  return table;
}

async function ensureLanguageTable(code) {
  if (loadedLanguageTables[code]) {
    return loadedLanguageTables[code];
  }

  // "?v=" gegen alte Stände im Service-Worker-Cache. Das Gleichheitszeichen ist
  // Pflicht: Ein Parameter ohne "=" lässt den ESP abstürzen (CLAUDE.md, R5).
  const url = "/app/i18n/" + encodeURIComponent(code) + ".json?v=" + encodeURIComponent(APP_VERSION);
  const response = await fetchWithTimeout(url, { cache: "default" }, LANGUAGE_FETCH_TIMEOUT_MS);

  if (response.status === 404) {
    throw { kind: "missing" };
  }

  if (!response.ok) {
    throw { kind: "failed", status: response.status };
  }

  // Länge prüfen, nicht nur response.ok: Bei 0 Byte ist ok weiterhin true, und
  // eine leere Datei hielte sich sonst dauerhaft im Service-Worker-Cache.
  const text = await response.text();

  if (!text || !text.trim().length) {
    throw { kind: "failed", reason: "empty" };
  }

  loadedLanguageTables[code] = sanitizeLanguageTable(JSON.parse(text));
  return loadedLanguageTables[code];
}

async function requestLanguage(code, persist) {
  const target = normalizeLanguage(code);
  const previous = currentLanguage;

  // Den Token beansprucht jede Anfrage, auch die sofort erfüllbare. Sonst
  // überholt eine noch laufende Ladung die spätere Wahl des Nutzers und setzt
  // eine Sprache, die er gar nicht mehr will -- nachgewiesen im Wettlauftest.
  pendingLanguage = target;

  if (target === previous || loadedLanguageTables[target]) {
    pendingLanguage = null;
    setLanguageSelectBusy(false);
    applyLanguage(target, persist);
    return;
  }

  setLanguageSelectBusy(true);

  try {
    await ensureLanguageTable(target);

    if (pendingLanguage !== target) {
      return;
    }

    applyLanguage(target, persist);
  } catch (error) {
    if (pendingLanguage !== target) {
      return;
    }

    // Sichtbar zurück, nicht stumm: Das Auswahlfeld darf nie eine Sprache zeigen,
    // die nicht dasteht. Zwei Fehlerklassen, weil sie verschiedene Abhilfen haben
    // -- 404 heisst "Datei neu hochladen", alles andere "noch einmal versuchen".
    applyLanguage(previous, false);
    const missing = error && error.kind === "missing";
    showStatusBanner(translateFormat(missing ? "language.load_missing" : "language.load_failed", {
      language: languageLabel(target)
    }), "error");
    console.warn("Sprachtabelle '" + target + "' konnte nicht geladen werden", error);
  } finally {
    if (pendingLanguage === target) {
      pendingLanguage = null;
      setLanguageSelectBusy(false);
    }
  }
}

function applyLanguage(language, persist) {
  currentLanguage = normalizeLanguage(language);

  // Erst nach dem geglückten Wechsel speichern. Nach einem Fehlschlag bleibt der
  // gespeicherte Wunsch unverändert stehen und wird beim nächsten Start erneut
  // versucht; document.documentElement.lang folgt in applyStaticTranslations
  // damit immer dem tatsächlich Angezeigten.
  if (persist !== false) {
    try {
      window.localStorage.setItem(LANGUAGE_STORAGE_KEY, currentLanguage);
    } catch (_) {
    }
  }

  applyStaticTranslations();
}

function bindElementEvent(id, eventName, handler) {
  const element = document.getElementById(id);

  if (!element) {
    return;
  }

  element.addEventListener(eventName, handler);
}

function bindDomEvent(target, eventName, handler, options) {
  if (!target || !target.addEventListener) {
    return;
  }

  target.addEventListener(eventName, handler, options);
}

function bindElementEvents(bindings) {
  bindings.forEach(([id, eventName, handler]) => {
    bindElementEvent(id, eventName, handler);
  });
}

function bindPrefixEvents(prefixes, bindingsByPrefix) {
  prefixes.forEach((prefix) => {
    bindingsByPrefix(prefix).forEach(([id, eventName, handler]) => {
      bindElementEvent(id, eventName, handler);
    });
  });
}

function bindQueryAll(root, selector, eventName, handler) {
  root.querySelectorAll(selector).forEach((element) => {
    element.addEventListener(eventName, () => handler(element));
  });
}

function bindDataAction(root, dataAttr, handler) {
  bindQueryAll(root, "[" + dataAttr + "]", "click", (element) => {
    handler(Number(element.getAttribute(dataAttr)));
  });
}

function bindIndexedSuffixAction(root, selector, eventName, handler) {
  bindQueryAll(root, selector, eventName, (element) => {
    handler(Number(element.id.split("-").pop()), element);
  });
}

function bindDelegatedEvent(root, eventName, selector, handler) {
  bindDomEvent(root, eventName, (event) => {
    const target = event.target && event.target.closest ? event.target.closest(selector) : null;
    if (!target || !root.contains(target)) {
      return;
    }
    handler(target, event);
  });
}

bindElementEvents([
  ["reload-button", "click", manualReloadApp],
  ["display-toggle-button", "click", toggleDisplayPower],
  ["ambilight-toggle-button", "click", toggleAmbilightPower],
  ["brightness-slider", "input", syncBrightnessLabel],
  ["brightness-save-button", "click", saveBrightness],
  ["auto-brightness-button", "click", toggleAutoBrightness],
  ["display-mode-save-button", "click", saveDisplayMode],
  ["display-it-is-button", "click", togglePermanentItIs],
  ["display-use-rgbw-button", "click", () => toggleFlagButton("display-use-rgbw-button", getDisplayUseRgbwSetUrl())],
  ["ticker-save-button", "click", saveTickerText],
  ["date-format-save-button", "click", saveDateTickerFormat],
  ["ticker-deceleration-save-button", "click", saveTickerDeceleration],
  ["test-display-button", "click", testDisplay],
  ["weather-appid-save-button", "click", saveWeatherAppId],
  ["weather-city-save-button", "click", saveWeatherCity],
  ["weather-coordinates-save-button", "click", saveWeatherCoordinates],
  ["weather-map-button", "click", openWeatherMapPicker],
  ["weather-now-button", "click", getWeatherNow],
  ["weather-forecast-button", "click", getWeatherForecast],
  ["weather-map-close-button", "click", closeWeatherMapPicker],
  ["weather-map-search-button", "click", searchWeatherLocation],
  ["weather-current-location-button", "click", useCurrentWeatherLocation],
  ["weather-map-apply-button", "click", applyWeatherMapSelection],
  ["network-scan-button", "click", refreshNetworkScan],
  ["network-client-save-button", "click", saveNetworkClient],
  ["network-ap-save-button", "click", saveNetworkAp],
  ["network-timeserver-save-button", "click", saveTimeServer],
  ["network-timezone-save-button", "click", saveTimezone],
  ["network-summertime-button", "click", toggleSummertime],
  ["network-nettime-button", "click", getNetTime],
  ["network-wps-button", "click", runWps],
  ["update-host-save-button", "click", saveUpdateHost],
  ["update-path-save-button", "click", saveUpdatePath],
  ["update-assets-button", "click", downloadUpdateAssets],
  ["update-app-files-button", "click", downloadUpdateAppFiles],
  ["local-app-folder-choose-button", "click", openLocalAppFolderPicker],
  ["local-app-folder-input", "change", handleLocalAppFolderSelection],
  ["local-app-install-button", "click", installLocalAppFiles],
  ["update-esp-button", "click", triggerEspUpdate],
  ["update-stm32-button", "click", triggerStm32Update],
  ["update-table-button", "click", triggerTableUpdate],
  ["settings-export-button", "click", exportSettingsBackup],
  ["settings-import-button", "click", importSettingsBackup],
  ["settings-import-file-input", "change", handleSettingsImportFileChange],
  ["maintenance-reset-stm32-button", "click", resetStm32],
  ["maintenance-reset-eeprom-button", "click", resetEeprom],
  ["files-format-fs-button", "click", formatLittleFsFromFiles],
  ["local-update-esp-form", "submit", uploadLocalEspUpdate],
  ["local-update-stm32-form", "submit", uploadLocalStm32Update],
  ["temperature-rtc-correction-save-button", "click", saveRtcTemperatureCorrection],
  ["temperature-ds18xx-correction-save-button", "click", saveDs18xxTemperatureCorrection],
  ["temperature-display-button", "click", displayTemperatureNow],
  ["ldr-min-button", "click", setLdrMinValue],
  ["ldr-max-button", "click", setLdrMaxValue],
  ["animation-mode-save-button", "click", saveAnimationMode],
  ["color-animation-mode-save-button", "click", saveColorAnimationMode],
  ["tft-save-button", "click", saveTftFlags],
  ["display-dim-save-button", "click", () => saveDimCurve("disp")],
  ["display-dim-preset-select", "change", () => applyDimPreset("disp")],
  ["display-dim-preset-apply-button", "click", () => applyDimPresetAndSave("disp")],
  ["ambilight-brightness-slider", "input", syncAmbilightBrightnessLabel],
  ["ambilight-brightness-save-button", "click", saveAmbilightBrightness],
  ["ambilight-mode-save-button", "click", saveAmbilightMode],
  ["ambilight-leds-save-button", "click", saveAmbilightLeds],
  ["ambilight-offset-save-button", "click", saveAmbilightOffset],
  ["ambilight-dim-save-button", "click", () => saveDimCurve("ambi")],
  ["ambilight-dim-preset-select", "change", () => applyDimPreset("ambi")],
  ["ambilight-dim-preset-apply-button", "click", () => applyDimPresetAndSave("ambi")],
  ["sync-ambilight-button", "click", () => toggleFlagButton("sync-ambilight-button", getSyncAmbilightSetUrl())],
  ["sync-markers-button", "click", () => toggleFlagButton("sync-markers-button", getSyncMarkersSetUrl())],
  ["fade-clock-seconds-button", "click", () => toggleFlagButton("fade-clock-seconds-button", getFadeClockSecondsSetUrl())],
  ["ambilight-markers-button", "click", () => toggleFlagButton("ambilight-markers-button", getAmbilightMarkersSetUrl())],
  ["timer-save-all-button", "click", () => saveAllTimerRows(false)],
  ["ambilight-timer-save-all-button", "click", () => saveAllTimerRows(true)],
  ["dfplayer-volume-slider", "input", syncDfplayerVolumeLabel],
  ["dfplayer-volume-save-button", "click", saveDfplayerVolume],
  ["dfplayer-mode-save-button", "click", saveDfplayerMode],
  ["dfplayer-bell-save-button", "click", saveDfplayerBellFlags],
  ["dfplayer-speak-save-button", "click", saveDfplayerSpeakCycle],
  ["dfplayer-silence-start-save-button", "click", saveDfplayerSilenceStart],
  ["dfplayer-silence-stop-save-button", "click", saveDfplayerSilenceStop],
  ["dfplayer-play-button", "click", playDfplayerTrack],
  ["debug-apply-button", "click", applyDebugOverrides],
  ["debug-reset-button", "click", resetDebugOverrides],
  ["stm32-log-refresh-button", "click", refreshStm32Log],
  ["stm32-log-clear-button", "click", clearStm32Log],
  ["stm32-log-jump-button", "click", jumpStm32LogToEnd],
  ["datetime-save-button", "click", saveDateTime],
  ["learn-ir-button", "click", learnIrRemote],
  ["update-progress-frame", "load", handleProgressFrameLoad],
  ["fs-upload-icon-form", "submit", (event) => uploadFsTargetFile(event, getFsUploadUrl("icon"), translate("maintenance.upload_file_done"))],
  ["fs-upload-weather-form", "submit", (event) => uploadFsTargetFile(event, getFsUploadUrl("weather"), translate("maintenance.upload_file_done"))],
  ["fs-upload-tables-form", "submit", (event) => uploadFsTargetFile(event, getFsUploadUrl("tables"), translate("maintenance.upload_table_done"))],
  ["fs-upload-display-form", "submit", (event) => uploadFsTargetFile(event, getFsUploadUrl("display"), translate("maintenance.upload_display_done"))]
]);

bindPrefixEvents(["display", "ambilight", "marker"], (prefix) => [
  [prefix + "-color-save-button", "click", () => saveColor(prefix)],
  [prefix + "-color-rgb", "input", () => updateLiveColorPreview(prefix)],
  [prefix + "-color-white", "input", () => updateLiveColorPreview(prefix)],
  [prefix + "-color-white", "input", () => syncWhiteChannelLabel(prefix)]
]);

const appVersionLabel = document.getElementById("app-version");
const appVersionCard = document.getElementById("app-version-card");

if (appVersionCard) {
  appVersionCard.textContent = APP_VERSION;
}
renderLanguageOptions();
bindElementEvent("language-select", "change", (event) => {
  void requestLanguage(event.target && event.target.value ? event.target.value : DEFAULT_LANGUAGE, true);
});
// Die Oberfläche startet immer auf Deutsch -- es gibt keinen Moment, in dem sie auf
// eine Datei wartet. Der gespeicherte Wunsch wird danach nachgeladen; scheitert das,
// erscheint dieselbe Meldung wie beim Umschalten und der Wunsch bleibt gespeichert.
applyLanguage(DEFAULT_LANGUAGE, false);
void requestLanguage(getStoredLanguage(), false);
renderLocalAppSelectionStatus();

window.setTimeout(() => {
  try {
    if (window.sessionStorage.getItem(PROGRESS_SCROLL_RESTORE_KEY)) {
      restoreProgressReturnScrollPosition();
    }
  } catch (_) {
  }
}, 250);
bindDelegatedEvent(document, "click", ".module-chip", (button) => {
  const target = button.getAttribute("data-module-target") || "main";

  if (!confirmModuleSwitchWithUnsavedEdits(target)) {
    return;
  }

  setActiveModule(target);
});
bindDelegatedEvent(document, "change", "#health-ambilight-select", () => {
  void saveAmbilightOnlineState();
});
document.addEventListener("input", handleDirtyFormInteraction, true);
document.addEventListener("change", handleDirtyFormInteraction, true);

const APP_STABILITY_MODE = {
  disableServiceWorkerRegistration: false,
  disableStartupAutoRefresh: false
};

let statusToneResetTimer = 0;
let buttonFeedbackTimers = new WeakMap();
let alignedAutoRefreshTimeout = 0;
let alignedAutoRefreshInterval = 0;
let moduleNavHintSyncFrame = 0;

loadDebugOverridesIntoUi();
clearReloadQueryMarker();
restoreActiveModule();
scheduleModuleNavHintSync();
window.setTimeout(() => {
  startupLoadScheduled = false;
  if (activeLoadCount > 0 || getCurrentSettingsSnapshot()) {
    return;
  }
  startupLoadIssued = true;
  startupLoadIssuedAt = Date.now();
  const startupModule = getActiveModuleName();
  void loadData(buildLoadOptionsForModule(startupModule, { startup: true }));
}, 0);
if (!APP_STABILITY_MODE.disableStartupAutoRefresh) {
  startAlignedAutoRefresh();
}
if (!APP_STABILITY_MODE.disableServiceWorkerRegistration) {
  scheduleServiceWorkerRegistration();
}
window.addEventListener("resize", () => {
  scheduleWordclockSizing();
  scheduleModuleNavHintSync();
});
window.addEventListener("pageshow", () => {
  if (shouldDelayStartupRefresh() || shouldSkipLifecycleRefresh()) {
    return;
  }
  void refreshVisibleModuleData({ pageShow: true });
});
document.addEventListener("visibilitychange", () => {
  // Im Hintergrund schaut niemand hin, jeder Poll kostet den STM aber Debugtext auf
  // der UART und blockiert dort den Hauptloop. Deshalb anhalten und beim Sichtbarwerden
  // mit einem sofortigen Lauf wieder aufnehmen. R2-11.
  //
  // ES SIND DREI POLLER, NICHT EINER. Nur stopAlignedAutoRefresh() anzuhalten reicht
  // nicht: Die Live-Farbe fragt alle 5 s ab, solange eine Farbanimation laeuft, und
  // das STM-Logbuch alle 2,5 s, solange das Modul "system" offen ist. Beide liefen im
  // Hintergrund weiter -- die Abnahme verlangt aber, dass ueber ein Fenster von 60 s
  // GAR KEIN Request mehr am Geraet ankommt. Die beiden sync*-Funktionen pruefen
  // document.hidden inzwischen selbst, deshalb genuegt es, sie hier erneut zu rufen:
  // einmal zum Abschalten, einmal zum Wiederanlaufen.
  //
  // ABSICHTLICH NICHT angehalten werden updateProgressPollTimer und stm32ProgressTimer:
  // Sie laufen nur waehrend eines Updates oder eines Flashvorgangs. Wer dabei den Tab
  // wechselt, soll den Fortschritt nicht verlieren -- und ein abgebrochener
  // Fortschrittspoller liesse die Oberflaeche in einem Zwischenstand stehen.
  if (document.hidden) {
    stopAlignedAutoRefresh();
    syncLiveDisplayColorPolling(getCurrentSettingsSnapshot());
    syncStm32LogPolling();
    return;
  }

  if (!APP_STABILITY_MODE.disableStartupAutoRefresh) {
    startAlignedAutoRefresh();
  }

  syncLiveDisplayColorPolling(getCurrentSettingsSnapshot());
  syncStm32LogPolling();

  if (shouldDelayStartupRefresh() || shouldSkipLifecycleRefresh()) {
    return;
  }

  void refreshVisibleModuleData({ visibilityRefresh: true });
});

function scheduleServiceWorkerRegistration() {
  const register = () => {
    void registerAppServiceWorker();
  };

  if ("requestIdleCallback" in window) {
    window.requestIdleCallback(register, { timeout: 2500 });
    return;
  }

  window.setTimeout(register, 1200);
}

// Ohne sicheren Kontext (HTTPS oder localhost) stellt der Browser navigator
// .serviceWorker gar nicht erst bereit: Offline-Betrieb und "zum Startbildschirm
// hinzufuegen" scheitern heute wortlos. Einmal pro Sitzung erklaeren — bei jedem
// Neuladen zu melden waere auf einem Geraet, das nur HTTP spricht, blosser Laerm.
// R2-10.
function reportInsecureContextOnce() {
  if (window.isSecureContext !== false) {
    return;
  }

  const host = window.location.hostname;
  if (host === "localhost" || host === "127.0.0.1" || host === "[::1]" || host === "::1") {
    return;
  }

  try {
    if (window.sessionStorage.getItem(INSECURE_CONTEXT_HINT_KEY)) {
      return;
    }
    window.sessionStorage.setItem(INSECURE_CONTEXT_HINT_KEY, "1");
  } catch (_) {
    // Kein sessionStorage (privater Modus): dann lieber einmal zu viel erklaeren als
    // den Nutzer im Unklaren lassen, warum die Installation fehlt.
  }

  showStatusBanner(translate("app.insecure_context"), "warn");
}

async function registerAppServiceWorker() {
  reportInsecureContextOnce();

  if (!("serviceWorker" in navigator)) {
    return;
  }

  try {
    const registration = await navigator.serviceWorker.register("/app/sw.js", { scope: "/app/" });
    appServiceWorkerRegistration = registration;
    bindServiceWorkerLifecycle(registration);
  } catch (error) {
    // Hier zu landen heisst: der Kontext war sicher, die Registrierung ist trotzdem
    // gescheitert. Ohne Ausgabe bliebe der Grund unauffindbar. Kein Banner — zum
    // Startzeitpunkt kann der Nutzer daran nichts aendern, die Bedienung laeuft
    // vollstaendig weiter, und der unsichere Kontext ist bereits eigens gemeldet.
    if (window.console && window.console.warn) {
      window.console.warn("Service Worker konnte nicht registriert werden:", error);
    }
  }
}

function bindServiceWorkerLifecycle(registration) {
  if (!registration) {
    return;
  }

  if (registration.waiting) {
    triggerWaitingServiceWorker(registration.waiting);
  }

  registration.addEventListener("updatefound", () => {
    const worker = registration.installing;

    if (!worker) {
      return;
    }

    worker.addEventListener("statechange", () => {
      if (worker.state === "installed" && navigator.serviceWorker.controller) {
        triggerWaitingServiceWorker(worker);
      }
    });
  });

  navigator.serviceWorker.addEventListener("controllerchange", () => {
    if (appServiceWorkerUpdateApplied) {
      return;
    }

    appServiceWorkerUpdateApplied = true;
    window.setTimeout(() => reloadAppPage(), 150);
  });
}

function triggerWaitingServiceWorker(worker) {
  if (!worker || !worker.postMessage) {
    return;
  }

  worker.postMessage({ type: "SKIP_WAITING" });
}

function stopAlignedAutoRefresh() {
  if (alignedAutoRefreshTimeout) {
    window.clearTimeout(alignedAutoRefreshTimeout);
    alignedAutoRefreshTimeout = 0;
  }
  if (alignedAutoRefreshInterval) {
    window.clearInterval(alignedAutoRefreshInterval);
    alignedAutoRefreshInterval = 0;
  }
}

function startAlignedAutoRefresh() {
  const intervalMs = 15000;
  const phaseOffsetMs = 2000;

  stopAlignedAutoRefresh();

  const now = Date.now();
  const nextAlignedTick = Math.floor(now / intervalMs) * intervalMs + intervalMs + phaseOffsetMs;
  const delayToNextTick = Math.max(phaseOffsetMs, nextAlignedTick - now);

  alignedAutoRefreshTimeout = window.setTimeout(() => {
    alignedAutoRefreshTimeout = 0;
    if (shouldAutoRefreshCurrentModule()) {
      void refreshVisibleModuleData({ auto: true });
    }
    alignedAutoRefreshInterval = window.setInterval(() => {
      if (shouldAutoRefreshCurrentModule()) {
        void refreshVisibleModuleData({ auto: true });
      }
    }, intervalMs);
  }, delayToNextTick);
}

function handleDirtyFormInteraction(event) {
  const target = event.target;
  if (!target || !(target instanceof HTMLElement)) {
    return;
  }
  if (!event.isTrusted) {
    return;
  }
  if (target.matches('button, iframe, [type="hidden"]')) {
    return;
  }
  const activeSection = target.closest(".module-section.is-active");
  const visibleModal = target.closest("#weather-map-modal:not(.is-hidden)");
  const overlayEditor = target.closest("#overlay-list");

  if (!activeSection && !visibleModal) {
    return;
  }
  if (overlayEditor) {
    return;
  }

  hasUnsavedEdits = true;
}

// Gegenstueck zu handleDirtyFormInteraction, und bewusst eine eigene Funktion statt
// einer Zuweisung an jeder Fundstelle: Wer einen neuen Speicherpfad baut, soll eine
// benannte Handlung aufrufen und nicht eine Variable kennen muessen.
//
// Aufgerufen wird sie ueberall dort, wo ein Schreibvorgang GELUNGEN ist und danach
// loadData() laeuft. Die Begruendung steht in runButtonRequest: Nach dem Neulesen
// tragen die Felder wieder Geraetestand, das Flag haette nichts mehr zu schuetzen --
// bliebe es stehen, legte es ueber "opts.auto && hasUnsavedEdits" in loadData() die
// Selbstaktualisierung fuer den Rest der Sitzung still (L33). Reine Ausloeseaktionen
// ("Wetter abrufen") ruehren es weiterhin nicht an, sonst verloeren sie eine offene
// Bearbeitung in einem ganz anderen Feld. Massnahme 4, B1.
function markEditsPersisted() {
  hasUnsavedEdits = false;
}

// B18 (L151): Der Modulwechsel hat bisher nicht gewarnt -- gemessen wurde eine
// geaenderte, nicht gespeicherte Ortsangabe, die beim Wechsel still verschwand. Kein
// Verbot, sondern eine Rueckfrage mit "trotzdem wechseln".
//
// Der Guard sitzt bewusst am KLICK und nicht in setActiveModule(): Diese Funktion
// wird auch beim Seitenstart (restoreActiveModule) und beim Ausblenden eines Moduls
// (updateModuleAvailability) gerufen. Eine Rueckfrage dort waere eine Rueckfrage ohne
// Handlung des Nutzers.
function confirmModuleSwitchWithUnsavedEdits(target) {
  if (!hasUnsavedEdits || target === getActiveModuleName()) {
    return true;
  }

  if (!window.confirm(translate("modules.unsaved_switch_confirm"))) {
    return false;
  }

  // Zugestimmt heisst verworfen: Das Flag stehen zu lassen wuerde bei jedem weiteren
  // Wechsel erneut fragen und zusaetzlich die Selbstaktualisierung stilllegen.
  markEditsPersisted();
  return true;
}

let statusBannerTimer = 0;

// Zeigt die Meldung dort, wo sie in JEDEM Bereich sichtbar ist. #updated-at bleibt
// zusaetzlich beschrieben, damit die Hauptseite ihren gewohnten Hinweis behaelt.
function showStatusBanner(message, tone) {
  const banner = document.getElementById("status-banner");
  if (!banner || !message) {
    return;
  }

  banner.textContent = message;
  banner.classList.remove("is-ok", "is-error", "is-warn");
  if (tone === "ok" || tone === "error" || tone === "warn") {
    banner.classList.add("is-" + tone);
  }
  banner.classList.add("is-visible");

  if (statusBannerTimer) {
    window.clearTimeout(statusBannerTimer);
  }
  statusBannerTimer = window.setTimeout(() => {
    banner.classList.remove("is-visible");
    statusBannerTimer = 0;
  }, tone === "error" ? 7000 : 4000);
}

function announceStatus(message, tone) {
  showStatusBanner(message, tone);

  const element = document.getElementById("updated-at");
  if (!element) {
    return;
  }

  element.textContent = message;
  element.classList.remove("is-ok", "is-error", "is-warn");

  if (statusToneResetTimer) {
    window.clearTimeout(statusToneResetTimer);
    statusToneResetTimer = 0;
  }

  if (tone === "ok") {
    element.classList.add("is-ok");
  } else if (tone === "error") {
    element.classList.add("is-error");
  } else if (tone === "warn") {
    element.classList.add("is-warn");
  }

  if (tone) {
    statusToneResetTimer = window.setTimeout(() => {
      element.classList.remove("is-ok", "is-error", "is-warn");
      statusToneResetTimer = 0;
    }, 2600);
  }
}

async function apiFetch(url, options) {
  const requestOptions = { ...(options || {}) };
  const timeoutMs = Number(requestOptions.timeoutMs || CONNECTION_STABILITY.apiWriteTimeoutMs);
  const attempts = Number(requestOptions.attempts || 1);

  delete requestOptions.timeoutMs;
  delete requestOptions.attempts;

  const response = await fetchWithRetry(url, {
    cache: "no-store",
    ...requestOptions
  }, timeoutMs, attempts);

  if (!response.ok) {
    throw new Error("http-" + response.status);
  }

  // Das Geraet weist ungueltige Eingaben mit HTTP 200 und {"ok":false,...} ab -- das
  // ist die Hausform, http_api_app_file_upload macht es seit jeher so. Ohne diese
  // Pruefung sah die Oberflaeche nur den Status, meldete "gespeichert" und log damit
  // den Nutzer an: Das Geraet hatte nichts geschrieben. Seit das Geraet eine Menge
  // Eingaben abweist statt sie still zurechtzubiegen, waere das die schlimmere
  // Variante des Fehlers gewesen, den wir gerade beheben.
  const payload = await readApiPayloadFromResponse(response);
  const apiError = readApiErrorFromPayload(payload);

  if (apiError) {
    const error = new Error("api-" + apiError.code);
    error.apiErrorCode = apiError.code;
    error.apiDetail = apiError.detail;
    throw error;
  }

  // Eine Antwort darf ok:true sein und trotzdem eine Warnung tragen: gespeichert, aber
  // wirkungslos. ok bleibt absichtlich true, damit kein Aufrufer und kein Import
  // bricht -- sichtbar wird die Warnung nur, wenn die PWA das Feld liest. L68.
  reportApiWarning(readApiWarningFromPayload(payload));

  if (settingsImportInProgress) {
    await sleep(180);
  }

  return response;
}

// Liest den Antwortrumpf EINMAL, OHNE ihn dem Aufrufer wegzunehmen. response.clone()
// ist hier Pflicht: Der Rumpf lässt sich nur einmal lesen, und mehrere Aufrufer rufen
// danach .json() oder .text() auf. Fehlerkennung und Warnung werden aus demselben
// Rumpf abgeleitet, damit nicht zweimal geparst wird.
async function readApiPayloadFromResponse(response) {
  const contentType = String(response.headers && response.headers.get("content-type") || "");

  // display_power und ambilight_power antworten mit reinem Text ("on"/"off").
  if (!contentType.toLowerCase().includes("json")) {
    return null;
  }

  try {
    return await response.clone().json();
  } catch (_) {
    // Ein unparsbarer Rumpf ist kein Fehler DIESER Pruefung. Wer den Inhalt
    // wirklich braucht, scheitert gleich selbst und mit besserer Meldung.
    return null;
  }
}

function readApiErrorFromPayload(payload) {
  if (!payload || payload.ok !== false) {
    return null;
  }

  return { code: Number(payload.error || 0), detail: String(payload.detail || "") };
}

function readApiWarningFromPayload(payload) {
  if (!payload || payload.warning === undefined || payload.warning === null) {
    return "";
  }

  return String(payload.warning).trim();
}

// Die Warnungstexte des Geräts sind englisch, weil die Legacy-Oberfläche es ist.
// Übersetzt wird deshalb über eine Zuordnung, nicht über den Text selbst. Ein
// künftiger Endpunkt mit Warnung braucht hier nur einen Eintrag; kennt die Tabelle
// den Text nicht, erscheint der Originaltext, aber er erscheint.
const API_WARNING_KEYS = {
  "ldr_min_value >= ldr_max_value, automatic brightness inactive": "api.warning.ldr_min_max"
};

let lastApiWarningText = "";
let lastApiWarningAt = 0;

function describeApiWarning(warning) {
  const key = API_WARNING_KEYS[warning];

  if (!key) {
    return warning;
  }

  const translated = translate(key);

  return translated && translated !== key ? translated : warning;
}

function reportApiWarning(warning) {
  if (!warning) {
    return;
  }

  const now = Date.now();

  // Der Backup-Import schreibt dieselbe Einstellung mehrfach, und die Messknöpfe
  // setzen Minimum und Maximum nacheinander. Ohne diese Sperre stuende dieselbe
  // Warnung mehrfach hintereinander im Banner und verdraengte sich selbst.
  if (warning === lastApiWarningText && now - lastApiWarningAt < 10000) {
    return;
  }

  lastApiWarningText = warning;
  lastApiWarningAt = now;
  console.warn("Das Gerät meldet eine Warnung: " + warning);
  announceStatus(describeApiWarning(warning), "warn");
}

// Die Fehlertexte des Geraets sind englisch, weil die Legacy-Oberflaeche es ist.
// Hier wird deutsch gesprochen, also uebersetzt die Kennung, nicht der Text. Das
// "detail" bleibt als Diagnose erhalten, falls eine Kennung einmal unbekannt ist.
function describeApiError(error, fallbackText) {
  if (!error || !error.apiErrorCode) {
    return fallbackText;
  }

  const key = "api.error." + error.apiErrorCode;
  const translated = translate(key);
  const base = translated && translated !== key ? translated : fallbackText;

  // Das "detail" nennt seit Runde 1 den Parameter UND den erlaubten Bereich -- also
  // genau die Auskunft, die aus "Der Wert liegt ausserhalb des erlaubten Bereichs"
  // eine brauchbare Meldung macht. Bisher ging es verloren, sobald die Kennung
  // bekannt war; gerade die haeufigste Kennung 2 ist ohne den Bereich wertlos. Es
  // steht englisch in Klammern dahinter, weil es aus der Firmware kommt und nicht
  // uebersetzt wird -- ersetzt wird der deutsche Satz dadurch nicht.
  return error.apiDetail ? base + " (" + error.apiDetail + ")" : base;
}

function clearButtonFeedback(button) {
  if (!button) {
    return;
  }

  const timer = buttonFeedbackTimers.get(button);
  if (timer) {
    window.clearTimeout(timer);
    buttonFeedbackTimers.delete(button);
  }

  button.classList.remove("is-busy", "is-success", "is-error");
}

function beginButtonFeedback(button, busyText) {
  if (!button) {
    return;
  }

  clearButtonFeedback(button);
  if (!button.dataset.restoreText) {
    button.dataset.restoreText = button.textContent;
  }
  button.disabled = true;
  button.classList.add("is-busy");
  button.textContent = busyText;
}

function finishButtonFeedback(button, idleText, state, temporaryText, preserveCurrentText) {
  if (!button) {
    return;
  }

  clearButtonFeedback(button);
  button.disabled = false;

  if (state === "success" || state === "error") {
    const restoreText = preserveCurrentText ? (button.textContent || idleText) : idleText;
    button.classList.add(state === "success" ? "is-success" : "is-error");
    button.textContent = temporaryText;
    const timer = window.setTimeout(() => {
      button.classList.remove("is-success", "is-error");
      button.textContent = restoreText;
      buttonFeedbackTimers.delete(button);
    }, 1400);
    buttonFeedbackTimers.set(button, timer);
  } else {
    button.textContent = idleText;
  }
}

function setActiveModule(moduleName) {
  const target = moduleName || "main";
  document.querySelectorAll(".module-chip").forEach((button) => {
    button.classList.toggle("is-active", button.getAttribute("data-module-target") === target);
  });
  document.querySelectorAll(".module-section").forEach((section) => {
    section.classList.toggle("is-active", section.getAttribute("data-module") === target);
  });
  try {
    localStorage.setItem(MODULE_STORAGE_KEY, target);
  } catch (error) {
    // Kein harter Fehler: das Modul bleibt aktiv, nur die Auswahl überlebt den Reload nicht.
    console.warn("Active module could not be stored, selection will not survive a reload", error);
  }
  if (target === "main") {
    scheduleWordclockSizing();
  }
  scheduleModuleNavHintSync();
  if (getCurrentSettingsSnapshot()) {
    void refreshVisibleModuleData({ moduleChange: true, moduleName: target });
  }
  syncLiveDisplayColorPolling(getCurrentSettingsSnapshot());
  syncStm32LogPolling();
}

function buildLoadOptionsForModule(moduleName, extraOptions) {
  const target = moduleName || "main";
  const opts = { ...(extraOptions || {}), moduleName: target };

  if (target === "maintenance") {
    opts.maintenancePriority = true;
  }

  return opts;
}

function refreshVisibleModuleData(extraOptions) {
  return loadData(buildLoadOptionsForModule(getActiveModuleName(), extraOptions));
}

function shouldAutoRefreshCurrentModule() {
  return !isBackgroundPauseActive();
}

function isBackgroundPauseActive() {
  return backgroundPauseUntil > Date.now();
}

function startBackgroundPause(durationMs, reloadAfter) {
  backgroundPauseUntil = Date.now() + Math.max(1000, Number(durationMs || 0));

  if (backgroundPauseFinishTimer) {
    window.clearTimeout(backgroundPauseFinishTimer);
  }

  backgroundPauseFinishTimer = window.setTimeout(() => {
    backgroundPauseFinishTimer = 0;
    backgroundPauseUntil = 0;
    if (reloadAfter !== false) {
      void loadData();
    }
  }, Math.max(1000, Number(durationMs || 0)) + 250);
}

function shouldDelayStartupRefresh() {
  return startupLoadScheduled || (!getCurrentSettingsSnapshot() && startupLoadIssued && (Date.now() - startupLoadIssuedAt) < 5000);
}

function shouldSkipLifecycleRefresh() {
  return !!(activeLoadCount > 0 || (lastSuccessfulLoadAt && (Date.now() - lastSuccessfulLoadAt) < 3000));
}

function restoreActiveModule() {
  let moduleName = "main";
  try {
    moduleName = localStorage.getItem(MODULE_STORAGE_KEY) || "main";
  } catch (error) {
    // Kein harter Fehler: es wird auf das Hauptmodul zurückgefallen.
    console.warn("Stored module could not be read, falling back to the main module", error);
    moduleName = "main";
  }
  if (!document.querySelector('.module-section[data-module="' + moduleName + '"]')) {
    moduleName = "main";
  }
  setActiveModule(moduleName);
}

function updateModuleAvailability(settings, debugOverrides) {
  const moduleState = getFeatureUiMeta(settings, debugOverrides).moduleState;
  const visibility = {
    ambilight: moduleState.ambilightOnline,
    dfplayer: moduleState.dfplayerOnline
  };

  Object.keys(visibility).forEach((moduleName) => {
    const visible = visibility[moduleName];
    const chip = document.querySelector('.module-chip[data-module-target="' + moduleName + '"]');
    const section = document.querySelector('.module-section[data-module="' + moduleName + '"]');

    if (chip) {
      chip.classList.toggle("is-hidden", !visible);
    }
    if (section) {
      section.classList.toggle("is-hidden", !visible);
    }
  });

  const activeSection = document.querySelector(".module-section.is-active");
  if (activeSection && activeSection.classList.contains("is-hidden")) {
    setActiveModule("main");
  }

  scheduleModuleNavHintSync();
}

function scheduleModuleNavHintSync() {
  if (moduleNavHintSyncFrame) {
    window.cancelAnimationFrame(moduleNavHintSyncFrame);
  }

  moduleNavHintSyncFrame = window.requestAnimationFrame(() => {
    moduleNavHintSyncFrame = 0;
    syncModuleNavHint();
  });
}

function syncModuleNavHint() {
  const shell = document.querySelector(".module-nav-shell");
  const nav = shell ? shell.querySelector(".module-nav") : null;

  if (!shell || !nav) {
    return;
  }

  shell.classList.add("is-measuring");
  const chips = Array.from(nav.querySelectorAll(".module-chip")).filter((chip) => chip.offsetParent !== null);
  const requiredWidth = chips.length ? Math.ceil(nav.scrollWidth) : 0;
  const availableWidth = Math.floor(nav.clientWidth);
  shell.classList.remove("is-measuring");
  const currentlyOverflowing = shell.classList.contains("has-overflow");
  const enterOverflowThreshold = availableWidth + 24;
  const leaveOverflowThreshold = availableWidth - 24;
  const hasOverflow = currentlyOverflowing
    ? requiredWidth > leaveOverflowThreshold
    : requiredWidth > enterOverflowThreshold;

  shell.classList.toggle("has-overflow", hasOverflow);
  if (!hasOverflow && nav.scrollLeft) {
    nav.scrollLeft = 0;
  }
}

async function loadData(options) {
  const opts = options || {};
  if (isBackgroundPauseActive() && !opts.allowDuringBackgroundPause) {
    return;
  }
  if (opts.auto && hasUnsavedEdits) {
    announceStatus(translate("status.auto_refresh_paused"), "warn");
    return;
  }
  if (opts.auto && settingsImportInProgress) {
    return;
  }
  if (opts.auto && activeLoadCount > 0) {
    return;
  }
  const requestId = ++loadRequestSerial;
  activeLoadCount += 1;
  const hadSnapshotBeforeLoad = !!getCurrentSettingsSnapshot();

  if (!hadSnapshotBeforeLoad && !opts.auto) {
    announceStatus(translate("status.waiting_for_data"), "warn");
  }

  try {
    const coreData = await loadCoreData();
    if (requestId !== loadRequestSerial) {
      return;
    }

    const settings = resolveSettingsSnapshot(coreData.settingsText);
    if (!hasSettingsSnapshotContent(settings) && !hadSnapshotBeforeLoad) {
      throw new Error("initial-data-pending");
    }

    clearInitialLoadRetry();
    initialLoadAttemptCount = 0;
    reloadBootstrapPending = false;
    clearReloadBootstrapLoads();
    setCurrentSettingsSnapshot(settings);
    setCurrentEepromSettings(getCurrentEepromSettings());
    setCurrentNetworkInfo(getCurrentNetworkInfo());
    const debugOverrides = getDebugOverrides();
    applyPersistedAmbilightState(settings);
    setCurrentLayoutPreview(getCurrentLayoutPreview(settings));
    const uiState = getUiFeatureState(settings, debugOverrides);
    const ambilightOnline = uiState.moduleState.ambilightOnline;
    renderOverview(settings, coreData.displayPower, coreData.ambilightPower, debugOverrides, getNormalizedUpdateStatus());
    try {
      renderWordclock(isDisplayPowerOn(coreData.displayPower, settings), settings, getCurrentLayoutPreview(settings));
    } catch (error) {
      console.error("Wordclock preview failed", error);
    }
    updateDisplayButton(coreData.displayPower);
    updateAmbilightButton(coreData.ambilightPower, ambilightOnline);
    updateAmbilightOnlineButton(ambilightOnline ? "on" : "off");
    updateAmbilightAvailability(ambilightOnline ? "on" : "off");
    updateBrightnessControl(
      settings.numvars[NUM.DISPLAY_BRIGHTNESS] || 0,
      settings.numvars[NUM.DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE] ? "on" : "off"
    );
    const settingsControlMeta = getSettingsControlUiMeta(settings, getCurrentNetworkInfo(), getCurrentEepromSettings());
    updateDisplayModeControl(settings);
    updateDisplayFlagControls(settings);
    updateTextControls(settings);
    updateWeatherControlsFromMeta(settingsControlMeta.weather);
    updateNetworkControlsFromMeta(settingsControlMeta.network);
    updateMaintenanceControlsFromMeta(settingsControlMeta.maintenance);
    updateDateTimeControlsFromMeta(settingsControlMeta.dateTime);
    updateTemperatureControlsFromMeta(settingsControlMeta.temperature);
    updateLdrControlsFromMeta(settingsControlMeta.ldr);
    try {
      updateAnimationControlsFromMeta(settingsControlMeta.animation);
    } catch (error) {
      console.error("Animation controls failed", error);
    }
    updateTftVisibility(settings, debugOverrides);
    updateTftControlsFromMeta(settingsControlMeta.tft);
    updateAmbilightBrightnessControl(settings.numvars[NUM.AMBILIGHT_BRIGHTNESS] || 0);
    updateAmbilightModeControl(settings);
    updateAmbilightNumberControls(settings);
    try {
      updateColorControls(settings, ambilightOnline, debugOverrides);
    } catch (error) {
      console.error("Color controls failed", error);
    }
    updateFlagControls(settings, ambilightOnline);
    updateDfplayerControls(settings, debugOverrides);
    try {
      renderAnimationProfiles(settings);
      renderColorAnimationProfiles(settings);
    } catch (error) {
      console.error("Animation profile render failed", error);
    }
    renderAmbilightModeProfiles(settings);
    renderDimCurves(settings);
    renderDfplayerAlarmRows(settings);
    overlayEditorState = captureOverlayEditorState();
    renderOverlayRows(settings);
    restoreOverlayEditorState(overlayEditorState);
    renderTimerRows(settings, false);
    renderTimerRows(settings, true);
    lastSuccessfulLoadAt = Date.now();
    announceStatus(translateFormat("status.updated_at", {
      time: new Date().toLocaleTimeString(currentLanguage === "en" ? "en-CH" : "de-CH")
    }));

    void loadSecondaryData(requestId, settings, coreData, debugOverrides, opts);
  } catch (error) {
    if (!getCurrentSettingsSnapshot()) {
      if (handleInitialLoadPending(error, opts)) {
        return;
      }
      announceStatus(translate("status.data_load_failed"), "error");
    } else {
      console.warn("Refresh incomplete, keeping previous snapshot", error);
    }
  } finally {
    activeLoadCount = Math.max(0, activeLoadCount - 1);
  }
}

function hasSettingsSnapshotContent(settings) {
  if (!settings || typeof settings !== "object") {
    return false;
  }

  return !!(
    Object.keys(settings.numvars || {}).length ||
    Object.keys(settings.strvars || {}).length ||
    Object.keys(settings.tmvars || {}).length
  );
}

function clearInitialLoadRetry() {
  if (!initialLoadRetryTimer) {
    return;
  }

  window.clearTimeout(initialLoadRetryTimer);
  initialLoadRetryTimer = 0;
}

function clearReloadBootstrapLoads() {
  if (!reloadBootstrapTimers.length) {
    return;
  }

  reloadBootstrapTimers.forEach((timerId) => window.clearTimeout(timerId));
  reloadBootstrapTimers = [];
}

function scheduleReloadBootstrapLoads() {
  clearReloadBootstrapLoads();

  if (!reloadBootstrapPending) {
    return;
  }

  reloadBootstrapTimers = RELOAD_BOOTSTRAP_LOAD_DELAYS_MS.map((delayMs) => window.setTimeout(() => {
    if (!reloadBootstrapPending || getCurrentSettingsSnapshot()) {
      return;
    }

    void loadData({ reloadBootstrap: true });
  }, delayMs));
}

function hasReloadQueryMarker() {
  return false;
}

function handleInitialLoadPending(error, options) {
  const opts = options || {};
  const isPendingInitialData = !!(error && error.message === "initial-data-pending");
  const retryIndex = initialLoadAttemptCount;
  const retryDelays = reloadBootstrapPending ? RELOAD_BOOTSTRAP_RETRY_DELAYS_MS : INITIAL_LOAD_RETRY_DELAYS_MS;

  if (!isPendingInitialData || retryIndex >= retryDelays.length) {
    return false;
  }

  initialLoadAttemptCount += 1;
  announceStatus("Wartet auf Daten...", "warn");
  if (reloadBootstrapPending) {
    scheduleReloadBootstrapLoads();
  }
  clearInitialLoadRetry();
  initialLoadRetryTimer = window.setTimeout(() => {
    initialLoadRetryTimer = 0;
    void loadData({ ...opts, initialRetry: true });
  }, retryDelays[retryIndex]);
  return true;
}

async function loadCoreData() {
  // Base snapshot first, richer metadata afterwards.
  const [settingsText, displayPowerText, ambilightPowerText] = await Promise.all([
    settleFetchText(getSettingsUrl(), "", CONNECTION_STABILITY.coreSettingsTimeoutMs, CONNECTION_STABILITY.fastReadAttempts),
    settleFetchText(getDisplayPowerUrl(), "off", CONNECTION_STABILITY.corePowerTimeoutMs, CONNECTION_STABILITY.fastReadAttempts),
    settleFetchText(getAmbilightPowerUrl(), "off", CONNECTION_STABILITY.corePowerTimeoutMs, CONNECTION_STABILITY.fastReadAttempts)
  ]);

  return {
    settingsText,
    displayPower: String(displayPowerText || "off").trim(),
    ambilightPower: String(ambilightPowerText || "off").trim()
  };
}

function resolveSettingsSnapshot(settingsText) {
  const parsedSettings = parseSettings(settingsText);
  const hasParsedContent =
    Object.keys(parsedSettings.numvars).length ||
    Object.keys(parsedSettings.strvars).length ||
    Object.keys(parsedSettings.tmvars).length;

  return hasParsedContent ? parsedSettings : (getCurrentSettingsSnapshot() || parsedSettings);
}

function isDisplayPowerOn(displayPowerText, settings) {
  const normalized = String(displayPowerText || "").trim().toLowerCase();

  if (normalized === "on") {
    return true;
  }

  if (normalized === "off") {
    return false;
  }

  return !!(settings && settings.numvars && settings.numvars[NUM.DISPLAY_POWER]);
}

async function loadSecondaryData(requestId, settings, coreData, debugOverrides, options) {
  const opts = options || {};
  const activeModule = opts.moduleName || getActiveModuleName();
  const moduleProfile = getModuleDataProfile(activeModule, opts);
  const maintenanceActive = moduleProfile.maintenance;
  const displayActive = moduleProfile.display;
  const systemActive = moduleProfile.system;
  const networkActive = moduleProfile.network;
  const overlaysActive = moduleProfile.overlays;
  const updateActive = moduleProfile.update;
  const mainActive = moduleProfile.main;

  // Secondary data is intentionally additive:
  // update/meta information may fail without taking down maintenance/files/network/overlay data.

  if (mainActive || maintenanceActive || updateActive) {
    try {
      const updateStatus = await settleFetchJson(getUpdateStatusUrl(), getNormalizedUpdateStatus(), CONNECTION_STABILITY.updateStatusTimeoutMs, CONNECTION_STABILITY.fastReadAttempts);
      if (requestId !== loadRequestSerial) {
        return;
      }
      setCurrentUpdateStatus(updateStatus);
      refreshUpdateUi(settings, coreData, debugOverrides);
    } catch (error) {
      console.warn("Update status load failed", error);
    }
  }

  if (mainActive || displayActive || maintenanceActive || updateActive) {
    try {
      const updateTableInfo = await settleFetchJson(getUpdateTableFilesUrl(), getNormalizedUpdateTableInfo(), CONNECTION_STABILITY.updateTableInfoTimeoutMs, CONNECTION_STABILITY.fastReadAttempts);
      if (requestId !== loadRequestSerial) {
        return;
      }
      setCurrentUpdateTableInfo(updateTableInfo);
      updateLayoutTableWarnings(settings, updateTableInfo);
    } catch (error) {
      console.warn("Update table info load failed", error);
    }
  }

  if (mainActive || maintenanceActive || updateActive) {
    try {
      refreshUpdateUi(settings, coreData, debugOverrides);
    } catch (error) {
      console.warn("Update UI refresh failed", error);
    }
  }

  if (mainActive) {
    try {
      const preview = await loadWordclockLayoutPreview(getNormalizedUpdateTableInfo(), settings);
      if (requestId !== loadRequestSerial) {
        return;
      }
      setCurrentLayoutPreview(preview);
      renderWordclock(isDisplayPowerOn(coreData.displayPower, settings), settings, getCurrentLayoutPreview(settings));
    } catch (error) {
      console.error("Wordclock preview refresh failed", error);
    }
  }

  if (networkActive) {
    try {
      setCurrentNetworkInfo(await settleFetchJson(getNetworkScanUrl(), getCurrentNetworkInfo() || { networks: [] }, CONNECTION_STABILITY.networkScanTimeoutMs, CONNECTION_STABILITY.slowReadAttempts));
      if (requestId !== loadRequestSerial) {
        return;
      }
      refreshNetworkUi(settings);
    } catch (error) {
      console.warn("Network scan load failed", error);
    }
  }

  if (overlaysActive) {
    try {
      setOverlayIconsCache(await settleFetchJson(getOverlayIconsUrl(), getOverlayIconsCache() || [], CONNECTION_STABILITY.overlayIconsTimeoutMs, CONNECTION_STABILITY.fastReadAttempts));
      if (requestId !== loadRequestSerial) {
        return;
      }
      refreshOverlayUi(settings);
    } catch (error) {
      console.warn("Overlay icons load failed", error);
    }
  }

  if (!maintenanceActive && !systemActive) {
    return;
  }

  if (maintenanceActive) {
    try {
      const fsInfo = await settleFetchJson(getFsInfoUrl(), {}, CONNECTION_STABILITY.fsInfoTimeoutMs, CONNECTION_STABILITY.slowReadAttempts);
      if (requestId !== loadRequestSerial) {
        return;
      }
      const fsList = await settleFetchJson(getFsListUrl(), { files: [] }, CONNECTION_STABILITY.fsListTimeoutMs, CONNECTION_STABILITY.slowReadAttempts);
      if (requestId !== loadRequestSerial) {
        return;
      }
      const eepromSettings = await settleFetchJson(getEepromSettingsUrl(), {}, CONNECTION_STABILITY.eepromSettingsTimeoutMs, CONNECTION_STABILITY.slowReadAttempts);
      if (requestId !== loadRequestSerial) {
        return;
      }

      setCurrentFsFilesFromList(fsList);
      setCurrentEepromSettings(eepromSettings);
      refreshMaintenanceUi(settings, fsInfo);
    } catch (error) {
      console.warn("Maintenance data load failed", error);
    }
  }

  if (maintenanceActive) {
    try {
      const stm32Log = await settleFetchJson(getStm32LogUrl(), { lines: [] }, CONNECTION_STABILITY.stm32LogTimeoutMs, CONNECTION_STABILITY.slowReadAttempts);
      if (requestId !== loadRequestSerial) {
        return;
      }
      updateStm32Log(stm32Log);
    } catch (error) {
      console.warn("STM32 log load failed", error);
    }
  }
}

function getModuleDataProfile(moduleName, options) {
  const opts = options || {};
  const activeModule = moduleName || "main";

  return {
    main: activeModule === "main",
    display: activeModule === "display",
    system: activeModule === "system",
    network: activeModule === "network",
    overlays: activeModule === "overlays",
    update: activeModule === "update",
    maintenance: activeModule === "maintenance" || !!opts.maintenancePriority
  };
}

function getActiveModuleName() {
  const activeSection = document.querySelector(".module-section.is-active");
  return activeSection ? (activeSection.dataset.module || "main") : "main";
}

function updateStm32Log(logData) {
  const meta = document.getElementById("stm32-log-meta");
  const output = document.getElementById("stm32-log-output");
  const lines = logData && Array.isArray(logData.lines) ? logData.lines : [];
  const count = typeof (logData && logData.count) === "number" ? logData.count : lines.length;
  const scrollSlackPx = 8;
  const wasAtBottom = (output.scrollTop + output.clientHeight) >= (output.scrollHeight - scrollSlackPx);

  if (!lines.length) {
    stm32LogShowsLines = false;
    meta.textContent = translate("system.logs_empty");
    output.textContent = translate("system.logs_empty");
    return;
  }

  stm32LogShowsLines = true;
  meta.textContent = translateFormat("system.logs_buffer", {
    count,
    suffix: count === 1 ? "" : "n"
  });
  output.textContent = lines.join("\n");

  if (wasAtBottom) {
    output.scrollTop = output.scrollHeight;
  }
}

function jumpStm32LogToEnd() {
  const button = document.getElementById("stm32-log-jump-button");
  const output = document.getElementById("stm32-log-output");

  beginButtonFeedback(button, translate("common.loading"));
  output.scrollTop = output.scrollHeight;
  finishButtonFeedback(button, translate("system.logs_jump_end"), "success", translate("system.logs_jump_done"));
}

// L142/B17: "Die STM-Logs erscheinen erst nach einem Reload." Die Ursache ist nicht
// abschliessend geklaert -- drei Spuren stehen: verschluckter Fehler, haengendes
// stm32LogRefreshInFlight, oder ein nach einem ESP-Neustart tatsaechlich leerer Ring.
// Unabhaengig davon ist EIN Zustand in allen drei Faellen falsch: ein Feld, das nichts
// zeigt und nichts sagt.
//
// Deshalb hier drei Dinge, jedes gegen eine Spur:
//  - der Fehler des stillen Zweigs wird gemeldet, statt verschluckt zu werden. Vorher
//    blieb "Noch keine STM32-Logs vorhanden" stehen -- eine FALSCHE Aussage, denn
//    geladen wurde gar nicht. Das ist dieselbe Klasse wie C16.
//  - stm32LogRefreshInFlight wird im finally zurueckgesetzt, auf JEDEM Pfad.
//  - ein uebersprungener Abruf meldet das, statt als Erfolg durchzugehen.
const STM32_LOG_SKIPPED = Symbol("stm32-log-skipped");

function reportStm32LogLoadFailure() {
  const meta = document.getElementById("stm32-log-meta");
  const output = document.getElementById("stm32-log-output");
  const message = translate("system.logs_load_error_hint");

  if (meta) {
    meta.textContent = message;
  }

  // Stehen bereits Zeilen da, bleiben sie stehen: Sie sind alt, aber sie sind echt.
  // Sie durch eine Fehlermeldung zu ersetzen wuerde Information vernichten.
  if (output && !stm32LogShowsLines) {
    output.textContent = message;
  }
}

async function fetchStm32Log(silent) {
  if (stm32LogRefreshInFlight) {
    return STM32_LOG_SKIPPED;
  }

  stm32LogRefreshInFlight = true;

  try {
    const response = await apiFetch(getStm32LogUrl());
    const data = await response.json();
    updateStm32Log(data);
    return data;
  } catch (error) {
    reportStm32LogLoadFailure();
    console.warn("STM32-Logbuch konnte nicht geladen werden", error);

    if (!silent) {
      throw error;
    }

    return null;
  } finally {
    stm32LogRefreshInFlight = false;
  }
}

function syncStm32LogPolling() {
  if (settingsImportInProgress || document.hidden) {
    if (stm32LogTimer) {
      window.clearInterval(stm32LogTimer);
      stm32LogTimer = 0;
    }
    return;
  }

  if (getActiveModuleName() !== "system") {
    if (stm32LogTimer) {
      window.clearInterval(stm32LogTimer);
      stm32LogTimer = 0;
    }
    return;
  }

  if (!stm32LogTimer) {
    void fetchStm32Log(true);
    stm32LogTimer = window.setInterval(() => {
      void fetchStm32Log(true);
    }, 2500);
  }
}

async function refreshStm32Log() {
  const button = document.getElementById("stm32-log-refresh-button");

  beginButtonFeedback(button, translate("system.logs_reload_busy"));

  try {
    const result = await fetchStm32Log(false);

    // Ein uebersprungener Abruf ist kein Erfolg. Frueher meldete der Knopf hier
    // "geladen", obwohl nichts geholt wurde.
    if (result === STM32_LOG_SKIPPED) {
      announceStatus(translate("system.logs_reload_in_flight"), "warn");
      finishButtonFeedback(button, translate("system.logs_reload"));
      return;
    }

    finishButtonFeedback(button, translate("system.logs_reload"), "success", translate("system.logs_loaded"));
  } catch (error) {
    announceStatus(translate("system.logs_load_failed"), "error");
    finishButtonFeedback(button, translate("system.logs_reload"), "error", translate("common.error"));
  }
}

async function clearStm32Log() {
  const button = document.getElementById("stm32-log-clear-button");

  if (!window.confirm(translate("system.logs_clear_confirm"))) {
    return;
  }

  beginButtonFeedback(button, translate("system.logs_clear_busy"));

  try {
    await apiFetch(getStm32LogClearUrl());
    updateStm32Log({ count: 0, lines: [] });
    announceStatus(translate("system.logs_cleared_status"), "ok");
    finishButtonFeedback(button, translate("system.logs_clear"), "success", translate("system.logs_cleared"));
  } catch (error) {
    announceStatus(translate("system.logs_clear_failed"), "error");
    finishButtonFeedback(button, translate("system.logs_clear"), "error", translate("common.error"));
  }
}

async function toggleDisplayPower() {
  const button = document.getElementById("display-toggle-button");
  await runStateToggleButton(button, getDisplayPowerSetUrl(), {
    currentValue: () => document.getElementById("display-power").dataset.state === "on" ? "on" : "off",
    idleText: button.dataset.restoreText || translate("main.display_toggle"),
    successText: (next) => (next === "on" ? translate("common.switched_on") : translate("common.switched_off")),
    errorText: translate("main.display_toggle_failed")
  });
}

async function toggleAmbilightPower() {
  const button = document.getElementById("ambilight-toggle-button");
  await runStateToggleButton(button, getAmbilightPowerSetUrl(), {
    currentValue: () => document.getElementById("ambilight-power").dataset.state === "on" ? "on" : "off",
    idleText: button.dataset.restoreText || translate("main.ambilight_toggle"),
    successText: (next) => (next === "on" ? translate("common.switched_on") : translate("common.switched_off")),
    errorText: translate("main.ambilight_toggle_failed")
  });
}

async function saveAmbilightOnlineState() {
  const select = document.getElementById("health-ambilight-select");
  const next = select.value === "on" ? "on" : "off";
  const previous = next === "on" ? "off" : "on";

  select.disabled = true;

  try {
    await apiFetch(getAmbilightOnlineSetUrl() + "?value=" + next);
    setPersistedAmbilightState(next);
    // Eigener Pfad neben runButtonRequest, deshalb dieselbe Behandlung. B1.
    markEditsPersisted();
    try {
      await loadData();
    } catch (_) {
    }
    announceStatus(translate("display.ambilight_state_saved"), "ok");
  } catch (error) {
    select.value = previous;
    announceStatus(translate("display.ambilight_state_set_failed"), "error");
  } finally {
    select.disabled = false;
  }
}

async function saveBrightness() {
  const slider = document.getElementById("brightness-slider");
  const value = slider.value;
  await runValueSave("brightness-save-button", getDisplayBrightnessSetUrl(), value, translate("display.save_brightness"), translate("display.brightness_save_failed"));
}

async function toggleAutoBrightness() {
  const button = document.getElementById("auto-brightness-button");
  await runStateToggleButton(button, getAutoBrightnessSetUrl(), {
    idleText: button.dataset.restoreText || translate("climate.auto_brightness"),
    errorText: translate("climate.auto_brightness_toggle_failed")
  });
}

async function togglePermanentItIs() {
  const button = document.getElementById("display-it-is-button");
  await runStateToggleButton(button, getDisplayItIsSetUrl(), {
    idleText: button.dataset.restoreText || translate("display.keep_it_is"),
    errorText: translate("display.it_is_set_failed")
  });
}

let lastSettingsParseErrorAt = 0;

// Ein beschaedigtes settings_xml liefert ein parsererror-Dokument. Ohne diese Pruefung
// greift parseSettings() darauf zu, findet nichts und liefert lauter Leerwerte — die
// Oberflaeche sieht dann aus wie "nichts konfiguriert", statt den Fehler zu zeigen.
// Haeufigster Ausloeser: ein Anfuehrungszeichen in einem Textfeld, das der ESP nicht
// maskiert. Genau dann braucht der Nutzer den Hinweis, weil der Rueckweg ueber die
// PWA nicht mehr offensteht. L26.
function reportSettingsParseError() {
  const now = Date.now();

  // Der Fehler besteht fort, bis jemand den Wert korrigiert. Bei jedem
  // Aktualisierungslauf erneut zu melden waere Dauerfeuer statt Information.
  if (lastSettingsParseErrorAt && (now - lastSettingsParseErrorAt) < 60000) {
    return;
  }

  lastSettingsParseErrorAt = now;
  announceStatus(translate("status.settings_parse_error"), "error");
}

function parseSettings(xmlText) {
  const xml = new DOMParser().parseFromString(xmlText, "application/xml");

  // Leerer Text ist der normale Startfall — parseSettings("") baut bewusst ein leeres
  // Geruest auf und darf nicht als Geraetefehler gemeldet werden.
  if (String(xmlText || "").trim() && xml.querySelector("parsererror")) {
    reportSettingsParseError();
  }

  const numvars = {};
  const strvars = {};
  const dispmodes = [];
  const dispanims = [];
  const coloranims = [];
  const almodes = [];
  const dspcolors = {};
  const tmvars = {};
  const num8arrays = {};
  const alarmtimes = [];
  const overlays = [];
  const nighttimes = [];
  const ambinighttimes = [];

  xml.querySelectorAll("numvar").forEach((node) => {
    numvars[Number(node.getAttribute("idx"))] = Number(node.getAttribute("value"));
  });

  [NUM.RTC_TEMP_CORRECTION, NUM.DS18XX_TEMP_CORRECTION].forEach((idx) => {
    const rawValue = Number(numvars[idx]);
    if (!Number.isFinite(rawValue)) {
      return;
    }

    numvars[idx] = rawValue > 127 ? rawValue - 256 : rawValue;
  });

  xml.querySelectorAll("strvar").forEach((node) => {
    strvars[Number(node.getAttribute("idx"))] = node.getAttribute("value") || "";
  });

  xml.querySelectorAll("tmvar").forEach((node) => {
    tmvars[Number(node.getAttribute("idx"))] = {
      year: Number(node.getAttribute("year") || 0),
      month: Number(node.getAttribute("month") || 0),
      day: Number(node.getAttribute("day") || 0),
      hour: Number(node.getAttribute("hour") || 0),
      minute: Number(node.getAttribute("minute") || 0),
      second: Number(node.getAttribute("second") || 0),
      wday: Number(node.getAttribute("wday") || 0)
    };
  });

  xml.querySelectorAll("dispmode").forEach((node) => {
    dispmodes.push({
      idx: Number(node.getAttribute("idx")),
      name: node.getAttribute("name") || ""
    });
  });

  xml.querySelectorAll("dispanim").forEach((node) => {
    dispanims.push({
      idx: Number(node.getAttribute("idx")),
      name: node.getAttribute("name") || "",
      deceleration: Number(node.getAttribute("dcl") || 0),
      default_deceleration: Number(node.getAttribute("def_dcl") || 0),
      flags: Number(node.getAttribute("flags") || 0)
    });
  });

  xml.querySelectorAll("coloranim").forEach((node) => {
    coloranims.push({
      idx: Number(node.getAttribute("idx")),
      name: node.getAttribute("name") || "",
      deceleration: Number(node.getAttribute("dcl") || 0),
      default_deceleration: Number(node.getAttribute("def_dcl") || 0),
      flags: Number(node.getAttribute("flags") || 0)
    });
  });

  xml.querySelectorAll("almode").forEach((node) => {
    almodes.push({
      idx: Number(node.getAttribute("idx")),
      name: node.getAttribute("name") || "",
      deceleration: Number(node.getAttribute("dcl") || 0),
      default_deceleration: Number(node.getAttribute("def_dcl") || 0),
      flags: Number(node.getAttribute("flags") || 0)
    });
  });

  xml.querySelectorAll("dspcolor").forEach((node) => {
    dspcolors[Number(node.getAttribute("idx"))] = {
      red: Number(node.getAttribute("red") || 0),
      green: Number(node.getAttribute("green") || 0),
      blue: Number(node.getAttribute("blue") || 0),
      white: Number(node.getAttribute("white") || 0)
    };
  });

  xml.querySelectorAll("num8array").forEach((node) => {
    const varIdx = Number(node.getAttribute("var"));
    const idx = Number(node.getAttribute("idx"));

    if (!num8arrays[varIdx]) {
      num8arrays[varIdx] = {};
    }

    num8arrays[varIdx][idx] = Number(node.getAttribute("value") || 0);
  });

  xml.querySelectorAll("alarmtime").forEach((node) => {
    alarmtimes.push({
      idx: Number(node.getAttribute("idx")),
      minutes: Number(node.getAttribute("minutes") || 0),
      flags: Number(node.getAttribute("flags") || 0)
    });
  });

  xml.querySelectorAll("overlay").forEach((node) => {
    overlays.push({
      idx: Number(node.getAttribute("idx")),
      type: Number(node.getAttribute("type") || 0),
      interval: Number(node.getAttribute("interval") || 0),
      duration: Number(node.getAttribute("duration") || 0),
      date_code: Number(node.getAttribute("date_code") || 0),
      date_start: Number(node.getAttribute("date_start") || 0),
      days: Number(node.getAttribute("days") || 0),
      flags: Number(node.getAttribute("flags") || 0),
      text: node.getAttribute("text") || ""
    });
  });

  xml.querySelectorAll("nighttime").forEach((node) => {
    nighttimes.push({
      idx: Number(node.getAttribute("idx")),
      minutes: Number(node.getAttribute("minutes") || 0),
      flags: Number(node.getAttribute("flags") || 0)
    });
  });

  xml.querySelectorAll("ambinighttime").forEach((node) => {
    ambinighttimes.push({
      idx: Number(node.getAttribute("idx")),
      minutes: Number(node.getAttribute("minutes") || 0),
      flags: Number(node.getAttribute("flags") || 0)
    });
  });

  return { numvars, strvars, tmvars, dispmodes, dispanims, coloranims, almodes, dspcolors, num8arrays, alarmtimes, overlays, nighttimes, ambinighttimes };
}

function renderOverview(settings, displayPower, ambilightPower, debugOverrides, updateStatus) {
  const overviewMeta = getOverviewUiMeta(settings, displayPower, ambilightPower, debugOverrides, updateStatus);
  const hw = overviewMeta.hardware;
  const ambilightOnline = overviewMeta.ambilightOnline;
  const dfplayerOnline = overviewMeta.dfplayerOnline;
  const configItems = overviewMeta.configItems.slice();

  setText("display-power", overviewMeta.displayPowerLabel);
  setText("ambilight-power", overviewMeta.ambilightPowerLabel);
  document.getElementById("display-power").dataset.state = overviewMeta.displayPowerState;
  document.getElementById("ambilight-power").dataset.state = overviewMeta.ambilightPowerState;
  setText("firmware-version", overviewMeta.firmwareVersion);
  setText("esp-version", overviewMeta.espVersion);
  setText("last-start-overview", overviewMeta.lastStartLabel);

  renderList("system-list", [
    ["Board", hw.board],
    ["Processor", hw.processor],
    ["Oscillator", hw.oscillator],
    ["Frequency", hw.frequency],
    ["Hardware", hw.hardware],
    ["Display", hw.display]
  ]);

  renderHealthList(settings, ambilightOnline, dfplayerOnline);

  renderList("config-list", configItems);
  renderPreviewDebug(settings);
  updateModuleAvailability(settings, debugOverrides);
  updateLayoutTableWarnings(settings);
}

function updateDisplayButton(displayPower) {
  const button = document.getElementById("display-toggle-button");
  button.textContent = displayPower === "on" ? translate("main.display_turn_off") : translate("main.display_turn_on");
}

function updateAmbilightButton(ambilightPower, ambilightOnline) {
  const button = document.getElementById("ambilight-toggle-button");
  button.classList.toggle("is-hidden", !ambilightOnline);
  button.textContent = ambilightPower === "on" ? translate("main.ambilight_turn_off") : translate("main.ambilight_turn_on");
}

function updateAmbilightOnlineButton(state) {
  const select = document.getElementById("health-ambilight-select");
  if (select) {
    select.value = state === "on" ? "on" : "off";
  }
}

function updateAmbilightAvailability(state) {
  const isOnline = state === "on";

  document.getElementById("ambilight-panel").classList.toggle("is-hidden", !isOnline);
  document.getElementById("ambilight-dim-panel").classList.toggle("is-hidden", !isOnline);
  document.getElementById("ambilight-timers-panel").classList.toggle("is-hidden", !isOnline);
  document.getElementById("ambilight-profile-panel").classList.toggle("is-hidden", !isOnline);

  [
    "ambilight-color-card",
    "marker-color-card",
    "color-flag-actions"
  ].forEach((id) => {
    const element = document.getElementById(id);
    if (element) {
      element.classList.toggle("is-hidden", !isOnline);
    }
  });
}

function updateDisplayFlagControls(settings) {
  const meta = getDisplayFormUiMeta(settings).display;
  setActionToggleButton("display-it-is-button", translate("display.keep_it_is_disable"), translate("display.keep_it_is"), meta.itIsActive);
}

function updateBrightnessControl(value, autoState) {
  const slider = document.getElementById("brightness-slider");
  const saveButton = document.getElementById("brightness-save-button");
  const autoButton = document.getElementById("auto-brightness-button");
  slider.value = value;
  slider.disabled = autoState === "on";
  saveButton.disabled = autoState === "on";
  setActionToggleButton("auto-brightness-button", translate("climate.disable_auto_brightness"), translate("climate.enable_auto_brightness"), autoState === "on");
  syncBrightnessLabel();
}

function updateDisplayModeControl(settings) {
  const meta = getDisplayFormUiMeta(settings).display;
  const select = document.getElementById("display-mode-select");
  select.innerHTML = meta.displayModes.map((mode) => (
    '<option value="' + mode.idx + '">' + escapeHtml(localizeDisplayModeName(mode.name || String(mode.idx))) + "</option>"
  )).join("");
  select.value = String(meta.currentDisplayMode);
}

function updateTextControls(settings) {
  const meta = getDisplayFormUiMeta(settings).display;
  document.getElementById("ticker-text-input").value = meta.tickerText;
  document.getElementById("date-format-input").value = meta.dateFormat;
  document.getElementById("ticker-deceleration-input").value = String(meta.tickerDeceleration);
}

function updateWeatherControls(settings) {
  updateWeatherControlsFromMeta(getSettingsControlUiMeta(settings).weather);
}

function updateWeatherControlsFromMeta(meta) {
  document.getElementById("weather-appid-input").value = meta.appId;
  document.getElementById("weather-city-input").value = meta.city;
  document.getElementById("weather-lon-input").value = meta.lon;
  document.getElementById("weather-lat-input").value = meta.lat;
  document.getElementById("weather-location-preview").textContent = meta.locationPreview;
}

// Vorbefuellen darf nie ueber eine laufende Eingabe huschen. Deshalb merkt sich das
// Feld, welchen Geraetewert es zuletzt bekommen hat: steht etwas anderes drin, hat der
// Nutzer getippt und behaelt das letzte Wort.
function prefillDeviceValue(input, deviceValue) {
  if (!input || document.activeElement === input) {
    return;
  }

  const applied = input.dataset.devicePrefill;
  if (input.value && input.value !== applied) {
    return;
  }

  input.value = deviceValue || "";
  input.dataset.devicePrefill = input.value;
}

function updateNetworkControls(settings, networkInfo) {
  updateNetworkControlsFromMeta(getSettingsControlUiMeta(settings, networkInfo).network);
}

function updateNetworkControlsFromMeta(meta) {
  const select = document.getElementById("network-ssid-select");

  select.innerHTML = meta.networks.length
    ? meta.networks.map((ssid) => '<option value="' + escapeHtml(ssid) + '"' + (ssid === meta.currentSsid ? " selected" : "") + ">" + escapeHtml(ssid) + "</option>").join("")
    : '<option value="">Keine WLANs gefunden</option>';

  prefillDeviceValue(document.getElementById("network-ap-ssid-input"), meta.apSsid);

  document.getElementById("network-timeserver-input").value = meta.timeserver;
  document.getElementById("network-timezone-input").value = String(meta.timezoneOffset);
  setActionToggleButton("network-summertime-button", translate("network.summertime_disable"), translate("network.summertime"), meta.summertime);

  document.getElementById("network-status-note").textContent =
    "SSID: " + (meta.currentSsid || "-") +
    " | IP: " + (meta.ip || "-") +
    " | Modus: " + (meta.mode || "-");
}

function updateMaintenanceControls(settings, eepromSettings) {
  updateMaintenanceControlsFromMeta(getSettingsControlUiMeta(settings, null, eepromSettings).maintenance);
}

function updateMaintenanceControlsFromMeta(meta) {
  document.getElementById("update-host-input").value = meta.updateHost;
  document.getElementById("update-path-input").value = meta.updatePath;
  renderList("backup-info-list", meta.infoItems);
  updateLayoutTableWarnings(getCurrentSettingsSnapshot());
}

function setSettingsBackupNote(message, tone) {
  const note = document.getElementById("settings-backup-note");
  if (note) {
    note.textContent = message;
  }
  if (tone) {
    announceStatus(message, tone);
  }
}

function handleSettingsImportFileChange() {
  const input = document.getElementById("settings-import-file-input");
  const file = input && input.files && input.files[0];
  setSettingsBackupNote(file ? translateFormat("backup.file_selected", { name: file.name }) : translate("backup.no_file_selected"));
}

function collectUsedOverlayIconNames(items) {
  return Array.from(new Set(
    (items || [])
      .filter((item) => Number(item.type || 0) === 1 && String(item.value || "").trim())
      .map((item) => String(item.value || "").trim())
  )).sort();
}

function buildBackupHeaderState(settings, updateTableInfo) {
  const safeSettings = settings || parseSettings("");
  return {
    settings: safeSettings,
    updateTableInfo: updateTableInfo || getCurrentUpdateTableInfo()
  };
}

function buildAssetBackup(headerState, updateTableInfo) {
  const state = headerState && headerState.settings
    ? headerState
    : buildBackupHeaderState(headerState, updateTableInfo);
  const backupMeta = getBackupAssetMeta(state.settings, null, state.updateTableInfo);
  const overlayItems = buildOverlayBackup(state.settings);

  return {
    layout_table: backupMeta.currentTable ? String(backupMeta.currentTable) : "",
    used_icons: collectUsedOverlayIconNames(overlayItems),
    asset_prefix: backupMeta.assetPrefix
  };
}

function buildBackupSourceMeta(headerState) {
  const state = headerState && headerState.settings
    ? headerState
    : buildBackupHeaderState(headerState);
  const displayMeta = getDisplayBackupMeta(state.settings);
  return {
    app_version: APP_VERSION,
    firmware_version: displayMeta.firmwareVersion,
    esp_version: displayMeta.espVersion,
    eeprom_version: displayMeta.eepromVersion
  };
}

function buildDisplayBackupSettings(settings) {
  const meta = getDisplayBackupMeta(settings);
  return {
    power: meta.power,
    mode: meta.mode,
    use_rgbw: meta.useRgbw,
    brightness: meta.brightness,
    automatic_brightness: meta.automaticBrightness,
    permanent_it_is: meta.permanentItIs,
    ticker_text: meta.tickerText,
    date_ticker_format: meta.dateTickerFormat,
    ticker_deceleration: meta.tickerDeceleration,
    color: cloneColor(settings.dspcolors[0]),
    dim_curve: meta.dimCurve
  };
}

function buildNetworkBackupSettings(settings, eepromSettings) {
  const meta = getNetworkBackupMeta(settings, eepromSettings);
  return {
    timeserver: meta.timeserver,
    timezone_offset: meta.timezoneOffset,
    summertime: meta.summertime,
    wifi_ssid: meta.wifiSsid,
    wifi_key: meta.wifiKey,
    ap_ssid: meta.apSsid,
    ap_key: meta.apKey,
    boot_as_ap: meta.bootAsAp
  };
}

function buildMaintenanceBackupSettings(settings) {
  const meta = getMaintenanceBackupMeta(settings);
  return {
    update_host: meta.updateHost,
    update_path: meta.updatePath
  };
}

function buildClimateBackupSettings(settings) {
  const meta = getClimateBackupMeta(settings);
  return {
    weather_appid: meta.weatherAppId,
    weather_city: meta.weatherCity,
    weather_lon: meta.weatherLon,
    weather_lat: meta.weatherLat,
    rtc_temp_correction: meta.rtcTempCorrection,
    ds18xx_temp_correction: meta.ds18xxTempCorrection,
    ldr_min: meta.ldrMin,
    ldr_max: meta.ldrMax
  };
}

function buildAnimationBackupSettings(settings) {
  const meta = getAnimationBackupMeta(settings);
  return {
    display_mode: meta.displayMode,
    color_mode: meta.colorMode,
    display_profiles: meta.displayProfiles.map((entry) => ({
      idx: Number(entry.idx),
      deceleration: Number(entry.deceleration || 0),
      favourite: !!(Number(entry.flags || 0) & 0x02)
    })),
    color_profiles: meta.colorProfiles.map((entry) => ({
      idx: Number(entry.idx),
      deceleration: Number(entry.deceleration || 0)
    }))
  };
}

function buildTftBackupSettings(tftFlags) {
  return {
    rgb: !!(tftFlags & 0x01),
    hflip: !!(tftFlags & 0x02),
    vflip: !!(tftFlags & 0x04)
  };
}

function buildAmbilightBackupSettings(settings) {
  const meta = getAmbilightBackupMeta(settings);
  return {
    online: meta.online,
    power: meta.power,
    mode: meta.mode,
    leds: meta.leds,
    offset: meta.offset,
    brightness: meta.brightness,
    color: cloneColor(settings.dspcolors[1]),
    marker_color: cloneColor(settings.dspcolors[2]),
    sync_ambilight: meta.syncAmbilight,
    sync_markers: meta.syncMarkers,
    fade_clock_seconds: meta.fadeClockSeconds,
    seconds_markers: meta.secondsMarkers,
    dim_curve: meta.dimCurve,
    profiles: meta.profiles.map((entry) => ({
      idx: Number(entry.idx),
      deceleration: Number(entry.deceleration || 0)
    }))
  };
}

function buildDfplayerBackupSettings(settings) {
  const meta = getDfplayerBackupMeta(settings);
  return {
    volume: meta.volume,
    mode: meta.mode,
    bell_flags: meta.bellFlags,
    speak_cycle: meta.speakCycle,
    silence_start: meta.silenceStart,
    silence_stop: meta.silenceStop,
    alarms: buildAlarmBackup(meta.alarms)
  };
}

function buildBackupSectionState(settings, eepromSettings) {
  const safeSettings = settings || parseSettings("");
  return {
    settings: safeSettings,
    eepromSettings: eepromSettings || getCurrentEepromSettings(),
    tftFlags: safeSettings.numvars[NUM.SSD1963_FLAGS] || 0
  };
}

function buildCoreBackupSections(sectionState) {
  const state = sectionState && sectionState.settings
    ? sectionState
    : buildBackupSectionState(sectionState);

  return {
    display: buildDisplayBackupSettings(state.settings),
    network: buildNetworkBackupSettings(state.settings, state.eepromSettings),
    maintenance: buildMaintenanceBackupSettings(state.settings),
    climate: buildClimateBackupSettings(state.settings)
  };
}

function buildGroupedBackupSections(sectionState, eepromSettings) {
  const state = sectionState && sectionState.settings
    ? sectionState
    : buildBackupSectionState(sectionState, eepromSettings);

  return {
    state,
    core: buildCoreBackupSections(state),
    advanced: buildAdvancedBackupSections(state),
    asset: buildAssetRelatedBackupSections(state)
  };
}

function buildSettingsBackupSections(sectionState, eepromSettings) {
  const sections = buildGroupedBackupSections(sectionState, eepromSettings);

  return {
    display: sections.core.display,
    network: sections.core.network,
    maintenance: sections.core.maintenance,
    climate: sections.core.climate,
    animations: sections.advanced.animations,
    tft: sections.advanced.tft,
    ambilight: sections.advanced.ambilight,
    dfplayer: sections.advanced.dfplayer,
    overlays: sections.asset.overlays,
    timers: sections.asset.timers
  };
}

function buildAdvancedBackupSections(sectionState) {
  const state = sectionState && sectionState.settings
    ? sectionState
    : buildBackupSectionState(sectionState);
  return {
    animations: buildAnimationBackupSettings(state.settings),
    tft: buildTftBackupSettings(state.tftFlags),
    ambilight: buildAmbilightBackupSettings(state.settings),
    dfplayer: buildDfplayerBackupSettings(state.settings)
  };
}

function buildAssetRelatedBackupSections(sectionState) {
  const state = sectionState && sectionState.settings
    ? sectionState
    : buildBackupSectionState(sectionState);
  const collections = buildCollectionBackupSections(state.settings);
  return {
    overlays: {
      items: collections.overlays
    },
    timers: collections.timers
  };
}

function buildBackupHeaderSections(headerState, updateTableInfo) {
  const state = headerState && headerState.settings
    ? headerState
    : buildBackupHeaderState(headerState, updateTableInfo);
  return {
    source: buildBackupSourceMeta(state),
    assets: buildAssetBackup(state)
  };
}

function buildComparableBackupSections(settings, eepromSettings) {
  const sections = buildGroupedBackupSections(settings, eepromSettings);

  return buildComparableSectionPayload(sections);
}

function buildComparableSectionPayload(sectionGroups) {
  return {
    display: sectionGroups.core.display,
    network: sectionGroups.core.network,
    maintenance: sectionGroups.core.maintenance,
    climate: sectionGroups.core.climate,
    overlays: sectionGroups.asset.overlays,
    timers: sectionGroups.asset.timers
  };
}

function buildAllBackupSections(settings, eepromSettings, updateTableInfo) {
  const sectionGroups = buildGroupedBackupSections(settings, eepromSettings);
  const headerState = buildBackupHeaderState(sectionGroups.state.settings, updateTableInfo);
  return {
    header: buildBackupHeaderSections(headerState),
    settings: buildSettingsBackupSections(sectionGroups.state),
    comparable: buildComparableSectionPayload(sectionGroups)
  };
}

function buildSettingsBackup(settings, eepromSettings, updateTableInfo) {
  const sections = buildAllBackupSections(settings, eepromSettings, updateTableInfo);

  return {
    format: BACKUP_FORMAT,
    version: BACKUP_VERSION,
    exported_at: new Date().toISOString(),
    source: sections.header.source,
    assets: sections.header.assets,
    settings: sections.settings
  };
}

function buildBackupRuntimeState(state) {
  return {
    settings: state && state.settings ? state.settings : (getCurrentSettingsSnapshot() || parseSettings("")),
    eepromSettings: state && state.eepromSettings ? state.eepromSettings : getCurrentEepromSettings(),
    updateTableInfo: state && state.updateTableInfo ? state.updateTableInfo : getCurrentUpdateTableInfo()
  };
}

function getCurrentBackupRuntimeState() {
  return buildBackupRuntimeState();
}

function getBackupExportState() {
  return getCurrentBackupRuntimeState();
}

function buildBackupDocumentState(exportState) {
  return buildBackupRuntimeState(exportState);
}

function buildBackupExportDocument(exportState) {
  const documentState = buildBackupDocumentState(exportState);
  return buildSettingsBackup(
    documentState.settings,
    documentState.eepromSettings,
    documentState.updateTableInfo
  );
}

// L81: Die Prüfung war strikte Gleichheit -- eine Sicherung aus einer älteren App
// wurde abgewiesen, und zwar mit der Meldung "mit einer neueren App erstellt", die
// das Gegenteil dessen behauptet, was vorliegt. Mit F1 (IR-Codes ins Backup) ist
// BACKUP_VERSION auf 3 gestiegen -- ohne diese Vorarbeit wären damit alle bisher
// angelegten Sicherungen auf einen Schlag unbrauchbar geworden, ausgerechnet in der
// Lage, für die der Nutzer sie angelegt hat. Ältere Dateien werden deshalb
// angenommen. Fehlende Abschnitte stören den Import nicht: getImportExecutionState
// und die Stufenbauer greifen durchgehend mit "|| {}" bzw. "|| null" zu.
//
// Eine Version, die sich nicht als positive Zahl lesen lässt, ist dagegen ein
// Formatfehler und kein Versionsfehler -- ohne Version ist die Datei gar nicht als
// Sicherung ausweisbar. Der Typ wird bewusst VOR der Umwandlung geprüft: Number(null),
// Number(""), Number(false) und Number([]) ergeben allesamt 0, und das bisherige
// "backup.version || 0" machte daraus stillschweigend eine Zahl.
function readBackupFileVersion(backup) {
  const raw = backup ? backup.version : undefined;

  if (typeof raw !== "number" && typeof raw !== "string") {
    return 0;
  }

  const version = Number(raw);
  return Number.isFinite(version) && version > 0 ? version : 0;
}

async function parseSettingsBackupFile(file) {
  const backup = JSON.parse(await file.text());

  if (backup.format !== BACKUP_FORMAT) {
    throw new Error("invalid-backup-format");
  }

  const version = readBackupFileVersion(backup);

  if (!version) {
    throw new Error("invalid-backup-format");
  }

  if (version > BACKUP_VERSION) {
    throw new Error("unsupported-backup-version");
  }

  return backup;
}

function buildImportedBackupState(backup) {
  const importedBackup = backup || {};
  const settings = importedBackup.settings || {};

  return {
    backup: importedBackup,
    settings,
    executionState: getImportExecutionState(importedBackup, settings)
  };
}

async function loadImportedBackupState(file) {
  return buildImportedBackupState(await parseSettingsBackupFile(file));
}

function getSettingsBackupImportErrorMessage(error) {
  if (error && error.message === "invalid-backup-format") {
    return translate("backup.invalid_format");
  }

  if (error && error.message === "unsupported-backup-version") {
    return translate("backup.incompatible_version");
  }

  // Bricht der Import an einem Wert ab, den das Geraet abweist, stand hier bisher nur
  // "Import fehlgeschlagen" -- ohne jeden Hinweis darauf, welcher Wert gemeint war.
  // describeApiError nennt Kennung und Bereich und faellt auf denselben Satz zurueck,
  // wenn der Fehler gar nicht vom Geraet kam.
  return describeApiError(error, translate("backup.import_failed"));
}

// ---------------------------------------------------------------------------
// F1 -- IR-Codes im Backup (specs/f1-ir-backup)
//
// Die 20 Tastencodes liegen sonst ausschliesslich im EEPROM des STM und haben keinen
// Rückweg: Der einzige Weg, sie zu erzeugen, ist /api/learn_ir -- und der blockiert
// die Uhr, bis ein Mensch 20 Tasten gedrückt hat.
//
// Kosten am Gerät: Ein Abzug erzeugt genau EIN STM-Kommando, nämlich den RPC hinter
// /api/ir_codes_request. Die 20 Antwortkommandos taktet der STM selbst, eines je
// Hauptloop-Durchlauf. Die Pollaufrufe auf /api/ir_codes_get sind rein lesend und
// erzeugen kein STM-Kommando; sie kosten nur den Debugtext, den der ESP ohnehin pro
// Request auf die UART zum STM schreibt. Deshalb 400 ms Abstand und nicht weniger.
const IR_BACKUP_POLL_INTERVAL_MS = 400;
const IR_BACKUP_POLL_ATTEMPTS = 15;

async function readIrCodesStatus() {
  const response = await apiFetch(getIrCodesGetUrl());
  const payload = await response.json();

  return payload && typeof payload === "object" ? payload : null;
}

function readIrCodeNumber(value, max) {
  const number = Number(value);

  return Number.isInteger(number) && number >= 0 && number <= max ? number : null;
}

// protocol 0 und 255 sind die Leer-Konvention des STM: "nie angelernt". Beides wird
// zu null, und zwar in allen drei Feldern -- eine halb gefüllte Zeile sähe aus wie
// ein Wert, der nur unvollständig gelesen wurde.
function isIrCodeLearned(entry) {
  const protocol = readIrCodeNumber(entry && entry.protocol, 255);

  return protocol !== null && protocol > 0 && protocol < 255;
}

function buildIrBackupKey(name, index, entry) {
  if (!isIrCodeLearned(entry)) {
    return { name, index, protocol: null, address: null, command: null };
  }

  const protocol = readIrCodeNumber(entry.protocol, 255);
  const address = readIrCodeNumber(entry.address, 65535);
  const command = readIrCodeNumber(entry.command, 65535);

  // Ein angelerntes Protokoll mit unlesbarer Adresse oder unlesbarem Kommando ist
  // kein "nie angelernt", sondern eine kaputte Antwort. Als null geschrieben würde
  // der Eintrag beim Import stillschweigend übersprungen -- deshalb null als
  // Rückgabe, was den ganzen Abschnitt verwirft.
  if (address === null || command === null) {
    return null;
  }

  return { name, index, protocol, address, command };
}

function buildIrBackupKeys(codes) {
  const byIndex = new Map();

  (codes || []).forEach((entry) => {
    const idx = readIrCodeNumber(entry && entry.idx, IR_BACKUP_KEY_NAMES.length - 1);

    if (idx !== null) {
      byIndex.set(idx, entry);
    }
  });

  const keys = [];
  let learned = 0;
  let invalid = 0;

  IR_BACKUP_KEY_NAMES.forEach((name, index) => {
    const key = buildIrBackupKey(name, index, byIndex.get(index));

    if (!key) {
      invalid += 1;
      return;
    }

    keys.push(key);

    if (key.protocol !== null) {
      learned += 1;
    }
  });

  return { keys, learned, invalid };
}

// Ein Abzug: anstossen, dann pollen.
//
// "requested" wird VOR "complete" ausgewertet, und das ist keine Stilfrage. Der ESP
// hält den Puffer nur im RAM. Startet er mitten im Abzug neu, meldet der Endpunkt
// requested:false, received:0, alle 20 Indizes in missing[] und ein leeres codes[].
// Wer allein an "complete" hängt, sieht davon nichts und sichert einen leeren Satz --
// eine Datei mit 20 leeren Tasten, die aussieht wie eine gültige, ist schlimmer als
// gar keine.
async function attemptIrCodesSnapshot() {
  await apiFetch(getIrCodesRequestUrl());

  let status = null;

  for (let attempt = 0; attempt < IR_BACKUP_POLL_ATTEMPTS; attempt += 1) {
    await sleep(IR_BACKUP_POLL_INTERVAL_MS);
    status = await readIrCodesStatus();

    if (!status || status.requested !== true) {
      return { ok: false, reason: "stale", status };
    }

    // Vollständigkeit kommt vom Gerät (mask == 0xFFFFF), nicht aus einer
    // Plausibilität der App.
    if (status.complete === true) {
      return { ok: true, reason: "", status };
    }
  }

  return { ok: false, reason: "incomplete", status };
}

async function runIrCodesSnapshot() {
  try {
    return await attemptIrCodesSnapshot();
  } catch (error) {
    // Kein stilles Schlucken: Der Grund wird mitgeführt und entscheidet über den
    // Text im Hinweis. Ein 404 heisst hier "ESP-Firmware älter als diese App" und
    // nicht "Netzfehler". Scheitern darf daran nur der IR-Abschnitt, nicht der
    // gesamte Export -- die übrigen zehn Abschnitte sind davon unberührt.
    return { ok: false, reason: "unavailable", status: null, error };
  }
}

// Genau EIN Wiederholungsversuch, und nur beim Zeitablauf. "stale" ist kein
// Timing-Problem, sondern ein verworfener Puffer -- ein zweiter Anlauf verdeckte das
// nur. "unavailable" ist in aller Regel der 404 einer älteren ESP-Firmware; auch dort
// hilft Wiederholen nicht, es kostet nur weitere sechs Sekunden.
async function collectIrBackupSection() {
  let result = await runIrCodesSnapshot();

  if (!result.ok && result.reason === "incomplete") {
    result = await runIrCodesSnapshot();
  }

  if (!result.ok) {
    return { section: null, reason: result.reason, status: result.status, learned: 0 };
  }

  const built = buildIrBackupKeys(result.status.codes);

  // Entweder vollständig oder gar nicht. 19 von 20 gesicherten Tasten sind eine
  // Falle: Die Datei sieht gültig aus, und die fehlende Taste merkt man erst beim
  // Restore, wenn das Original längst weg ist.
  if (built.invalid > 0 || built.keys.length !== IR_BACKUP_KEY_NAMES.length) {
    return { section: null, reason: "invalid", status: result.status, learned: 0 };
  }

  return {
    section: { keys: built.keys },
    reason: "",
    status: result.status,
    learned: built.learned
  };
}

function getIrBackupExportNote(irResult) {
  const result = irResult || {};
  const status = result.status || {};
  const expected = readIrCodeNumber(status.expected, 255);
  const values = {
    learned: Number(result.learned || 0),
    received: Number(status.received || 0),
    expected: expected === null ? IR_BACKUP_KEY_NAMES.length : expected
  };

  if (result.section) {
    return { message: translateFormat("backup.ir_included", values), tone: "ok" };
  }

  if (result.reason === "stale") {
    return { message: translateFormat("backup.ir_skipped_stale", values), tone: "warn" };
  }

  if (result.reason === "unavailable") {
    return { message: translateFormat("backup.ir_skipped_unavailable", values), tone: "warn" };
  }

  if (result.reason === "invalid") {
    return { message: translateFormat("backup.ir_skipped_invalid", values), tone: "warn" };
  }

  return { message: translateFormat("backup.ir_skipped_incomplete", values), tone: "warn" };
}

async function prepareSettingsBackupExport() {
  await ensureBackupExportState();

  const backup = buildBackupExportDocument(getBackupExportState());

  setSettingsBackupNote(translate("backup.ir_collecting"));

  const ir = await collectIrBackupSection();

  // Angefügt wird NUR bei vollständigem Abzug. Fehlt der Abschnitt, entsteht die
  // Sicherung trotzdem -- sie ist dann sichtbar ohne IR-Codes, und der Hinweis sagt
  // warum. Kein bestehendes Feld wird dabei berührt, deshalb bleibt eine Sicherung
  // der Version 2 ohne Migration lesbar.
  if (ir.section) {
    backup.settings.ir = ir.section;
  }

  return { backup, ir };
}

function cloneColor(color) {
  return {
    red: Number((color && color.red) || 0),
    green: Number((color && color.green) || 0),
    blue: Number((color && color.blue) || 0),
    white: Number((color && color.white) || 0)
  };
}

function buildDimCurveArray(values) {
  const result = [];
  for (let idx = 0; idx <= 15; idx += 1) {
    result.push(Number(values && values[idx] !== undefined ? values[idx] : 0));
  }
  return result;
}

function buildAlarmBackup(items) {
  return (items || []).slice().sort((a, b) => a.idx - b.idx).map((item) => ({
    idx: Number(item.idx),
    active: !!(Number(item.flags || 0) & 0x80),
    from: (Number(item.flags || 0) & 0x38) >> 3,
    to: Number(item.flags || 0) & 0x07,
    hour: Math.floor(Number(item.minutes || 0) / 60),
    minute: Number(item.minutes || 0) % 60
  }));
}

function buildCollectionBackupState(settings) {
  const safeSettings = settings || {};
  return {
    settings: safeSettings,
    overlayCount: Number((((safeSettings || {}).numvars || [])[NUM.OVERLAY_N_OVERLAYS] || 0))
  };
}

function buildCollectionBackupSections(collectionState) {
  const state = collectionState && collectionState.settings
    ? collectionState
    : buildCollectionBackupState(collectionState);

  return {
    overlays: buildOverlayBackup(state),
    timers: {
      display: buildTimerBackup(state.settings.nighttimes || []),
      ambilight: buildTimerBackup(state.settings.ambinighttimes || [])
    }
  };
}

// Overlay-Parameter kennen seit dem Parametervertrag aus Runde 1 keine stille
// Abkuerzung mehr: interval muss 1..255 sein, duration 5..9, days 1..255. Eine 0 hiess
// bisher "nimm die Vorgabe" -- der ESP hat sie selbst ersetzt und {"ok":true} gemeldet.
// Jetzt weist er sie ab. Die PWA sendet deshalb genau den Wert, den der ESP frueher
// eingesetzt hat: Das Geraet bekommt denselben Zustand wie vorher, nur steht er jetzt
// in der Anfrage statt in einer stillen Korrektur.
const OVERLAY_PARAM_RULES = {
  interval: { fallback: 5, min: 1, max: 255, labelKey: "backup.field.overlay_interval" },
  duration: { fallback: 5, min: 5, max: 9, labelKey: "backup.field.overlay_duration" },
  days: { fallback: 1, min: 1, max: 255, labelKey: "backup.field.overlay_days" }
};

function overlayParamOrDefault(value, rule) {
  const number = Number(value);

  return Number.isFinite(number) && number !== 0 ? number : rule.fallback;
}

function readOverlayImportParam(value, rule) {
  return readClampedImportNumber(overlayParamOrDefault(value, rule), rule.min, rule.max, rule.labelKey);
}

// month und day sind ein Paar: beide gesetzt oder beide 0. Eine Teilangabe hat das
// Geraet frueher stillschweigend verworfen und Erfolg gemeldet, heute weist es sie ab.
// Ein Startdatum ohne Tag hat also nie gewirkt -- daraus wird deshalb wieder "kein
// Startdatum" und kein Fehlschlag an einem Feld, das ohnehin folgenlos war.
function pairOverlayStartDate(month, day) {
  const monthNumber = Number(month) || 0;
  const dayNumber = Number(day) || 0;

  if (monthNumber >= 1 && monthNumber <= 12 && dayNumber >= 1 && dayNumber <= 31) {
    return { month: monthNumber, day: dayNumber };
  }

  return { month: 0, day: 0 };
}

function overlayStartDateIsPartial(month, day) {
  const monthSet = (Number(month) || 0) > 0;
  const daySet = (Number(day) || 0) > 0;

  return monthSet !== daySet;
}

function normalizeOverlayBackupItems(items, count) {
  return (items || [])
    .filter((item) => count === undefined || Number(item.idx) < count)
    .slice()
    .sort((a, b) => a.idx - b.idx)
    .map((item) => ({
      idx: Number(item.idx),
      active: !!(Number(item.flags || 0) & 0x01),
      type: Number(item.type || 0),
      value: item.text || "",
      interval: overlayParamOrDefault(item.interval, OVERLAY_PARAM_RULES.interval),
      duration: overlayParamOrDefault(item.duration, OVERLAY_PARAM_RULES.duration),
      date_code: Number(item.date_code || 0),
      // Ein date_start mit Monat ohne Tag -- aus der Zeit, als das Geraet die
      // Teilangabe noch annahm -- wuerde beim Zurueckspielen abgewiesen. Die Sicherung
      // traegt deshalb schon das Paar, nicht die Haelfte.
      ...pairOverlayStartDate(
        item.date_start ? ((Number(item.date_start) >> 8) & 0xff) : 0,
        item.date_start ? (Number(item.date_start) & 0xff) : 0
      ),
      days: overlayParamOrDefault(item.days, OVERLAY_PARAM_RULES.days)
    }));
}

function buildOverlayBackup(collectionState) {
  const state = collectionState && collectionState.settings
    ? collectionState
    : buildCollectionBackupState(collectionState);
  return normalizeOverlayBackupItems(state.settings.overlays || [], state.overlayCount);
}

function normalizeTimerBackupItems(items) {
  return (items || []).slice().sort((a, b) => a.idx - b.idx).map((item) => ({
    idx: Number(item.idx),
    active: !!(Number(item.flags || 0) & 0x80),
    switch_on: !!(Number(item.flags || 0) & 0x40),
    from: (Number(item.flags || 0) & 0x38) >> 3,
    to: Number(item.flags || 0) & 0x07,
    hour: Math.floor(Number(item.minutes || 0) / 60),
    minute: Number(item.minutes || 0) % 60
  }));
}

function buildTimerBackup(items) {
  return normalizeTimerBackupItems(items);
}

function normalizedOverlayItemsEqual(expectedItems, actualItems) {
  if (expectedItems.length !== actualItems.length) {
    return false;
  }

  return expectedItems.every((entry, index) => {
    const current = actualItems[index] || {};
    return !!entry.active === !!current.active &&
      Number(entry.type || 0) === Number(current.type || 0) &&
      String(entry.value || "") === String(current.value || "") &&
      Number(entry.interval || 0) === Number(current.interval || 0) &&
      Number(entry.duration || 0) === Number(current.duration || 0) &&
      Number(entry.date_code || 0) === Number(current.date_code || 0) &&
      Number(entry.month || 0) === Number(current.month || 0) &&
      Number(entry.day || 0) === Number(current.day || 0) &&
      Number(entry.days || 0) === Number(current.days || 0);
  });
}

function normalizedTimerItemsEqual(expectedItems, actualItems) {
  if (expectedItems.length !== actualItems.length) {
    return false;
  }

  return expectedItems.every((entry, index) => {
    const current = actualItems[index] || {};
    return Number(entry.idx || 0) === Number(current.idx || 0) &&
      !!entry.active === !!current.active &&
      !!entry.switch_on === !!current.switch_on &&
      Number(entry.from || 0) === Number(current.from || 0) &&
      Number(entry.to || 0) === Number(current.to || 0) &&
      Number(entry.hour || 0) === Number(current.hour || 0) &&
      Number(entry.minute || 0) === Number(current.minute || 0);
  });
}

function getDisplayBackupMeta(settings) {
  const coreMeta = getCoreBackupUiMeta(settings, getCurrentEepromSettings(), getCurrentNetworkInfo());
  const flags = settings.numvars[NUM.DISPLAY_FLAGS] || 0;

  return {
    firmwareVersion: settings.strvars[STR.VERSION] || "",
    espVersion: getUpdateStatusString(null, "esp_version") || settings.strvars[STR.ESP8266_VERSION] || "",
    eepromVersion: settings.strvars[STR.EEPROM_VERSION] || "",
    power: !!settings.numvars[NUM.DISPLAY_POWER],
    mode: Number(coreMeta.display.currentDisplayMode || 0),
    useRgbw: !!settings.numvars[NUM.DISPLAY_USE_RGBW],
    brightness: Number(settings.numvars[NUM.DISPLAY_BRIGHTNESS] || 0),
    automaticBrightness: !!settings.numvars[NUM.DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE],
    permanentItIs: !!(flags & 0x01),
    tickerText: coreMeta.display.tickerText,
    dateTickerFormat: coreMeta.display.dateFormat,
    tickerDeceleration: Number(coreMeta.display.tickerDeceleration || 0),
    dimCurve: buildDimCurveArray(getDimCurveUiMeta(settings).displayValues)
  };
}

function getNetworkBackupMeta(settings, eepromSettings) {
  const coreMeta = getCoreBackupUiMeta(settings, eepromSettings, getCurrentNetworkInfo());

  return {
    timeserver: coreMeta.network.timeserver,
    timezoneOffset: Number(coreMeta.network.timezoneOffset || 0),
    summertime: !!coreMeta.network.summertime,
    wifiSsid: eepromSettings && eepromSettings.ssid ? eepromSettings.ssid : "",
    wifiKey: eepromSettings && eepromSettings.key ? eepromSettings.key : "",
    apSsid: eepromSettings && eepromSettings.ap_ssid ? eepromSettings.ap_ssid : "",
    apKey: eepromSettings && eepromSettings.ap_key ? eepromSettings.ap_key : "",
    bootAsAp: !!(eepromSettings && eepromSettings.boot_as_ap)
  };
}

function getMaintenanceBackupMeta(settings) {
  const coreMeta = getCoreBackupUiMeta(settings, getCurrentEepromSettings(), getCurrentNetworkInfo());

  return {
    updateHost: coreMeta.maintenance.updateHost,
    updatePath: coreMeta.maintenance.updatePath
  };
}

function getClimateBackupMeta(settings) {
  const coreMeta = getCoreBackupUiMeta(settings, getCurrentEepromSettings(), getCurrentNetworkInfo());

  return {
    weatherAppId: coreMeta.weather.appId,
    weatherCity: coreMeta.weather.city,
    weatherLon: coreMeta.weather.lon,
    weatherLat: coreMeta.weather.lat,
    rtcTempCorrection: Number(coreMeta.temperature.rtcCorrection || 0),
    ds18xxTempCorrection: Number(coreMeta.temperature.ds18xxCorrection || 0),
    ldrMin: Number(settings.numvars[NUM.LDR_MIN_VALUE] || 0),
    ldrMax: Number(settings.numvars[NUM.LDR_MAX_VALUE] || 0)
  };
}

function getAnimationBackupMeta(settings) {
  const coreMeta = getCoreBackupUiMeta(settings, getCurrentEepromSettings(), getCurrentNetworkInfo());

  return {
    displayMode: Number(coreMeta.animation.displayAnimationMode || 0),
    colorMode: Number(coreMeta.animation.colorAnimationMode || 0),
    displayProfiles: coreMeta.animation.displayAnimations || [],
    colorProfiles: coreMeta.animation.colorAnimations || []
  };
}

function getAmbilightBackupMeta(settings) {
  const ambilightMeta = getAmbilightUiMeta(settings);
  const flagMeta = getFlagUiMeta(settings, true);
  const dimMeta = getDimCurveUiMeta(settings);
  const currentMode = (settings.almodes || []).find((entry) => entry.idx === ambilightMeta.currentMode) || null;

  return {
    online: !!settings.numvars[NUM.AMBILIGHT_IS_UP],
    power: !!settings.numvars[NUM.DISPLAY_AMBILIGHT_POWER],
    mode: Number(ambilightMeta.currentMode || 0),
    leds: Number(ambilightMeta.leds || 0),
    offset: Number(ambilightMeta.offset || 0),
    brightness: Number(settings.numvars[NUM.AMBILIGHT_BRIGHTNESS] || 0),
    syncAmbilight: flagMeta.syncAmbilight,
    syncMarkers: flagMeta.syncMarkers,
    fadeClockSeconds: flagMeta.fadeClockSeconds,
    secondsMarkers: !!(((currentMode && currentMode.flags) || 0) & 0x02),
    dimCurve: buildDimCurveArray(dimMeta.ambilightValues),
    profiles: getAmbilightProfileUiMeta(settings).items || []
  };
}

function getDfplayerBackupMeta(settings) {
  const controlMeta = getDfplayerControlUiMeta(settings, { dfplayer: "on" });

  return {
    volume: Number(controlMeta.volume || 0),
    mode: Number(controlMeta.mode || 0),
    bellFlags: Number(settings.numvars[NUM.DFPLAYER_BELL_FLAGS] || 0),
    speakCycle: Number(controlMeta.speakCycle || 0),
    silenceStart: Number(settings.numvars[NUM.DFPLAYER_SILENCE_START] || 0),
    silenceStop: Number(settings.numvars[NUM.DFPLAYER_SILENCE_STOP] || 0),
    alarms: getDfplayerUiMeta(settings).alarms
  };
}

async function ensureBackupExportState() {
  if (!getCurrentSettingsSnapshot()) {
    await loadData();
  }

  if (!getCurrentEepromSettings().ok) {
    setCurrentEepromSettings(await settleFetchJson(getEepromSettingsUrl(), {}, 5000));
  }

  // Ohne diese Werte stuenden im Backup vier leere WLAN-Felder -- und der Import
  // schriebe sie spaeter zurueck. Lieber gar kein Backup als eines, das beim
  // Einspielen die Zugangsdaten loescht.
  if (!getCurrentEepromSettings().ok) {
    const error = new Error("eeprom-settings-unavailable");
    error.backupReason = "eeprom-settings-unavailable";
    throw error;
  }

  if (!getUpdateTableCurrentFile(getCurrentUpdateTableInfo())) {
    setCurrentUpdateTableInfo(await settleFetchJson(getUpdateTableFilesUrl(), {}, 5000));
  }
}

// Ein Download, zwei Aufrufer: die Sicherungsdatei des Exports und die Rückfalldatei
// vor dem IR-Restore. Bewusst eine Funktion -- zwei Kopien laufen irgendwann
// auseinander, und die Rückfalldatei ist der einzige Rückweg, den es gibt.
function buildBackupFileTimestamp() {
  return new Date().toISOString().replace(/[:]/g, "-").replace(/\..+/, "");
}

function triggerJsonDownload(fileName, data) {
  const blob = new Blob([JSON.stringify(data, null, 2)], { type: "application/json" });
  const link = document.createElement("a");

  link.href = URL.createObjectURL(blob);
  link.download = fileName;
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
  window.setTimeout(() => URL.revokeObjectURL(link.href), 1000);
}

async function exportSettingsBackup() {
  const button = document.getElementById("settings-export-button");

  beginButtonFeedback(button, translate("backup.exporting"));

  try {
    const exportResult = await prepareSettingsBackupExport();
    triggerJsonDownload("wordclock-settings-" + buildBackupFileTimestamp() + ".json", exportResult.backup);
    const irNote = getIrBackupExportNote(exportResult.ir);
    setSettingsBackupNote(translate("backup.export_success") + " " + irNote.message, irNote.tone);
    finishButtonFeedback(button, translate("backup.export_button"), "success", translate("backup.exported"));
  } catch (error) {
    const reason = error && error.backupReason === "eeprom-settings-unavailable"
      ? "backup.export_eeprom_unavailable"
      : "backup.export_failed";
    setSettingsBackupNote(translate(reason), "error");
    finishButtonFeedback(button, translate("backup.export_button"), "error", translate("common.error"));
  }
}

async function importSettingsBackup() {
  const input = document.getElementById("settings-import-file-input");
  const button = document.getElementById("settings-import-button");
  const file = input && input.files && input.files[0];

  if (!file) {
    setSettingsBackupNote(translate("backup.choose_file_first"), "warn");
    return;
  }

  beginButtonFeedback(button, translate("backup.importing"));

  try {
    const importedState = await loadImportedBackupState(file);

    if (!window.confirm(translateFormat("backup.import_confirm", { file: file.name }))) {
      finishButtonFeedback(button, translate("backup.import_button"));
      return;
    }

    setSettingsBackupNote(translate("backup.import_start"));
    markEditsPersisted();
    await applySettingsBackup(importedState);
    finishButtonFeedback(button, translate("backup.import_button"), "success", translate("backup.imported"));
  } catch (error) {
    setSettingsBackupNote(getSettingsBackupImportErrorMessage(error), "error");
    finishButtonFeedback(button, translate("backup.import_button"), "error", translate("common.error"));
  } finally {
    if (input) {
      input.value = "";
    }
  }
}

async function applySettingsBackup(backup) {
  const importedState = backup && backup.executionState
    ? backup
    : buildImportedBackupState(backup);

  resetSkippedImportFields();
  resetAdjustedImportFields();
  resetFailedImportStages();
  resetIrImportNotice();
  resetImportedBackupVersionNotice();
  noteImportedBackupVersion(readBackupFileVersion(importedState.backup));

  // Früh angesagt, nicht erst in der Schlussmeldung: Der Import dauert über den
  // STM-Neustart hinweg mehrere Sekunden, und bricht er unterwegs ab, erfährt der
  // Nutzer den Grund sonst gar nicht. Der Hinweis steht am Ende trotzdem noch einmal
  // in getImportNoticeSummary -- dort zusammen mit dem, was tatsächlich passiert ist.
  const versionNotice = getImportedBackupVersionSummary();
  if (versionNotice) {
    announceStatus(versionNotice, "warn");
  }

  settingsImportInProgress = true;
  try {
    await runImportWorkflow(importedState.executionState);
  } finally {
    settingsImportInProgress = false;
  }
}

function networkImportNeedsRetry(network, settings, eepromSettings) {
  if (!network || !settings) {
    return false;
  }

  const current = getImportComparisonMeta(settings, eepromSettings).network;

  return importOptionalFieldMismatch(network, current, ["timeserver"]) ||
    Number(network.timezone_offset || 0) !== Number(current.timezone_offset || 0) ||
    !!network.summertime !== !!current.summertime ||
    (!!(eepromSettings && eepromSettings.ok) && !!network.boot_as_ap !== !!current.boot_as_ap);
}

async function runImportPhase(step, options) {
  const opts = options || {};
  await step();

  if (opts.pauseMs > 0) {
    await sleep(opts.pauseMs);
  }

  if (opts.reload) {
    await loadData();
  }
}

async function runTimedImportPhase(step, pauseMs) {
  await runImportPhase(step, { pauseMs });
}

async function runReloadingImportPhase(step, pauseMs) {
  await runImportPhase(step, { pauseMs, reload: true });
}

// L203, zweiter Fund: Die Schleife hatte keinen Fehlerfang je Stufe. Ein einziger vom
// Geraet abgewiesener Wert warf aus runImportStageList heraus und beendete damit nicht
// nur die laufende Stufe, sondern den ganzen Rest des Imports. Bei
// importDisplaySettings waeren das Ticker, Farbe und Dimmkurve gewesen -- und seit
// Runde 1 weist der ESP ab, statt still zu klemmen, die Eintrittswahrscheinlichkeit ist
// also GESTIEGEN, nicht gesunken.
//
// Was dieser Fang leistet und was nicht, bewusst getrennt gesagt: Die folgenden Stufen
// laufen jetzt weiter, und der Nutzer erfaehrt am Ende namentlich, welche Stufe
// abgebrochen ist. INNERHALB einer Stufe bricht die Kette weiterhin ab -- die
// Importfunktionen sind Folgen von await-Aufrufen. Deshalb nennt die Meldung die Stufe
// und sagt, dass darin etwas offengeblieben ist, statt Vollstaendigkeit zu behaupten.
async function runImportStageList(stages) {
  for (const stage of (stages || [])) {
    if (!stage || stage.when === false) {
      continue;
    }

    if (stage.note) {
      setSettingsBackupNote(stage.note);
    }

    try {
      if (stage.reloadOnly) {
        await reloadImportedData(stage.pauseMs || 0);
        continue;
      }

      if (typeof stage.run !== "function") {
        if (stage.pauseMs > 0) {
          await sleep(stage.pauseMs);
        }
        continue;
      }

      if (stage.reload) {
        await runReloadingImportPhase(stage.run, stage.pauseMs || 0);
      } else {
        await runTimedImportPhase(stage.run, stage.pauseMs || 0);
      }
    } catch (error) {
      noteFailedImportStage(stage.note, error);
    }
  }
}

async function runConditionalImportRetryStages(stages) {
  for (const stage of (stages || [])) {
    if (!stage) {
      continue;
    }

    await runConditionalImportRetry(
      !!stage.shouldRetry,
      stage.note,
      stage.run,
      stage.options
    );
  }
}

async function reloadImportedData(pauseMs) {
  if (pauseMs > 0) {
    await sleep(pauseMs);
  }
  await loadData();
}

async function rerunImportStep(note, step, options) {
  setSettingsBackupNote(note);
  await runImportPhase(step, options);
}

async function runConditionalImportRetry(shouldRetry, note, step, options) {
  if (!shouldRetry) {
    return;
  }

  await rerunImportStep(note, step, options);
}

function buildImportExecutionState(baseState) {
  return {
    backup: baseState && baseState.backup ? baseState.backup : {},
    settings: baseState && baseState.settings ? baseState.settings : {},
    assets: baseState && baseState.assets ? baseState.assets : {},
    climate: baseState && baseState.climate ? baseState.climate : null,
    network: baseState && baseState.network ? baseState.network : null,
    maintenance: baseState && baseState.maintenance ? baseState.maintenance : null
  };
}

function buildImportRuntimeState(baseState) {
  const state = baseState || {};
  return {
    execution: buildImportExecutionState(state),
    retry: buildImportRetryExecutionState(state.settings, state.snapshot, state.eepromSettings)
  };
}

function getImportExecutionState(backup, settings) {
  const importSettings = settings || {};
  return buildImportRuntimeState({
    backup: backup || {},
    settings: importSettings,
    assets: (backup && backup.assets) || {},
    climate: importSettings.climate || null,
    network: importSettings.network || null,
    maintenance: importSettings.maintenance || null
  }).execution;
}

function buildImportWorkflowPlan(executionState) {
  return {
    primary: buildPrimaryImportStages(executionState),
    postPrimary: buildPostPrimaryImportStages(executionState),
    restart: buildImportRestartPlan(executionState)
  };
}

async function runImportWorkflow(executionState) {
  const plan = buildImportWorkflowPlan(executionState);
  await runImportStageList(plan.primary);
  await runImportStageList(plan.postPrimary);
  await finalizeImportRestartAndReload(plan.restart);
}

function buildPostPrimaryImportStages(executionState) {
  const settings = executionState && executionState.settings ? executionState.settings : {};
  const network = executionState && executionState.network ? executionState.network : null;
  const climate = executionState && executionState.climate ? executionState.climate : null;

  return [
    {
      note: translate("backup.import_validate"),
      reloadOnly: true,
      pauseMs: 600
    },
    {
      note: translate("backup.import_verify_sections"),
      run: () => retryImportedSectionsIfNeeded(settings)
    },
    {
      note: translate("backup.import_reload_data"),
      reloadOnly: true,
      pauseMs: 400
    },
    {
      when: !!network,
      note: translate("backup.import_network_final"),
      run: () => importNetworkSettings(network),
      reload: true,
      pauseMs: 400
    },
    {
      when: !!network,
      note: translate("backup.import_network_verify"),
      run: () => retryImportedNetworkSettingsIfNeeded(network)
    },
    {
      note: translate("backup.import_network_wait"),
      pauseMs: 400
    },
    {
      when: !!climate,
      note: translate("backup.import_sensor_final"),
      run: () => importSensorCorrectionSettings(climate),
      reload: true,
      pauseMs: 600
    },
    {
      when: !!climate,
      note: translate("backup.import_sensor_verify"),
      run: () => rerunSensorCorrectionImportIfNeeded(climate)
    },
    {
      note: translate("backup.import_persist_critical"),
      run: () => finalizeImportedCriticalPersistenceSettings(settings)
    },
    {
      when: !!climate,
      note: translate("backup.import_persist_temperature"),
      run: () => finalizeTemperatureCorrectionPersistence(climate)
    },
    {
      note: translate("backup.import_wait_persist"),
      reloadOnly: true,
      pauseMs: 5000
    }
  ];
}

async function runPostPrimaryImportStages(settings) {
  await runImportStageList(buildPostPrimaryImportStages(getImportExecutionState(null, settings)));
}

async function rerunSensorCorrectionImportIfNeeded(climate) {
  if (!climate || !sensorCorrectionsImportNeedsRetry(climate, getCurrentSettingsSnapshot())) {
    return;
  }

  await rerunImportStep(
    translate("backup.import_temperature_retry"),
    () => importSensorCorrectionSettings(climate),
    { pauseMs: 600, reload: true }
  );
}

function buildImportRestartPlan(executionState) {
  return {
    climate: executionState && executionState.climate ? executionState.climate : null
  };
}

// Beide Listen landen im selben Hinweis: Übersprungenes zuerst, Zurechtgebogenes
// danach. Der Nutzer soll nach einem Import an genau einer Stelle sehen, was nicht so
// übernommen wurde, wie es in der Datei stand.
function getImportNoticeSummary() {
  return [
    getImportedBackupVersionSummary(),
    getFailedImportStagesSummary(),
    getSkippedImportFieldsSummary(),
    getAdjustedImportFieldsSummary(),
    getIrImportSummary()
  ].filter(Boolean).join(" ");
}

function appendImportNoticeHint(message) {
  const summary = getImportNoticeSummary();
  return summary ? message + " " + summary : message;
}

async function finalizeImportRestartAndReload(restartPlan) {
  const climate = restartPlan && restartPlan.climate ? restartPlan.climate : null;

  setSettingsBackupNote(translate("backup.import_restart_now"));
  announceStatus(translate("backup.import_restart"), "warn");
  await apiFetch(getMaintenanceResetStm32Url());
  await sleep(4500);
  try {
    if (climate) {
      await finalizeTemperatureCorrectionPersistence(climate);
    }
    setSettingsBackupNote(translate("backup.import_restart_reload_data"));
    await loadData();
    // Erst hier abgefragt, nicht am Funktionsanfang: Die Persistenz der
    // Temperaturkorrektur oben kann selbst noch einen zurechtgebogenen Wert melden,
    // und mit einem vorab festgehaltenen Zwischenstand hätte der Hinweis gefehlt.
    const notice = getImportNoticeSummary();
    setSettingsBackupNote(appendImportNoticeHint(translate("backup.import_done_reload")), notice ? "warn" : "success");
    announceStatus(notice || translate("backup.import_reload"), notice ? "warn" : "ok");
  } catch (error) {
    const notice = getImportNoticeSummary();
    setSettingsBackupNote(appendImportNoticeHint(translate("backup.import_restart_refreshing")), notice ? "warn" : "success");
    announceStatus(notice || translate("backup.import_reconnect"), notice ? "warn" : "ok");
  }
  // Wurde etwas übersprungen oder zurechtgebogen, bleibt der Hinweis länger stehen --
  // nach dem Neuladen der App ist er weg, und 1,2 s reichen nicht zum Lesen.
  setTimeout(reloadAppPage, getImportNoticeSummary() ? 6000 : 1200);
}

// ---------------------------------------------------------------------------
// F1 -- IR-Codes zurückschreiben (specs/f1-ir-backup, AK13 bis AK16)
//
// Das ist der destruktive Teil der Massnahme. Ein falsch geschriebener Code macht die
// betroffene Taste unbrauchbar, und der einzige Rückweg ist erneutes Anlernen über
// /api/learn_ir -- ein Vorgang, der die Uhr blockiert, bis ein Mensch alle Tasten
// gedrückt hat, und der deshalb in der Gefahrenliste steht. Einen zweiten Weg zurück
// gibt es nicht. Daraus folgt alles Weitere: Rückfalldatei VOR dem ersten
// Schreibzugriff, Bestätigung mit Zahl und Namen, und eine Gegenprobe, an der die
// Erfolgsmeldung hängt.
//
// Kosten am Gerät: Ein /api/ir_code_set erzeugt genau EIN STM-Kommando und rund 80 ms
// EEPROM-Schreibzeit, in der der STM die Kommandobrücke nicht bedient. Sein
// Empfangsring verwirft bei Überlauf still, ohne Log und ohne Zähler -- deshalb
// sequenziell mit Pause, niemals parallel. Dazu kommen zwei Abzüge (vorher, Gegenprobe)
// mit je einem RPC-Kommando.
const IR_RESTORE_WRITE_PAUSE_MS = 120;
const IR_RESTORE_NAME_PREVIEW = 10;
const IR_RESTORE_FALLBACK_PREFIX = "wordclock-ir-vorher-";

// Der Stufen-Hinweis wird von der nächsten Import-Stufe überschrieben, und der Import
// läuft danach noch über den STM-Neustart hinweg weiter. Was die Gegenprobe gefunden
// hat, muss den ganzen Import überleben -- deshalb zusätzlich über
// getImportNoticeSummary(), wie bei übersprungenen und zurechtgebogenen Feldern.
let irImportNoticeMessage = "";

function resetIrImportNotice() {
  irImportNoticeMessage = "";
}

function noteIrImportResult(message) {
  if (!message) {
    return;
  }

  irImportNoticeMessage = message;
  setSettingsBackupNote(message, "warn");
  announceStatus(message, "warn");
  console.warn("Import: " + message);
}

function getIrImportSummary() {
  return irImportNoticeMessage;
}

// Die Schlüssel stehen ausgeschrieben, nicht als "backup.ir_reason_" + reason
// zusammengesetzt: Ein zusammengesetzter Schlüssel ist mit grep nicht auffindbar, und
// ein fehlender stuende dann wörtlich im Satz, statt beim Prüfen aufzufallen.
const IR_SNAPSHOT_REASON_KEYS = {
  stale: "backup.ir_reason_stale",
  unavailable: "backup.ir_reason_unavailable",
  invalid: "backup.ir_reason_invalid",
  download: "backup.ir_reason_download",
  incomplete: "backup.ir_reason_incomplete"
};

function getIrSnapshotReasonText(reason) {
  return translate(IR_SNAPSHOT_REASON_KEYS[reason] || IR_SNAPSHOT_REASON_KEYS.incomplete);
}

// Zwanzig technische Bezeichner in einem window.confirm sind auf dem Telefon nicht mehr
// lesbar. Gekürzt wird deshalb die Anzeige, nicht die Information: Die vollständige
// Liste geht in jedem Fall in die Konsole.
function formatIrKeyNames(names) {
  const list = (names || []).filter(Boolean);

  if (list.length <= IR_RESTORE_NAME_PREVIEW) {
    return list.join(", ");
  }

  return list.slice(0, IR_RESTORE_NAME_PREVIEW).join(", ") +
    translateFormat("backup.ir_restore_more_names", { rest: list.length - IR_RESTORE_NAME_PREVIEW });
}

// Zuordnung über den NAMEN, nicht über den Index. src/remote-ir/remote-ir.h:19-34
// enthält einen auskommentierten Block "New Modes (future use)"; wird er je aktiviert,
// verschiebt sich die Nummerierung, und eine indexbasierte Zuordnung schriebe still auf
// die falschen Tasten. Der Index aus der Datei wird bewusst NICHT verwendet -- er ist
// dort informativ. Geschrieben wird gegen die Position in IR_BACKUP_KEY_NAMES.
//
// Die Formprüfung ist die Verteidigung gegen eine von Hand bearbeitete Datei: genau so
// viele Einträge wie Namen, und jeder erwartete Name genau einmal.
function buildIrRestorePlan(section) {
  const keys = section && Array.isArray(section.keys) ? section.keys : null;

  if (!keys || keys.length !== IR_BACKUP_KEY_NAMES.length) {
    return { ok: false, writes: [], skipped: 0, invalid: [] };
  }

  const byName = new Map();

  keys.forEach((entry) => {
    const name = entry && typeof entry.name === "string" ? entry.name : "";

    if (name && !byName.has(name)) {
      byName.set(name, entry);
    }
  });

  if (byName.size !== IR_BACKUP_KEY_NAMES.length ||
      !IR_BACKUP_KEY_NAMES.every((name) => byName.has(name))) {
    return { ok: false, writes: [], skipped: 0, invalid: [] };
  }

  const writes = [];
  const invalid = [];
  let skipped = 0;

  IR_BACKUP_KEY_NAMES.forEach((name, index) => {
    const entry = byName.get(name);
    const raw = entry.protocol;

    // null, undefined, 0 und 255 sind dieselbe Konvention: "nie angelernt". Übersprungen,
    // nicht geschrieben -- ein Restore, der hier löschte, vernichtete im Zweifel Arbeit,
    // und /api/ir_code_set wiese protocol 0 und 255 ohnehin ab. AK13.
    if (raw === null || raw === undefined || Number(raw) === 0 || Number(raw) === 255) {
      skipped += 1;
      return;
    }

    const protocol = readIrCodeNumber(raw, 254);
    const address = readIrCodeNumber(entry.address, 65535);
    const command = readIrCodeNumber(entry.command, 65535);

    // Lokal geprüft, bevor etwas rausgeht: Ein Wert ausserhalb des Bereichs holte vom
    // Gerät nur die Kennung 2 zurück, und apiFetch machte daraus eine Ausnahme mitten in
    // der Schleife. Hier wird er gezählt und benannt, und die übrigen Tasten laufen
    // weiter.
    if (protocol === null || protocol < 1 || address === null || command === null) {
      invalid.push(name);
      return;
    }

    writes.push({ name, index, protocol, address, command });
  });

  return { ok: true, writes, skipped, invalid };
}

// AK15 -- der Rückweg entsteht VOR dem ersten Schreibzugriff, nicht danach.
//
// Unvollständig heisst hier: gar keine Datei. Eine Rückfalldatei mit 19 von 20 Tasten
// liefe beim nächsten Import in genau die Formprüfung oben und wäre nutzlos -- sie sähe
// nur aus wie ein Rückweg. Die Rohantwort geht stattdessen in die Konsole, damit der
// Stand nicht völlig verloren ist.
//
// Die Datei ist eine vollwertige Sicherung der Version 3 mit ausschliesslich dem
// Abschnitt settings.ir. Sie lässt sich damit über den normalen Import wieder
// einspielen; alle übrigen Import-Stufen steigen bei fehlendem Abschnitt aus.
async function createIrRestoreFallbackFile() {
  let result = null;

  try {
    result = await collectIrBackupSection();
  } catch (error) {
    console.warn("Import: Rückfalldatei der IR-Codes nicht angelegt, Abzug fehlgeschlagen.", error);
    return { ok: false, reason: "unavailable", fileName: "" };
  }

  if (!result.section) {
    console.warn("Import: Rückfalldatei der IR-Codes nicht angelegt, Abzug unbrauchbar (" +
      String(result.reason) + "). Rohstand:", result.status);
    return { ok: false, reason: result.reason, fileName: "" };
  }

  const fileName = IR_RESTORE_FALLBACK_PREFIX + buildBackupFileTimestamp() + ".json";

  try {
    triggerJsonDownload(fileName, {
      format: BACKUP_FORMAT,
      version: BACKUP_VERSION,
      exported_at: new Date().toISOString(),
      settings: { ir: result.section }
    });
  } catch (error) {
    console.warn("Import: Rückfalldatei der IR-Codes liess sich nicht herunterladen.", error,
      result.section);
    return { ok: false, reason: "download", fileName: "" };
  }

  return { ok: true, reason: "", fileName };
}

// AK16 -- die einzige Gegenprobe, die es gibt.
//
// {"ok":true} von /api/ir_code_set heisst "abgeschickt", nicht "angekommen": Wird das
// Kommando auf der UART verstümmelt, weist der STM es ab, und der ESP erfährt davon
// nichts. Jeder Schreibaufruf invalidiert den ESP-Puffer (AK9) -- dieser Abzug ist
// deshalb zwangsläufig ein frischer Wert vom STM und nicht das Echo der eigenen Eingabe.
//
// Verglichen wird nur, was tatsächlich geschrieben wurde. Übersprungene Tasten sind
// absichtlich unverändert; sie in den Vergleich zu nehmen, erzeugte Fehlalarme.
async function verifyIrRestore(written) {
  let result = null;

  try {
    result = await collectIrBackupSection();
  } catch (error) {
    console.warn("Import: Gegenprobe der IR-Codes fehlgeschlagen.", error);
    return { ok: false, reason: "unavailable", mismatches: [] };
  }

  if (!result.section) {
    return { ok: false, reason: result.reason, mismatches: [] };
  }

  const byIndex = new Map(result.section.keys.map((key) => [key.index, key]));
  const mismatches = [];

  written.forEach((entry) => {
    const key = byIndex.get(entry.index);

    if (!key || key.protocol !== entry.protocol || key.address !== entry.address ||
        key.command !== entry.command) {
      mismatches.push(entry.name);
      console.warn("Import: IR-Taste " + entry.name + " (idx " + entry.index +
        ") weicht nach dem Schreiben ab.", { gewollt: entry, gelesen: key || null });
    }
  });

  return { ok: true, reason: "", mismatches };
}

async function writeIrRestorePlan(writes) {
  const written = [];
  const failed = [];

  for (let position = 0; position < writes.length; position += 1) {
    const entry = writes[position];

    setSettingsBackupNote(translateFormat("backup.ir_restore_writing", {
      done: position + 1,
      count: writes.length,
      name: entry.name
    }));

    try {
      await apiFetchQuery(getIrCodeSetUrl(), {
        idx: entry.index,
        protocol: entry.protocol,
        address: entry.address,
        command: entry.command
      });
      written.push(entry);
    } catch (error) {
      // Kein Abbruch der ganzen Stufe: Die übrigen Tasten sind von diesem Fehlschlag
      // unabhängig, und ein Abbruch in der Mitte liesse einen halb geschriebenen Satz
      // zurück, über den niemand mehr etwas erfährt. Gezählt, benannt und gemeldet.
      failed.push(entry.name);
      console.warn("Import: IR-Taste " + entry.name + " (idx " + entry.index +
        ") nicht geschrieben.", error);
    }

    // Flusskontrolle, nicht Kosmetik. apiFetch legt während eines Imports ohnehin 180 ms
    // ein; die Pause steht hier trotzdem ausdrücklich, damit die Taktung nicht an einem
    // Flag hängt, das ausserhalb des Imports nicht gesetzt ist.
    await sleep(IR_RESTORE_WRITE_PAUSE_MS);
  }

  return { written, failed };
}

// Jede Zahl steht als "{n} von {m}". Das ist keine Kosmetik: "1 Tasten" ist falsches
// Deutsch, und eine zweite Schluesselmenge nur fuer den Singular waere Aufwand ohne
// Gegenwert.
function buildIrRestoreExtras(plan, failed) {
  const extras = [];
  const total = IR_BACKUP_KEY_NAMES.length;

  if (plan.skipped > 0) {
    extras.push(translateFormat("backup.ir_restore_skipped", { skipped: plan.skipped, total }));
  }

  if (plan.invalid.length > 0) {
    extras.push(translateFormat("backup.ir_restore_invalid_entries", {
      count: plan.invalid.length,
      total,
      names: formatIrKeyNames(plan.invalid)
    }));
  }

  if (failed.length > 0) {
    extras.push(translateFormat("backup.ir_restore_write_failed", {
      failed: failed.length,
      count: plan.writes.length,
      names: formatIrKeyNames(failed)
    }));
  }

  return extras;
}

function reportIrRestoreOutcome(plan, fallback, written, failed, verification) {
  const extras = buildIrRestoreExtras(plan, failed);
  const fallbackHint = fallback.ok
    ? translateFormat("backup.ir_restore_fallback_hint", { file: fallback.fileName })
    : translate("backup.ir_restore_fallback_none");

  if (!verification.ok) {
    noteIrImportResult([translateFormat("backup.ir_restore_unverified", {
      written: written.length,
      total: IR_BACKUP_KEY_NAMES.length,
      reason: getIrSnapshotReasonText(verification.reason),
      fallback: fallbackHint
    })].concat(extras).join(" "));
    return;
  }

  if (verification.mismatches.length > 0) {
    noteIrImportResult([translateFormat("backup.ir_restore_mismatch", {
      mismatches: verification.mismatches.length,
      written: written.length,
      names: formatIrKeyNames(verification.mismatches),
      fallback: fallbackHint
    })].concat(extras).join(" "));
    return;
  }

  const success = translateFormat("backup.ir_restore_ok", {
    written: written.length,
    total: IR_BACKUP_KEY_NAMES.length
  });

  // Die Erfolgsmeldung hängt am Abzug, nicht am ok:true der Schreibaufrufe (AK16).
  // Blieb nebenbei etwas liegen -- übersprungen, unbrauchbar, fehlgeschlagen --, bleibt
  // es eine Warnung, auch wenn das Geschriebene stimmt.
  if (extras.length > 0) {
    noteIrImportResult([success].concat(extras).join(" "));
    return;
  }

  setSettingsBackupNote(success, "success");
  console.info("Import: " + success);
}

async function importIrCodes(section) {
  // Fehlt der Abschnitt, ist es eine Sicherung der Version 2 oder eine ohne IR-Abzug.
  // Die Stufe entfällt dann stumm -- das ist AK17 und kein Fehler.
  if (!section) {
    return;
  }

  const plan = buildIrRestorePlan(section);

  if (!plan.ok) {
    noteIrImportResult(translateFormat("backup.ir_restore_section_invalid", {
      expected: IR_BACKUP_KEY_NAMES.length
    }));
    return;
  }

  if (plan.writes.length === 0) {
    console.info("Import: IR-Abschnitt enthält keine angelernte Taste, Stufe entfällt.");

    const extras = buildIrRestoreExtras(plan, []);

    if (plan.invalid.length > 0) {
      noteIrImportResult(extras.join(" "));
    }
    return;
  }

  console.info("Import: IR-Tasten zum Überschreiben: " +
    plan.writes.map((entry) => entry.name).join(", "));

  setSettingsBackupNote(translate("backup.ir_restore_snapshot"));

  const fallback = await createIrRestoreFallbackFile();
  const fallbackSentence = fallback.ok
    ? translateFormat("backup.ir_restore_fallback_ok", { file: fallback.fileName })
    : translateFormat("backup.ir_restore_fallback_failed", {
        reason: getIrSnapshotReasonText(fallback.reason)
      });

  // AK14 -- die Rückfrage nennt die Zahl UND die Namen, nicht nur "wirklich?".
  if (!window.confirm(translateFormat("backup.ir_restore_confirm_1", {
    count: plan.writes.length,
    total: IR_BACKUP_KEY_NAMES.length,
    names: formatIrKeyNames(plan.writes.map((entry) => entry.name)),
    fallback: fallbackSentence
  }))) {
    noteIrImportResult(translate("backup.ir_restore_cancelled"));
    return;
  }

  // AK15 -- ohne Rückfalldatei eine zweite Hürde, mit Abbruch als empfohlener Antwort.
  // Dasselbe zweistufige Muster wie maintenance.reset_eeprom_confirm_1/_2.
  if (!fallback.ok && !window.confirm(translate("backup.ir_restore_confirm_2"))) {
    noteIrImportResult(translate("backup.ir_restore_cancelled"));
    return;
  }

  const result = await writeIrRestorePlan(plan.writes);

  // Ist nichts angekommen, gibt es auch nichts nachzulesen. Der Abzug kostete sechs
  // Sekunden und bestätigte nur, was die Fehlerliste ohnehin sagt.
  if (result.written.length === 0) {
    reportIrRestoreOutcome(plan, fallback, result.written, result.failed,
      { ok: true, reason: "", mismatches: [] });
    return;
  }

  setSettingsBackupNote(translate("backup.ir_restore_verifying"));

  const verification = await verifyIrRestore(result.written);

  reportIrRestoreOutcome(plan, fallback, result.written, result.failed, verification);
}

function buildPrimaryImportStages(executionState) {
  const settings = executionState && executionState.settings ? executionState.settings : {};
  const assets = executionState && executionState.assets ? executionState.assets : {};
  return [
    { note: translate("backup.import_service"), run: () => importMaintenanceSettings(settings.maintenance), pauseMs: 250 },
    { note: translate("backup.import_assets"), run: () => restoreBackupAssets(assets, settings), pauseMs: 250 },
    { note: translate("backup.import_display"), run: () => importDisplaySettings(settings.display), reload: true, pauseMs: 600 },
    { note: translate("backup.import_climate"), run: () => importClimateSettings(settings.climate), pauseMs: 250 },
    { note: translate("backup.import_animations"), run: () => importAnimationSettings(settings.animations), pauseMs: 250 },
    { note: translate("backup.import_tft"), run: () => importTftSettings(settings.tft), pauseMs: 250 },
    { note: translate("backup.import_ambilight"), run: () => importAmbilightSettings(settings.ambilight), pauseMs: 250 },
    { note: translate("backup.import_dfplayer"), run: () => importDfplayerSettings(settings.dfplayer), pauseMs: 250 },
    { note: translate("backup.import_overlays"), run: () => importOverlaySettings(settings.overlays), reload: true, pauseMs: 1200 },
    { note: translate("backup.import_timers"), run: () => importTimerSettings(settings.timers), reload: true, pauseMs: 1200 },
    // Hinter den Timern und vor dem abschliessenden maintenance_reset_stm32: Der
    // Neustart ist hier erwünscht. Die Codes stehen dann im EEPROM, und dass sie den
    // Neustart überleben, ist genau das, was read_configuration_from_eep() beim Start
    // beweist. Kein reload: true -- die IR-Codes sind nicht Teil von loadData().
    { note: translate("backup.import_ir"), run: () => importIrCodes(settings.ir), pauseMs: 250 }
  ];
}

function buildImportRetryExecutionState(settings, snapshot, eepromSettings) {
  const currentSnapshot = snapshot || getCurrentSettingsSnapshot();
  const currentEepromSettings = eepromSettings || getCurrentEepromSettings();

  return {
    settings: settings || {},
    snapshot: currentSnapshot,
    eepromSettings: currentEepromSettings
  };
}

function buildSectionRetryStages(retryState) {
  const settings = retryState && retryState.settings ? retryState.settings : {};
  const snapshot = retryState && retryState.snapshot ? retryState.snapshot : null;

  return [
    {
      shouldRetry: displayImportNeedsRetry(settings.display, snapshot),
      note: translate("backup.import_display_retry"),
      run: () => importDisplaySettings(settings.display),
      options: { pauseMs: 800, reload: true }
    },
    {
      shouldRetry: climateImportNeedsRetry(settings.climate, snapshot),
      note: translate("backup.import_climate_retry"),
      run: () => importClimateSettings(settings.climate)
    },
    {
      shouldRetry: overlaysImportNeedsRetry(settings.overlays, snapshot),
      note: translate("backup.import_overlays_retry"),
      run: () => importOverlaySettings(settings.overlays)
    },
    {
      shouldRetry: timersImportNeedsRetry(settings.timers, snapshot),
      note: translate("backup.import_timers_retry"),
      run: () => importTimerSettings(settings.timers),
      options: { pauseMs: 1200, reload: true }
    }
  ];
}

function buildClimateFinalizationRetryStages(retryState) {
  const climate = retryState && retryState.settings ? retryState.settings.climate : null;
  const snapshot = retryState && retryState.snapshot ? retryState.snapshot : null;

  return [
    {
      shouldRetry: sensorCorrectionsImportNeedsRetry(climate, snapshot),
      note: translate("backup.import_temperature_retry"),
      run: () => importSensorCorrectionSettings(climate),
      options: { pauseMs: 1200, reload: true }
    },
    {
      shouldRetry: rtcCorrectionImportNeedsRetry(climate, snapshot),
      note: translate("backup.import_rtc_final"),
      run: () => importRtcCorrectionSetting(climate),
      options: { pauseMs: 1800, reload: true }
    },
    {
      shouldRetry: rtcCorrectionImportNeedsRetry(climate, snapshot),
      note: translate("backup.import_rtc_retry"),
      run: () => importRtcCorrectionSetting(climate),
      options: { pauseMs: 2200, reload: true }
    }
  ];
}

function buildNetworkRetryStages(retryState) {
  const network = retryState && retryState.settings ? retryState.settings.network : null;
  const snapshot = retryState && retryState.snapshot ? retryState.snapshot : null;
  const eepromSettings = retryState && retryState.eepromSettings ? retryState.eepromSettings : getCurrentEepromSettings();

  return [
    {
      shouldRetry: networkImportNeedsRetry(network, snapshot, eepromSettings),
      note: translate("backup.import_network_retry"),
      run: () => importNetworkSettings(network),
      options: { pauseMs: 400, reload: true }
    }
  ];
}

function buildNetworkTimeRetryStages(retryState) {
  const network = retryState && retryState.settings ? retryState.settings.network : null;
  const snapshot = retryState && retryState.snapshot ? retryState.snapshot : null;
  const eepromSettings = retryState && retryState.eepromSettings ? retryState.eepromSettings : getCurrentEepromSettings();

  return [
    {
      shouldRetry: networkImportNeedsRetry(network, snapshot, eepromSettings),
      note: translate("backup.import_network_time_retry"),
      run: () => importNetworkTimeSettings(network),
      options: { pauseMs: 1500, reload: true }
    }
  ];
}

function buildMaintenanceRetryStages(retryState) {
  const maintenance = retryState && retryState.settings ? retryState.settings.maintenance : null;
  const snapshot = retryState && retryState.snapshot ? retryState.snapshot : null;

  return [
    {
      shouldRetry: maintenanceImportNeedsRetry(maintenance, snapshot),
      note: translate("backup.import_maintenance_retry"),
      run: () => importMaintenanceSettings(maintenance),
      options: { pauseMs: 1500, reload: true }
    }
  ];
}

async function retryImportedNetworkSettingsIfNeeded(network) {
  if (!network) {
    return;
  }

  await runConditionalImportRetryStages(
    buildNetworkRetryStages(buildImportRetryExecutionState({ network }))
  );
}

async function finalizeImportedNetworkTimeSettings(network) {
  if (!network) {
    return;
  }

  setSettingsBackupNote(translate("backup.import_network_time_final"));
  await runReloadingImportPhase(() => importNetworkTimeSettings(network), 1000);

  await runConditionalImportRetryStages(
    buildNetworkTimeRetryStages(buildImportRetryExecutionState({ network }))
  );
}

async function finalizeImportedCriticalPersistenceSettings(settings) {
  const climate = settings && settings.climate ? settings.climate : null;
  const network = settings && settings.network ? settings.network : null;
  const maintenance = settings && settings.maintenance ? settings.maintenance : null;

  if (climate) {
    setSettingsBackupNote(translate("backup.import_temperature_final"));
    await runTimedImportPhase(() => importSensorCorrectionSettings(climate), 1000);
  }

  if (network) {
    await finalizeImportedNetworkTimeSettings(network);
  }

  if (maintenance) {
    setSettingsBackupNote(translate("backup.import_maintenance_final"));
    await runReloadingImportPhase(() => importMaintenanceSettings(maintenance), 1000);

    await runConditionalImportRetryStages(
      buildMaintenanceRetryStages(buildImportRetryExecutionState({ maintenance }))
    );
  }

  if (climate) {
    await loadData();
    await runConditionalImportRetryStages(
      buildClimateFinalizationRetryStages(buildImportRetryExecutionState({ climate }))
    );
  }
}

function overlayItemsEqual(expectedItems, actualSettings) {
  const state = buildCollectionBackupState(actualSettings);
  return overlayBackupItemsEqual(
    normalizeOverlayBackupItems(expectedItems || [], state.overlayCount),
    buildOverlayBackup(state)
  );
}

// L203: Hier lag der Fehler, und er war der unangenehmen Sorte -- die Nachkontrolle
// MELDETE Uebereinstimmung, wo sie keine geprueft hatte.
//
// Beide Aufrufer uebergeben bereits normalisierte Eintraege: overlayItemsEqual()
// normalisiert vorher selbst (wegen der Zaehlergrenze), und overlaysImportNeedsRetry()
// vergleicht die Sicherungsdatei gegen buildOverlayBackup(), das ebenfalls die
// Normalform liefert. Ein ZWEITER Durchlauf durch normalizeOverlayBackupItems() las
// dann item.flags, item.text und item.date_start -- Felder, die eine normalisierte
// Zeile gar nicht mehr hat. Ergebnis: active, value, month und day fielen auf BEIDEN
// Seiten auf false/""/0 und wurden faktisch nicht verglichen. Blind war die Pruefung
// also ausgerechnet fuer die Felder, die der Vertrag aus Runde 1 streng nimmt.
//
// Die Gattungsunterscheidung bleibt trotzdem noetig: Eine Sicherung aus einer frueheren
// Fassung kann die Rohform tragen. Deshalb wird sie erkannt statt angenommen.
function isRawOverlayBackupItem(item) {
  return !!item && (item.flags !== undefined || item.text !== undefined || item.date_start !== undefined);
}

function toNormalizedOverlayItems(items) {
  const list = items || [];

  if (list.some(isRawOverlayBackupItem)) {
    return normalizeOverlayBackupItems(list);
  }

  // Bereits normalisiert: nur ordnen und die Typen vereinheitlichen. Kein zweiter
  // Durchlauf durch die Rohform-Normalisierung.
  return list
    .slice()
    .sort((a, b) => Number(a.idx) - Number(b.idx))
    .map((item) => ({
      idx: Number(item.idx),
      active: !!item.active,
      type: Number(item.type || 0),
      value: item.value || "",
      interval: Number(item.interval || 0),
      duration: Number(item.duration || 0),
      date_code: Number(item.date_code || 0),
      month: Number(item.month || 0),
      day: Number(item.day || 0),
      days: Number(item.days || 0)
    }));
}

function overlayBackupItemsEqual(expectedItems, actualItems) {
  const expected = toNormalizedOverlayItems(expectedItems);
  const actual = toNormalizedOverlayItems(actualItems);

  return normalizedOverlayItemsEqual(expected, actual);
}

function timerItemsEqual(expectedItems, actualItems) {
  const expected = normalizeTimerBackupItems(expectedItems || []);
  const actual = normalizeTimerBackupItems(actualItems || []);

  return normalizedTimerItemsEqual(expected, actual);
}

function getImportRetryCurrentSections(settings, eepromSettings) {
  return getImportRetryState(settings, eepromSettings).current;
}

function importFieldMismatch(expected, current, keys) {
  return (keys || []).some((key) => String((expected || {})[key] || "") !== String((current || {})[key] || ""));
}

// Ein Feld, das in der Sicherung leer ist, wurde bewusst nicht geschrieben. Es darf
// die Stufe danach auch nicht als "noch nicht angekommen" gelten lassen, sonst läuft
// jeder Durchgang in einen Wiederholungsversuch, der wieder nichts sendet.
function importOptionalFieldMismatch(expected, current, keys) {
  return (keys || []).some((key) => {
    const wanted = String((expected || {})[key] || "");
    return !!wanted && wanted !== String((current || {})[key] || "");
  });
}

function importNumericFieldMismatch(expected, current, keys) {
  return (keys || []).some((key) => Number((expected || {})[key] || 0) !== Number((current || {})[key] || 0));
}

function importBooleanFieldMismatch(expected, current, keys) {
  return (keys || []).some((key) => !!((expected || {})[key]) !== !!((current || {})[key]));
}

function displayImportNeedsRetry(display, settings) {
  if (!display || !settings) {
    return false;
  }

  const current = getImportRetryCurrentSections(settings).display;

  return importBooleanFieldMismatch(display, current, ["power", "use_rgbw", "automatic_brightness", "permanent_it_is"]) ||
    importNumericFieldMismatch(display, current, ["mode", "brightness", "ticker_deceleration"]) ||
    importFieldMismatch(display, current, ["ticker_text"]) ||
    importOptionalFieldMismatch(display, current, ["date_ticker_format"]);
}

function climateImportNeedsRetry(climate, settings) {
  if (!climate || !settings) {
    return false;
  }

  const current = getImportRetryCurrentSections(settings).climate;
  // Enthält die Sicherung weder Ort noch vollständiges Koordinatenpaar, hat der Import
  // die Ortsangabe bewusst stehen lassen — dann gibt es hier nichts zu wiederholen.
  const wantsLocation = !!normalizeImportText(climate.weather_city) ||
    (!!normalizeImportText(climate.weather_lon) && !!normalizeImportText(climate.weather_lat));

  return (wantsLocation && importFieldMismatch(climate, current, ["weather_city", "weather_lon", "weather_lat"])) ||
    importNumericFieldMismatch(climate, current, ["ldr_min", "ldr_max"]);
}

function sensorCorrectionsImportNeedsRetry(climate, settings) {
  if (!climate || !settings) {
    return false;
  }

  const current = getImportRetryCurrentSections(settings).climate;

  return importNumericFieldMismatch(climate, current, ["rtc_temp_correction", "ds18xx_temp_correction"]);
}

function rtcCorrectionImportNeedsRetry(climate, settings) {
  if (!climate || !settings) {
    return false;
  }

  const current = getImportRetryCurrentSections(settings).climate;

  return importNumericFieldMismatch(climate, current, ["rtc_temp_correction"]);
}

function maintenanceImportNeedsRetry(maintenance, settings) {
  if (!maintenance || !settings) {
    return false;
  }

  const current = getImportRetryCurrentSections(settings).maintenance;

  return importOptionalFieldMismatch(maintenance, current, ["update_host", "update_path"]);
}

function buildImportSectionState(settings, eepromSettings) {
  const currentEepromSettings = eepromSettings || getCurrentEepromSettings();
  const groups = buildGroupedBackupSections(settings, currentEepromSettings);

  return {
    settings: groups.state.settings,
    eepromSettings: currentEepromSettings,
    comparable: buildComparableSectionPayload(groups)
  };
}

function buildImportComparisonState(settings, eepromSettings) {
  const sectionState = buildImportSectionState(settings, eepromSettings);

  return {
    current: sectionState.comparable,
    eepromSettings: sectionState.eepromSettings
  };
}

function getImportComparisonMeta(settings, eepromSettings) {
  return buildImportComparisonState(settings, eepromSettings).current;
}

function getImportRetryState(settings, eepromSettings) {
  return buildImportComparisonState(settings, eepromSettings);
}

function overlaysImportNeedsRetry(overlays, settings) {
  if (!overlays || !settings) {
    return false;
  }

  const current = getImportRetryCurrentSections(settings);
  return !overlayBackupItemsEqual(overlays.items || [], current.overlays.items || []);
}

function timersImportNeedsRetry(timers, settings) {
  if (!timers || !settings) {
    return false;
  }

  const current = getImportRetryCurrentSections(settings);

  return !timerItemsEqual(timers.display || [], current.timers.display || []) ||
    !timerItemsEqual(timers.ambilight || [], current.timers.ambilight || []);
}

async function retryImportedSectionsIfNeeded(settings) {
  const retryState = buildImportRetryExecutionState(settings);

  if (!retryState.snapshot) {
    return;
  }

  await runConditionalImportRetryStages(buildSectionRetryStages(retryState));
}

function normalizeFsFileName(name) {
  return String(name || "").split("/").pop();
}

function hasFsFile(files, fileName) {
  const wanted = normalizeFsFileName(fileName);
  return (files || []).some((entry) => normalizeFsFileName(entry && entry.name) === wanted);
}

async function ensureFsFileList(forceRefresh) {
  if (!forceRefresh && getCurrentFsFiles().length) {
    return getCurrentFsFiles();
  }

  const fsList = await settleFetchJson(getFsListUrl(), { files: [] }, 6000);
  return setCurrentFsFilesFromList(fsList);
}

async function ensureUpdateTableInfo(forceRefresh) {
  if (!forceRefresh && getUpdateTableCurrentFile(getCurrentUpdateTableInfo())) {
    return getCurrentUpdateTableInfo();
  }

  setCurrentUpdateTableInfo(await settleFetchJson(getUpdateTableFilesUrl(), {}, 6000));
  return getCurrentUpdateTableInfo();
}

function getLayoutTableFamilyPrefix(fileName) {
  const normalized = normalizeFsFileName(fileName);
  const preferredPrefix = getResolvedAssetMeta(null, null, null).tablesFamilyPrefix;

  if (preferredPrefix && normalized.indexOf(preferredPrefix) === 0) {
    return preferredPrefix;
  }

  if (normalized.indexOf("wc12h-tables-") === 0) {
    return "wc12h-tables-";
  }
  if (normalized.indexOf("wc24h-tables-") === 0) {
    return "wc24h-tables-";
  }
  return "";
}

function getAssetPrefixFromHardwareConfig(config) {
  const wcType = Number(config || 0) & HW.WC_MASK;

  if (wcType === HW.WC_24H) {
    return "wc24h";
  }
  if (wcType === HW.WC_12H) {
    return "wc12h";
  }
  if (wcType === HW.UCLOCK) {
    return "uc";
  }

  return "";
}

function getDisplayLedMask(config) {
  return Number(config || 0) & HW.LED_MASK;
}

function isTftDisplayFromConfig(config) {
  return getDisplayLedMask(config) === HW.LED_TFT_RGB;
}

function getResolvedLayoutColumns(fileName) {
  return getResolvedLayoutMeta(null, null, null).columns
    || (String(fileName || "").indexOf("wc24h-") === 0 ? 18 : 11);
}

function getKnownAssetPrefix(settings, backupAssets, layoutTable) {
  return getResolvedAssetMeta(settings, backupAssets, { current_table: layoutTable || "" }).assetPrefix;
}

function getOverlayAssetFiles(settings, assets) {
  const resolvedAssetMeta = getResolvedAssetMeta(settings, assets || null, null);
  const configuredIconFile = resolvedAssetMeta.targets.icon;
  const configuredWeatherFile = resolvedAssetMeta.targets.weather;
  const prefix = resolvedAssetMeta.assetPrefix;

  if (!prefix && configuredIconFile && configuredWeatherFile) {
    return {
      iconFile: configuredIconFile,
      weatherFile: configuredWeatherFile
    };
  }

  return prefix ? {
    iconFile: prefix + "-icon.txt",
    weatherFile: prefix + "-weather.txt"
  } : {
    iconFile: "",
    weatherFile: ""
  };
}

async function triggerTableRestoreDownload(fileName) {
  await apiFetch(getUpdateDownloadTableBaseUrl() + encodeURIComponent(fileName));
}

async function restoreBackupLayoutTable(assets) {
  const desiredTable = normalizeFsFileName(assets && assets.layout_table);
  if (!desiredTable) {
    return;
  }

  let files = await ensureFsFileList(true);
  let updateTableInfo = await ensureUpdateTableInfo(true);
  const familyPrefix = getLayoutTableFamilyPrefix(desiredTable);

  if (hasFsFile(files, desiredTable) && normalizeFsFileName(getUpdateTableCurrentFile(updateTableInfo)) === desiredTable) {
    return;
  }

  setSettingsBackupNote(translate("backup.restoring_layout_table"));

  if (familyPrefix) {
    const filesToRemove = files
      .map((entry) => normalizeFsFileName(entry && entry.name))
      .filter((name) => name && name.indexOf(familyPrefix) === 0 && name !== desiredTable);

    for (const fileName of filesToRemove) {
      await apiFetch(getFsRemoveBaseUrl() + encodeURIComponent(fileName));
    }
  }

  await triggerTableRestoreDownload(desiredTable);
  await sleep(250);
  files = await ensureFsFileList(true);
  updateTableInfo = await ensureUpdateTableInfo(true);
  clearCurrentLayoutPreview();

  if (!hasFsFile(files, desiredTable) || normalizeFsFileName(getUpdateTableCurrentFile(updateTableInfo)) !== desiredTable) {
    throw new Error("layout-restore-failed");
  }
}

async function restoreBackupOverlayAssets(assets, settings) {
  const assetFiles = getOverlayAssetFiles(settings, assets);
  const usedIcons = Array.isArray(assets && assets.used_icons) ? assets.used_icons.filter(Boolean).map((name) => String(name).trim()) : [];
  if (!assetFiles.iconFile && !usedIcons.length) {
    return;
  }

  let files = await ensureFsFileList(true);
  setOverlayIconsCache(await settleFetchJson(getOverlayIconsUrl(), getOverlayIconsCache() || [], 3000));
  let icons = getOverlayIconsCache().slice();
  const filesOk = assetFiles.iconFile ? (hasFsFile(files, assetFiles.iconFile) && hasFsFile(files, assetFiles.weatherFile)) : true;
  const iconsOk = usedIcons.every((iconName) => icons.indexOf(iconName) >= 0);

  if (filesOk && iconsOk) {
    return;
  }

  setSettingsBackupNote(translate("backup.restoring_assets"));
  const response = await apiFetch(getUpdateDownloadAssetsUrl());
  const result = await response.json().catch(() => ({}));
  if (!result || !result.ok) {
    throw new Error("overlay-assets-download-failed");
  }

  await sleep(250);
  files = await ensureFsFileList(true);
  setOverlayIconsCache(await settleFetchJson(getOverlayIconsUrl(), getOverlayIconsCache() || [], 3000));
  icons = getOverlayIconsCache().slice();

  const restoredFilesOk = assetFiles.iconFile ? (hasFsFile(files, assetFiles.iconFile) && hasFsFile(files, assetFiles.weatherFile)) : true;
  const restoredIconsOk = usedIcons.every((iconName) => icons.indexOf(iconName) >= 0);

  if (!restoredFilesOk || !restoredIconsOk) {
    throw new Error("overlay-assets-restore-failed");
  }
}

async function restoreBackupAssets(assets, settings) {
  setSettingsBackupNote(translate("backup.checking_files"));
  await restoreBackupLayoutTable(assets);
  await restoreBackupOverlayAssets(assets, settings || getCurrentSettingsSnapshot() || {});
}

async function apiFetchQuery(endpoint, params) {
  const queryString = buildQueryString(params);
  return apiFetch(endpoint + (queryString ? "?" + queryString : ""));
}

async function apiFetchValue(endpoint, value) {
  return apiFetchQuery(endpoint, { value });
}

async function apiFetchHourMinute(endpoint, value) {
  const totalMinutes = Number(value || 0);
  return apiFetchQuery(endpoint, {
    hour: Math.floor(totalMinutes / 60),
    minute: totalMinutes % 60
  });
}

async function importIndexedEntries(endpoint, entries, count, fallbackFactory, buildParams) {
  const entryMap = new Map((entries || []).map((entry) => [Number(entry.idx || 0), entry]));

  for (let idx = 0; idx < count; idx += 1) {
    const entry = entryMap.get(idx) || fallbackFactory(idx);
    await apiFetchQuery(endpoint, buildParams(entry, idx));
  }
}

// Ein Feld, das in der Sicherung leer ist oder fehlt, heisst "nicht ändern" — nicht
// "leer schreiben". Die Firmware weist leere Textwerte inzwischen mit
// {"ok":false,"error":1} ab, und apiFetch macht daraus eine Ausnahme: Ein
// bedingungslos gesendetes "" brach die ganze Import-Stufe ab. Ein Gerät ohne
// Wetter-API-Schlüssel verlor so ldr_min und ldr_max, eines ohne Update-Host den
// update_path -- der Import einer gültigen Sicherung schlug also fehl.
const skippedImportFieldKeys = [];

function resetSkippedImportFields() {
  skippedImportFieldKeys.length = 0;
}

function noteSkippedImportField(labelKey) {
  if (labelKey && !skippedImportFieldKeys.includes(labelKey)) {
    skippedImportFieldKeys.push(labelKey);
    console.warn("Import: Feld übersprungen, in der Sicherung leer: " + labelKey);
  }
}

function getSkippedImportFieldsSummary() {
  if (!skippedImportFieldKeys.length) {
    return "";
  }

  return translateFormat("backup.import_skipped_fields", {
    fields: skippedImportFieldKeys.map((key) => translate(key)).join(", ")
  });
}

// Zweite Liste neben den übersprungenen Feldern, bewusst nicht dieselbe: Die beiden
// Meldungen sagen Gegenteiliges. "Übersprungen" heisst "nicht geschrieben, auf der Uhr
// unverändert", "zurechtgebogen" heisst "geschrieben, aber anders als in der Datei".
// In einer gemeinsamen Liste wäre der Satz für eine der beiden Hälften falsch. Die
// Mechanik drumherum — zurücksetzen, Zusammenfassung, Warnfarbe, längere Standzeit —
// bleibt dagegen gemeinsam, damit nur eine Stelle über das Aussehen entscheidet. L65.
const adjustedImportFieldEntries = new Map();

function resetAdjustedImportFields() {
  adjustedImportFieldEntries.clear();
}

function noteAdjustedImportField(labelKey, originalValue, usedValue) {
  if (!labelKey || adjustedImportFieldEntries.has(labelKey)) {
    return;
  }

  adjustedImportFieldEntries.set(labelKey, { original: originalValue, used: usedValue });
  console.warn("Import: Wert aus der Sicherung ausserhalb des gültigen Bereichs: " +
    labelKey + " " + String(originalValue) + " -> " + String(usedValue));
}

function getAdjustedImportFieldsSummary() {
  if (!adjustedImportFieldEntries.size) {
    return "";
  }

  const fields = [...adjustedImportFieldEntries.entries()].map(([key, entry]) => (
    translate(key) + " (" + String(entry.original) + " → " + String(entry.used) + ")"
  ));

  return translateFormat("backup.import_adjusted_fields", { fields: fields.join(", ") });
}

// Vierte Liste in derselben Mechanik: abgebrochene Importstufen (L203). Sie sagt
// wieder etwas anderes als die drei anderen -- nicht "nicht geschrieben, weil leer"
// und nicht "geschrieben, aber anders", sondern "mittendrin abgebrochen, der Rest
// dieser Stufe ist offen". Genau deshalb eine eigene Liste und kein Anhaengen an eine
// bestehende: Der Satz waere sonst fuer eine der Haelften falsch.
const failedImportStageNotes = [];

function resetFailedImportStages() {
  failedImportStageNotes.length = 0;
}

function noteFailedImportStage(note, error) {
  const label = String(note || "").trim() || "?";

  if (!failedImportStageNotes.includes(label)) {
    failedImportStageNotes.push(label);
  }

  // Die Begruendung des Geraets gehoert in die Konsole, nicht in die Sammelmeldung:
  // Dort stuenden bei mehreren Stufen mehrere Fehlertexte hintereinander, und die
  // Zusammenfassung waere nicht mehr lesbar.
  console.warn("Import: Stufe abgebrochen: " + label, error);
}

function getFailedImportStagesSummary() {
  if (!failedImportStageNotes.length) {
    return "";
  }

  return translateFormat("backup.import_stage_failed", {
    stages: failedImportStageNotes.join(", ")
  });
}

// Dritte Notiz neben "übersprungen" und "zurechtgebogen", und anders als die beiden
// keine Liste: Sie gilt für die Datei als Ganzes. Eine ältere Sicherung wird seit L81
// importiert statt abgewiesen -- der Nutzer soll aber erfahren, warum danach möglich-
// erweise Einstellungen unverändert geblieben sind, die die Uhr heute kennt. Das ist
// eine Mitteilung, kein Fehler: Der Import läuft weiter.
let importedBackupFileVersion = 0;

function resetImportedBackupVersionNotice() {
  importedBackupFileVersion = 0;
}

function noteImportedBackupVersion(version) {
  importedBackupFileVersion = Number(version || 0);

  if (importedBackupFileVersion > 0 && importedBackupFileVersion < BACKUP_VERSION) {
    console.warn("Import: Sicherung aus älterer App-Version: " +
      String(importedBackupFileVersion) + " statt " + String(BACKUP_VERSION));
  }
}

function getImportedBackupVersionSummary() {
  if (!(importedBackupFileVersion > 0) || importedBackupFileVersion >= BACKUP_VERSION) {
    return "";
  }

  return translateFormat("backup.import_older_version", {
    version: String(importedBackupFileVersion),
    current: String(BACKUP_VERSION)
  });
}

// Ein Wert aus einer Sicherungsdatei ist kein Formularfeld: Es tippt niemand, und ein
// Abbruch mitten im Import wäre nach L57 der gefährlichere Zustand. Die Stufe läuft
// deshalb weiter und der Wert wird geklammert — aber nicht mehr stillschweigend.
function readClampedImportNumber(value, min, max, labelKey) {
  const number = Number(value || 0);

  // Die 0 war hier bis Runde 1 harmlos, weil jeder betroffene Bereich sie enthielt.
  // Seit der Vertrag Untergrenzen ueber 0 kennt -- overlay duration faengt bei 5 an --
  // waere sie der naechste abgewiesene Wert und damit ein Abbruch der Import-Stufe.
  // Genommen wird deshalb die 0 nur, wenn sie im Bereich liegt, sonst dessen Rand.
  if (!Number.isFinite(number)) {
    const substitute = Math.max(min, Math.min(max, 0));

    noteAdjustedImportField(labelKey, value, substitute);
    return substitute;
  }

  const clamped = Math.max(min, Math.min(max, number));

  if (clamped !== number) {
    noteAdjustedImportField(labelKey, number, clamped);
  }

  return clamped;
}

// Drei Setter haben eine Obergrenze, die erst das Geraet kennt: Anzeigemodus,
// Anzeigeanimation und Farbanimation haengen an den geladenen Tabellen. Eine Sicherung
// von einem Geraet mit mehr Eintraegen traegt deshalb einen Wert, den DIESES Geraet
// seit Runde 1 abweist -- und ein abgewiesener Wert beendet die ganze Import-Stufe,
// samt allem, was in ihr noch folgen sollte. Die Obergrenze kommt aus dem aktuellen
// Abzug; liegt keiner vor, wird nicht geklammert, sondern der Wert unveraendert
// gesendet -- eine erfundene Grenze waere schlechter als die Fehlermeldung des Geraets.
function readImportIndexAgainstDeviceList(value, listName, labelKey) {
  const snapshot = getCurrentSettingsSnapshot();
  const list = snapshot && Array.isArray(snapshot[listName]) ? snapshot[listName] : null;

  if (!list || !list.length) {
    return Number(value || 0);
  }

  return readClampedImportNumber(value, 0, list.length - 1, labelKey);
}

function normalizeImportText(value) {
  return value === undefined || value === null ? "" : String(value).trim();
}

// Schreibt nur, wenn die Sicherung wirklich einen Wert enthält. Der Rückgabewert sagt,
// ob geschrieben wurde; die Stufe läuft in beiden Fällen weiter.
async function importOptionalValue(endpoint, value, labelKey) {
  const text = normalizeImportText(value);

  if (!text) {
    noteSkippedImportField(labelKey);
    return false;
  }

  await apiFetchValue(endpoint, text);
  return true;
}

async function importNetworkSettings(network) {
  if (!network) {
    return;
  }

  await importNetworkTimeSettings(network);

  // Ein Backup ohne SSID wuerde das Geraet ohne WLAN und ohne Accesspoint
  // zuruecklassen -- erreichbar dann nur noch ueber die serielle Schnittstelle.
  // Zeitserver und Zeitzone sind oben bereits importiert, die sind ungefaehrlich.
  if (!network.wifi_ssid) {
    setSettingsBackupNote(translate("backup.network_skipped_empty_ssid"), "warn");
    return;
  }

  const query = new URLSearchParams({
    ssid: network.wifi_ssid || "",
    key: network.wifi_key || "",
    ap_ssid: network.ap_ssid || "",
    ap_key: network.ap_key || "",
    boot_as_ap: network.boot_as_ap ? "on" : "off"
  });
  await apiFetch(getEepromSettingsSetUrl() + "?" + query.toString());
}

async function importNetworkTimeSettings(network) {
  if (!network) {
    return;
  }

  if (await importOptionalValue(getNetworkTimeserverSetUrl(), network.timeserver, "backup.field.timeserver")) {
    await sleep(700);
  }
  await apiFetchValue(getNetworkTimezoneSetUrl(), Number(network.timezone_offset || 0));
  await sleep(300);
  await apiFetchValue(getNetworkSummertimeSetUrl(), network.summertime ? "on" : "off");
  await sleep(300);
}

async function importDisplaySettings(display) {
  if (!display) {
    return;
  }

  await apiFetchValue(getDisplayPowerSetUrl(), display.power ? "on" : "off");
  await sleep(180);
  await apiFetchValue(getDisplayModeSetUrl(), readImportIndexAgainstDeviceList(display.mode, "dispmodes", "backup.field.display_mode"));
  await sleep(180);
  await apiFetchValue(getDisplayUseRgbwSetUrl(), display.use_rgbw ? "on" : "off");
  await sleep(180);
  await apiFetchValue(getAutoBrightnessSetUrl(), display.automatic_brightness ? "on" : "off");
  await sleep(220);
  await apiFetchValue(getDisplayBrightnessSetUrl(), readClampedImportNumber(display.brightness, 0, 15, "backup.field.display_brightness"));
  await sleep(220);
  await apiFetchValue(getDisplayItIsSetUrl(), display.permanent_it_is ? "on" : "off");
  await sleep(220);
  // Der leere Ticker ist ein gültiger Zustand und der einzige Weg, ihn abzuschalten.
  // ticker_set nimmt ihn deshalb bewusst an, und der Import schreibt ihn auch leer.
  await apiFetchValue(getTickerSetUrl(), display.ticker_text || "");
  await sleep(350);
  if (await importOptionalValue(getDateTickerFormatSetUrl(), display.date_ticker_format, "backup.field.date_ticker_format")) {
    await sleep(900);
  }
  await apiFetchValue(getTickerDecelerationSetUrl(), readClampedImportNumber(display.ticker_deceleration, 0, 255, "backup.field.ticker_deceleration"));
  await sleep(1800);
  await saveImportedColor(getDisplayColorSetUrl(), display.color);
  await sleep(250);
  await saveImportedDimCurve(getDisplayDimLevelSetUrl(), display.dim_curve);
  await sleep(500);
}

async function importMaintenanceSettings(maintenance) {
  if (!maintenance) {
    return;
  }

  if (await importOptionalValue(getUpdateHostSetUrl(), maintenance.update_host, "backup.field.update_host")) {
    await sleep(900);
  }
  if (await importOptionalValue(getUpdatePathSetUrl(), maintenance.update_path, "backup.field.update_path")) {
    await sleep(500);
  }
}

async function importClimateSettings(climate) {
  if (!climate) {
    return;
  }

  if (await importOptionalValue(getWeatherAppIdSetUrl(), climate.weather_appid, "backup.field.weather_appid")) {
    await sleep(250);
  }

  await importWeatherLocationSettings(climate);

  await apiFetchValue(getLdrMinValueSetUrl(), readClampedImportNumber(climate.ldr_min, 0, 4095, "backup.field.ldr_min"));
  await sleep(150);
  await apiFetchValue(getLdrMaxValueSetUrl(), readClampedImportNumber(climate.ldr_max, 0, 4095, "backup.field.ldr_max"));
}

// Ort und Koordinaten sind Alternativen, und das Gerät lässt die eine Angabe nur
// leeren, solange die andere steht. Die Reihenfolge entscheidet deshalb: Zuerst wird
// die Angabe geschrieben, die in der Sicherung gefüllt ist, erst danach die andere
// geleert. Andernfalls trifft ein leerer Ort auf ein Gerät ohne Koordinaten, das
// Gerät antwortet mit Kennung 1 — und ldr_min sowie ldr_max fielen aus der Stufe.
async function importWeatherLocationSettings(climate) {
  const city = normalizeImportText(climate.weather_city);
  const lon = normalizeImportText(climate.weather_lon);
  const lat = normalizeImportText(climate.weather_lat);
  const hasCoordinates = !!lon && !!lat;

  if (!city && !hasCoordinates) {
    noteSkippedImportField("backup.field.weather_location");
    return;
  }

  if (city) {
    await apiFetchValue(getWeatherCitySetUrl(), city);
    await sleep(250);
    await apiFetchQuery(getWeatherCoordinatesSetUrl(), {
      lon: hasCoordinates ? lon : "",
      lat: hasCoordinates ? lat : ""
    });
    await sleep(250);
    return;
  }

  await apiFetchQuery(getWeatherCoordinatesSetUrl(), { lon, lat });
  await sleep(250);
  await apiFetchValue(getWeatherCitySetUrl(), "");
  await sleep(250);
}

async function importSensorCorrectionSettings(climate) {
  if (!climate) {
    return;
  }

  await apiFetchValue(getTemperatureRtcCorrectionSetUrl(), readClampedImportNumber(climate.rtc_temp_correction, -20, 20, "backup.field.rtc_temp_correction"));
  await sleep(400);
  await apiFetchValue(getTemperatureDs18xxCorrectionSetUrl(), readClampedImportNumber(climate.ds18xx_temp_correction, -20, 20, "backup.field.ds18xx_temp_correction"));
  await sleep(400);
}

async function importRtcCorrectionSetting(climate) {
  if (!climate) {
    return;
  }

  await apiFetchValue(getTemperatureRtcCorrectionSetUrl(), readClampedImportNumber(climate.rtc_temp_correction, -20, 20, "backup.field.rtc_temp_correction"));
}

async function finalizeTemperatureCorrectionPersistence(climate) {
  if (!climate) {
    return;
  }

  const rtcCorrection = readClampedImportNumber(climate.rtc_temp_correction, -20, 20, "backup.field.rtc_temp_correction");
  const ds18xxCorrection = readClampedImportNumber(climate.ds18xx_temp_correction, -20, 20, "backup.field.ds18xx_temp_correction");
  setSettingsBackupNote(translate("backup.import_sensor_persist_final"));
  await apiFetchValue(getTemperatureRtcCorrectionSetUrl(), rtcCorrection);
  await sleep(500);
  await apiFetchValue(getTemperatureDs18xxCorrectionSetUrl(), ds18xxCorrection);
  await sleep(1800);
  await loadData();
}

async function importAnimationSettings(animations) {
  if (!animations) {
    return;
  }

  await apiFetchValue(getAnimationModeSetUrl(), readImportIndexAgainstDeviceList(animations.display_mode, "dispanims", "backup.field.animation_mode"));
  await apiFetchValue(getColorAnimationModeSetUrl(), readImportIndexAgainstDeviceList(animations.color_mode, "coloranims", "backup.field.color_animation_mode"));

  for (const entry of (animations.display_profiles || [])) {
    await apiFetchQuery(getAnimationProfileSetUrl(), {
      idx: Number(entry.idx || 0),
      // Anders als bei den uebrigen Verzoegerungen faengt dieser Bereich bei 1 an.
      // Ein fehlendes Feld wurde bisher zur 0 und vom Geraet still auf die Vorgabe
      // gezogen; heute waere es ein Abbruch der Animations-Stufe.
      deceleration: readClampedImportNumber(Number(entry.deceleration) || 1, 1, 15, "backup.field.animation_deceleration"),
      favourite: entry.favourite ? "on" : "off"
    });
  }

  for (const entry of (animations.color_profiles || [])) {
    await apiFetchQuery(getColorAnimationProfileSetUrl(), {
      idx: Number(entry.idx || 0),
      deceleration: readClampedImportNumber(entry.deceleration, 0, 15, "backup.field.color_animation_deceleration")
    });
  }
}

async function importTftSettings(tft) {
  if (!tft) {
    return;
  }

  await apiFetchQuery(getTftFlagsSetUrl(), {
    rgb: tft.rgb ? "on" : "off",
    hflip: tft.hflip ? "on" : "off",
    vflip: tft.vflip ? "on" : "off"
  });
}

async function importAmbilightSettings(ambilight) {
  if (!ambilight) {
    return;
  }

  const ambilightOnlineValue = ambilight.online ? "on" : "off";
  await apiFetchValue(getAmbilightOnlineSetUrl(), ambilightOnlineValue);
  setPersistedAmbilightState(ambilightOnlineValue);
  await apiFetchValue(getAmbilightPowerSetUrl(), ambilight.power ? "on" : "off");
  await apiFetchValue(getAmbilightModeSetUrl(), readClampedImportNumber(ambilight.mode, 0, 4, "backup.field.ambilight_mode"));
  await apiFetchValue(getAmbilightLedsSetUrl(), readClampedImportNumber(ambilight.leds, 0, 999, "backup.field.ambilight_leds"));
  await apiFetchValue(getAmbilightOffsetSetUrl(), readClampedImportNumber(ambilight.offset, 0, 999, "backup.field.ambilight_offset"));
  await apiFetchValue(getAmbilightBrightnessSetUrl(), readClampedImportNumber(ambilight.brightness, 0, 15, "backup.field.ambilight_brightness"));
  await saveImportedColor(getAmbilightColorSetUrl(), ambilight.color);
  await saveImportedColor(getMarkerColorSetUrl(), ambilight.marker_color);
  await apiFetchValue(getSyncAmbilightSetUrl(), ambilight.sync_ambilight ? "on" : "off");
  await apiFetchValue(getSyncMarkersSetUrl(), ambilight.sync_markers ? "on" : "off");
  await apiFetchValue(getFadeClockSecondsSetUrl(), ambilight.fade_clock_seconds ? "on" : "off");
  await apiFetchValue(getAmbilightMarkersSetUrl(), ambilight.seconds_markers ? "on" : "off");
  await saveImportedDimCurve(getAmbilightDimLevelSetUrl(), ambilight.dim_curve);

  for (const entry of (ambilight.profiles || [])) {
    await apiFetchQuery(getAmbilightModeProfileSetUrl(), {
      idx: Number(entry.idx || 0),
      deceleration: readClampedImportNumber(entry.deceleration, 0, 15, "backup.field.ambilight_deceleration")
    });
  }
}

async function importDfplayerSettings(dfplayer) {
  if (!dfplayer) {
    return;
  }

  await apiFetchValue(getDfplayerVolumeSetUrl(), readClampedImportNumber(dfplayer.volume, 0, 30, "backup.field.dfplayer_volume"));
  await apiFetchValue(getDfplayerModeSetUrl(), readClampedImportNumber(dfplayer.mode, 0, 2, "backup.field.dfplayer_mode"));
  await apiFetchQuery(getDfplayerBellFlagsSetUrl(), {
    m15: (Number(dfplayer.bell_flags || 0) & 0x01) ? "on" : "off",
    m30: (Number(dfplayer.bell_flags || 0) & 0x02) ? "on" : "off",
    m45: (Number(dfplayer.bell_flags || 0) & 0x04) ? "on" : "off"
  });
  await apiFetchValue(getDfplayerSpeakCycleSetUrl(), readClampedImportNumber(dfplayer.speak_cycle, 0, 255, "backup.field.dfplayer_speak_cycle"));
  await apiFetchHourMinute(getDfplayerSilenceStartSetUrl(), Number(dfplayer.silence_start || 0));
  await apiFetchHourMinute(getDfplayerSilenceStopSetUrl(), Number(dfplayer.silence_stop || 0));

  await importIndexedEntries(
    getDfplayerAlarmSetUrl(),
    dfplayer.alarms || [],
    8,
    (idx) => ({ idx, active: false, from: 0, to: 0, hour: 0, minute: 0 }),
    (entry) => ({
      idx: Number(entry.idx || 0),
      active: entry.active ? "on" : "off",
      from: readClampedImportNumber(entry.from, 0, 6, "backup.field.dfplayer_alarm"),
      to: readClampedImportNumber(entry.to, 0, 6, "backup.field.dfplayer_alarm"),
      hour: readClampedImportNumber(entry.hour, 0, 23, "backup.field.dfplayer_alarm"),
      minute: readClampedImportNumber(entry.minute, 0, 59, "backup.field.dfplayer_alarm")
    })
  );
}

async function importOverlaySettings(overlays) {
  if (!overlays) {
    return;
  }

  const items = Array.isArray(overlays.items) ? overlays.items.slice().sort((a, b) => a.idx - b.idx) : [];
  const failedDeletes = [];

  // Gelöscht wird nur so weit, wie das Gerät tatsächlich belegt ist. Der frühere Lauf
  // über alle 32 Plätze lebte davon, dass overlay_delete für einen unbelegten Platz
  // still {"ok":true} meldete. Seit L70 weist der ESP ungültige Indizes mit Kennung 2
  // ab, apiFetch wirft darauf, und bei drei Overlays hätte der Import 29 Fehlschläge
  // gemeldet, obwohl alles richtig lief.
  //
  // Der stärkere Grund ist aber die Last: Jedes Löschen ist ein Kommando an den STM.
  // 32 am Stück sind rund 8,6 s Hauptloop-Stillstand, bei drei belegten Plätzen bleiben
  // davon drei Kommandos übrig. "Lösche etwas, das es nicht gibt" ist keine sinnvolle
  // Anfrage -- und absteigend bleibt jeder Index gültig, weil der ESP nach dem Löschen
  // nach unten zusammenschiebt.
  const snapshot = getCurrentSettingsSnapshot();
  const knownOverlayCount = snapshot && snapshot.numvars
    ? Number(snapshot.numvars[NUM.OVERLAY_N_OVERLAYS] || 0)
    : 32;

  for (let idx = knownOverlayCount - 1; idx >= 0; idx -= 1) {
    try {
      await apiFetchQuery(getOverlayDeleteUrl(), { idx });
    } catch (error) {
      // Kennung 2 heisst "diesen Platz gibt es nicht". Ist der Zählerstand oben zu
      // hoch gewesen -- etwa ohne frischen Snapshot --, ist das kein Fehlschlag,
      // sondern das erwartete Ende der Liste.
      if (error && error.apiErrorCode === 2) {
        continue;
      }

      // Der Import läuft bewusst weiter, sonst bleibt das Gerät halb geleert zurück.
      // Ein fehlgeschlagenes Löschen ist aber nicht folgenlos: die Schreibschleife
      // unten beschreibt nur die Plätze 0..items.length-1, alles darüber behält
      // seinen alten Inhalt.
      failedDeletes.push(idx);
      console.warn("Overlay " + idx + " could not be cleared, old content may remain", error);
    }
  }

  if (failedDeletes.length) {
    console.warn("Overlay import: " + failedDeletes.length + " slot(s) not cleared", failedDeletes.join(", "));
    setSettingsBackupNote(translateFormat("backup.overlay_clear_failed", { count: failedDeletes.length }), "error");
  }

  await sleep(1500);

  for (let idx = 0; idx < items.length; idx += 1) {
    const entry = items[idx] || {};
    const entryFlags = Number(entry.flags || 0);
    const entryDateStart = Number(entry.date_start || 0);
    const entryValue = entry.value !== undefined ? entry.value : entry.text;
    const rawMonth = Number(entry.month || ((entryDateStart >> 8) & 0xff) || 0);
    const rawDay = Number(entry.day || (entryDateStart & 0xff) || 0);
    const startDate = pairOverlayStartDate(rawMonth, rawDay);

    // Eine Teilangabe wird zum Paar 0/0 ergaenzt statt gesendet: Das Geraet weist sie
    // seit Runde 1 ab, und eine abgewiesene Anfrage beendet den ganzen Overlay-Import.
    // Gewirkt hat sie nie -- aber stillschweigend verschwinden soll sie auch nicht.
    if (startDate.month !== rawMonth || startDate.day !== rawDay) {
      noteAdjustedImportField(
        "backup.field.overlay_start_date",
        String(rawMonth) + "/" + String(rawDay),
        String(startDate.month) + "/" + String(startDate.day)
      );
    }

    await apiFetchQuery(getOverlaySetUrl(), {
      idx,
      active: (entry.active !== undefined ? entry.active : !!(entryFlags & 0x01)) ? "on" : "off",
      type: Number(entry.type || 0),
      value: entryValue || "",
      interval: readOverlayImportParam(entry.interval, OVERLAY_PARAM_RULES.interval),
      duration: readOverlayImportParam(entry.duration, OVERLAY_PARAM_RULES.duration),
      date_code: Number(entry.date_code || 0),
      month: startDate.month,
      day: startDate.day,
      days: readOverlayImportParam(entry.days, OVERLAY_PARAM_RULES.days)
    });
    await sleep(idx === 0 ? 1100 : 700);
  }
  await sleep(1200);
}

async function importTimerSettings(timers) {
  if (!timers) {
    return;
  }

  setSettingsBackupNote(translate("backup.importing_timers"));

  const importTimerGroup = async (endpoint, entries) => {
    const entryMap = new Map((entries || []).map((entry) => [Number(entry.idx || 0), entry]));

    for (let idx = 0; idx < 8; idx += 1) {
      const entry = entryMap.get(idx) || { idx, active: false, switch_on: false, from: 0, to: 0, hour: 0, minute: 0 };
      await apiFetchQuery(endpoint, {
        idx: Number(entry.idx || 0),
        active: entry.active ? "on" : "off",
        switch_on: entry.switch_on ? "on" : "off",
        from: readClampedImportNumber(entry.from, 0, 6, "backup.field.timer"),
        to: readClampedImportNumber(entry.to, 0, 6, "backup.field.timer"),
        hour: readClampedImportNumber(entry.hour, 0, 23, "backup.field.timer"),
        minute: readClampedImportNumber(entry.minute, 0, 59, "backup.field.timer")
      });
      await sleep(idx < 2 ? 420 : 260);
    }
  };

  await importTimerGroup(getTimerSetUrl(), timers.display || []);
  await sleep(700);
  await importTimerGroup(getAmbilightTimerSetUrl(), timers.ambilight || []);
  await sleep(900);
}

async function saveImportedColor(endpoint, color) {
  if (!color) {
    return;
  }

  // Jeder Anteil geht als Byte an den STM, das Geraet nimmt 0..63. Ein groesserer
  // Wert wurde bisher geklemmt und als Erfolg gemeldet; heute weist er die Anfrage ab,
  // und mit ihr die ganze Stufe -- einschliesslich Dimmkurve und allem danach.
  await apiFetchQuery(endpoint, {
    red: readClampedImportNumber(color.red, 0, 63, "backup.field.color"),
    green: readClampedImportNumber(color.green, 0, 63, "backup.field.color"),
    blue: readClampedImportNumber(color.blue, 0, 63, "backup.field.color"),
    white: readClampedImportNumber(color.white, 0, 63, "backup.field.color")
  });
}

async function saveImportedDimCurve(endpoint, values) {
  if (!Array.isArray(values)) {
    return;
  }

  for (let idx = 0; idx < values.length && idx <= 15; idx += 1) {
    await apiFetchQuery(endpoint, { idx, value: readClampedImportNumber(values[idx], 0, 15, "backup.field.dim_curve") });
  }
}

function updateDateTimeControls(settings) {
  updateDateTimeControlsFromMeta(getSettingsControlUiMeta(settings).dateTime);
}

function updateDateTimeControlsFromMeta(meta) {
  document.getElementById("datetime-year-input").value = meta.year;
  document.getElementById("datetime-month-input").value = meta.month;
  document.getElementById("datetime-day-input").value = meta.day;
  document.getElementById("datetime-hour-input").value = meta.hour;
  document.getElementById("datetime-minute-input").value = meta.minute;
  document.getElementById("datetime-preview").textContent = meta.preview;
}

function updateTemperatureControls(settings) {
  updateTemperatureControlsFromMeta(getSettingsControlUiMeta(settings).temperature);
}

function updateTemperatureControlsFromMeta(meta) {
  renderList("temperature-list", meta.items);
  document.getElementById("temperature-ds18xx-correction-input").value = String(meta.ds18xxCorrection);
  document.getElementById("temperature-rtc-correction-input").value = String(meta.rtcCorrection);
}

function updateLdrControls(settings) {
  updateLdrControlsFromMeta(getSettingsControlUiMeta(settings).ldr);
}

function updateLdrControlsFromMeta(meta) {
  renderList("ldr-list", meta.items);
  document.getElementById("ldr-min-button").disabled = !meta.canStoreBounds;
  document.getElementById("ldr-max-button").disabled = !meta.canStoreBounds;
}

function updateAnimationControls(settings) {
  updateAnimationControlsFromMeta(getSettingsControlUiMeta(settings).animation);
}

function updateAnimationControlsFromMeta(meta) {
  const animationSelect = document.getElementById("animation-mode-select");
  const colorAnimationSelect = document.getElementById("color-animation-mode-select");

  animationSelect.innerHTML = meta.displayAnimations.map((entry) => (
    '<option value="' + entry.idx + '">' + escapeHtml(localizeAnimationName(entry.name || String(entry.idx))) + "</option>"
  )).join("");
  animationSelect.value = String(meta.displayAnimationMode);

  colorAnimationSelect.innerHTML = meta.colorAnimations.map((entry) => (
    '<option value="' + entry.idx + '">' + escapeHtml(localizeAnimationName(entry.name || String(entry.idx))) + "</option>"
  )).join("");
  colorAnimationSelect.value = String(meta.colorAnimationMode);
}

function updateTftControls(settings) {
  updateTftControlsFromMeta(getSettingsControlUiMeta(settings).tft);
}

function updateTftControlsFromMeta(meta) {
  document.getElementById("tft-rgb-checkbox").checked = meta.rgb;
  document.getElementById("tft-hflip-checkbox").checked = meta.hflip;
  document.getElementById("tft-vflip-checkbox").checked = meta.vflip;
}

function updateTftVisibility(settings, debugOverrides) {
  document.getElementById("tft-panel").classList.toggle("is-hidden", !getFeatureUiMeta(settings, debugOverrides).hasTft);
}

function renderPreviewDebug(settings) {
  const previewMeta = getPreviewUiMeta(settings);

  renderList("preview-debug-list", [
    [translate("preview.color_animation"), previewMeta.animationLabel],
    [translate("preview.persisted_display_color"), formatRgbwColor(previewMeta.staticColor)],
    [translate("preview.live_device_color"), formatRgbwColor(currentLiveDisplayColor)],
    [translate("preview.layout_file"), previewMeta.layoutFile]
  ]);
}

function updateAmbilightBrightnessControl(value) {
  const slider = document.getElementById("ambilight-brightness-slider");
  slider.value = value;
  syncAmbilightBrightnessLabel();
}

function updateAmbilightModeControl(settings) {
  updateAmbilightModeControlFromMeta(getAmbilightUiMeta(settings));
}

function updateAmbilightModeControlFromMeta(meta) {
  const select = document.getElementById("ambilight-mode-select");
  select.innerHTML = meta.modes.map((mode) => (
    '<option value="' + mode.idx + '">' + escapeHtml(localizeAmbilightModeName(mode.name || String(mode.idx))) + "</option>"
  )).join("");
  select.value = String(meta.currentMode);
}

function updateAmbilightNumberControls(settings) {
  updateAmbilightNumberControlsFromMeta(getAmbilightUiMeta(settings));
}

function updateAmbilightNumberControlsFromMeta(meta) {
  document.getElementById("ambilight-leds-input").value = String(meta.leds);
  document.getElementById("ambilight-offset-input").value = String(meta.offset);
}

function updateColorControls(settings, ambilightOnline, debugOverrides) {
  const colorMeta = getColorUiMeta(settings, ambilightOnline, debugOverrides);
  const capabilities = colorMeta.capabilities;
  const useRgbw = colorMeta.useRgbw;
  const colorAnimationMode = colorMeta.colorAnimationMode;
  const persistedLiveColor = colorMeta.persistedLiveColor;
  const displayColor = colorMeta.displayColor;
  const ambilightColor = colorMeta.ambilightColor;
  const markerColor = colorMeta.markerColor;

  if (!currentLiveDisplayColor && persistedLiveColor) {
    currentLiveDisplayColor = persistedLiveColor;
  }

  applyColorCapabilities(capabilities, useRgbw, ambilightOnline, colorAnimationMode);
  setColorControl("display", displayColor, useRgbw, false);
  setColorControl("ambilight", ambilightColor, useRgbw);
  setColorControl("marker", markerColor, useRgbw);
  if (colorAnimationMode === 0) {
    applyWordclockTheme(displayColor, useRgbw, colorAnimationMode);
  } else if (colorAnimationMode === 1) {
    applyWordclockTheme(currentLiveDisplayColor || RAINBOW_PREVIEW_COLOR, useRgbw, colorAnimationMode);
  } else if (colorAnimationMode === 2) {
    applyWordclockTheme(currentLiveDisplayColor || getDaylightPreviewColor(settings), useRgbw, colorAnimationMode);
  } else if (currentLiveDisplayColor) {
    applyWordclockTheme(currentLiveDisplayColor, useRgbw, colorAnimationMode);
  }
  syncLiveDisplayColorPolling(settings);
}

function applyColorCapabilities(capabilities, useRgbw, ambilightOnline, colorAnimationMode) {
  const note = document.getElementById("color-capability-note");
  const displayColorNote = document.getElementById("display-color-note");
  const canEditDisplayColor = capabilities.hasColor && colorAnimationMode === 0;

  note.textContent = capabilities.whiteChannel && !useRgbw
    ? translate("display.white_channel_inactive")
    : capabilities.note;
  if (displayColorNote) {
    displayColorNote.textContent = canEditDisplayColor
      ? translate("display.color_direct_available")
      : translate("display.color_direct_unavailable");
  }

  document.getElementById("display-color-card").classList.toggle("is-hidden", !canEditDisplayColor);
  document.getElementById("display-color-white-field").classList.toggle("is-hidden", !useRgbw);

  // Der Rueckweg fuer DISPLAY_USE_RGBW (L215). Er haengt bewusst NUR an
  // capabilities.whiteChannel und nicht an useRgbw: Steht das Flag auf 0, blendet
  // isRgbwUiActive() jeden Weisskanal-Regler aus -- genau dann wird dieser Schalter
  // gebraucht, und genau dann darf er nicht mitverschwinden. Er sitzt im Modul
  // "display", nicht im Ambilight-Block, weil updateAmbilightAvailability() dort alles
  // ausblendet, sobald kein Ambilight online ist (an dieser Uhr dauerhaft, L11).
  const useRgbwActions = document.getElementById("display-use-rgbw-actions");
  if (useRgbwActions) {
    useRgbwActions.classList.toggle("is-hidden", !capabilities.whiteChannel);
  }
  setActionToggleButton("display-use-rgbw-button", translate("display.use_rgbw_disable"), translate("display.use_rgbw"), useRgbw, capabilities.whiteChannel);

  document.getElementById("ambilight-color-card").classList.toggle("is-hidden", !capabilities.hasColor || !ambilightOnline);
  document.getElementById("marker-color-card").classList.toggle("is-hidden", !capabilities.hasColor || !ambilightOnline);
  document.getElementById("ambilight-color-white-field").classList.toggle("is-hidden", !useRgbw || !ambilightOnline);
  document.getElementById("marker-color-white-field").classList.toggle("is-hidden", !useRgbw || !ambilightOnline);

  document.getElementById("color-flag-actions").classList.toggle("is-hidden", !capabilities.hasColor || !ambilightOnline);
}

function updateDfplayerControls(settings, debugOverrides) {
  updateDfplayerControlsFromMeta(getDfplayerControlUiMeta(settings, debugOverrides));
}

function updateDfplayerControlsFromMeta(meta) {
  const isUp = meta.online;
  const note = document.getElementById("dfplayer-note");

  document.getElementById("dfplayer-panel").classList.toggle("is-hidden", !isUp);
  note.textContent = isUp ? translate("dfplayer.note_online") : translate("dfplayer.note_offline");

  if (!isUp) {
    return;
  }

  document.getElementById("dfplayer-volume-slider").value = meta.volume;
  syncDfplayerVolumeLabel();
  document.getElementById("dfplayer-mode-select").value = String(meta.mode);
  document.getElementById("dfplayer-bell-15").checked = meta.bell15;
  document.getElementById("dfplayer-bell-30").checked = meta.bell30;
  document.getElementById("dfplayer-bell-45").checked = meta.bell45;
  document.getElementById("dfplayer-speak-cycle-input").value = String(meta.speakCycle);
  document.getElementById("dfplayer-silence-start-input").value = meta.silenceStart;
  document.getElementById("dfplayer-silence-stop-input").value = meta.silenceStop;
  document.getElementById("dfplayer-bell-section").classList.toggle("is-hidden", meta.mode !== 1);
  document.getElementById("dfplayer-speak-section").classList.toggle("is-hidden", meta.mode !== 2);
}

function renderDfplayerAlarmRows(settings) {
  renderDfplayerAlarmRowsFromMeta(getDfplayerUiMeta(settings).alarms);
}

function renderDfplayerAlarmRowsFromMeta(alarms) {
  const root = document.getElementById("dfplayer-alarm-list");

  root.innerHTML = alarms.map((alarm) => {
    const time = minutesToTimeValue(alarm.minutes || 0);
    const fromDay = (alarm.flags & 0x38) >> 3;
    const toDay = alarm.flags & 0x07;
    const active = (alarm.flags & 0x80) ? "checked" : "";
    const idx = alarm.idx;

    return (
      '<section class="alarm-card">' +
        '<div class="card-headline"><div><span class="label">' + escapeHtml(translate("dfplayer.alarm_title")) + ' ' + escapeHtml(String(idx + 1).padStart(3, "0")) + '</span><p class="card-subline">' + escapeHtml(translate("dfplayer.alarm_subline")) + '</p></div></div>' +
        '<div class="chip-row">' +
          '<label class="chip-toggle"><input type="checkbox" id="df-alarm-active-' + idx + '" ' + active + '> ' + escapeHtml(translate("common.active")) + '</label>' +
        '</div>' +
        '<div class="form-section">' +
          '<p class="section-label">' + escapeHtml(translate("timers.period")) + '</p>' +
          '<div class="timer-fields-grid">' +
            '<label class="field"><span class="label">' + escapeHtml(translate("dfplayer.from")) + '</span><select id="df-alarm-from-' + idx + '">' + buildWeekdayOptions(fromDay) + "</select></label>" +
            '<label class="field"><span class="label">' + escapeHtml(translate("dfplayer.to")) + '</span><select id="df-alarm-to-' + idx + '">' + buildWeekdayOptions(toDay) + "</select></label>" +
            '<label class="field"><span class="label">' + escapeHtml(translate("dfplayer.time")) + '</span><input id="df-alarm-time-' + idx + '" type="time" value="' + escapeHtml(time) + '"></label>' +
          '</div>' +
        '</div>' +
        '<div class="profile-actions">' +
          '<button class="button" type="button" data-alarm-save="' + idx + '">' + escapeHtml(translate("common.save")) + '</button>' +
        '</div>' +
      "</section>"
    );
  }).join("");

  bindDataAction(root, "data-alarm-save", saveDfplayerAlarm);
}

function renderAnimationProfiles(settings) {
  renderAnimationProfilesFromMeta(getAnimationProfileUiMeta(settings).items);
}

function renderAnimationProfilesFromMeta(items) {
  const root = document.getElementById("animation-profile-list");

  root.innerHTML = items.map((item) => (
    '<section class="profile-card">' +
      '<div class="panel-head compact"><div><span class="label">' + escapeHtml(item.name || String(item.idx)) + "</span></div></div>" +
      '<div class="control-stack">' +
        '<div class="slider-row"><label class="label" for="an-dec-' + item.idx + '">' + escapeHtml(translate("animations.delay")) + '</label><strong id="an-dec-value-' + item.idx + '" class="value-pill">' + escapeHtml(String(item.deceleration || 1)) + '</strong></div>' +
        '<input id="an-dec-' + item.idx + '" type="range" min="1" max="15" value="' + escapeHtml(String(item.deceleration || 1)) + '">' +
        '<label class="checkbox-line"><input type="checkbox" id="an-fav-' + item.idx + '"' + ((item.flags & 0x02) ? " checked" : "") + '> ' + escapeHtml(translate("animations.favorite")) + '</label>' +
        '<div class="profile-actions">' +
          '<button class="button" type="button" data-an-default="' + item.idx + '">' + escapeHtml(translate("animations.default")) + '</button>' +
          '<button class="button" type="button" data-an-save="' + item.idx + '">' + escapeHtml(translate("animations.profile_save")) + '</button>' +
        "</div>" +
      "</div>" +
    "</section>"
  )).join("");

  bindDataAction(root, "data-an-save", saveAnimationProfile);
  bindDataAction(root, "data-an-default", resetAnimationProfileDefault);
  bindIndexedSuffixAction(root, 'input[id^="an-dec-"]', "input", (idx) => syncProfileRangeValue("an", idx));
}

function renderColorAnimationProfiles(settings) {
  renderColorAnimationProfilesFromMeta(getColorAnimationProfileUiMeta(settings).items);
}

function renderColorAnimationProfilesFromMeta(items) {
  const root = document.getElementById("color-animation-profile-list");

  root.innerHTML = items.map((item) => (
    '<section class="profile-card">' +
      '<div class="panel-head compact"><div><span class="label">' + escapeHtml(item.name || String(item.idx)) + "</span></div></div>" +
      '<div class="control-stack">' +
        '<div class="slider-row"><label class="label" for="can-dec-' + item.idx + '">' + escapeHtml(translate("animations.delay")) + '</label><strong id="can-dec-value-' + item.idx + '" class="value-pill">' + escapeHtml(String(item.deceleration || 0)) + '</strong></div>' +
        '<input id="can-dec-' + item.idx + '" type="range" min="0" max="15" value="' + escapeHtml(String(item.deceleration || 0)) + '">' +
        '<div class="profile-actions">' +
          '<button class="button" type="button" data-can-default="' + item.idx + '">' + escapeHtml(translate("animations.default")) + '</button>' +
          '<button class="button" type="button" data-can-save="' + item.idx + '">' + escapeHtml(translate("animations.profile_save")) + '</button>' +
        "</div>" +
      "</div>" +
    "</section>"
  )).join("");

  bindDataAction(root, "data-can-save", saveColorAnimationProfile);
  bindDataAction(root, "data-can-default", resetColorAnimationProfileDefault);
  bindIndexedSuffixAction(root, 'input[id^="can-dec-"]', "input", (idx) => syncProfileRangeValue("can", idx));
}

function syncProfileRangeValue(prefix, idx) {
  const input = document.getElementById(prefix + "-dec-" + idx);
  const value = document.getElementById(prefix + "-dec-value-" + idx);
  if (input && value) {
    value.textContent = input.value;
  }
}

function renderAmbilightModeProfiles(settings) {
  renderAmbilightModeProfilesFromMeta(getAmbilightProfileUiMeta(settings).items);
}

function renderAmbilightModeProfilesFromMeta(items) {
  const root = document.getElementById("ambilight-profile-list");

  root.innerHTML = items.length ? items.map((item) => (
    '<section class="alarm-card">' +
      '<div class="panel-head compact"><div><span class="label">' + escapeHtml(localizeAmbilightModeName(item.name || String(item.idx))) + "</span></div></div>" +
      '<div class="alarm-grid">' +
        '<label class="field"><span class="label">' + escapeHtml(translate("animations.delay")) + '</span><input id="alm-dec-' + item.idx + '" type="range" min="0" max="15" value="' + escapeHtml(String(item.deceleration || 0)) + '"></label>' +
        '<div class="hero-actions">' +
          '<button class="button" type="button" data-alm-default="' + item.idx + '">' + escapeHtml(translate("animations.default")) + '</button>' +
          '<button class="button" type="button" data-alm-save="' + item.idx + '">' + escapeHtml(translate("animations.profile_save")) + '</button>' +
        "</div>" +
      "</div>" +
    "</section>"
  )).join("") : '<p class="hint">' + escapeHtml(translate("display.ambilight_modes_unavailable")) + '</p>';

  bindDataAction(root, "data-alm-save", saveAmbilightModeProfile);
  bindDataAction(root, "data-alm-default", resetAmbilightModeProfile);
}

function renderFileSystem(fsInfo, files, settings) {
  const meta = getMaintenanceUiMeta(settings, getCurrentEepromSettings(), fsInfo, files);
  renderList("fs-info-list", meta.fsInfoItems);

  const root = document.getElementById("fs-file-list");
  root.innerHTML = meta.files.length ? meta.files.map((file) => (
    '<section class="file-row">' +
      '<div class="file-row-head">' +
        '<div class="file-name">' + escapeHtml(file.name || "-") + '</div>' +
        '<strong class="file-size">' + escapeHtml(formatBytes(file.size ?? 0)) + '</strong>' +
      '</div>' +
      '<div class="file-actions">' +
          '<button class="button" type="button" data-fs-show="' + escapeHtml(file.name || "") + '">' + escapeHtml(translate("common.display")) + '</button>' +
          '<button class="button" type="button" data-fs-delete="' + escapeHtml(file.name || "") + '">' + escapeHtml(translate("common.delete")) + '</button>' +
      "</div>" +
    "</section>"
  )).join("") : '<p class="hint">' + escapeHtml(translate("maintenance.no_files")) + '</p>';

  bindQueryAll(root, "[data-fs-show]", "click", (button) => showFsFile(button.getAttribute("data-fs-show") || ""));
  bindQueryAll(root, "[data-fs-delete]", "click", (button) => deleteFsFile(button.getAttribute("data-fs-delete") || ""));

  updateFsUploadTargets(settings);
}

function formatBytes(bytes) {
  const value = Number(bytes || 0);
  if (value >= 1000 * 1000) {
    return (value / (1000 * 1000)).toFixed(value >= 10 * 1000 * 1000 ? 0 : 1) + " MB";
  }
  if (value >= 1000) {
    return (value / 1000).toFixed(value >= 10 * 1000 ? 0 : 1) + " kB";
  }
  return String(value) + " Bytes";
}

function updateFsUploadTargets(settings) {
  const uploadMeta = getFsUploadMeta(settings);
  const targets = uploadMeta.targets;

  updateUploadFormActions();
  updateUploadFormVisibility("fs-upload-icon-form", "fs-upload-icon-label", targets.icon);
  updateUploadFormVisibility("fs-upload-weather-form", "fs-upload-weather-label", targets.weather);
  updateUploadFormVisibility("fs-upload-tables-form", "fs-upload-tables-label", targets.tables);
  updateUploadFormVisibility("fs-upload-display-form", "fs-upload-display-label", targets.display);
  setUploadFormSupported("fs-upload-icon-form", uploadMeta.targetUploadsSupported, translate("maintenance.target_uploads_unsupported"));
  setUploadFormSupported("fs-upload-weather-form", uploadMeta.targetUploadsSupported, translate("maintenance.target_uploads_unsupported"));
  setUploadFormSupported("fs-upload-tables-form", uploadMeta.targetUploadsSupported, translate("maintenance.target_uploads_unsupported"));
  setUploadFormSupported("fs-upload-display-form", uploadMeta.targetUploadsSupported, translate("maintenance.target_uploads_unsupported"));
}

function updateUploadFormVisibility(formId, labelId, fileName) {
  const form = document.getElementById(formId);
  const label = document.getElementById(labelId);
  form.classList.toggle("is-hidden", !fileName);
  if (fileName) {
    if (formId === "fs-upload-display-form") {
      label.textContent = translate("maintenance.special_display_target") + ": " + fileName;
    } else {
      label.textContent = fileName;
    }
  }
}

function updateUploadFormActions() {
  const actions = [
    ["fs-upload-icon-form", getFsUploadUrl("icon")],
    ["fs-upload-weather-form", getFsUploadUrl("weather")],
    ["fs-upload-tables-form", getFsUploadUrl("tables")],
    ["fs-upload-display-form", getFsUploadUrl("display")]
  ];

  actions.forEach(([formId, action]) => {
    const form = document.getElementById(formId);

    if (!form || !action) {
      return;
    }

    form.setAttribute("action", action);
  });

  updateUploadInputAccepts();
}

function updateUploadInputAccepts() {
  const acceptPairs = [
    ["#fs-upload-icon-form input[type=\"file\"]", getFsUploadTxtAccept()],
    ["#fs-upload-weather-form input[type=\"file\"]", getFsUploadTxtAccept()],
    ["#fs-upload-tables-form input[type=\"file\"]", getFsUploadTxtAccept()],
    ["#fs-upload-display-form input[type=\"file\"]", getFsUploadTxtAccept()],
    ["#local-update-esp-file-input", getLocalEspUpdateAccept()],
    ["#local-update-stm32-file-input", getLocalStm32UploadAccept()]
  ];

  acceptPairs.forEach(([selector, accept]) => {
    const input = document.querySelector(selector);
    if (input && accept) {
      input.setAttribute("accept", accept);
    }
  });
}

function setUploadFormSupported(formId, supported, unsupportedMessage) {
  const form = document.getElementById(formId);

  if (!form) {
    return;
  }

  form.dataset.unsupportedMessage = unsupportedMessage || "";
  form.querySelectorAll("input, button").forEach((element) => {
    element.disabled = !supported;
  });
}

// Release Notes kommen als HTML vom konfigurierbaren Update-Host über HTTP und
// gelten damit als nicht vertrauenswürdig. Sie werden bewusst nicht escaped,
// sondern über eine Whitelist gefiltert und anschliessend aus frisch erzeugten
// DOM-Knoten neu aufgebaut. Kein innerHTML mit Fremdinhalt.
const RELEASE_NOTES_ALLOWED_TAGS = [
  "H1", "H2", "H3", "H4", "H5", "H6",
  "P", "BR", "HR", "DIV", "SPAN",
  "UL", "OL", "LI", "DL", "DT", "DD",
  "B", "STRONG", "I", "EM", "U", "SMALL", "SUB", "SUP",
  "CODE", "PRE", "BLOCKQUOTE",
  "TABLE", "THEAD", "TBODY", "TR", "TH", "TD",
  "A"
];

const RELEASE_NOTES_ALLOWED_LINK_SCHEMES = ["http:", "https:", "mailto:"];
const RELEASE_NOTES_MAX_DEPTH = 24;

function getSafeReleaseNotesLink(href) {
  try {
    const parsed = new URL(String(href || ""), window.location.origin);
    if (RELEASE_NOTES_ALLOWED_LINK_SCHEMES.indexOf(parsed.protocol) < 0) {
      console.warn("Release notes: link dropped, scheme not allowed", parsed.protocol);
      return "";
    }
    return parsed.href;
  } catch (error) {
    console.warn("Release notes: link dropped, URL not parsable", href, error);
    return "";
  }
}

function sanitizeReleaseNotesChildren(source, target, depth, dropped) {
  const nodes = Array.prototype.slice.call(source.childNodes);

  for (let idx = 0; idx < nodes.length; idx += 1) {
    const node = nodes[idx];

    if (node.nodeType === 3) {
      target.appendChild(document.createTextNode(node.nodeValue || ""));
      continue;
    }

    if (node.nodeType !== 1) {
      continue;
    }

    const tag = String(node.tagName || "").toUpperCase();

    if (depth >= RELEASE_NOTES_MAX_DEPTH || RELEASE_NOTES_ALLOWED_TAGS.indexOf(tag) < 0) {
      dropped.push(tag || "?");
      continue;
    }

    const element = document.createElement(tag);

    if (tag === "A") {
      const href = getSafeReleaseNotesLink(node.getAttribute("href"));
      if (href) {
        element.setAttribute("href", href);
        element.setAttribute("target", "_blank");
        element.setAttribute("rel", "noopener noreferrer");
      }
    }

    sanitizeReleaseNotesChildren(node, element, depth + 1, dropped);
    target.appendChild(element);
  }
}

function buildSanitizedReleaseNotes(html) {
  const fragment = document.createDocumentFragment();
  let parsed = null;

  try {
    parsed = new DOMParser().parseFromString(String(html), "text/html");
  } catch (error) {
    console.warn("Release notes: HTML not parsable, falling back to plain text", error);
  }

  if (!parsed || !parsed.body) {
    fragment.appendChild(document.createTextNode(String(html)));
    return fragment;
  }

  const dropped = [];
  sanitizeReleaseNotesChildren(parsed.body, fragment, 0, dropped);

  if (dropped.length) {
    console.warn("Release notes: " + dropped.length + " element(s) removed by whitelist", dropped.join(", "));
  }

  return fragment;
}

function renderReleaseNotes(host, html) {
  if (!host) {
    return;
  }

  host.textContent = "";

  const raw = typeof html === "string" ? html.trim() : "";
  const fragment = raw ? buildSanitizedReleaseNotes(raw) : null;

  if (fragment && fragment.childNodes.length && (fragment.textContent || "").trim()) {
    host.appendChild(fragment);
    return;
  }

  const placeholder = document.createElement("p");
  placeholder.textContent = translate("maintenance.no_release_notes");
  host.appendChild(placeholder);
}

function updateUpdateStatus(updateStatus, updateTableInfo, settings) {
  const updateMeta = getUpdateModuleMeta(updateStatus, updateTableInfo, settings);
  const summary = updateMeta.summary;
  const serverFilesMeta = updateMeta.serverFiles;
  const view = getUpdateUiViewMeta(updateMeta);

  renderList("update-status-list", summary.items);

  const select = document.getElementById("update-stm32-select");
  select.innerHTML = view.stm32Files.length
    ? view.stm32Files.map((file) => '<option value="' + escapeHtml(file) + '"' + (file === view.stm32Default ? " selected" : "") + ">" + escapeHtml(file) + "</option>").join("")
    : '<option value="">' + escapeHtml(translate("maintenance.no_stm32_files")) + '</option>';

  const tableField = document.getElementById("update-table-field");
  const tableSelect = document.getElementById("update-table-select");
  const tableButton = document.getElementById("update-table-button");
  const assetsButton = document.getElementById("update-assets-button");
  const appFilesButton = document.getElementById("update-app-files-button");
  const serverFilesBlock = document.getElementById("update-server-files-block");
  const tableFiles = serverFilesMeta.tableFiles;
  const currentTable = serverFilesMeta.currentTable;

  tableField.classList.toggle("is-hidden", !serverFilesMeta.tableAvailable);
  tableSelect.innerHTML = tableFiles.length
    ? tableFiles.map((file) => '<option value="' + escapeHtml(file) + '"' + (file === currentTable ? " selected" : "") + ">" + escapeHtml(file) + "</option>").join("")
    : '<option value="">' + escapeHtml(translate("maintenance.no_layout_tables")) + '</option>';
  tableButton.classList.toggle("is-hidden", !serverFilesMeta.tableActionSupported);
  assetsButton.classList.toggle("is-hidden", !serverFilesMeta.assetsActionSupported);
  appFilesButton.classList.toggle("is-hidden", !serverFilesMeta.appFilesActionSupported);
  serverFilesBlock.classList.toggle("is-hidden", !serverFilesMeta.anyActionSupported);

  renderReleaseNotes(document.getElementById("update-release-notes"), view.releaseNotes);
  document.getElementById("update-esp-button").disabled = !view.canUpdate;
  document.getElementById("update-stm32-button").disabled = !view.stm32Files.length;
  tableButton.disabled = !serverFilesMeta.tableAvailable || !serverFilesMeta.tableActionSupported;
  assetsButton.disabled = !serverFilesMeta.assetsAvailable;
  appFilesButton.disabled = !serverFilesMeta.appFilesAvailable;

}

function updateLocalUpdateControls(updateStatus) {
  const meta = getUpdateModuleMeta(updateStatus).localUpdate;
  const note = document.getElementById("local-update-note");
  note.textContent = !meta.supported
    ? translate("maintenance.local_update_unavailable")
    : (!meta.localEspSupported || !meta.localStm32Supported)
      ? translate("maintenance.local_update_partial_support")
      : translate("maintenance.local_update_select_file");
  document.getElementById("local-update-esp-file-input").disabled = !meta.supported || !meta.localEspSupported;
  document.getElementById("local-update-esp-submit-button").disabled = !meta.supported || !meta.localEspSupported;
  document.getElementById("local-update-stm32-file-input").disabled = !meta.supported || !meta.localStm32Supported;
  document.getElementById("local-update-stm32-submit-button").disabled = !meta.supported || !meta.localStm32Supported;
}

function refreshUpdateUi(settings, coreData, debugOverrides) {
  const meta = getCurrentUpdateUiMeta();

  updateUploadFormActions();
  renderOverview(settings, coreData.displayPower, coreData.ambilightPower, debugOverrides, meta.status);
  updateUpdateStatus(meta.status, meta.tableInfo, settings);
  updateLocalUpdateControls(meta.status);
}

// network_scan liefert dasselbe Netz mehrfach, einmal je Accesspoint und Kanal — im
// Testdurchlauf acht Einträge für fünf Netze. Doppelte Zeilen in der Auswahlliste
// helfen niemandem. Der Endpunkt liefert inzwischen {"ssid":"…","rssi":-67}, damit
// gewinnt der stärkste Eintrag. Der blosse Name wird weiterhin angenommen: Eine
// ältere ESP-Firmware darf die Netzauswahl nicht leer lassen; ohne Feldstärke bleibt
// es beim ersten Treffer. L34c, L41.
function dedupeScannedNetworks(list) {
  const strongest = new Map();

  (Array.isArray(list) ? list : []).forEach((entry) => {
    const ssid = typeof entry === "string" ? entry : String((entry && entry.ssid) || "");
    if (!ssid) {
      return;
    }

    const rawRssi = entry && typeof entry === "object" ? Number(entry.rssi) : NaN;
    const rssi = Number.isFinite(rawRssi) ? rawRssi : null;
    const known = strongest.get(ssid);

    if (!known || (rssi !== null && (known.rssi === null || rssi > known.rssi))) {
      strongest.set(ssid, { ssid, rssi });
    }
  });

  return Array.from(strongest.values()).map((entry) => entry.ssid);
}

function getNetworkUiMeta(settings, networkInfo, eepromSettings) {
  const timezone = decodeTimezone(settings.numvars[NUM.TIMEZONE] || 0);
  // Der AP-Name steht im EEPROM und wird von /api/eeprom_settings geliefert. Ohne ihn
  // behauptet die Oberflaeche, es sei nichts hinterlegt. L34a.
  const eeprom = eepromSettings || getCurrentEepromSettings();
  return {
    networks: dedupeScannedNetworks(networkInfo && networkInfo.networks),
    apSsid: eeprom && eeprom.ap_ssid ? String(eeprom.ap_ssid) : "",
    currentSsid: networkInfo && networkInfo.ssid ? networkInfo.ssid : "",
    ip: networkInfo && networkInfo.ip ? networkInfo.ip : "",
    mode: networkInfo && networkInfo.mode ? networkInfo.mode : "",
    timeserver: settings.strvars[STR.TIMESERVER] || "",
    timezoneOffset: timezone.offset,
    summertime: timezone.summertime
  };
}

function getDisplayUiMeta(settings) {
  return {
    itIsActive: !!((settings.numvars[NUM.DISPLAY_FLAGS] || 0) & 0x01),
    currentDisplayMode: settings.numvars[NUM.DISPLAY_MODE] || 0,
    displayModes: settings.dispmodes.length ? settings.dispmodes : [
      { idx: 0, name: "Normal" },
      { idx: 1, name: "Sekunden" },
      { idx: 2, name: "Datum" },
      { idx: 3, name: "Temperatur" },
      { idx: 4, name: "Ticker" }
    ],
    tickerText: settings.strvars[STR.TICKER_TEXT] || "",
    dateFormat: settings.strvars[STR.DATE_TICKER_FORMAT] || "",
    tickerDeceleration: settings.numvars[NUM.TICKER_DECELERATION] || 0
  };
}

function getWeatherUiMeta(settings) {
  const city = settings.strvars[STR.WEATHER_CITY] || "";
  const lon = settings.strvars[STR.WEATHER_LON] || "";
  const lat = settings.strvars[STR.WEATHER_LAT] || "";
  const parts = [];

  if (city) {
    parts.push(city);
  }
  if (lon || lat) {
    parts.push((lon || "-") + " / " + (lat || "-"));
  }

  return {
    appId: settings.strvars[STR.WEATHER_APPID] || "",
    city,
    lon,
    lat,
    locationPreview: parts.length
      ? translate("weather.current_location_prefix") + " " + parts.join(" | ")
      : translate("climate.location_ready")
  };
}

function getMaintenanceUiMeta(settings, eepromSettings, fsInfo, files) {
  const fsInfoItems = [];

  if (fsInfo && fsInfo.total !== undefined) {
    fsInfoItems.push(
      [translate("maintenance.fs_total"), formatBytes(fsInfo.total)],
      [translate("maintenance.fs_used"), formatBytes(fsInfo.used)],
      [translate("maintenance.fs_block_size"), formatBytes(fsInfo.block_size)],
      [translate("maintenance.fs_page_size"), formatBytes(fsInfo.page_size)],
      [translate("maintenance.fs_max_open_files"), String(fsInfo.max_open_files)],
      [translate("maintenance.fs_max_path_length"), String(fsInfo.max_path_length)]
    );
  }

  return {
    updateHost: settings.strvars[STR.UPDATE_HOST] || "",
    updatePath: settings.strvars[STR.UPDATE_PATH] || "",
    infoItems: [
      [translate("maintenance.boot_mode_client"), eepromSettings && eepromSettings.ssid ? eepromSettings.ssid : "-"],
      [translate("network.ap_eyebrow"), eepromSettings && eepromSettings.ap_ssid ? eepromSettings.ap_ssid : "-"],
      [translate("maintenance.boot_mode"), eepromSettings && eepromSettings.boot_as_ap ? translate("maintenance.boot_mode_ap") : translate("maintenance.boot_mode_client")]
    ],
    fsInfoItems: fsInfoItems.length ? fsInfoItems : [["LittleFS", translate("maintenance.no_data")]],
    files: Array.isArray(files) ? files : []
  };
}

function getEnvironmentUiMeta(settings) {
  return {
    weather: getWeatherUiMeta(settings),
    dateTime: getDateTimeUiMeta(settings),
    temperature: getTemperatureUiMeta(settings),
    ldr: getLdrUiMeta(settings)
  };
}

function getDisplayFormUiMeta(settings) {
  return {
    display: getDisplayUiMeta(settings),
    animation: getAnimationUiMeta(settings),
    tft: getTftUiMeta(settings)
  };
}

function getSettingsControlUiMeta(settings, networkInfo, eepromSettings) {
  const environmentMeta = getEnvironmentUiMeta(settings);
  const displayFormMeta = getDisplayFormUiMeta(settings);

  return {
    weather: environmentMeta.weather,
    network: getNetworkUiMeta(settings, networkInfo, eepromSettings),
    maintenance: getMaintenanceUiMeta(settings, eepromSettings, null, null),
    dateTime: environmentMeta.dateTime,
    temperature: environmentMeta.temperature,
    ldr: environmentMeta.ldr,
    animation: displayFormMeta.animation,
    tft: displayFormMeta.tft
  };
}

function getCoreBackupUiMeta(settings, eepromSettings, networkInfo) {
  const settingsControlMeta = getSettingsControlUiMeta(settings, networkInfo, eepromSettings);
  const displayFormMeta = getDisplayFormUiMeta(settings);

  return {
    display: displayFormMeta.display,
    weather: settingsControlMeta.weather,
    network: settingsControlMeta.network,
    maintenance: settingsControlMeta.maintenance,
    temperature: settingsControlMeta.temperature,
    ldr: settingsControlMeta.ldr,
    animation: displayFormMeta.animation
  };
}

function getAdvancedSettingsUiMeta(settings, ambilightOnline, debugOverrides) {
  return {
    ambilight: getAmbilightUiMeta(settings),
    dfplayer: getDfplayerUiMeta(settings),
    animationProfiles: getAnimationProfileUiMeta(settings),
    colorAnimationProfiles: getColorAnimationProfileUiMeta(settings),
    ambilightProfiles: getAmbilightProfileUiMeta(settings),
    dimCurves: getDimCurveUiMeta(settings),
    dfplayerControl: getDfplayerControlUiMeta(settings, debugOverrides),
    overlays: getOverlayUiMeta(settings),
    timers: getTimerUiMeta(settings, false),
    ambilightTimers: getTimerUiMeta(settings, true),
    flags: getFlagUiMeta(settings, ambilightOnline)
  };
}

function getDateTimeUiMeta(settings) {
  const current = settings.tmvars[0] || {};

  return {
    year: current.year ? String(current.year) : "",
    month: current.month ? String(current.month) : "",
    day: current.day ? String(current.day) : "",
    hour: current.hour !== undefined ? String(current.hour) : "",
    minute: current.minute !== undefined ? String(current.minute) : "",
    preview: formatDateTimePreview(current)
  };
}

function getTemperatureUiMeta(settings) {
  return {
    items: [
      ["DS18xx", formatHalfDegreeValue(settings.numvars[NUM.DS18XX_IS_UP] ? settings.numvars[NUM.DS18XX_TEMP_INDEX] : null)],
      ["DS18xx online", onOff(settings.numvars[NUM.DS18XX_IS_UP])],
      ["RTC", formatHalfDegreeValue(settings.numvars[NUM.RTC_IS_UP] ? settings.numvars[NUM.RTC_TEMP_INDEX] : null)],
      ["RTC online", onOff(settings.numvars[NUM.RTC_IS_UP])]
    ],
    ds18xxCorrection: settings.numvars[NUM.DS18XX_TEMP_CORRECTION] || 0,
    rtcCorrection: settings.numvars[NUM.RTC_TEMP_CORRECTION] || 0
  };
}

function getLdrUiMeta(settings) {
  const autoBrightness = !!settings.numvars[NUM.DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE];

  return {
    canStoreBounds: autoBrightness,
    items: [
      [translate("climate.auto_brightness"), autoBrightness ? translate("climate.status_on") : translate("climate.status_off")],
      [translate("climate.current_ldr_value"), String(settings.numvars[NUM.LDR_RAW_VALUE] || 0)],
      [translate("climate.minimum"), String(settings.numvars[NUM.LDR_MIN_VALUE] || 0)],
      [translate("climate.maximum"), String(settings.numvars[NUM.LDR_MAX_VALUE] || 0)]
    ]
  };
}

function getAnimationUiMeta(settings) {
  return {
    displayAnimations: settings.dispanims || [],
    displayAnimationMode: settings.numvars[NUM.ANIMATION_MODE] || 0,
    colorAnimations: settings.coloranims || [],
    colorAnimationMode: settings.numvars[NUM.COLOR_ANIMATION_MODE] || 0
  };
}

function getTftUiMeta(settings) {
  const flags = settings.numvars[NUM.SSD1963_FLAGS] || 0;

  return {
    rgb: !!(flags & 0x01),
    hflip: !!(flags & 0x02),
    vflip: !!(flags & 0x04)
  };
}

function getAmbilightUiMeta(settings) {
  return {
    currentMode: settings.numvars[NUM.AMBILIGHT_MODE] || 0,
    modes: settings.almodes.length ? settings.almodes : [
      { idx: 0, name: "Uhr" },
      { idx: 1, name: "Regenbogen" }
    ],
    leds: settings.numvars[NUM.AMBILIGHT_LEDS] || 0,
    offset: settings.numvars[NUM.AMBILIGHT_OFFSET] || 0
  };
}

function getDfplayerUiMeta(settings) {
  return {
    alarms: (settings.alarmtimes || []).slice().sort((a, b) => a.idx - b.idx)
  };
}

function getAnimationProfileUiMeta(settings) {
  return {
    items: (settings.dispanims || []).filter((entry) => entry.flags & 0x01).sort((a, b) => a.idx - b.idx)
  };
}

function getColorAnimationProfileUiMeta(settings) {
  return {
    items: (settings.coloranims || []).filter((entry) => entry.flags & 0x01).sort((a, b) => a.idx - b.idx)
  };
}

function getAmbilightProfileUiMeta(settings) {
  return {
    items: (settings.almodes || []).filter((entry) => entry.flags & 0x01).sort((a, b) => a.idx - b.idx)
  };
}

function getDimCurveUiMeta(settings) {
  return {
    displayValues: settings.num8arrays[0] || {},
    ambilightValues: settings.num8arrays[1] || {}
  };
}

function getDfplayerControlUiMeta(settings, debugOverrides) {
  const bellFlags = settings.numvars[NUM.DFPLAYER_BELL_FLAGS] || 0;
  const mode = settings.numvars[NUM.DFPLAYER_MODE] || 0;

  return {
    online: getFeatureUiMeta(settings, debugOverrides).moduleState.dfplayerOnline,
    volume: settings.numvars[NUM.DFPLAYER_VOLUME] || 0,
    mode,
    bell15: !!(bellFlags & 0x01),
    bell30: !!(bellFlags & 0x02),
    bell45: !!(bellFlags & 0x04),
    speakCycle: settings.numvars[NUM.DFPLAYER_SPEAK_CYCLE] || 0,
    silenceStart: minutesToTimeValue(settings.numvars[NUM.DFPLAYER_SILENCE_START] || 0),
    silenceStop: minutesToTimeValue(settings.numvars[NUM.DFPLAYER_SILENCE_STOP] || 0)
  };
}

function getOverlayUiMeta(settings) {
  const count = settings.numvars[NUM.OVERLAY_N_OVERLAYS] || 0;
  const items = (settings.overlays || []).filter((overlay) => overlay.idx < count).slice();

  if (count < 32) {
    items.push({
      idx: count,
      type: 0,
      interval: 5,
      duration: 5,
      date_code: 0,
      date_start: 0,
      days: 1,
      flags: 0,
      text: "",
      isNew: true
    });
  }

  return { items };
}

function getTimerUiMeta(settings, isAmbilight) {
  return {
    items: (isAmbilight ? settings.ambinighttimes : settings.nighttimes || []).slice().sort((a, b) => a.idx - b.idx)
  };
}

function getFlagUiMeta(settings, ambilightOnline) {
  const flags = settings.numvars[NUM.DISPLAY_FLAGS] || 0;

  // Der ESP schreibt das Markerflag nach CLOCK_AMBILIGHT_MODE_VAR, und das ist in der
  // Aufzaehlung Index 1 (vars.h). Index 0 ist NORMAL. Wer Index 0 liest, sieht den
  // Schalter dauerhaft auf "aus" und kann ihn deshalb einschalten, aber nie wieder
  // ausschalten. L31.
  const clockMode = (settings.almodes || []).find((entry) => entry.idx === 1) || null;
  const markersEnabled = !!(((clockMode && clockMode.flags) || 0) & 0x02);

  // Zustand und Bedienbarkeit sind zwei verschiedene Dinge: die Bits stehen im
  // Geraetespeicher, auch wenn gerade kein Ambilight antwortet. Wer beides verundet,
  // beschriftet einen gesetzten Schalter mit "aktivieren" und zeigt damit das
  // Gegenteil der Wahrheit. L34b.
  return {
    syncAmbilight: !!(flags & 0x02),
    syncMarkers: !!(flags & 0x04),
    fadeClockSeconds: !!(flags & 0x08),
    ambilightMarkers: markersEnabled,
    controlsEnabled: !!ambilightOnline
  };
}

function getUpdateUiViewMeta(updateMeta) {
  return {
    canUpdate: !!(updateMeta && updateMeta.summary && updateMeta.summary.canUpdate),
    stm32Default: updateMeta && updateMeta.summary ? updateMeta.summary.stm32Default : "",
    stm32Files: updateMeta && updateMeta.summary ? updateMeta.summary.stm32Files : [],
    releaseNotes: updateMeta && updateMeta.summary ? updateMeta.summary.releaseNotes : ""
  };
}

function renderDimCurves(settings) {
  renderDimCurvesFromMeta(getDimCurveUiMeta(settings));
}

function renderDimCurvesFromMeta(meta) {
  populateDimPresetSelect("display-dim-preset-select");
  populateDimPresetSelect("ambilight-dim-preset-select");
  renderDimCurveList("display-dim-list", meta.displayValues, "disp");
  renderDimCurveList("ambilight-dim-list", meta.ambilightValues, "ambi");
  syncDimPresetSelection("disp");
  syncDimPresetSelection("ambi");
}

function renderDimCurveList(rootId, values, prefix) {
  const root = document.getElementById(rootId);
  const rows = [];

  for (let idx = 0; idx <= 15; idx += 1) {
    const value = values[idx] ?? 0;
    const levelLabel = translateFormat("display.dim_level", { idx });
    rows.push(
      '<section class="dim-card">' +
        '<div class="slider-row">' +
          '<span class="label">' + escapeHtml(levelLabel) + '</span>' +
          '<input id="' + prefix + '-dim-' + idx + '" type="range" min="0" max="15" value="' + escapeHtml(String(value)) + '">' +
          '<strong id="' + prefix + '-dim-value-' + idx + '" class="value-pill">' + escapeHtml(String(value)) + '</strong>' +
        '</div>' +
      '</section>'
    );
  }

  root.innerHTML = rows.join("");
  bindIndexedSuffixAction(root, 'input[id^="' + prefix + '-dim-"]', "input", (idx) => {
    syncDimCurveValue(prefix, idx);
    syncDimPresetSelection(prefix);
  });
}

function populateDimPresetSelect(selectId) {
  const select = document.getElementById(selectId);
  if (!select || select.options.length) {
    return;
  }
  select.innerHTML = Object.keys(DIM_CURVE_PRESET_NAMES).map((key) => (
    '<option value="' + escapeHtml(key) + '">' + escapeHtml(DIM_CURVE_PRESET_NAMES[key]) + "</option>"
  )).join("");
}

function getDimCurveValues(prefix) {
  const values = [];
  for (let idx = 0; idx <= 15; idx += 1) {
    const input = document.getElementById(prefix + "-dim-" + idx);
    values.push(Math.max(0, Math.min(15, Number(input && input.value ? input.value : 0))));
  }
  return values;
}

function findMatchingDimPreset(values) {
  return Object.keys(DIM_CURVE_PRESETS).find((key) => {
    const preset = DIM_CURVE_PRESETS[key];
    return preset.every((value, idx) => value === values[idx]);
  }) || "custom";
}

function syncDimPresetSelection(prefix) {
  const select = document.getElementById(prefix === "ambi" ? "ambilight-dim-preset-select" : "display-dim-preset-select");
  if (!select) {
    return;
  }
  select.value = findMatchingDimPreset(getDimCurveValues(prefix));
}

function syncDimCurveValue(prefix, idx) {
  const input = document.getElementById(prefix + "-dim-" + idx);
  const value = document.getElementById(prefix + "-dim-value-" + idx);
  if (input && value) {
    value.textContent = input.value;
  }
}

function applyDimPreset(prefix) {
  const select = document.getElementById(prefix === "ambi" ? "ambilight-dim-preset-select" : "display-dim-preset-select");
  const preset = DIM_CURVE_PRESETS[select.value] || DIM_CURVE_PRESETS.linear;
  for (let idx = 0; idx <= 15; idx += 1) {
    const input = document.getElementById(prefix + "-dim-" + idx);
    if (input) {
      input.value = String(preset[idx] ?? 0);
      syncDimCurveValue(prefix, idx);
    }
  }
  syncDimPresetSelection(prefix);
}

async function applyDimPresetAndSave(prefix) {
  const button = document.getElementById(prefix === "ambi" ? "ambilight-dim-preset-apply-button" : "display-dim-preset-apply-button");
  const buttonText = translate("display.apply_preset_save");

  beginButtonFeedback(button, translate("common.saving"));
  applyDimPreset(prefix);

  try {
    await persistDimCurve(prefix, button, buttonText);
  } catch (error) {
    announceStatus(translate("display.preset_apply_failed"), "error");
    finishButtonFeedback(button, buttonText, "error", translate("common.error"));
  }
}

function renderOverlayRows(settings) {
  renderOverlayRowsFromMeta(getOverlayUiMeta(settings).items);
}

function renderOverlayRowsFromMeta(items) {
  const root = document.getElementById("overlay-list");

  root.innerHTML = items.map((overlay) => {
    const type = overlay.type || 0;
    const month = overlay.date_start ? (overlay.date_start >> 8) : 0;
    const day = overlay.date_start ? (overlay.date_start & 0xff) : 0;
    const mp3 = parseOverlayMp3Value(overlay.text || "");
    // Das Feld traegt min=5 und max=9, und seit Runde 1 weist das Geraet alles
    // dazwischen ab. Ein aelterer Bestandswert ausserhalb des Bereichs -- moeglich,
    // solange der ESP ihn selbst klemmte -- stuende sonst unveraenderbar in einem oft
    // ausgeblendeten Feld und blockierte jedes Speichern dieses Overlays.
    const duration = overlay.duration >= 5 && overlay.duration <= 9 ? overlay.duration : 5;
    const showIcon = type === 1;
    const showText = type === 6;
    const showMp3 = type === 7;
    const showDuration = type === 1 || type === 4 || type === 8;
    const showDateStart = overlay.date_code === 0;
    const showDays = !(overlay.date_code === 0 && !overlay.date_start);
    const idx = overlay.idx;
    const title = overlay.isNew ? translate("overlays.new_title") : "Overlay " + String(idx);
    const overlayTypeNames = getOverlayTypeNames();
    const overlayTypeName = isKnownListIndex(overlayTypeNames, type)
      ? overlayTypeNames[type]
      : translateFormat("common.unknown_device_value", { value: String(type) });

    return (
      '<section class="overlay-card" data-overlay-idx="' + idx + '"' + (overlay.isNew ? ' data-overlay-new="1"' : '') + '>' +
        '<div class="card-headline">' +
          '<div><span class="label">' + escapeHtml(title) + '</span><p class="card-subline">' + escapeHtml(overlayTypeName) + '</p></div>' +
          (overlay.isNew ? '<span class="state-pill">' + escapeHtml(translate("overlays.new_badge")) + '</span>' : '') +
        '</div>' +
        '<div class="chip-row">' +
          '<label class="chip-toggle"><input type="checkbox" id="ov-active-' + idx + '"' + ((overlay.flags & 0x01) ? " checked" : "") + '> ' + escapeHtml(translate("common.active")) + '</label>' +
        '</div>' +
        '<div class="overlay-layout">' +
          '<div class="form-section">' +
            '<p class="section-label">' + escapeHtml(translate("overlays.content")) + '</p>' +
            '<label class="field"><span class="label">' + escapeHtml(translate("overlays.type")) + '</span><select id="ov-type-' + idx + '">' + buildNamedOptions(overlayTypeNames, type) + "</select></label>" +
            '<label id="ov-icon-wrap-' + idx + '" class="field' + (showIcon ? '' : ' is-hidden') + '"><span class="label">' + escapeHtml(translate("overlays.icon")) + '</span><select id="ov-icon-' + idx + '">' + buildIconOptions(overlay.text || "") + '</select></label>' +
            '<label id="ov-value-wrap-' + idx + '" class="field' + (showText ? '' : ' is-hidden') + '"><span class="label">' + escapeHtml(translate("overlays.value")) + '</span><input id="ov-value-' + idx + '" type="text" maxlength="32" value="' + escapeHtml(overlay.text || "") + '"></label>' +
            '<div id="ov-mp3-wrap-' + idx + '" class="time-grid' + (showMp3 ? '' : ' is-hidden') + '">' +
              '<label class="field"><span class="label">' + escapeHtml(translate("overlays.folder")) + '</span><input id="ov-folder-' + idx + '" type="number" min="0" max="99" step="1" value="' + escapeHtml(mp3.folder) + '"></label>' +
              '<label class="field"><span class="label">' + escapeHtml(translate("overlays.track")) + '</span><input id="ov-track-' + idx + '" type="number" min="0" max="999" step="1" value="' + escapeHtml(mp3.track) + '"></label>' +
            '</div>' +
          '</div>' +
          '<div class="form-section">' +
            '<p class="section-label">' + escapeHtml(translate("overlays.time_and_date")) + '</p>' +
            '<div class="overlay-time-grid">' +
              '<label class="field"><span class="label">' + escapeHtml(translate("overlays.interval")) + '</span><input id="ov-interval-' + idx + '" type="number" min="1" max="255" step="1" value="' + escapeHtml(String(overlay.interval || 5)) + '"></label>' +
              '<label id="ov-duration-wrap-' + idx + '" class="field' + (showDuration ? '' : ' is-hidden') + '"><span class="label">' + escapeHtml(translate("overlays.duration")) + '</span><input id="ov-duration-' + idx + '" type="number" min="5" max="9" step="1" value="' + escapeHtml(String(duration)) + '"></label>' +
            '</div>' +
            '<label id="ov-datecode-wrap-' + idx + '" class="field"><span class="label">' + escapeHtml(translate("overlays.date_code")) + '</span><select id="ov-datecode-' + idx + '">' + buildNamedOptions(getOverlayDateCodeNames(), overlay.date_code) + '</select></label>' +
            '<div class="overlay-date-grid">' +
              '<label id="ov-day-wrap-' + idx + '" class="field' + (showDateStart ? '' : ' is-hidden') + '"><span class="label">' + escapeHtml(translate("overlays.day")) + '</span><select id="ov-day-' + idx + '">' + buildDayOptions(day) + '</select></label>' +
              '<label id="ov-month-wrap-' + idx + '" class="field' + (showDateStart ? '' : ' is-hidden') + '"><span class="label">' + escapeHtml(translate("overlays.month")) + '</span><select id="ov-month-' + idx + '">' + buildMonthOptions(month) + '</select></label>' +
              '<label id="ov-days-wrap-' + idx + '" class="field' + (showDays ? '' : ' is-hidden') + '"><span class="label">' + escapeHtml(translate("overlays.days")) + '</span><input id="ov-days-' + idx + '" type="number" min="1" max="255" step="1" value="' + escapeHtml(String(overlay.days || 1)) + '"></label>' +
            '</div>' +
          '</div>' +
          '<div class="profile-actions overlay-actions">' +
            '<button class="button" type="button" data-overlay-save="' + idx + '">' + escapeHtml(overlay.isNew ? translate("overlays.create") : translate("overlays.save")) + "</button>" +
            (overlay.isNew
              ? '<button class="button is-hidden" type="button" data-overlay-cancel="' + idx + '">' + escapeHtml(translate("overlays.cancel")) + '</button>'
              : '<button class="button" type="button" data-overlay-display="' + idx + '">' + escapeHtml(translate("overlays.display")) + '</button><button class="button" type="button" data-overlay-delete="' + idx + '">' + escapeHtml(translate("overlays.delete")) + '</button>') +
          '</div>' +
        "</div>" +
      "</section>"
    );
  }).join("");

  bindDataAction(root, "data-overlay-save", saveOverlay);
  bindDataAction(root, "data-overlay-cancel", cancelOverlayEdit);
  bindDataAction(root, "data-overlay-display", displayOverlay);
  bindDataAction(root, "data-overlay-delete", deleteOverlay);
  bindIndexedSuffixAction(root, "[id^='ov-type-']", "change", (idx) => {
    void handleOverlayTypeChange(idx);
  });
  bindIndexedSuffixAction(root, "input[id^='ov-'], select[id^='ov-']", "input", (idx) => updateOverlayDraftActions(idx));
  bindIndexedSuffixAction(root, "input[id^='ov-'], select[id^='ov-']", "change", (idx) => updateOverlayDraftActions(idx));
  bindIndexedSuffixAction(root, "[id^='ov-datecode-']", "change", (idx) => updateOverlayRowVisibility(idx));
  bindIndexedSuffixAction(root, "[id^='ov-month-'], [id^='ov-day-']", "input", (idx) => updateOverlayRowVisibility(idx));
  items.forEach((overlay) => {
    if (overlay.isNew) {
      captureOverlayDraftBaseline(overlay.idx);
      updateOverlayDraftActions(overlay.idx);
    }
  });
}

function renderTimerRows(settings, isAmbilight) {
  renderTimerRowsFromMeta(getTimerUiMeta(settings, isAmbilight).items, isAmbilight);
}

const TIMER_FLAG_ACTIVE = 0x80;
const TIMER_FLAG_FROM_DAY = 0x38;
const TIMER_FLAG_TO_DAY = 0x07;

// Die Uhr löst den Tagesbereich nach drei Regeln auf (night.c:166-180): gleicher Von-
// und Bis-Tag meint genau diesen einen Tag, von < bis meint die Spanne, von > bis
// meint die Spanne über den Sonntag hinweg.
function getTimerWeekdays(flags) {
  const fromDay = (flags & TIMER_FLAG_FROM_DAY) >> 3;
  const toDay = flags & TIMER_FLAG_TO_DAY;
  const days = [];

  for (let wday = 0; wday <= 6; wday += 1) {
    const matches = fromDay === toDay
      ? wday === fromDay
      : (fromDay < toDay ? (wday >= fromDay && wday <= toDay) : !(wday > toDay && wday < fromDay));

    if (matches) {
      days.push(wday);
    }
  }

  return days;
}

// Zwei aktive Timer auf derselben Minute sind nicht falsch, nur unklar: Die Uhr prüft
// die Slots aufsteigend und hält beim ersten Treffer an (night.c:153-188), der
// niedrigere Index greift also zuerst. Deshalb ein Hinweis und kein Verbot — der
// Nutzer soll nur wissen, welcher gewinnt. L72.
//
// Verglichen werden nicht nur deckungsgleiche Tagesbereiche: "Mo-Fr" und "Mi-Mi"
// treffen sich am Mittwoch genauso, und dort sieht man es noch schlechter. Geprüft
// wird deshalb auf Überschneidung der aufgelösten Tagesmengen.
function findTimerConflicts(items) {
  const conflicts = new Map();
  const active = (items || []).filter((item) => (item.flags & TIMER_FLAG_ACTIVE));

  for (let first = 0; first < active.length; first += 1) {
    for (let second = first + 1; second < active.length; second += 1) {
      if ((active[first].minutes || 0) !== (active[second].minutes || 0)) {
        continue;
      }

      const daysOfFirst = getTimerWeekdays(active[first].flags);
      const daysOfSecond = getTimerWeekdays(active[second].flags);

      if (!daysOfFirst.some((day) => daysOfSecond.includes(day))) {
        continue;
      }

      for (const pair of [[active[first], active[second]], [active[second], active[first]]]) {
        const partners = conflicts.get(pair[0].idx) || [];
        partners.push(pair[1].idx);
        conflicts.set(pair[0].idx, partners);
      }
    }
  }

  return conflicts;
}

// Gibt reinen Text zurück, kein Markup: Die Einsetzstelle escaped, und dort gehört
// es auch hin. Eine Funktion, die fertiges HTML liefert, nimmt der Prüfung S7 die
// Möglichkeit, zwischen sauber und ungefiltert zu unterscheiden.
function buildTimerConflictText(idx, partners) {
  if (!partners || !partners.length) {
    return "";
  }

  const slotLabel = translate("timers.slot");
  const others = partners.slice().sort((a, b) => a - b).map((partner) => slotLabel + " " + partner).join(", ");
  const winner = Math.min(idx, ...partners);

  return translateFormat("timers.conflict_hint", { others, winner });
}

function renderTimerRowsFromMeta(items, isAmbilight) {
  const root = document.getElementById(isAmbilight ? "ambilight-timer-list" : "timer-list");
  const conflicts = findTimerConflicts(items);

  root.innerHTML = items.map((item) => {
    const idx = item.idx;
    const time = minutesToTimeValue(item.minutes || 0);
    const fromDay = (item.flags & 0x38) >> 3;
    const toDay = item.flags & 0x07;
    const active = (item.flags & 0x80) ? "checked" : "";
    const switchOn = (item.flags & 0x40) ? "checked" : "";
    const prefix = isAmbilight ? "at" : "t";
    const conflictText = buildTimerConflictText(idx, conflicts.get(idx));

    return (
      '<section class="alarm-card">' +
        '<div class="card-headline"><div><span class="label">' + escapeHtml(translate("timers.slot")) + ' ' + escapeHtml(String(idx)) + '</span><p class="card-subline">' + escapeHtml(isAmbilight ? translate("timers.ambilight_slot_subline") : translate("timers.slot_subline")) + '</p></div></div>' +
        (conflictText ? '<p class="module-warning">' + escapeHtml(conflictText) + "</p>" : "") +
        '<div class="chip-row">' +
          '<label class="chip-toggle"><input type="checkbox" id="' + prefix + '-active-' + idx + '" ' + active + '> ' + escapeHtml(translate("common.active")) + '</label>' +
          '<label class="field timer-action-field"><span class="label">' + escapeHtml(translate("timers.action")) + '</span><select id="' + prefix + '-action-' + idx + '"><option value="on"' + ((item.flags & 0x40) ? ' selected' : '') + '>' + escapeHtml(translate("timers.switch_on")) + '</option><option value="off"' + (!(item.flags & 0x40) ? ' selected' : '') + '>' + escapeHtml(translate("timers.switch_off")) + '</option></select></label>' +
        '</div>' +
        '<div class="form-section">' +
          '<p class="section-label">' + escapeHtml(translate("timers.period")) + '</p>' +
          '<div class="timer-fields-grid">' +
            '<label class="field"><span class="label">' + escapeHtml(translate("timers.from_day")) + '</span><select id="' + prefix + '-from-' + idx + '">' + buildWeekdayOptions(fromDay) + "</select></label>" +
            '<label class="field"><span class="label">' + escapeHtml(translate("timers.to_day")) + '</span><select id="' + prefix + '-to-' + idx + '">' + buildWeekdayOptions(toDay) + "</select></label>" +
            '<label class="field"><span class="label">' + escapeHtml(translate("timers.time")) + '</span><input id="' + prefix + '-time-' + idx + '" type="time" value="' + escapeHtml(time) + '"></label>' +
          '</div>' +
        '</div>' +
        '<div class="profile-actions">' +
          '<button class="button" type="button" data-' + prefix + '-save="' + idx + '">' + escapeHtml(translate("common.save")) + '</button>' +
          '<button class="button" type="button" data-' + prefix + '-clear="' + idx + '">' + escapeHtml(translate("timers.clear_slot")) + '</button>' +
        '</div>' +
      "</section>"
    );
  }).join("");

  bindDataAction(root, "data-" + (isAmbilight ? "at" : "t") + "-save", (idx) => saveTimerRow(idx, isAmbilight));
  bindDataAction(root, "data-" + (isAmbilight ? "at" : "t") + "-clear", (idx) => clearTimerRow(idx, isAmbilight));
}

function setColorControl(prefix, color, useRgbw, syncTheme) {
  const current = color || { red: 0, green: 0, blue: 0, white: 0 };
  const rgbInput = document.getElementById(prefix + "-color-rgb");
  const whiteInput = document.getElementById(prefix + "-color-white");

  rgbInput.value = rgb63ToHex(current);
  whiteInput.value = String(current.white || 0);
  whiteInput.disabled = !useRgbw;
  syncWhiteChannelLabel(prefix);
  updateLiveColorPreview(prefix, syncTheme !== false);
}

function updateFlagControls(settings, ambilightOnline) {
  updateFlagControlsFromMeta(getFlagUiMeta(settings, ambilightOnline));
}

function updateFlagControlsFromMeta(meta) {
  // Beschriftung aus dem echten Bitzustand, Bedienbarkeit getrennt davon aus der
  // Erreichbarkeit des Ambilights. L34b.
  const available = meta.controlsEnabled !== false;

  setActionToggleButton("sync-ambilight-button", translate("display.unsync_ambilight"), translate("display.sync_ambilight"), meta.syncAmbilight, available);
  setActionToggleButton("sync-markers-button", translate("display.unsync_markers"), translate("display.sync_markers"), meta.syncMarkers, available);
  setActionToggleButton("fade-clock-seconds-button", translate("display.fade_clock_seconds_disable"), translate("display.fade_clock_seconds"), meta.fadeClockSeconds, available);
  setActionToggleButton("ambilight-markers-button", translate("display.ambilight_markers_disable"), translate("display.ambilight_markers"), meta.ambilightMarkers, available);
}

function setActionToggleButton(id, onText, offText, enabled, available) {
  const button = document.getElementById(id);
  if (!button) {
    return;
  }

  button.dataset.state = enabled ? "on" : "off";
  button.textContent = enabled ? onText : offText;

  // Bewusst NICHT "primary": Seit Massnahme 10 ist "primary" sichtbar hervorgehoben und
  // heisst "das ist die Hauptaktion dieser Karte". Ein Schalter, der nur seinen Zustand
  // zeigt, ist keine Hauptaktion — in der RGBW-Karte leuchteten so bis zu vier Knoepfe
  // gleichzeitig und die Karte hatte keine erkennbare Hauptaktion mehr. "is-on" traegt
  // allein die Bedeutung "eingeschaltet". L43.
  button.classList.toggle("is-on", enabled);
  button.setAttribute("aria-pressed", enabled ? "true" : "false");

  // Nur Aufrufer, die die Bedienbarkeit kennen, duerfen sie setzen. Alle anderen
  // Schalter bleiben unberuehrt, sonst aktivierte dieser Zweig sie ungewollt.
  if (available === undefined) {
    return;
  }

  button.disabled = !available;
  if (available) {
    button.removeAttribute("title");
  } else {
    button.title = translate("flags.ambilight_offline_hint");
  }
}

function updateLiveColorPreview(prefix, syncTheme) {
  const preview = document.getElementById(prefix + "-color-preview");
  const rgbInput = document.getElementById(prefix + "-color-rgb");
  const whiteInput = document.getElementById(prefix + "-color-white");
  // Reine Anzeige: waehrend des Tippens ist das Feld kurz leer, und die Vorschau darf
  // deswegen nicht auf 0 springen. Dann bleibt der zuletzt gueltige Wert stehen —
  // abgewiesen wird hier nichts, es wird ja auch nichts gespeichert. L29.
  const whiteRaw = String(whiteInput.value === null || whiteInput.value === undefined ? "" : whiteInput.value).trim();
  const whiteNumber = Number(whiteRaw);
  const whiteIsValid = !!whiteRaw && Number.isFinite(whiteNumber);

  if (whiteIsValid) {
    whiteInput.dataset.lastValidWhite = String(whiteNumber);
  }

  const white = whiteIsValid ? whiteNumber : Number(whiteInput.dataset.lastValidWhite || 0);
  const rgb = hexToRgb63(rgbInput.value);

  preview.style.background = buildColorPreview({
    red: rgb.red,
    green: rgb.green,
    blue: rgb.blue,
    white
  }, !whiteInput.disabled);

  if (prefix === "display" && syncTheme !== false) {
    applyWordclockTheme({
      red: rgb.red,
      green: rgb.green,
      blue: rgb.blue,
      white
    }, !whiteInput.disabled, 0);
  }
}

function syncBrightnessLabel() {
  document.getElementById("brightness-value").textContent = document.getElementById("brightness-slider").value;
}

function syncAmbilightBrightnessLabel() {
  document.getElementById("ambilight-brightness-value").textContent = document.getElementById("ambilight-brightness-slider").value;
}

function syncDfplayerVolumeLabel() {
  document.getElementById("dfplayer-volume-value").textContent = document.getElementById("dfplayer-volume-slider").value;
}

function syncWhiteChannelLabel(prefix) {
  const input = document.getElementById(prefix + "-color-white");
  const value = document.getElementById(prefix + "-color-white-value");

  if (input && value) {
    value.textContent = input.value;
  }
}

async function saveDisplayMode() {
  const value = document.getElementById("display-mode-select").value;
  await runValueSave("display-mode-save-button", getDisplayModeSetUrl(), value, translate("display.save_mode"), translate("display.mode_save_failed"));
}

async function saveTickerText() {
  const value = document.getElementById("ticker-text-input").value;
  await runValueSave("ticker-save-button", getTickerSetUrl(), value, translate("display.save_ticker"), translate("display.ticker_save_failed"));
}

async function saveDateTickerFormat() {
  const value = document.getElementById("date-format-input").value;
  await runValueSave("date-format-save-button", getDateTickerFormatSetUrl(), value, translate("display.save_date_format"), translate("display.date_format_save_failed"));
}

async function saveTickerDeceleration() {
  const input = document.getElementById("ticker-deceleration-input");
  const number = readNumberInputOrReport(input, 0, 255);

  if (number === null) {
    return;
  }

  const value = number;
  input.value = String(value);
  await runValueSave("ticker-deceleration-save-button", getTickerDecelerationSetUrl(), value, translate("display.save_ticker_delay"), translate("display.ticker_delay_save_failed"));
}

async function testDisplay() {
  const button = document.getElementById("test-display-button");
  const restoreText = translate("display.run_test");
  const maxRunTimeMs = 45000;

  beginButtonFeedback(button, translate("common.running"));
  startBackgroundPause(maxRunTimeMs, true);

  try {
    await apiFetch(getDisplayTestUrl());
    announceStatus(translate("display.test_running"), "ok");
    window.setTimeout(() => {
      finishButtonFeedback(button, restoreText);
    }, maxRunTimeMs);
  } catch (error) {
    announceStatus(translate("display.test_start_failed"), "error");
    finishButtonFeedback(button, restoreText, "error", translate("common.error"));
  }
}

async function saveWeatherAppId() {
  const value = document.getElementById("weather-appid-input").value || "";
  await runValueSave("weather-appid-save-button", getWeatherAppIdSetUrl(), value, translate("climate.save_api_key"), translate("climate.api_key_save_failed"));
}

async function saveWeatherCity() {
  const value = document.getElementById("weather-city-input").value || "";
  await runValueSave("weather-city-save-button", getWeatherCitySetUrl(), value, translate("climate.save_city"), translate("climate.city_save_failed"));
}

async function saveWeatherCoordinates() {
  const lon = document.getElementById("weather-lon-input").value || "";
  const lat = document.getElementById("weather-lat-input").value || "";
  await runQuerySave("weather-coordinates-save-button", getWeatherCoordinatesSetUrl(), { lon, lat }, translate("climate.save_coordinates"), translate("climate.coordinates_save_failed"));
}

async function getWeatherNow() {
  await runWeatherAction("weather-now-button", getWeatherNowUrl(), translate("climate.fetch_weather"), translate("climate.weather_request_failed"));
}

async function getWeatherForecast() {
  await runWeatherAction("weather-forecast-button", getWeatherForecastUrl(), translate("climate.fetch_forecast"), translate("climate.forecast_request_failed"));
}

async function runWeatherAction(buttonId, endpoint, buttonText, errorText) {
  startBackgroundPause(12000, false);

  await runButtonRequestById(buttonId, {
    busyText: translate("common.running"),
    idleText: buttonText,
    successText: "angefragt",
    errorText,
    request: () => apiFetch(endpoint, { timeoutMs: 12000, attempts: 1 })
  });
}

function ensureLeafletStylesheet() {
  let link = document.querySelector('link[data-leaflet-styles="1"]');

  if (link) {
    return Promise.resolve();
  }

  link = document.createElement("link");
  link.rel = "stylesheet";
  link.href = LEAFLET_CSS_URL;
  link.crossOrigin = "";
  link.setAttribute("data-leaflet-styles", "1");

  return new Promise((resolve, reject) => {
    link.onload = () => resolve();
    link.onerror = () => reject(new Error("leaflet-css"));
    document.head.appendChild(link);
  });
}

function ensureLeafletScript() {
  if (window.L) {
    return Promise.resolve(window.L);
  }

  const existing = document.querySelector('script[data-leaflet-script="1"]');
  if (existing) {
    return new Promise((resolve, reject) => {
      existing.addEventListener("load", () => resolve(window.L), { once: true });
      existing.addEventListener("error", () => reject(new Error("leaflet-js")), { once: true });
    });
  }

  return new Promise((resolve, reject) => {
    const script = document.createElement("script");
    script.src = LEAFLET_JS_URL;
    script.crossOrigin = "";
    script.defer = true;
    script.setAttribute("data-leaflet-script", "1");
    script.onload = () => resolve(window.L);
    script.onerror = () => reject(new Error("leaflet-js"));
    document.body.appendChild(script);
  });
}

function ensureLeafletAssets() {
  if (window.L) {
    return Promise.resolve(window.L);
  }

  if (leafletAssetsPromise) {
    return leafletAssetsPromise;
  }

  leafletAssetsPromise = Promise.all([
    ensureLeafletStylesheet(),
    ensureLeafletScript()
  ]).then(([, leaflet]) => leaflet)
    .finally(() => {
      leafletAssetsPromise = null;
    });

  return leafletAssetsPromise;
}

// Das Kartenmodal war bis hierher nur eine Ebene mit dunklem Hintergrund. Mit Tab
// landete man dahinter in Feldern, die man nicht sieht, Escape tat nichts, und nach
// dem Schliessen sass der Fokus am Seitenanfang statt wieder auf der Schaltflaeche,
// die das Modal geoeffnet hatte. Fuer role="dialog" verlangt WAI-ARIA genau diese
// drei Dinge: Fokus hinein, Fokus drin halten, Fokus zurueck.
let modalReturnFocus = null;

function focusableInModal(modal) {
  const candidates = modal.querySelectorAll(
    'a[href], button:not([disabled]), input:not([disabled]), select:not([disabled]),' +
    ' textarea:not([disabled]), [tabindex]:not([tabindex="-1"])'
  );
  // Leaflet haengt eigene Bedienelemente in die Karte, die zeitweise unsichtbar sind.
  // offsetParent === null faengt alles ab, was display:none ist oder in einem solchen
  // Vorfahren steckt -- genau die Elemente, auf die Tab ohnehin nicht springt.
  return Array.prototype.filter.call(candidates, (element) => element.offsetParent !== null);
}

function handleModalKeydown(event) {
  const modal = document.getElementById("weather-map-modal");

  if (!modal || modal.classList.contains("is-hidden")) {
    return;
  }

  if (event.key === "Escape") {
    event.preventDefault();
    closeWeatherMapPicker();
    return;
  }

  if (event.key !== "Tab") {
    return;
  }

  const items = focusableInModal(modal);

  if (!items.length) {
    return;
  }

  const first = items[0];
  const last = items[items.length - 1];

  if (event.shiftKey && (document.activeElement === first || !modal.contains(document.activeElement))) {
    event.preventDefault();
    last.focus();
  } else if (!event.shiftKey && document.activeElement === last) {
    event.preventDefault();
    first.focus();
  }
}

function handleModalBackdropClick(event) {
  // Nur der Klick auf die Abdeckung selbst schliesst. Ein Klick in die Karte oder auf
  // ein Feld blubbert zwar bis hierher, hat dann aber ein anderes Ziel.
  if (event.target === event.currentTarget) {
    closeWeatherMapPicker();
  }
}

async function openWeatherMapPicker() {
  const modal = document.getElementById("weather-map-modal");
  const status = document.getElementById("weather-map-status");
  const shell = document.querySelector(".shell");

  modalReturnFocus = document.activeElement;
  modal.classList.remove("is-hidden");
  modal.setAttribute("aria-hidden", "false");
  document.body.classList.add("has-modal");

  // inert nimmt dem Hintergrund Fokus UND Mausereignisse in einem Zug. Wo es fehlt
  // (aeltere iOS-Versionen), traegt die Tab-Falle unten allein.
  if (shell && "inert" in HTMLElement.prototype) {
    shell.inert = true;
  }

  document.addEventListener("keydown", handleModalKeydown, true);
  modal.addEventListener("mousedown", handleModalBackdropClick);

  const firstField = document.getElementById("weather-map-search-input");

  if (firstField) {
    firstField.focus();
  }

  status.textContent = translate("weather.map_loading");

  try {
    await ensureLeafletAssets();
    initializeWeatherMap();
  } catch (_) {
    status.textContent = translate("weather.map_load_failed");
  }
}

function closeWeatherMapPicker() {
  const modal = document.getElementById("weather-map-modal");
  const shell = document.querySelector(".shell");

  modal.classList.add("is-hidden");
  modal.setAttribute("aria-hidden", "true");
  document.body.classList.remove("has-modal");

  if (shell && "inert" in HTMLElement.prototype) {
    shell.inert = false;
  }

  document.removeEventListener("keydown", handleModalKeydown, true);
  modal.removeEventListener("mousedown", handleModalBackdropClick);

  // Zurueck auf die Schaltflaeche, die geoeffnet hat -- sonst beginnt die
  // Tastaturbedienung nach dem Schliessen wieder ganz oben.
  if (modalReturnFocus && document.contains(modalReturnFocus)) {
    modalReturnFocus.focus();
  }

  modalReturnFocus = null;
}

function initializeWeatherMap() {
  const status = document.getElementById("weather-map-status");

  if (!window.L) {
    status.textContent = translate("weather.map_load_failed");
    return;
  }

  if (!weatherMap) {
    weatherMap = window.L.map("weather-map", { zoomControl: true }).setView([47.3769, 8.5417], 8);

    window.L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
      maxZoom: 19,
      attribution: '&copy; OpenStreetMap'
    }).addTo(weatherMap);

    weatherMap.on("click", (event) => {
      setWeatherMapSelection(event.latlng.lat, event.latlng.lng, document.getElementById("weather-map-city-input").value || "");
      reverseLookupWeatherLocation(event.latlng.lat, event.latlng.lng);
    });
  }

  syncWeatherMapFromInputs();
  status.textContent = translate("weather.map_hint");
  setTimeout(() => weatherMap.invalidateSize(), 50);
}

function syncWeatherMapFromInputs() {
  const lat = Number(document.getElementById("weather-lat-input").value);
  const lon = Number(document.getElementById("weather-lon-input").value);
  const city = document.getElementById("weather-city-input").value || "";

  document.getElementById("weather-map-city-input").value = formatWeatherCityName(city);
  document.getElementById("weather-map-lon-input").value = formatWeatherCoordinate(lon);
  document.getElementById("weather-map-lat-input").value = formatWeatherCoordinate(lat);

  if (weatherMap && Number.isFinite(lat) && Number.isFinite(lon)) {
    setWeatherMapSelection(lat, lon, city);
    weatherMap.setView([lat, lon], 10);
  }
}

// Das Gerät speichert Längen- und Breitengrad in je acht Zeichen und den Ort in
// 32 Byte (MAX_WEATHER_LON_LEN, MAX_WEATHER_LAT_LEN, MAX_WEATHER_CITY_LEN in vars.h).
// Das maxlength der Felder greift nur beim Tippen: Eine programmatische Zuweisung
// bleibt vollständig stehen, tooLong ist dabei false, und erst das Gerät schneidet
// hart ab. Aus "-123.4567" wurde dort "-123.456". L74.
const WEATHER_COORDINATE_MAX_CHARS = 8;
const WEATHER_CITY_MAX_BYTES = 32;

// Lieber eine Nachkommastelle weniger als eine abgeschnittene Zahl: Bei einem
// dreistelligen negativen Grad braucht toFixed(4) neun Zeichen, toFixed(3) acht.
// Auf drei Stellen GERUNDET liegt der Punkt zudem näher am Ziel als auf drei Stellen
// abgeschnitten, der Fehler wird also kleiner als der bisherige.
function formatWeatherCoordinate(value) {
  const number = Number(value);

  if (!Number.isFinite(number)) {
    return "";
  }

  for (let digits = 4; digits >= 0; digits -= 1) {
    const text = number.toFixed(digits);

    if (text.length <= WEATHER_COORDINATE_MAX_CHARS) {
      return text;
    }
  }

  // Nur für Werte ausserhalb des Gradbereichs erreichbar. Dann lieber eine zu lange
  // Zahl, die das Gerät abweist, als eine stillschweigend verbogene.
  return number.toFixed(0);
}

// Leaflet liefert beim Schieben über den Kartenrand hinaus Längengrade ausserhalb von
// -180..180 (zweite Weltkopie: 540 statt 180). Das ist derselbe Punkt auf der Erde,
// nur eine andere Zahl — und eine, die nie in acht Zeichen passt.
function wrapLongitude(value) {
  const number = Number(value);

  if (!Number.isFinite(number)) {
    return number;
  }

  return ((((number + 180) % 360) + 360) % 360) - 180;
}

function utf8ByteLengthOfCodePoint(codePoint) {
  if (codePoint < 0x80) {
    return 1;
  }

  if (codePoint < 0x800) {
    return 2;
  }

  if (codePoint < 0x10000) {
    return 3;
  }

  return 4;
}

// Kürzt auf eine vollständige Zeichengrenze. Ein halbes Mehrbyte-Zeichen am Ende wäre
// genau L46, nur auf der PWA-Seite. Die for...of-Schleife läuft über Codepoints, ein
// Ersatzzeichenpaar bleibt deshalb zusammen.
function truncateToUtf8Bytes(text, maxBytes) {
  const value = text === undefined || text === null ? "" : String(text);
  let result = "";
  let used = 0;

  for (const character of value) {
    const size = utf8ByteLengthOfCodePoint(character.codePointAt(0));

    if (used + size > maxBytes) {
      return result;
    }

    result += character;
    used += size;
  }

  return result;
}

function formatWeatherCityName(city) {
  return truncateToUtf8Bytes(city, WEATHER_CITY_MAX_BYTES);
}

function setWeatherMapSelection(lat, lon, city) {
  if (!weatherMap || !window.L) {
    return;
  }

  const latText = formatWeatherCoordinate(lat);
  const lonText = formatWeatherCoordinate(wrapLongitude(lon));

  // Ohne gültige Koordinate gibt es nichts zu setzen. Vorher lief hier ein NaN bis in
  // den Marker und in die Eingabefelder.
  if (!latText || !lonText) {
    return;
  }

  // Marker und gespeicherter Wert müssen denselben Punkt meinen, sonst steht die Nadel
  // woanders als das, was die Uhr bekommt.
  const roundedLat = Number(latText);
  const roundedLon = Number(lonText);
  const boundedCity = formatWeatherCityName(city);
  selectedWeatherLocation = {
    city: boundedCity,
    lat: roundedLat,
    lon: roundedLon
  };

  document.getElementById("weather-map-city-input").value = boundedCity;
  document.getElementById("weather-map-lat-input").value = latText;
  document.getElementById("weather-map-lon-input").value = lonText;

  if (!weatherMarker) {
    weatherMarker = window.L.marker([roundedLat, roundedLon], { draggable: true }).addTo(weatherMap);
    weatherMarker.on("dragend", () => {
      const latlng = weatherMarker.getLatLng();
      setWeatherMapSelection(latlng.lat, latlng.lng, document.getElementById("weather-map-city-input").value || "");
      reverseLookupWeatherLocation(latlng.lat, latlng.lng);
    });
  } else {
    weatherMarker.setLatLng([roundedLat, roundedLon]);
  }
}

async function searchWeatherLocation() {
  const button = document.getElementById("weather-map-search-button");
  const query = (document.getElementById("weather-map-search-input").value || "").trim();
  const status = document.getElementById("weather-map-status");

  if (!query) {
    status.textContent = translate("weather.enter_location_first");
    return;
  }

  beginButtonFeedback(button, translate("weather.search_busy"));
  status.textContent = translate("weather.searching");

  try {
    const url = "https://nominatim.openstreetmap.org/search?format=jsonv2&limit=1&q=" + encodeURIComponent(query);
    const response = await fetch(url, {
      headers: { Accept: "application/json" },
      cache: "no-store"
    });
    const results = await response.json();

    if (!Array.isArray(results) || !results.length) {
      status.textContent = translate("weather.no_result");
      finishButtonFeedback(button, translate("weather.search_button"), "error", translate("weather.no_result"));
      return;
    }

    const result = results[0];
    const lat = Number(result.lat);
    const lon = Number(result.lon);
    const city = result.display_name || query;

    setWeatherMapSelection(lat, lon, city);
    weatherMap.setView([lat, lon], 11);
    status.textContent = translate("weather.location_found");
    finishButtonFeedback(button, translate("weather.search_button"), "success", translate("weather.found"));
  } catch (error) {
    status.textContent = translate("weather.search_failed");
    finishButtonFeedback(button, translate("weather.search_button"), "error", translate("common.error"));
  }
}

function useCurrentWeatherLocation() {
  const button = document.getElementById("weather-current-location-button");
  const status = document.getElementById("weather-map-status");

  if (!navigator.geolocation) {
    useApproximateWeatherLocation();
    return;
  }

  if (!window.isSecureContext && window.location.hostname !== "localhost" && window.location.hostname !== "127.0.0.1") {
    useApproximateWeatherLocation();
    return;
  }

  beginButtonFeedback(button, translate("weather.current_location_busy"));
  status.textContent = translate("weather.current_location_reading");

  navigator.geolocation.getCurrentPosition(
    (position) => {
      const lat = position.coords.latitude;
      const lon = position.coords.longitude;

      setWeatherMapSelection(lat, lon, document.getElementById("weather-map-city-input").value || "");
      weatherMap.setView([lat, lon], 12);
      status.textContent = translate("weather.current_location_set");
      reverseLookupWeatherLocation(lat, lon);
      finishButtonFeedback(button, translate("weather.current_location_label"), "success", translate("weather.apply_map_done"));
    },
    (error) => {
      if (error && error.code === 1) {
        useApproximateWeatherLocation(translate("weather.geo_denied"));
      } else if (error && error.code === 2) {
        useApproximateWeatherLocation(translate("weather.geo_unavailable"));
      } else if (error && error.code === 3) {
        useApproximateWeatherLocation(translate("weather.geo_timeout"));
      } else {
        useApproximateWeatherLocation(translate("weather.geo_failed"));
      }
    },
    { enableHighAccuracy: true, timeout: 10000, maximumAge: 30000 }
  );
}

async function useApproximateWeatherLocation(initialMessage) {
  const button = document.getElementById("weather-current-location-button");
  const status = document.getElementById("weather-map-status");

  beginButtonFeedback(button, translate("weather.locating_short"));
  status.textContent = initialMessage || translate("weather.approx_location_start");

  try {
    const response = await fetch("https://ipapi.co/json/", {
      headers: { Accept: "application/json" },
      cache: "no-store"
    });
    const result = await response.json();
    const lat = Number(result.latitude);
    const lon = Number(result.longitude);
    const city = result.city || result.region || result.country_name || "";

    if (!Number.isFinite(lat) || !Number.isFinite(lon)) {
      throw new Error("no-location");
    }

    setWeatherMapSelection(lat, lon, city);
    if (weatherMap) {
      weatherMap.setView([lat, lon], 10);
    }
    status.textContent = city
      ? translateFormat("weather.approx_location_city", { city })
      : translate("weather.approx_location_set");
    finishButtonFeedback(button, translate("weather.current_location_label"), "success", translate("weather.apply_map_done"));
  } catch (error) {
    status.textContent = translate("weather.location_unavailable");
    finishButtonFeedback(button, translate("weather.current_location_label"), "error", translate("common.error"));
  }
}

async function reverseLookupWeatherLocation(lat, lon) {
  const status = document.getElementById("weather-map-status");

  try {
    const url = "https://nominatim.openstreetmap.org/reverse?format=jsonv2&lat=" + encodeURIComponent(lat) + "&lon=" + encodeURIComponent(lon);
    const response = await fetch(url, {
      headers: { Accept: "application/json" },
      cache: "no-store"
    });
    const result = await response.json();
    const address = result.address || {};
    const city = address.city || address.town || address.village || address.hamlet || result.display_name || "";

    // display_name von Nominatim ist die volle Adresskette und reisst die 32 Byte des
    // Geräts regelmässig. Hier wird auf einer Zeichengrenze gekürzt — und gesagt, dass
    // gekürzt wurde. Einen stillschweigend verkleinerten Ortsnamen sucht sonst niemand.
    const boundedCity = formatWeatherCityName(city);

    document.getElementById("weather-map-city-input").value = boundedCity;
    if (selectedWeatherLocation) {
      selectedWeatherLocation.city = boundedCity;
    }
    status.textContent = boundedCity === city
      ? translate("weather.map_applied")
      : translateFormat("weather.city_shortened", { city: boundedCity });
  } catch (error) {
    status.textContent = translate("weather.reverse_failed");
  }
}

async function applyWeatherMapSelection() {
  const city = document.getElementById("weather-map-city-input").value || "";
  const lon = document.getElementById("weather-map-lon-input").value || "";
  const lat = document.getElementById("weather-map-lat-input").value || "";

  document.getElementById("weather-city-input").value = city;
  document.getElementById("weather-lon-input").value = lon;
  document.getElementById("weather-lat-input").value = lat;
  document.getElementById("weather-location-preview").textContent = translateFormat("weather.map_preview", {
    city: city || "-",
    lon: lon || "-",
    lat: lat || "-"
  });

  await runButtonRequestById("weather-map-apply-button", {
    busyText: translate("weather.apply_map_busy"),
    idleText: translate("weather.apply_map_idle"),
    successText: translate("weather.apply_map_done"),
    errorText: translate("weather.apply_map_failed"),
    successStatusText: translate("weather.apply_map_success"),
    reload: true,
    request: async () => {
      // Erst die Koordinaten, dann der Ort: Ein Punkt auf der Karte ohne Namen
      // würde als leerer Ort auf ein Gerät ohne Koordinaten treffen und mit
      // Kennung 1 abgewiesen. Mit gesetzten Koordinaten ist der leere Ort erlaubt.
      await apiFetchQuery(getWeatherCoordinatesSetUrl(), { lon, lat });
      await apiFetchValue(getWeatherCitySetUrl(), city || "");
      closeWeatherMapPicker();
    }
  });
}

async function refreshNetworkScan() {
  const button = document.getElementById("network-scan-button");

  beginButtonFeedback(button, translate("network.scan_busy"));

  try {
    await loadData();
    announceStatus(translate("network.scan_success"), "ok");
    finishButtonFeedback(button, translate("network.scan"), "success", translate("common.loaded"));
  } catch (error) {
    announceStatus(translate("network.scan_failed"), "error");
    finishButtonFeedback(button, translate("network.scan"), "error", translate("common.error"));
  }
}

async function saveNetworkClient() {
  // Ein Netz, das seine SSID nicht ausstrahlt, taucht in network_scan nie auf und ist
  // deshalb aus der Trefferliste nicht waehlbar. Steht im freien Feld etwas, hat es
  // Vorrang. Das Feld kann je nach Markupstand fehlen — defensiv lesen. L24.
  const manualInput = document.getElementById("network-ssid-manual-input");
  const manualSsid = manualInput ? String(manualInput.value || "").trim() : "";
  const selectElement = document.getElementById("network-ssid-select");
  const ssid = manualSsid || (selectElement ? selectElement.value || "" : "");
  const key = document.getElementById("network-key-input").value || "";

  // Eine leere SSID zu speichern haengt die Uhr vom WLAN ab und laesst sie nur noch
  // ueber den Accesspoint erreichbar — das ist nie gewollt.
  if (!ssid) {
    announceStatus(translate("network.ssid_missing"), "error");
    return;
  }

  await runQueryButtonRequestById("network-client-save-button", {
    endpoint: getNetworkClientSetUrl(),
    query: { ssid, key },
    busyText: translate("common.connecting"),
    idleText: translate("network.connect_client"),
    successText: translate("common.started"),
    errorText: translate("network.connect_client_error"),
    successStatusText: translate("network.connect_client_started"),
    reloadDelayMs: 1500
  });
}

async function saveNetworkAp() {
  const ssid = document.getElementById("network-ap-ssid-input").value || "";
  const key = document.getElementById("network-ap-key-input").value || "";
  await runQueryButtonRequestById("network-ap-save-button", {
    endpoint: getNetworkApSetUrl(),
    query: { ssid, key },
    busyText: translate("common.starting"),
    idleText: translate("network.start_ap"),
    successText: translate("common.started"),
    errorText: translate("network.start_ap_error"),
    successStatusText: translate("network.start_ap_started"),
    reloadDelayMs: 1500
  });
}

async function saveTimeServer() {
  await runTextSave("network-timeserver-save-button", getNetworkTimeserverSetUrl(), document.getElementById("network-timeserver-input").value || "", translate("network.save_timeserver"), translate("network.timeserver_save_failed"));
}

async function saveTimezone() {
  const input = document.getElementById("network-timezone-input");
  const number = readNumberInputOrReport(input, -12, 14);

  if (number === null) {
    return;
  }

  const value = number;
  input.value = String(value);
  await runQuerySave("network-timezone-save-button", getNetworkTimezoneSetUrl(), { value }, translate("network.save_timezone"), translate("network.timezone_save_failed"), {
    request: async () => {
      await apiFetchValue(getNetworkTimezoneSetUrl(), value);
      await apiFetch(getNetworkGetTimeUrl());
    }
  });
}

async function toggleSummertime() {
  const button = document.getElementById("network-summertime-button");
  await runStateToggleButton(button, getNetworkSummertimeSetUrl(), {
    idleText: button.dataset.restoreText || translate("network.summertime"),
    errorText: translate("network.summertime_set_failed"),
    preserveCurrentText: true
  });
}

async function saveDateTime() {
  // Frueher clampNumber mit Rueckfallwerten: Eine Stunde 25 wurde still zu 23, ein
  // leeres Jahresfeld zu 2026 — und die Uhr meldete "gespeichert". Jetzt werden alle
  // fuenf Felder geprueft, bevor ueberhaupt gesendet wird. Massnahme 17.
  const values = readNumberFieldsOrReport([
    { name: "year", id: "datetime-year-input", min: 2000, max: 2999 },
    { name: "month", id: "datetime-month-input", min: 1, max: 12 },
    { name: "day", id: "datetime-day-input", min: 1, max: 31 },
    { name: "hour", id: "datetime-hour-input", min: 0, max: 23 },
    { name: "minute", id: "datetime-minute-input", min: 0, max: 59 }
  ]);

  if (!values) {
    return;
  }

  const { year, month, day, hour, minute } = values;
  await runQuerySave("datetime-save-button", getDateTimeSetUrl(), { year, month, day, hour, minute }, translate("system.datetime_save"), translate("system.datetime_save_failed"));
}

async function learnIrRemote() {
  await runSimpleAction("learn-ir-button", getLearnIrUrl(), translate("system.learn_ir"), translate("system.learn_ir_failed"), translate("system.learn_ir_started"));
}

async function getNetTime() {
  await runSimpleAction("network-nettime-button", getNetworkGetTimeUrl(), translate("network.fetch_network_time"), translate("network.nettime_failed"), translate("network.nettime_requested"));
}

async function runWps() {
  await runSimpleAction("network-wps-button", getNetworkWpsUrl(), "WPS", translate("network.wps_failed"), translate("network.wps_started"));
}

async function saveUpdateHost() {
  await runTextSave("update-host-save-button", getUpdateHostSetUrl(), document.getElementById("update-host-input").value || "", translate("maintenance.save_update_host"), translate("maintenance.update_host_save_failed"));
  await refreshUpdateServerAvailability();
}

async function saveUpdatePath() {
  await runTextSave("update-path-save-button", getUpdatePathSetUrl(), document.getElementById("update-path-input").value || "", translate("maintenance.save_update_path"), translate("maintenance.update_path_save_failed"));
  await refreshUpdateServerAvailability();
}

async function uploadLocalEspUpdate(event) {
  event.preventDefault();

  const fileInput = document.getElementById("local-update-esp-file-input");
  const button = document.getElementById("local-update-esp-submit-button");
  const file = fileInput.files && fileInput.files[0];

  if (!file) {
    document.getElementById("local-update-note").textContent = translate("maintenance.local_esp_choose_first");
    return;
  }

  // Eine 0-Byte-Datei in den Flashpfad wiegt schwerer als jede leere .gz: danach laeuft
  // auf dem ESP keine Firmware mehr, die ein weiteres Update annehmen koennte.
  // Massnahme 7.
  if (!file.size) {
    document.getElementById("local-update-note").textContent = translateFormat("maintenance.file_empty", { file: file.name });
    announceStatus(translate("maintenance.file_empty_status"), "error");
    finishButtonFeedback(button, translate("maintenance.local_esp_update"), "error", translate("common.error"));
    return;
  }

  if (!isBinFileName(file.name)) {
    document.getElementById("local-update-note").textContent = translate("maintenance.local_esp_wrong_file");
    announceStatus(translate("maintenance.local_esp_expected"), "error");
    finishButtonFeedback(button, translate("maintenance.local_esp_update"), "error", translate("common.error"));
    return;
  }

  button.disabled = true;
  button.textContent = translate("common.uploading");
  stopUpdateProgressPolling();
  announceStatus(translate("maintenance.local_esp_uploading"), "warn");
  document.getElementById("local-update-note").textContent = translate("maintenance.local_esp_uploading");
  pendingProgressAction = "esp-local-update";
  pendingProgressButtonId = "local-update-esp-submit-button";
  button.dataset.restoreText = translate("maintenance.local_esp_update");
  showRemoteUpdateProgressShell("esp-local-update", translate("maintenance.local_update_preparing"), pendingProgressButtonId);

  try {
    await uploadRawFile(
      buildUploadUrl(getLocalEspUpdateUrl(), file.name),
      file,
      (loaded, total) => {
        const percent = total ? Math.min(100, Math.round((loaded / total) * 100)) : 0;
        button.textContent = translate("common.uploading") + " " + percent + "%";
        document.getElementById("local-update-note").textContent = translateFormat("maintenance.local_esp_upload_progress", { percent });
        document.getElementById("update-progress-note").textContent = document.getElementById("local-update-note").textContent;
        document.getElementById("updated-at").textContent = document.getElementById("update-progress-note").textContent;
      }
    );
  } catch (error) {
    stopUpdateProgressPolling();
    document.getElementById("local-update-note").textContent = translateFormat("maintenance.local_esp_upload_failed", { error: error.message || translate("common.unknown_error") });
    button.disabled = false;
    button.textContent = translate("maintenance.local_esp_update");
    // Wie in failStm32Update: mit 0 ms verschwindet die Meldung im selben Frame, in dem
    // sie gesetzt wird. R2-5.
    finishProgressUi(2200);
    return;
  }

  button.textContent = translate("common.running");
  try {
    await apiFetch(getLocalEspRestartUrl());
  } catch (error) {
  }
  document.getElementById("update-progress-note").textContent = translate("maintenance.local_esp_uploaded_wait");
  announceStatus(translate("maintenance.local_esp_uploaded_wait"), "warn");
  startEspUpdateReconnectWatch(true);
}

async function uploadLocalStm32Update(event) {
  event.preventDefault();

  const fileInput = document.getElementById("local-update-stm32-file-input");
  const button = document.getElementById("local-update-stm32-submit-button");
  const file = fileInput.files && fileInput.files[0];

  if (!file) {
    document.getElementById("local-update-note").textContent = translate("maintenance.local_stm32_choose_first");
    return;
  }

  // Siehe ESP-Pfad: eine leere .hex schreibt einen leeren Flash. Massnahme 7.
  if (!file.size) {
    document.getElementById("local-update-note").textContent = translateFormat("maintenance.file_empty", { file: file.name });
    announceStatus(translate("maintenance.file_empty_status"), "error");
    finishButtonFeedback(button, translate("maintenance.local_stm32_update"), "error", translate("common.error"));
    return;
  }

  if (!isMatchingLocalStm32File(file.name)) {
    const expected = getExpectedLocalStm32Filename(getCurrentUpdateStatus()) || translate("maintenance.local_stm32_expected_fallback");
    document.getElementById("local-update-note").textContent = translateFormat("maintenance.local_stm32_wrong_file", { expected });
    announceStatus(translate("maintenance.local_stm32_expected"), "error");
    finishButtonFeedback(button, translate("maintenance.local_stm32_update"), "error", translate("common.error"));
    return;
  }

  button.disabled = true;
  button.textContent = translate("common.uploading");
  announceStatus(translate("maintenance.local_stm32_uploading"), "warn");
  document.getElementById("local-update-note").textContent = translate("maintenance.local_stm32_uploading");

  try {
    await startStm32StreamingUpload(file, "local-update-stm32-submit-button", translate("maintenance.local_stm32_update"));
  } catch (error) {
    document.getElementById("local-update-note").textContent = translate("maintenance.local_stm32_update_failed");
    button.disabled = false;
    button.textContent = translate("maintenance.local_stm32_update");
    return;
  }
}

const GZIP_UPLOAD_EXTENSIONS = new Set([".html", ".css", ".js", ".json", ".webmanifest", ".svg"]);

function shouldGzipUpload(assetPath) {
  const dot = assetPath.lastIndexOf(".");
  return dot >= 0 && GZIP_UPLOAD_EXTENSIONS.has(assetPath.slice(dot).toLowerCase());
}

async function gzipBlob(blob) {
  const stream = blob.stream().pipeThrough(new CompressionStream("gzip"));
  const compressed = await new Response(stream).arrayBuffer();
  return new Blob([compressed], { type: "application/octet-stream" });
}

function uploadRawFile(url, file, onProgress, onUploadComplete) {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();

    xhr.open("POST", url, true);
    xhr.setRequestHeader("Content-Type", "application/octet-stream");

    bindDomEvent(xhr.upload, "progress", (event) => {
      if (onProgress) {
        onProgress(event.loaded, event.total || file.size);
      }
    });

    bindDomEvent(xhr.upload, "load", () => {
      if (onUploadComplete) {
        onUploadComplete();
      }
    });

    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 300) {
        let payload = {};

        try {
          payload = JSON.parse(xhr.responseText || "{}");
        } catch (error) {
        }

        if (payload.ok === false) {
          const detail = payload.detail ? " (" + payload.detail + ")" : "";
          reject(new Error("Fehlercode " + String(payload.error ?? "-") + detail));
        } else {
          resolve(payload);
        }
      } else {
        reject(new Error("upload failed"));
      }
    };

    xhr.onerror = () => reject(new Error("upload failed"));
    xhr.send(file);
  });
}

function buildUploadUrl(url, fileName, extraParams) {
  const absoluteUrl = new URL(url, window.location.origin);
  absoluteUrl.searchParams.set("filename", fileName || "");
  if (extraParams && typeof extraParams === "object") {
    Object.entries(extraParams).forEach(([key, value]) => {
      if (value === undefined || value === null || value === "") {
        return;
      }
      absoluteUrl.searchParams.set(key, String(value));
    });
  }
  return absoluteUrl.pathname + absoluteUrl.search;
}

function setFsActionStatus(message) {
  const node = document.getElementById("fs-action-status");
  if (node) {
    node.textContent = message;
  }
}

function openLocalAppFolderPicker() {
  const input = document.getElementById("local-app-folder-input");
  if (window.showDirectoryPicker) {
    void openLocalAppDirectoryPicker();
    return;
  }

  if (input) {
    input.value = "";
    input.click();
  }
}

async function collectLocalAppFilesFromDirectoryHandle(handle, prefix) {
  const files = [];
  const currentPrefix = prefix ? prefix + "/" : "";

  for await (const entry of handle.values()) {
    if (entry.kind === "file") {
      files.push({
        relativePath: currentPrefix + entry.name,
        file: await entry.getFile()
      });
      continue;
    }

    if (entry.kind === "directory") {
      const nested = await collectLocalAppFilesFromDirectoryHandle(entry, currentPrefix + entry.name);
      files.push(...nested);
    }
  }

  return files;
}

async function openLocalAppDirectoryPicker() {
  try {
    const handle = await window.showDirectoryPicker({ mode: "read" });
    const fileEntries = await collectLocalAppFilesFromDirectoryHandle(handle, "");
    applyLocalAppFileEntries(fileEntries);
  } catch (error) {
    if (error && error.name === "AbortError") {
      return;
    }

    announceStatus(translate("local_app.folder_read_failed"), "error");
    setFsActionStatus(translate("local_app.folder_browser_failed"));
  }
}

function normalizeLocalAppAssetPath(relativePath) {
  const normalized = String(relativePath || "").replace(/\\/g, "/");

  if (!normalized) {
    return "";
  }

  if (LOCAL_APP_REQUIRED_ASSETS.includes(normalized)) {
    return normalized;
  }

  const trimmed = normalized.replace(/^\.?\/*/, "");

  if (LOCAL_APP_REQUIRED_ASSETS.includes("app/" + trimmed)) {
    return "app/" + trimmed;
  }

  const marker = "/app/";
  const markerIndex = normalized.lastIndexOf(marker);

  if (markerIndex >= 0) {
    const candidate = normalized.slice(markerIndex + 1);
    if (LOCAL_APP_REQUIRED_ASSETS.includes(candidate)) {
      return candidate;
    }
  }

  return "";
}

function readLocalAppFileEntry(entry) {
  return new Promise((resolve, reject) => {
    entry.file(resolve, reject);
  });
}

function readLocalAppDirectoryEntries(reader) {
  return new Promise((resolve, reject) => {
    const collected = [];

    function readBatch() {
      reader.readEntries((entries) => {
        if (!entries || !entries.length) {
          resolve(collected);
          return;
        }

        collected.push(...entries);
        readBatch();
      }, reject);
    }

    readBatch();
  });
}

async function collectLocalAppFilesFromEntry(entry) {
  if (!entry) {
    return [];
  }

  if (entry.isFile) {
    const file = await readLocalAppFileEntry(entry);
    return [{ relativePath: entry.fullPath || file.webkitRelativePath || file.name || "", file }];
  }

  if (!entry.isDirectory) {
    return [];
  }

  const reader = entry.createReader();
  const children = await readLocalAppDirectoryEntries(reader);
  const nested = await Promise.all(children.map((child) => collectLocalAppFilesFromEntry(child)));
  return nested.flat();
}

async function collectLocalAppSelectedFiles(input) {
  const directFiles = Array.from((input && input.files) || []);

  if (directFiles.length) {
    return directFiles.map((file) => ({
      relativePath: file.webkitRelativePath || file.name || "",
      file
    }));
  }

  const entryList = Array.from((input && input.webkitEntries) || []);

  if (!entryList.length) {
    return [];
  }

  const nested = await Promise.all(entryList.map((entry) => collectLocalAppFilesFromEntry(entry)));
  return nested.flat();
}

function applyLocalAppFileEntries(fileEntries) {
  const selectedFiles = new Map();

  fileEntries.forEach(({ relativePath, file }) => {
    const normalizedAssetPath = normalizeLocalAppAssetPath(relativePath);
    if (normalizedAssetPath && !selectedFiles.has(normalizedAssetPath)) {
      selectedFiles.set(normalizedAssetPath, file);
    }
  });

  localAppSelectedFiles = selectedFiles;
  renderLocalAppSelectionStatus();
  setFsActionStatus(
    selectedFiles.size
      ? translateFormat("local_app.folder_checked", { found: selectedFiles.size, total: LOCAL_APP_REQUIRED_ASSETS.length })
      : translate("local_app.folder_none")
  );
}

function renderLocalAppSelectionStatus() {
  const listRoot = document.getElementById("local-app-files-list");
  const note = document.getElementById("local-app-folder-note");
  const installButton = document.getElementById("local-app-install-button");
  const foundCount = LOCAL_APP_REQUIRED_ASSETS.filter((assetPath) => localAppSelectedFiles.has(assetPath)).length;
  const missingAssets = LOCAL_APP_REQUIRED_ASSETS.filter((assetPath) => !localAppSelectedFiles.has(assetPath));
  const infoItems = LOCAL_APP_REQUIRED_ASSETS.map((assetPath) => [
    assetPath,
    localAppSelectedFiles.has(assetPath) ? "gefunden" : "fehlt"
  ]);

  if (listRoot) {
    renderList("local-app-files-list", infoItems);
  }

  if (note) {
    if (!foundCount) {
      note.textContent = translate("local_app.note_empty");
    } else if (!missingAssets.length) {
      note.textContent = translateFormat("local_app.note_complete", {
        found: LOCAL_APP_REQUIRED_ASSETS.length,
        total: LOCAL_APP_REQUIRED_ASSETS.length
      });
    } else {
      note.textContent = translateFormat("local_app.note_missing", {
        found: foundCount,
        total: LOCAL_APP_REQUIRED_ASSETS.length,
        missing: missingAssets.join(", ")
      });
    }
  }

  if (installButton) {
    installButton.disabled = missingAssets.length > 0;
  }
}

async function handleLocalAppFolderSelection(event) {
  const input = event.currentTarget;
  const fileEntries = await collectLocalAppSelectedFiles(input);
  applyLocalAppFileEntries(fileEntries);
}

async function installLocalAppFiles() {
  const button = document.getElementById("local-app-install-button");
  const uploadUrl = getAppFileUploadUrl();
  const missingAssets = LOCAL_APP_REQUIRED_ASSETS.filter((assetPath) => !localAppSelectedFiles.has(assetPath));

  if (missingAssets.length) {
    announceStatus(translate("local_app.incomplete"), "error");
    renderLocalAppSelectionStatus();
    setFsActionStatus(translate("local_app.missing_required"));
    return;
  }

  if (!uploadUrl) {
    announceStatus(translate("local_app.upload_unsupported"), "error");
    return;
  }

  // Eine leere .gz fuehrt auf dem Geraet zum weissen Bildschirm, und der Service Worker
  // haelt sie danach fest — der Fehler ueberlebt jedes Neuladen. Deshalb vor dem ersten
  // Upload pruefen, nicht erst unterwegs: ein Abbruch mitten in der Reihe liesse einen
  // halb beschriebenen Satz App-Dateien zurueck. Massnahme 7.
  const emptyAssets = LOCAL_APP_REQUIRED_ASSETS.filter((assetPath) => {
    const asset = localAppSelectedFiles.get(assetPath);
    return !asset || !asset.size;
  });

  if (emptyAssets.length) {
    announceStatus(translate("local_app.empty_assets"), "error");
    setFsActionStatus(translateFormat("local_app.empty_assets_detail", { assets: emptyAssets.join(", ") }));
    return;
  }

  if (!window.confirm(translate("local_app.install_confirm"))) {
    return;
  }

  setProgressActionContext("app-local-install", "local-app-install-button", translate("local_app.install_button"));
  showRemoteUpdateProgressShell("app-local-install", translate("local_app.installing"), "local-app-install-button");
  beginButtonFeedback(button, "installiert...");
  announceStatus(translate("local_app.installing"), "warn");
  setFsActionStatus(translate("local_app.installing_fs"));

  try {
    for (let index = 0; index < LOCAL_APP_REQUIRED_ASSETS.length; index += 1) {
      const assetPath = LOCAL_APP_REQUIRED_ASSETS[index];
      const file = localAppSelectedFiles.get(assetPath);
      const progressMessage = translateFormat("local_app.progress", {
        step: index + 1,
        total: LOCAL_APP_REQUIRED_ASSETS.length,
        asset: assetPath
      });

      document.getElementById("update-progress-note").textContent = progressMessage;
      document.getElementById("updated-at").textContent = progressMessage;
      setFsActionStatus(progressMessage);

      const isGz = assetPath.endsWith(".gz");
      const serverAssetPath = isGz ? assetPath.slice(0, -3) : assetPath;
      const extraParams = { step: index + 1, total: LOCAL_APP_REQUIRED_ASSETS.length };

      if (isGz) {
        extraParams.encoding = "gzip";
      }

      await uploadRawFile(
        buildUploadUrl(uploadUrl, serverAssetPath, extraParams),
        file,
        () => {
        }
      );
    }

    document.getElementById("update-progress-note").textContent = translate("local_app.installed_reload");
    document.getElementById("updated-at").textContent = translate("local_app.installed_reload");
    announceStatus(translate("local_app.installed_status"), "ok");
    setFsActionStatus(translate("local_app.installed_fs"));
    finishButtonFeedback(button, translate("local_app.install_button"), "success", translate("local_app.installed"));
    window.setTimeout(reloadAppPage, 900);
  } catch (error) {
    const message = translateFormat("local_app.install_failed_detail", { error: error.message || "unknown error" });
    document.getElementById("update-progress-note").textContent = message;
    document.getElementById("updated-at").textContent = message;
    announceStatus(translate("local_app.install_failed"), "error");
    setFsActionStatus(message);
    finishButtonFeedback(button, translate("local_app.install_button"), "error", translate("common.error"));
    resetProgressButton();
    finishProgressUi(1200);
  }
}

async function runConfirmedButtonAction(buttonId, confirmMessage, options) {
  if (confirmMessage && !window.confirm(confirmMessage)) {
    return false;
  }

  await runButtonRequestById(buttonId, options);
  return true;
}

function getUploadActionButtonText(button, fallback) {
  return (button && button.dataset && button.dataset.restoreText) || fallback;
}

async function runManagedRawUpload(options) {
  const {
    button,
    file,
    uploadUrl,
    startStatusText,
    installStatusText,
    successStatusText,
    successAnnounceText,
    idleText,
    successText,
    onProgressText,
    onInstalled,
    onSuccess
  } = options || {};

  if (!button.dataset.restoreText) {
    button.dataset.restoreText = button.textContent;
  }

  beginButtonFeedback(button, translate("common.uploading"));
  setFsActionStatus(startStatusText);
  if (successAnnounceText) {
    announceStatus(startStatusText, "warn");
  }

  await uploadRawFile(
    uploadUrl,
    file,
    (loaded, total) => {
      const percent = total ? Math.min(100, Math.round((loaded / total) * 100)) : 0;
      button.textContent = translateFormat("common.uploading_percent", { percent });
      setFsActionStatus(onProgressText ? onProgressText(percent) : startStatusText);
    },
    () => {
      button.classList.add("is-busy");
      button.textContent = translate("common.installing");
      setFsActionStatus(installStatusText);
      if (typeof onInstalled === "function") {
        onInstalled();
      }
    }
  );

  setFsActionStatus(successStatusText);
  if (successAnnounceText) {
    announceStatus(successAnnounceText, "ok");
  }
  finishButtonFeedback(button, idleText || getUploadActionButtonText(button, translate("common.file_upload")), "success", successText || translate("common.uploaded"));

  if (typeof onSuccess === "function") {
    await onSuccess();
  }
}

function isMatchingFsUploadFile(url, fileName, targetName) {
  if (!fileName || !targetName) {
    return false;
  }

  const uploadPath = new URL(url, window.location.origin).pathname;
  const tablesPath = new URL(getFsUploadUrl("tables"), window.location.origin).pathname;

  if (uploadPath === tablesPath) {
    const prefix = targetName.replace(/local\.txt$/i, "");
    return fileName.startsWith(prefix) && fileName.toLowerCase().endsWith(".txt");
  }

  return fileName === targetName;
}

function isTxtFileName(fileName) {
  const accept = getFsUploadTxtAccept();
  return fileNameMatchesAccept(fileName, accept || ".txt,text/plain");
}

function isHexFileName(fileName) {
  const accept = getLocalStm32UploadAccept();
  return fileNameMatchesAccept(fileName, accept || ".hex,text/plain");
}

function isBinFileName(fileName) {
  const accept = getLocalEspUpdateAccept();
  return fileNameMatchesAccept(fileName, accept || ".bin,application/octet-stream");
}

function isMatchingLocalStm32File(fileName) {
  const expected = getExpectedLocalStm32Filename(getCurrentUpdateStatus());

  if (!isHexFileName(fileName)) {
    return false;
  }

  if (!expected) {
    return true;
  }

  return fileName === expected;
}

function fileNameMatchesAccept(fileName, accept) {
  if (typeof fileName !== "string" || !fileName) {
    return false;
  }

  const normalized = fileName.toLowerCase();
  const tokens = String(accept || "")
    .split(",")
    .map((entry) => entry.trim().toLowerCase())
    .filter(Boolean);

  if (!tokens.length) {
    return true;
  }

  return tokens.some((token) => token.startsWith(".") && normalized.endsWith(token));
}

function buildDeviceReadyProbes() {
  if (hasConfiguredUrl("reconnect_probe_url")) {
    return [
      {
        url: getReconnectProbeUrl(),
        mode: "json",
        validate: (data) => !!(data && data.ok && data.ready)
      }
    ];
  }

  if (hasConfiguredUrl("device_ready_url")) {
    return [
      {
        url: getDeviceReadyUrl(),
        mode: "json",
        validate: (data) => !!(data && data.ok && data.ready)
      }
    ];
  }

  return [
    {
      url: getSettingsUrl(),
      mode: "text",
      validate: (text) => typeof text === "string" && text.indexOf("<numvar") >= 0 && text.indexOf("<strvar") >= 0
    }
  ];
}

const URL_DEFAULTS = {
  settings_url: "/api/settings_xml",
  display_power_url: "/api/display_power",
  update_status_url: "/api/update_status",
  display_power_set_url: "/api/display_power_set",
  display_test_url: "/api/test_display",
  display_brightness_set_url: "/api/display_brightness_set",
  display_it_is_set_url: "/api/display_it_is_set",
  display_mode_set_url: "/api/display_mode_set",
  display_use_rgbw_set_url: "/api/display_use_rgbw_set",
  ticker_set_url: "/api/ticker_set",
  date_ticker_format_set_url: "/api/date_ticker_format_set",
  ticker_deceleration_set_url: "/api/ticker_deceleration_set",
  ambilight_power_url: "/api/ambilight_power",
  ambilight_power_set_url: "/api/ambilight_power_set",
  ambilight_online_set_url: "/api/ambilight_online_set",
  power_status_url: "/api/power_status",
  maintenance_reset_stm32_url: "/api/maintenance_reset_stm32",
  maintenance_reset_eeprom_url: "/api/maintenance_reset_eeprom",
  maintenance_format_fs_url: "/api/maintenance_format_fs",
  auto_brightness_set_url: "/api/auto_brightness_set",
  network_timeserver_set_url: "/api/network_timeserver_set",
  network_client_set_url: "/api/network_client_set",
  network_ap_set_url: "/api/network_ap_set",
  network_timezone_set_url: "/api/network_timezone_set",
  network_summertime_set_url: "/api/network_summertime_set",
  update_host_set_url: "/api/update_host_set",
  update_path_set_url: "/api/update_path_set",
  datetime_set_url: "/api/datetime_set",
  weather_appid_set_url: "/api/weather_appid_set",
  weather_city_set_url: "/api/weather_city_set",
  weather_coordinates_set_url: "/api/weather_coordinates_set",
  weather_get_now_url: "/api/weather_get_now",
  weather_get_forecast_url: "/api/weather_get_forecast",
  learn_ir_url: "/api/learn_ir",
  ir_codes_request_url: "/api/ir_codes_request",
  ir_codes_get_url: "/api/ir_codes_get",
  ir_code_set_url: "/api/ir_code_set",
  network_get_time_url: "/api/network_get_time",
  network_wps_url: "/api/network_wps",
  temperature_display_url: "/api/temperature_display",
  temperature_rtc_correction_set_url: "/api/temperature_rtc_correction_set",
  temperature_ds18xx_correction_set_url: "/api/temperature_ds18xx_correction_set",
  ldr_min_set_url: "/api/ldr_min_set",
  ldr_max_set_url: "/api/ldr_max_set",
  ldr_min_value_set_url: "/api/ldr_min_value_set",
  ldr_max_value_set_url: "/api/ldr_max_value_set",
  animation_mode_set_url: "/api/animation_mode_set",
  color_animation_mode_set_url: "/api/color_animation_mode_set",
  sync_ambilight_set_url: "/api/sync_ambilight_set",
  sync_markers_set_url: "/api/sync_markers_set",
  fade_clock_seconds_set_url: "/api/fade_clock_seconds_set",
  ambilight_markers_set_url: "/api/ambilight_markers_set",
  ambilight_brightness_set_url: "/api/ambilight_brightness_set",
  ambilight_mode_set_url: "/api/ambilight_mode_set",
  ambilight_leds_set_url: "/api/ambilight_leds_set",
  ambilight_offset_set_url: "/api/ambilight_offset_set",
  display_color_set_url: "/api/display_color_set",
  ambilight_color_set_url: "/api/ambilight_color_set",
  marker_color_set_url: "/api/marker_color_set",
  dfplayer_volume_set_url: "/api/dfplayer_volume_set",
  dfplayer_mode_set_url: "/api/dfplayer_mode_set",
  dfplayer_bell_flags_set_url: "/api/dfplayer_bell_flags_set",
  dfplayer_speak_cycle_set_url: "/api/dfplayer_speak_cycle_set",
  dfplayer_silence_start_set_url: "/api/dfplayer_silence_start_set",
  dfplayer_silence_stop_set_url: "/api/dfplayer_silence_stop_set",
  dfplayer_play_url: "/api/dfplayer_play",
  dfplayer_alarm_set_url: "/api/dfplayer_alarm_set",
  overlay_set_url: "/api/overlay_set",
  overlay_display_url: "/api/overlay_display",
  overlay_delete_url: "/api/overlay_delete",
  timer_set_url: "/api/timer_set",
  ambilight_timer_set_url: "/api/ambilight_timer_set",
  device_ready_url: "/api/device_ready",
  reconnect_probe_url: "/api/reconnect_probe",
  remote_esp_update_url: "/update?action=update",
  remote_stm32_flash_url: "/api/remote_stm32_flash",
  update_download_assets_url: "/api/update_download_assets",
  update_download_app_bundle_url: "/api/update_download_app_bundle",
  update_download_table_base_url: "/api/update_download_table?filename=",
  app_file_upload_url: "/api/app_file_upload",
  fs_info_url: "/api/fs_info",
  fs_list_url: "/api/fs_list",
  eeprom_settings_url: "/api/eeprom_settings",
  update_table_files_url: "/api/update_table_files",
  fs_show_base_url: "/api/fs_show?filename=",
  fs_remove_base_url: "/api/fs_remove?filename=",
  fs_upload_icon_url: "/api/fs_upload_icon",
  fs_upload_weather_url: "/api/fs_upload_weather",
  fs_upload_tables_url: "/api/fs_upload_tables",
  fs_upload_display_url: "/api/fs_upload_display",
  fs_upload_txt_accept: ".txt,text/plain",
  local_stm32_upload_url: "/api/local_stm32_upload",
  local_stm32_upload_accept: ".hex,text/plain",
  local_stm32_flash_url: "/api/local_stm32_flash",
  local_esp_update_url: "/api/local_esp_update",
  local_esp_update_accept: ".bin,application/octet-stream",
  local_esp_restart_url: "/api/local_esp_restart",
  stm32_log_url: "/api/stm32_log",
  stm32_log_clear_url: "/api/stm32_log_clear",
  update_progress_url: "/api/update_progress",
  network_scan_url: "/api/network_scan",
  overlay_icons_url: "/api/overlay_icons",
  live_display_color_url: "/api/live_display_color",
  eeprom_settings_set_url: "/api/eeprom_settings_set",
  display_dim_level_set_url: "/api/display_dim_level_set",
  ambilight_dim_level_set_url: "/api/ambilight_dim_level_set",
  animation_profile_set_url: "/api/animation_profile_set",
  animation_profile_default_url: "/api/animation_profile_default",
  color_animation_profile_set_url: "/api/color_animation_profile_set",
  color_animation_profile_default_url: "/api/color_animation_profile_default",
  ambilight_mode_profile_set_url: "/api/ambilight_mode_profile_set",
  ambilight_mode_profile_default_url: "/api/ambilight_mode_profile_default",
  tft_flags_set_url: "/api/tft_flags_set"
};

function getUrlDefault(key) {
  return URL_DEFAULTS[key] || "";
}

function getConfiguredUrlOrDefault(key) {
  return getPreferredUrl(key, getUrlDefault(key));
}

function createConfiguredUrlGetter(key) {
  return function () {
    return getConfiguredUrlOrDefault(key);
  };
}

const getSettingsUrl = createConfiguredUrlGetter("settings_url");
const getDisplayPowerUrl = createConfiguredUrlGetter("display_power_url");
const getUpdateStatusUrl = createConfiguredUrlGetter("update_status_url");
const getDisplayPowerSetUrl = createConfiguredUrlGetter("display_power_set_url");
const getDisplayTestUrl = createConfiguredUrlGetter("display_test_url");
const getDisplayBrightnessSetUrl = createConfiguredUrlGetter("display_brightness_set_url");
const getDisplayItIsSetUrl = createConfiguredUrlGetter("display_it_is_set_url");
const getDisplayModeSetUrl = createConfiguredUrlGetter("display_mode_set_url");
// B12 (L112) -- Entscheidung, geprueft am Quelltext des ESP, nicht geschaetzt.
//
// Vier schreibende Endpunkte werden ausschliesslich aus dem Backup-Import gerufen:
// display_use_rgbw_set, ldr_min_value_set, ldr_max_value_set, eeprom_settings_set.
// Die Frage von L112 ist nicht "gibt es einen Knopf", sondern "kommt der Nutzer
// wieder heraus, wenn ein Import den Wert verstellt hat". Je Endpunkt:
//
//  - ldr_min_value_set / ldr_max_value_set: DER RUECKWEG BESTEHT. Die beiden
//    Messknoepfe "Minimum/Maximum einmessen" (ldr_min_set / ldr_max_set) setzen die
//    Grenzen aus dem aktuellen Rohwert neu. Ein Zahlenfeld daneben waere der zweite,
//    unschaerfere Weg zum selben Ziel -- und er brauchte den Rohwert, der laut L208 (2)
//    der Messung des STM nachhinkt. Bewusst kein Bedienelement.
//
//  - eeprom_settings_set: DER RUECKWEG BESTEHT, ueber zwei getrennte Masken.
//    network_client_set schreibt SSID und Schluessel und loescht dabei
//    EEPROM_FLAG_BOOT_AS_AP, network_ap_set schreibt die AP-Daten und setzt das Flag
//    (http.cpp). Alle fuenf Felder des Sammelsetzers sind damit erreichbar. Ein
//    zusaetzliches Bedienelement waere ein zweiter Schreiber auf dieselben EEPROM-
//    Felder -- genau die Doppelung, die bei C9c6 zum Befund wurde.
//
//  - display_use_rgbw_set: DER RUECKWEG BESTEHT seit L215/B25, ueber
//    #display-use-rgbw-button in der Kachel "Weitere Steuerung" des Moduls "display".
//    Steht DISPLAY_USE_RGBW auf 0, blendet isRgbwUiActive() alle Weisskanal-Regler
//    aus; dieser Schalter bleibt davon unberuehrt, weil applyColorCapabilities() ihn
//    allein an capabilities.whiteChannel haengt. Das Markup hat der ui-developer
//    gelegt (R3), die Anbindung steht hier.
const getDisplayUseRgbwSetUrl = createConfiguredUrlGetter("display_use_rgbw_set_url");
const getTickerSetUrl = createConfiguredUrlGetter("ticker_set_url");
const getDateTickerFormatSetUrl = createConfiguredUrlGetter("date_ticker_format_set_url");
const getTickerDecelerationSetUrl = createConfiguredUrlGetter("ticker_deceleration_set_url");
const getAmbilightPowerUrl = createConfiguredUrlGetter("ambilight_power_url");
const getAmbilightPowerSetUrl = createConfiguredUrlGetter("ambilight_power_set_url");
const getAmbilightOnlineSetUrl = createConfiguredUrlGetter("ambilight_online_set_url");
// B11 (L111): getPowerStatusUrl ist ENTFERNT worden, nicht in Benutzung genommen.
// Er war der einzige tote unter allen Gettern -- null Aufrufstellen, systematisch
// geprueft. Ein Getter ohne Aufrufer ist kein Vorrat, sondern eine Behauptung ueber
// eine Faehigkeit, die niemand prueft.
//
// Benutzen waere das schlechtere von beidem: /api/power_status liefert ausschliesslich
// DISPLAY_POWER und DISPLAY_AMBILIGHT_POWER (http.cpp, http_api_power_status), und
// beide stehen bereits in settings_xml, das die Oberflaeche ohnehin zyklisch liest.
// Ein zweiter Abruf dafuer waere eine zusaetzliche HTTP-Verbindung je Runde -- auf
// einem Geraet, bei dem ab vier parallelen Verbindungen gar keine Antwort mehr kommt
// (L149). Der Schluessel power_status_url bleibt in den Vorgaben stehen: Die Tabelle
// spiegelt die vom ESP veroeffentlichte Endpunktliste, und dort steht er.
const getMaintenanceResetStm32Url = createConfiguredUrlGetter("maintenance_reset_stm32_url");
const getMaintenanceResetEepromUrl = createConfiguredUrlGetter("maintenance_reset_eeprom_url");
const getMaintenanceFormatFsUrl = createConfiguredUrlGetter("maintenance_format_fs_url");
const getAutoBrightnessSetUrl = createConfiguredUrlGetter("auto_brightness_set_url");
const getNetworkTimeserverSetUrl = createConfiguredUrlGetter("network_timeserver_set_url");
const getNetworkClientSetUrl = createConfiguredUrlGetter("network_client_set_url");
const getNetworkApSetUrl = createConfiguredUrlGetter("network_ap_set_url");
const getNetworkTimezoneSetUrl = createConfiguredUrlGetter("network_timezone_set_url");
const getNetworkSummertimeSetUrl = createConfiguredUrlGetter("network_summertime_set_url");
const getUpdateHostSetUrl = createConfiguredUrlGetter("update_host_set_url");
const getUpdatePathSetUrl = createConfiguredUrlGetter("update_path_set_url");
const getDateTimeSetUrl = createConfiguredUrlGetter("datetime_set_url");
const getWeatherAppIdSetUrl = createConfiguredUrlGetter("weather_appid_set_url");
const getWeatherCitySetUrl = createConfiguredUrlGetter("weather_city_set_url");
const getWeatherCoordinatesSetUrl = createConfiguredUrlGetter("weather_coordinates_set_url");
const getWeatherNowUrl = createConfiguredUrlGetter("weather_get_now_url");
const getWeatherForecastUrl = createConfiguredUrlGetter("weather_get_forecast_url");
const getLearnIrUrl = createConfiguredUrlGetter("learn_ir_url");
const getIrCodesRequestUrl = createConfiguredUrlGetter("ir_codes_request_url");
const getIrCodesGetUrl = createConfiguredUrlGetter("ir_codes_get_url");
const getIrCodeSetUrl = createConfiguredUrlGetter("ir_code_set_url");
const getNetworkGetTimeUrl = createConfiguredUrlGetter("network_get_time_url");
const getNetworkWpsUrl = createConfiguredUrlGetter("network_wps_url");
const getTemperatureDisplayUrl = createConfiguredUrlGetter("temperature_display_url");
const getTemperatureRtcCorrectionSetUrl = createConfiguredUrlGetter("temperature_rtc_correction_set_url");
const getTemperatureDs18xxCorrectionSetUrl = createConfiguredUrlGetter("temperature_ds18xx_correction_set_url");
const getLdrMinSetUrl = createConfiguredUrlGetter("ldr_min_set_url");
const getLdrMaxSetUrl = createConfiguredUrlGetter("ldr_max_set_url");
const getLdrMinValueSetUrl = createConfiguredUrlGetter("ldr_min_value_set_url");
const getLdrMaxValueSetUrl = createConfiguredUrlGetter("ldr_max_value_set_url");
const getAnimationModeSetUrl = createConfiguredUrlGetter("animation_mode_set_url");
const getColorAnimationModeSetUrl = createConfiguredUrlGetter("color_animation_mode_set_url");
const getSyncAmbilightSetUrl = createConfiguredUrlGetter("sync_ambilight_set_url");
const getSyncMarkersSetUrl = createConfiguredUrlGetter("sync_markers_set_url");
const getFadeClockSecondsSetUrl = createConfiguredUrlGetter("fade_clock_seconds_set_url");
const getAmbilightMarkersSetUrl = createConfiguredUrlGetter("ambilight_markers_set_url");
const getAmbilightBrightnessSetUrl = createConfiguredUrlGetter("ambilight_brightness_set_url");
const getAmbilightModeSetUrl = createConfiguredUrlGetter("ambilight_mode_set_url");
const getAmbilightLedsSetUrl = createConfiguredUrlGetter("ambilight_leds_set_url");
const getAmbilightOffsetSetUrl = createConfiguredUrlGetter("ambilight_offset_set_url");
const getDisplayColorSetUrl = createConfiguredUrlGetter("display_color_set_url");
const getAmbilightColorSetUrl = createConfiguredUrlGetter("ambilight_color_set_url");
const getMarkerColorSetUrl = createConfiguredUrlGetter("marker_color_set_url");
const getDfplayerVolumeSetUrl = createConfiguredUrlGetter("dfplayer_volume_set_url");
const getDfplayerModeSetUrl = createConfiguredUrlGetter("dfplayer_mode_set_url");
const getDfplayerBellFlagsSetUrl = createConfiguredUrlGetter("dfplayer_bell_flags_set_url");
const getDfplayerSpeakCycleSetUrl = createConfiguredUrlGetter("dfplayer_speak_cycle_set_url");
const getDfplayerSilenceStartSetUrl = createConfiguredUrlGetter("dfplayer_silence_start_set_url");
const getDfplayerSilenceStopSetUrl = createConfiguredUrlGetter("dfplayer_silence_stop_set_url");
const getDfplayerPlayUrl = createConfiguredUrlGetter("dfplayer_play_url");
const getDfplayerAlarmSetUrl = createConfiguredUrlGetter("dfplayer_alarm_set_url");
const getOverlaySetUrl = createConfiguredUrlGetter("overlay_set_url");
const getOverlayDisplayUrl = createConfiguredUrlGetter("overlay_display_url");
const getOverlayDeleteUrl = createConfiguredUrlGetter("overlay_delete_url");
const getTimerSetUrl = createConfiguredUrlGetter("timer_set_url");
const getAmbilightTimerSetUrl = createConfiguredUrlGetter("ambilight_timer_set_url");
const getDeviceReadyUrl = createConfiguredUrlGetter("device_ready_url");
const getReconnectProbeUrl = createConfiguredUrlGetter("reconnect_probe_url");
const getRemoteEspUpdateUrl = createConfiguredUrlGetter("remote_esp_update_url");
const getRemoteStm32FlashUrl = createConfiguredUrlGetter("remote_stm32_flash_url");
const getUpdateDownloadAssetsUrl = createConfiguredUrlGetter("update_download_assets_url");
const getUpdateDownloadTableBaseUrl = createConfiguredUrlGetter("update_download_table_base_url");
const getAppFileUploadUrl = createConfiguredUrlGetter("app_file_upload_url");
const getFsInfoUrl = createConfiguredUrlGetter("fs_info_url");
const getFsListUrl = createConfiguredUrlGetter("fs_list_url");
const getEepromSettingsUrl = createConfiguredUrlGetter("eeprom_settings_url");
const getUpdateTableFilesUrl = createConfiguredUrlGetter("update_table_files_url");
const getFsShowBaseUrl = createConfiguredUrlGetter("fs_show_base_url");
const getFsRemoveBaseUrl = createConfiguredUrlGetter("fs_remove_base_url");

function getFsUploadUrl(kind) {
  const map = {
    icon: "fs_upload_icon_url",
    weather: "fs_upload_weather_url",
    tables: "fs_upload_tables_url",
    display: "fs_upload_display_url"
  };
  const fallback = {
    icon: getUrlDefault("fs_upload_icon_url"),
    weather: getUrlDefault("fs_upload_weather_url"),
    tables: getUrlDefault("fs_upload_tables_url"),
    display: getUrlDefault("fs_upload_display_url")
  };
  return getPreferredUrl(map[kind], fallback[kind] || "");
}

const getFsUploadTxtAccept = createConfiguredUrlGetter("fs_upload_txt_accept");
const getLocalStm32UploadUrl = createConfiguredUrlGetter("local_stm32_upload_url");
const getLocalStm32UploadAccept = createConfiguredUrlGetter("local_stm32_upload_accept");
const getLocalStm32FlashUrl = createConfiguredUrlGetter("local_stm32_flash_url");
const getLocalEspUpdateUrl = createConfiguredUrlGetter("local_esp_update_url");
const getLocalEspUpdateAccept = createConfiguredUrlGetter("local_esp_update_accept");
const getLocalEspRestartUrl = createConfiguredUrlGetter("local_esp_restart_url");
const getStm32LogUrl = createConfiguredUrlGetter("stm32_log_url");
const getStm32LogClearUrl = createConfiguredUrlGetter("stm32_log_clear_url");
const getUpdateProgressUrl = createConfiguredUrlGetter("update_progress_url");
const getNetworkScanUrl = createConfiguredUrlGetter("network_scan_url");
const getOverlayIconsUrl = createConfiguredUrlGetter("overlay_icons_url");
const getLiveDisplayColorUrl = createConfiguredUrlGetter("live_display_color_url");
const getEepromSettingsSetUrl = createConfiguredUrlGetter("eeprom_settings_set_url");
const getDisplayDimLevelSetUrl = createConfiguredUrlGetter("display_dim_level_set_url");
const getAmbilightDimLevelSetUrl = createConfiguredUrlGetter("ambilight_dim_level_set_url");
const getAnimationProfileSetUrl = createConfiguredUrlGetter("animation_profile_set_url");
const getAnimationProfileDefaultUrl = createConfiguredUrlGetter("animation_profile_default_url");
const getColorAnimationProfileSetUrl = createConfiguredUrlGetter("color_animation_profile_set_url");
const getColorAnimationProfileDefaultUrl = createConfiguredUrlGetter("color_animation_profile_default_url");
const getAmbilightModeProfileSetUrl = createConfiguredUrlGetter("ambilight_mode_profile_set_url");
const getAmbilightModeProfileDefaultUrl = createConfiguredUrlGetter("ambilight_mode_profile_default_url");
const getTftFlagsSetUrl = createConfiguredUrlGetter("tft_flags_set_url");

function getNormalizedUpdateStatus(updateStatus) {
  return updateStatus && typeof updateStatus === "object"
    ? updateStatus
    : getCurrentUpdateStatus();
}

function getNormalizedUpdateTableInfo(updateTableInfo) {
  return updateTableInfo && typeof updateTableInfo === "object"
    ? updateTableInfo
    : getCurrentUpdateTableInfo();
}

function setCurrentUpdateStatus(updateStatus) {
  currentUpdateStatus = getNormalizedUpdateStatus(updateStatus);
  return currentUpdateStatus;
}

function getCurrentUpdateStatus() {
  return currentUpdateStatus && typeof currentUpdateStatus === "object" ? currentUpdateStatus : {};
}

function setCurrentUpdateTableInfo(updateTableInfo) {
  currentUpdateTableInfo = getNormalizedUpdateTableInfo(updateTableInfo);
  currentUpdateTableInfoLoaded = true;
  updateLayoutTableWarnings(getCurrentSettingsSnapshot(), currentUpdateTableInfo);
  return currentUpdateTableInfo;
}

function getCurrentUpdateTableInfo() {
  return currentUpdateTableInfo && typeof currentUpdateTableInfo === "object" ? currentUpdateTableInfo : {};
}

function getCurrentUpdateUiMeta() {
  return {
    status: getCurrentUpdateStatus(),
    tableInfo: getCurrentUpdateTableInfo()
  };
}

function setCurrentNetworkInfo(networkInfo) {
  currentNetworkInfo = networkInfo && typeof networkInfo === "object"
    ? networkInfo
    : getCurrentNetworkInfo();
  return currentNetworkInfo;
}

function getCurrentNetworkInfo() {
  return currentNetworkInfo && typeof currentNetworkInfo === "object" ? currentNetworkInfo : {};
}

function setOverlayIconsCache(icons) {
  overlayIconsCache = Array.isArray(icons) ? icons : getOverlayIconsCache();
  return overlayIconsCache;
}

function getOverlayIconsCache() {
  return Array.isArray(overlayIconsCache) ? overlayIconsCache : [];
}

function setCurrentFsFilesFromList(fsList) {
  currentFsFiles = Array.isArray(fsList && fsList.files) ? fsList.files : [];
  return currentFsFiles;
}

function getCurrentFsFiles() {
  return Array.isArray(currentFsFiles) ? currentFsFiles : [];
}

function setCurrentEepromSettings(eepromSettings) {
  if (eepromSettings && eepromSettings.ok) {
    currentEepromSettings = eepromSettings;
  }
  return getCurrentEepromSettings();
}

function getCurrentEepromSettings() {
  return currentEepromSettings || {};
}

function setCurrentSettingsSnapshot(settings) {
  currentSettingsSnapshot = settings && typeof settings === "object" ? settings : getCurrentSettingsSnapshot();
  return getCurrentSettingsSnapshot();
}

function getCurrentSettingsSnapshot() {
  return currentSettingsSnapshot;
}

function captureOverlayEditorState() {
  const cards = Array.from(document.querySelectorAll("#overlay-list .overlay-card"));

  if (!cards.length) {
    return null;
  }

  const items = cards.map((card) => {
    const idx = Number(card.getAttribute("data-overlay-idx"));
    return {
      idx,
      isNew: card.getAttribute("data-overlay-new") === "1",
      state: serializeOverlayDraftState(idx)
    };
  });

  return {
    items,
    hasDraftChanges: items.some((entry) => isOverlayDraftDirty(entry.idx))
  };
}

function restoreOverlayEditorState(savedState) {
  if (!savedState || !Array.isArray(savedState.items)) {
    return false;
  }

  savedState.items.forEach((entry) => {
    let state;
    try {
      state = JSON.parse(entry.state || "{}");
    } catch (_) {
      state = null;
    }

    if (!state || !getOverlayDraftCard(entry.idx)) {
      return;
    }

    const setValue = (prefix, value) => {
      const element = document.getElementById(prefix + entry.idx);
      if (element) {
        element.value = value;
      }
    };

    const activeElement = document.getElementById("ov-active-" + entry.idx);
    if (activeElement) {
      activeElement.checked = !!state.active;
    }
    setValue("ov-type-", state.type);
    setValue("ov-icon-", state.icon);
    setValue("ov-value-", state.value);
    setValue("ov-folder-", state.folder);
    setValue("ov-track-", state.track);
    setValue("ov-interval-", state.interval);
    setValue("ov-duration-", state.duration);
    setValue("ov-datecode-", state.dateCode);
    setValue("ov-month-", state.month);
    setValue("ov-day-", state.day);
    setValue("ov-days-", state.days);
    updateOverlayRowVisibility(entry.idx);
    updateOverlayDraftActions(entry.idx);
  });

  return !!savedState.hasDraftChanges;
}

function refreshNetworkUi(settings) {
  updateNetworkControls(settings, getCurrentNetworkInfo());
}

function refreshOverlayUi(settings) {
  overlayEditorState = captureOverlayEditorState();
  renderOverlayRows(settings);
  restoreOverlayEditorState(overlayEditorState);
}

function refreshMaintenanceUi(settings, fsInfo) {
  updateMaintenanceControls(settings, getCurrentEepromSettings());
  renderFileSystem(fsInfo, getCurrentFsFiles(), settings);
}

function getCurrentLayoutPreview(settings) {
  return currentLayoutPreview || restoreStoredLayoutPreview() || getDefaultLayoutPreview(settings);
}

function setCurrentLayoutPreview(preview) {
  if (preview && Array.isArray(preview.rows) && preview.rows.length) {
    currentLayoutPreview = preview;
    storeLayoutPreview(preview);
  }
  return currentLayoutPreview;
}

function clearCurrentLayoutPreview() {
  currentLayoutPreview = null;
}

function getCurrentLayoutPreviewFile() {
  return currentLayoutPreview && currentLayoutPreview.file ? currentLayoutPreview.file : "";
}

function getCachedLayoutPreview(fileName) {
  return fileName ? layoutPreviewCache[fileName] || null : null;
}

function setCachedLayoutPreview(fileName, preview) {
  if (!fileName || !preview) {
    return;
  }
  layoutPreviewCache[fileName] = preview;
}

function getDefaultLayoutPreviewFile(settings) {
  const resolvedAssetMeta = getResolvedAssetMeta(settings, null, null);
  const resolvedLayoutMeta = getResolvedLayoutMeta(settings, null, null);
  const assetPrefix = resolvedAssetMeta.assetPrefix;

  return resolvedLayoutMeta.previewFile
    || (assetPrefix === "wc24h" ? "wc24h-tables-de.txt" : (assetPrefix === "uc" ? "" : "wc12h-tables-de.txt"))
    || "wc12h-tables-de.txt";
}

async function loadLayoutPreviewRowsMap() {
  if (layoutPreviewRowsMap) {
    return layoutPreviewRowsMap;
  }
  if (layoutPreviewRowsMapPromise) {
    return layoutPreviewRowsMapPromise;
  }

  layoutPreviewRowsMapPromise = fetchWithTimeout(LAYOUT_PREVIEW_ROWS_URL, { cache: "no-store" }, 3500)
    .then((response) => response.ok ? response.json() : {})
    .then((payload) => {
      layoutPreviewRowsMap = payload && typeof payload === "object" ? payload : {};
      return layoutPreviewRowsMap;
    })
    .catch(() => {
      layoutPreviewRowsMap = {};
      return layoutPreviewRowsMap;
    })
    .finally(() => {
      layoutPreviewRowsMapPromise = null;
    });

  return layoutPreviewRowsMapPromise;
}

async function getLayoutPreviewRows(fileName) {
  const normalizedFileName = normalizeLayoutFileName(fileName);
  if (!normalizedFileName) {
    return [];
  }

  if (Array.isArray(DEFAULT_LAYOUT_PREVIEW_ROWS[normalizedFileName])) {
    return DEFAULT_LAYOUT_PREVIEW_ROWS[normalizedFileName].slice();
  }

  const rowsMap = await loadLayoutPreviewRowsMap();
  return Array.isArray(rowsMap[normalizedFileName]) ? rowsMap[normalizedFileName].slice() : [];
}

function getPreviewUiMeta(settings) {
  const mode = Number(settings && settings.numvars ? (settings.numvars[NUM.COLOR_ANIMATION_MODE] || 0) : 0);
  const modeEntry = settings && Array.isArray(settings.coloranims)
    ? settings.coloranims.find((entry) => Number(entry.idx) === mode)
    : null;

  return {
    layoutFile: getCurrentLayoutPreviewFile() || "Fallback",
    staticColor: settings && settings.dspcolors ? settings.dspcolors[0] : null,
    animationLabel: modeEntry ? localizeAnimationName(modeEntry.name || String(mode)) : String(mode)
  };
}

function getWordclockRenderMeta(active, settings, layoutPreview) {
  const preview = layoutPreview || {};
  const rows = Array.isArray(preview.rows) && preview.rows.length ? preview.rows : fallbackWordclockRows;
  const current = settings && settings.tmvars ? settings.tmvars[0] : {};

  return {
    preview,
    rows,
    renderSignature: [
      active ? "1" : "0",
      preview.file || "",
      rows.join("|"),
      String(current.hour || 0),
      String(current.minute || 0),
      String(settings && settings.numvars ? (settings.numvars[NUM.DISPLAY_FLAGS] || 0) : 0),
      String(settings && settings.numvars ? (settings.numvars[NUM.DISPLAY_MODE] || 0) : 0)
    ].join("::")
  };
}

function getLayoutPreviewMeta(settings, layoutPreview) {
  const config = settings && settings.numvars ? (settings.numvars[NUM.HARDWARE_CONFIGURATION] || 0) : 0;
  const current = settings && settings.tmvars ? settings.tmvars[0] : {};
  const minute = Number(current.minute || 0);
  const displayPower = Number(settings && settings.numvars ? (settings.numvars[NUM.DISPLAY_POWER] || 0) : 0);
  const is12hLayout = isTwelveHourLayout(layoutPreview, config);

  return {
    config,
    minute,
    displayPower,
    is12hLayout,
    activeCornerCount: is12hLayout && displayPower ? (minute % 5) : 0
  };
}

function getPreferredUrl(key, fallback) {
  const status = getNormalizedUpdateStatus();
  const value = status[key];
  return typeof value === "string" && value ? value : fallback;
}

function hasConfiguredUrl(key) {
  const status = getNormalizedUpdateStatus();
  return typeof status[key] === "string" && !!status[key];
}

function getStatusString(key) {
  const status = getNormalizedUpdateStatus();
  return typeof status[key] === "string" ? status[key] : "";
}

function getStatusBoolean(key) {
  const status = getNormalizedUpdateStatus();
  return typeof status[key] === "boolean" ? status[key] : null;
}

function getStatusNumber(key) {
  const status = getNormalizedUpdateStatus();
  return typeof status[key] === "number" && Number.isFinite(status[key]) ? status[key] : 0;
}

function getStatusMeta() {
  const asset = {
    assetPrefix: getStatusString("asset_prefix").trim(),
    tablesFamilyPrefix: getStatusString("fs_upload_tables_prefix").trim(),
    iconTarget: getStatusString("fs_upload_icon_target"),
    weatherTarget: getStatusString("fs_upload_weather_target"),
    tablesTarget: getStatusString("fs_upload_tables_target"),
    displayTarget: getStatusString("fs_upload_display_target")
  };
  const layout = {
    previewFile: getStatusString("default_layout_preview_file"),
    columns: getStatusNumber("default_layout_columns"),
    isTwelveHour: getStatusBoolean("layout_is_12h")
  };
  const labels = {
    hardware: getStatusString("hardware_label"),
    processor: getStatusString("processor_label"),
    board: getStatusString("board_label"),
    frequency: getStatusString("frequency_label"),
    oscillator: getStatusString("oscillator_label"),
    display: getStatusString("display_label")
  };
  const hardware = {
    labels: Object.values(labels).some(Boolean) ? labels : null,
    display: (() => {
      const mode = getStatusString("display_led_mode");
      const hasTft = getStatusBoolean("display_has_tft");
      const hasWhiteChannel = getStatusBoolean("display_has_white_channel");
      const label = getStatusString("display_label");

      if (!mode && hasTft === null && hasWhiteChannel === null && !label) {
        return null;
      }

      return {
        mode,
        hasTft: !!hasTft,
        hasWhiteChannel: !!hasWhiteChannel,
        label
      };
    })()
  };

  return { asset, layout, hardware };
}

function getResolvedStatusMeta(settings, backupAssets, updateTableInfo) {
  const statusMeta = getStatusMeta();
  const config = settings && settings.numvars ? Number(settings.numvars[NUM.HARDWARE_CONFIGURATION] || 0) : 0;
  const fallbackTargets = getFsUploadTargets(config);
  const normalizedLayoutTable = normalizeFsFileName(
    getUpdateTableCurrentFile(updateTableInfo)
      || (backupAssets && backupAssets.layout_table)
      || ""
  );
  const prefixFromBackup = String(backupAssets && backupAssets.asset_prefix ? backupAssets.asset_prefix : "").trim();
  let assetPrefix = statusMeta.asset.assetPrefix || prefixFromBackup;

  if (!assetPrefix) {
    if (normalizedLayoutTable.indexOf("wc24h-") === 0) {
      assetPrefix = "wc24h";
    } else if (normalizedLayoutTable.indexOf("wc12h-") === 0) {
      assetPrefix = "wc12h";
    } else if (normalizedLayoutTable.indexOf("uc-") === 0) {
      assetPrefix = "uc";
    } else {
      assetPrefix = getAssetPrefixFromHardwareConfig(config);
    }
  }

  const targets = {
    icon: statusMeta.asset.iconTarget || fallbackTargets.icon,
    weather: statusMeta.asset.weatherTarget || fallbackTargets.weather,
    tables: statusMeta.asset.tablesTarget || fallbackTargets.tables,
    display: statusMeta.asset.displayTarget || fallbackTargets.display
  };
  const tablesFamilyPrefix = statusMeta.asset.tablesFamilyPrefix
    || (targets.tables ? targets.tables.replace(/local\.txt$/i, "") : "");
  const overlayAssetFiles = assetPrefix ? {
    iconFile: assetPrefix + "-icon.txt",
    weatherFile: assetPrefix + "-weather.txt"
  } : {
    iconFile: targets.icon || "",
    weatherFile: targets.weather || ""
  };
  const currentTable = getUpdateTableCurrentFile(updateTableInfo);

  return {
    asset: {
      assetPrefix,
      tablesFamilyPrefix,
      currentTable,
      targets,
      overlayAssetFiles
    },
    layout: {
      previewFile: statusMeta.layout.previewFile,
      columns: statusMeta.layout.columns,
      isTwelveHour: statusMeta.layout.isTwelveHour
    },
    hardware: statusMeta.hardware
  };
}

function getResolvedAssetMeta(settings, backupAssets, updateTableInfo) {
  return getResolvedStatusMeta(settings, backupAssets, updateTableInfo).asset;
}

function getResolvedLayoutMeta(settings, backupAssets, updateTableInfo) {
  return getResolvedStatusMeta(settings, backupAssets, updateTableInfo).layout;
}

function getResolvedHardwareMeta(settings, backupAssets, updateTableInfo) {
  return getResolvedStatusMeta(settings, backupAssets, updateTableInfo).hardware;
}

function getFsUploadMeta(settings) {
  const resolvedAssetMeta = getResolvedAssetMeta(settings, null, null);

  return {
    targets: resolvedAssetMeta.targets,
    targetUploadsSupported: !!(
      getFsUploadUrl("icon") ||
      getFsUploadUrl("weather") ||
      getFsUploadUrl("tables") ||
      getFsUploadUrl("display")
    )
  };
}

function getBackupAssetMeta(settings, backupAssets, updateTableInfo) {
  const resolvedAssetMeta = getResolvedAssetMeta(settings, backupAssets || null, updateTableInfo);
  return {
    currentTable: resolvedAssetMeta.currentTable,
    assetPrefix: resolvedAssetMeta.assetPrefix
  };
}

function getUpdateStatusBoolean(updateStatus, key) {
  const status = getNormalizedUpdateStatus(updateStatus);
  return typeof status[key] === "boolean" ? status[key] : null;
}

function getUpdateStatusNumber(updateStatus, key) {
  const status = getNormalizedUpdateStatus(updateStatus);
  return typeof status[key] === "number" && Number.isFinite(status[key]) ? status[key] : 0;
}

function getUpdateStatusString(updateStatus, key) {
  const status = getNormalizedUpdateStatus(updateStatus);
  return typeof status[key] === "string" ? status[key] : "";
}

function getUpdateStatusArray(updateStatus, key) {
  const status = getNormalizedUpdateStatus(updateStatus);
  return Array.isArray(status[key]) ? status[key] : [];
}

function getUpdateTableInfoString(updateTableInfo, key) {
  const info = getNormalizedUpdateTableInfo(updateTableInfo);
  return typeof info[key] === "string" ? info[key] : "";
}

function getUpdateTableInfoArray(updateTableInfo, key) {
  const info = getNormalizedUpdateTableInfo(updateTableInfo);
  return Array.isArray(info[key]) ? info[key] : [];
}

function getUpdateTableCurrentFile(updateTableInfo) {
  return getUpdateTableInfoString(updateTableInfo, "current_table");
}

function getUpdateTableFilesList(updateTableInfo) {
  return getUpdateTableInfoArray(updateTableInfo, "table_files");
}

function getLayoutTableWarningFileName(settings, updateTableInfo) {
  const assetMeta = getResolvedAssetMeta(settings || getCurrentSettingsSnapshot(), null, updateTableInfo || getCurrentUpdateTableInfo());
  const prefix = String(assetMeta && assetMeta.assetPrefix ? assetMeta.assetPrefix : "").trim().toLowerCase();
  return prefix ? (prefix + "-tables-xx.txt") : "wcxx-tables-xx.txt";
}

function getLayoutTableWarningMessage(settings, updateTableInfo) {
  return "Bitte die Layout-Tabelle " + getLayoutTableWarningFileName(settings, updateTableInfo) + " auf LittleFS installieren.";
}

function updateLayoutTableWarnings(settings, updateTableInfo) {
  const infoLoaded = currentUpdateTableInfoLoaded || !!(updateTableInfo && typeof updateTableInfo === "object" && Object.keys(updateTableInfo).length);
  const element = document.getElementById("layout-table-warning-global");
  if (!element) {
    return;
  }

  if (!infoLoaded) {
    element.classList.add("is-hidden");
    element.textContent = "";
    return;
  }

  const hasLayoutTable = !!normalizeFsFileName(getUpdateTableCurrentFile(updateTableInfo || getCurrentUpdateTableInfo()));
  const message = hasLayoutTable ? "" : getLayoutTableWarningMessage(settings, updateTableInfo);

  element.classList.toggle("is-hidden", hasLayoutTable);
  element.textContent = hasLayoutTable ? "" : message;
}

function canOtaUpdate(updateStatus) {
  return getUpdateStatusBoolean(updateStatus, "can_update");
}

function isLocalUpdateSupported(updateStatus) {
  const value = getUpdateStatusBoolean(updateStatus, "local_update_supported");
  return value === null ? true : value;
}

function getLocalUpdateMessage(updateStatus) {
  return getUpdateStatusString(updateStatus, "local_update_message");
}

function getUpdateFlashSize(updateStatus) {
  return getUpdateStatusNumber(updateStatus, "flash_size");
}

function getUpdateAvailableVersion(updateStatus, key) {
  return getUpdateStatusString(updateStatus, key);
}

function getUpdateReleaseNotes(updateStatus) {
  return getUpdateAvailableVersion(updateStatus, "release_notes");
}

function getUpdateStm32Default(updateStatus) {
  return getUpdateAvailableVersion(updateStatus, "stm32_default");
}

function getUpdateStm32Files(updateStatus) {
  return getUpdateStatusArray(updateStatus, "stm32_files");
}

function areUpdateAssetsAvailable(updateStatus) {
  return getUpdateStatusBoolean(updateStatus, "assets_available");
}

function isUpdateAppFilesAvailable(updateStatus) {
  return !!(
    getUpdateAvailableVersion(updateStatus, "app_available") ||
    getUpdateAvailableVersion(updateStatus, "app_version")
  );
}

function getExpectedLocalStm32Filename(updateStatus) {
  const explicit = getUpdateStatusString(updateStatus, "local_stm32_expected_filename");
  const fallback = getUpdateStm32Default(updateStatus);
  return explicit || fallback || "";
}

function getLocalUpdateControlMeta(updateStatus) {
  return {
    supported: isLocalUpdateSupported(updateStatus),
    message: getLocalUpdateMessage(updateStatus),
    localEspSupported: !!getLocalEspUpdateUrl(),
    localStm32Supported: !!getLocalStm32UploadUrl()
  };
}

function getRemoteUpdateUrlMeta() {
  return {
    espUrl: getRemoteEspUpdateUrl(),
    stm32ApiUrl: getRemoteStm32FlashUrl()
  };
}

function getRemoteUpdateSupportMeta(updateStatus) {
  return {
    espApiSupported: !!getUpdateStatusBoolean(updateStatus, "remote_esp_update_api_supported"),
    stm32ApiSupported: !!getUpdateStatusBoolean(updateStatus, "remote_stm32_flash_api_supported"),
    urls: getRemoteUpdateUrlMeta()
  };
}

function getRemoteUpdateControlMeta(updateStatus) {
  const supportMeta = getRemoteUpdateSupportMeta(updateStatus);

  return {
    esp: {
      apiSupported: supportMeta.espApiSupported,
      url: supportMeta.urls.espUrl || getUrlDefault("remote_esp_update_url"),
      canStart: !!(supportMeta.urls.espUrl || getUrlDefault("remote_esp_update_url"))
    },
    stm32: {
      apiSupported: supportMeta.stm32ApiSupported,
      url: supportMeta.urls.stm32ApiUrl || getUrlDefault("remote_stm32_flash_url"),
      canStart: !!(supportMeta.urls.stm32ApiUrl || getUrlDefault("remote_stm32_flash_url"))
    }
  };
}

function getUpdateModuleMeta(updateStatus, updateTableInfo, settings) {
  return {
    summary: getUpdateSummaryMeta(updateStatus, settings || parseSettings("")),
    serverFiles: getUpdateServerFilesMeta(updateStatus, updateTableInfo),
    localUpdate: getLocalUpdateControlMeta(updateStatus),
    remoteUpdate: getRemoteUpdateControlMeta(updateStatus)
  };
}

function getUpdateVersionUiMeta(updateStatus, settings) {
  const displayMeta = getDisplayBackupMeta(settings || parseSettings(""));
  const canUpdate = canOtaUpdate(updateStatus);

  return {
    flashSize: getUpdateFlashSize(updateStatus),
    canUpdate,
    wcVersion: getUpdateAvailableVersion(updateStatus, "wc_version") || displayMeta.firmwareVersion || "-",
    wcAvailable: getUpdateAvailableVersion(updateStatus, "wc_available") || "-",
    espVersion: getUpdateAvailableVersion(updateStatus, "esp_version") || displayMeta.espVersion || "-",
    espAvailable: getUpdateAvailableVersion(updateStatus, "esp_available") || "-",
    appVersion: APP_VERSION,
    appAvailable: getUpdateAvailableVersion(updateStatus, "app_available") || "-"
  };
}

function getUpdateSummaryMeta(updateStatus, settings) {
  const versionMeta = getUpdateVersionUiMeta(updateStatus, settings);
  const stm32Default = getUpdateStm32Default(updateStatus);
  const stm32Files = getUpdateStm32Files(updateStatus);
  const releaseNotes = getUpdateReleaseNotes(updateStatus);

  return {
    canUpdate: versionMeta.canUpdate,
    stm32Default,
    stm32Files,
    releaseNotes,
    items: [
      [translate("maintenance.version_flash"), versionMeta.flashSize ? formatBytes(versionMeta.flashSize) : "-"],
      [translate("maintenance.version_ota"), versionMeta.canUpdate ? translate("maintenance.version_available_yes") : translate("maintenance.version_available_no")],
      [translate("maintenance.version_wc"), versionMeta.wcVersion],
      [translate("maintenance.version_wc_available"), versionMeta.wcAvailable],
      [translate("maintenance.version_esp"), versionMeta.espVersion],
      [translate("maintenance.version_esp_available"), versionMeta.espAvailable],
      [translate("maintenance.version_app"), versionMeta.appVersion],
      [translate("maintenance.version_app_available"), versionMeta.appAvailable],
      [translate("maintenance.version_stm32_default"), stm32Default || "-"]
    ]
  };
}

function getUpdateServerFilesMeta(updateStatus, updateTableInfo) {
  const tableFiles = getUpdateTableFilesList(updateTableInfo);
  const currentTable = getUpdateTableCurrentFile(updateTableInfo);
  const tableActionSupported = !!getUpdateDownloadTableBaseUrl();
  const assetsActionSupported = !!getUpdateDownloadAssetsUrl();
  const appFilesActionSupported = true;

  return {
    tableFiles,
    currentTable,
    tableAvailable: tableFiles.length > 0,
    assetsAvailable: areUpdateAssetsAvailable(updateStatus),
    appFilesAvailable: isUpdateAppFilesAvailable(updateStatus),
    tableActionSupported,
    assetsActionSupported,
    appFilesActionSupported,
    anyActionSupported: tableActionSupported || assetsActionSupported || appFilesActionSupported
  };
}

function getDefaultReconnectProbes() {
  if (getReconnectProbeUrl()) {
    return [
      { url: getReconnectProbeUrl(), mode: "json", validate: (data) => !!(data && data.ok && data.ready) }
    ];
  }

  return [
    { url: getDisplayPowerUrl(), mode: "response" },
    { url: getSettingsUrl(), mode: "response" }
  ];
}

function normalizeUrlPath(url) {
  if (typeof url !== "string" || !url) {
    return "";
  }

  try {
    return new URL(url, window.location.origin).pathname;
  } catch (error) {
    console.warn("normalizeUrlPath: URL not parsable", url, error);
    return url.split("#")[0].split("?")[0];
  }
}

function isTablesUploadUrl(url) {
  const path = normalizeUrlPath(url);
  const tablesPath = normalizeUrlPath(getFsUploadUrl("tables"));
  return !!path && !!tablesPath && path === tablesPath;
}

async function downloadUpdateAssets() {
  await runConfirmedButtonAction(
    "update-assets-button",
    translate("maintenance.assets_confirm"),
    {
      busyText: translate("common.running"),
      idleText: translate("maintenance.load_icon_files"),
      successText: translate("common.ready"),
      errorText: translate("maintenance.assets_load_failed"),
      successStatusText: translate("maintenance.assets_loaded"),
      reloadDelayMs: 1200,
      request: () => apiFetch(getUpdateDownloadAssetsUrl())
    }
  );
}

async function downloadUpdateAppFiles() {
  if (!window.confirm(translate("maintenance.app_install_confirm"))) {
    return;
  }

  const button = document.getElementById("update-app-files-button");

  setProgressActionContext("app-file-install", "update-app-files-button", translate("maintenance.app_install_button"));
  showRemoteUpdateProgressShell("app-file-install", translate("maintenance.app_install_loading"), "update-app-files-button");
  beginButtonFeedback(button, translate("common.loading"));
  announceStatus(translate("maintenance.app_install_loading"), "warn");

  try {
    const response = await fetchWithTimeout("/app/?action=install", { cache: "no-store" }, 45000);

    if (!response.ok) {
      throw new Error("http-" + response.status);
    }

    button.classList.add("is-busy");
    button.textContent = translate("common.running");
    document.getElementById("update-progress-note").textContent = translate("maintenance.app_install_loaded");
    document.getElementById("updated-at").textContent = translate("maintenance.app_install_loaded");
    announceStatus(translate("maintenance.app_install_running"), "warn");
    await sleep(250);

    button.classList.add("is-busy");
    button.textContent = translate("common.reloading");
    document.getElementById("update-progress-note").textContent = translate("maintenance.app_install_reload");
    document.getElementById("updated-at").textContent = translate("maintenance.app_install_reload");
    announceStatus(translate("maintenance.app_install_success"), "ok");
    setTimeout(reloadAppPage, 900);
  } catch (error) {
    document.getElementById("update-progress-note").textContent = translate("maintenance.app_install_failed_detail");
    announceStatus(translate("maintenance.app_install_failed"), "error");
    finishButtonFeedback(button, translate("maintenance.app_install_button"), "error", translate("common.error"));
    resetProgressButton();
    finishProgressUi(1200);
  }
}

async function uploadFsTargetFile(event, url, successMessage) {
  event.preventDefault();

  const form = event.currentTarget;
  const label = form.querySelector(".label");
  const fileInput = form.querySelector('input[type="file"]');
  const button = form.querySelector('button[type="submit"]');
  const file = fileInput && fileInput.files && fileInput.files[0];
  const targetName = label ? label.textContent : translate("common.file");

  if (!file) {
    document.getElementById("fs-action-status").textContent = translateFormat("maintenance.choose_file_for_target", { target: targetName });
    return;
  }

  // Eine 0-Byte-Datei nimmt der ESP anstandslos an und ersetzt damit eine
  // funktionierende Datei durch nichts. Massnahme 7.
  if (!file.size) {
    document.getElementById("fs-action-status").textContent = translateFormat("maintenance.file_empty", { file: file.name });
    announceStatus(translate("maintenance.file_empty_status"), "error");
    finishButtonFeedback(button, button.dataset.restoreText || translate("common.file_upload"), "error", translate("common.error"));
    return;
  }

  if (!isMatchingFsUploadFile(url, file.name, targetName)) {
    document.getElementById("fs-action-status").textContent =
      isTablesUploadUrl(url)
        ? translateFormat("maintenance.file_expected_pattern", { target: targetName, pattern: targetName.replace("local.txt", "*.txt") })
        : translateFormat("maintenance.file_expected_exact", { target: targetName });
    announceStatus(translateFormat("maintenance.target_expected", { target: targetName }), "error");
    finishButtonFeedback(button, button.dataset.restoreText || translate("common.file_upload"), "error", translate("common.error"));
    return;
  }

  if (!isTxtFileName(file.name)) {
    document.getElementById("fs-action-status").textContent = translateFormat("maintenance.txt_required", { target: targetName });
    announceStatus(translate("common.invalid_file_extension"), "error");
    finishButtonFeedback(button, button.dataset.restoreText || translate("common.file_upload"), "error", translate("common.error"));
    return;
  }

  try {
    await runManagedRawUpload({
      button,
      file,
      uploadUrl: buildUploadUrl(url, file.name),
      startStatusText: translateFormat("maintenance.target_uploading", { target: targetName, name: file.name }),
      installStatusText: translateFormat("maintenance.target_uploaded_saving", { target: targetName }),
      successStatusText: successMessage,
      successAnnounceText: successMessage,
      idleText: translate("common.file_upload"),
      successText: translate("common.uploaded"),
      onProgressText: (percent) => translateFormat("maintenance.target_upload_progress", { target: targetName, percent }),
      onSuccess: async () => {
        await loadData();
      }
    });
  } catch (error) {
    setFsActionStatus(translateFormat("maintenance.file_upload_failed_detail", { target: targetName, error: error.message || "unknown error" }));
    announceStatus(translateFormat("maintenance.file_upload_failed", { target: targetName }), "error");
    finishButtonFeedback(button, getUploadActionButtonText(button, translate("common.file_upload")), "error", translate("common.error"));
  }
}

function triggerEspUpdate() {
  const remoteUpdate = getRemoteUpdateActionMeta("esp");

  if (!window.confirm(remoteUpdate.confirmText)) {
    return;
  }

  startRemoteUpdateAction(remoteUpdate);
  startEspUpdateReconnectWatch(false);
}

function startEspUpdateReconnectWatch(isLocalUpdate) {
  waitForDeviceReady(isLocalUpdate ? 90000 : 120000, isLocalUpdate ? 3000 : 1500, translate("maintenance.esp_ready_reload"), true, {
    forcedReloadAfterMs: 90000,
    reloadWatchdogDelayMs: 95000,
    requireReconnectCycle: true,
    requiredStableSuccesses: 2,
    probes: buildDeviceReadyProbes(),
    waitingMessage: isLocalUpdate
      ? translate("maintenance.local_esp_waiting")
      : translate("maintenance.remote_esp_waiting")
  });
}

function triggerStm32Update() {
  const remoteUpdate = getRemoteUpdateActionMeta("stm32");
  const fileName = document.getElementById("update-stm32-select").value || "";

  if (!fileName) {
    announceStatus(translate("maintenance.stm32_file_choose_first"), "warn");
    return;
  }

  if (!window.confirm(remoteUpdate.confirmText(fileName))) {
    return;
  }

  startRemoteUpdateAction(remoteUpdate, fileName);
}

function getRemoteUpdateActionMeta(kind) {
  const remoteUpdateMeta = getRemoteUpdateControlMeta(getCurrentUpdateStatus());

  if (kind === "esp") {
    return {
      kind: "esp",
      actionType: "esp-update",
      buttonId: "update-esp-button",
      buttonText: translate("maintenance.update_esp"),
      confirmText: translate("maintenance.update_esp_confirm"),
      startMessage: translate("maintenance.update_esp_start"),
      useProgressFormSubmit: true,
      keepFrameActiveInBackground: true,
      buildUrl: () => remoteUpdateMeta.esp.url
    };
  }

  return {
    kind: "stm32",
    actionType: "stm32-flash",
      buttonId: "update-stm32-button",
      buttonText: translate("maintenance.flash_stm32"),
      confirmText: (fileName) => translateFormat("maintenance.flash_stm32_confirm", { file: fileName }),
      startMessage: translate("maintenance.flash_stm32_started"),
      buildUrl: (fileName) => remoteUpdateMeta.stm32.url + "?filename=" + encodeURIComponent(fileName) + "&stream=1"
    };
}

function triggerTableUpdate() {
  const fileName = document.getElementById("update-table-select").value || "";

  if (!fileName) {
    announceStatus(translate("maintenance.layout_choose_first"), "warn");
    return;
  }

  if (!window.confirm(translateFormat("maintenance.layout_confirm", { file: fileName }))) {
    return;
  }

  const button = document.getElementById("update-table-button");
  beginButtonFeedback(button, translate("common.loading"));
  document.getElementById("updated-at").textContent = translate("maintenance.layout_loading");
  announceStatus(translate("maintenance.layout_loading"), "warn");

  apiFetch(getUpdateDownloadTableBaseUrl() + encodeURIComponent(fileName))
    .then(async () => {
      announceStatus(translate("maintenance.layout_loaded"), "ok");
      finishButtonFeedback(button, translate("maintenance.layout_button"), "success", translate("common.loaded"));
      await loadData();
    })
    .catch(() => {
      announceStatus(translate("maintenance.layout_load_failed"), "error");
      finishButtonFeedback(button, translate("maintenance.layout_button"), "error", translate("common.error"));
    });
}

async function resetStm32() {
  await runConfirmedButtonAction(
    "maintenance-reset-stm32-button",
    translate("maintenance.reset_stm32_confirm"),
    {
      busyText: translate("common.running"),
      idleText: translate("maintenance.reset_stm32"),
      successText: translate("common.ready"),
      errorText: translate("maintenance.reset_stm32_failed"),
      successStatusText: translate("maintenance.reset_stm32_started_wait"),
      request: async () => {
        await apiFetch(getMaintenanceResetStm32Url());
        await sleep(4000);
        announceStatus(translate("maintenance.reset_stm32_ok"), "ok");
        await loadData();
      }
    }
  );
}

async function waitForStm32ResetAndReload(timeoutMs, initialDelayMs) {
  const deadline = Date.now() + (timeoutMs || 30000);
  let reconnectObserved = false;
  let stableSuccessCount = 0;

  await sleep(initialDelayMs || 0);

  while (Date.now() < deadline) {
    try {
      const response = await fetchWithTimeout(getSettingsUrl() + "?_ts=" + Date.now(), { cache: "no-store" }, 1500);
      const text = response.ok ? await response.text() : "";
      const ready = text.indexOf("<numvar") >= 0 && text.indexOf("<strvar") >= 0;

      if (ready) {
        if (reconnectObserved) {
          stableSuccessCount += 1;
          if (stableSuccessCount >= 2) {
            announceStatus(translate("maintenance.reset_stm32_reconnect"), "ok");
            await sleep(300);
            await reloadAppPage();
            return;
          }
        }
      } else {
        reconnectObserved = true;
        stableSuccessCount = 0;
      }
    } catch (error) {
      reconnectObserved = true;
      stableSuccessCount = 0;
    }

    await sleep(1500);
  }

  announceStatus(translate("maintenance.reset_stm32_reconnect_unclear"), "warn");
  await sleep(300);
  await reloadAppPage();
}

async function resetEeprom() {
  if (!window.confirm(translate("maintenance.reset_eeprom_confirm_1"))) {
    return;
  }

  if (!window.confirm(translate("maintenance.reset_eeprom_confirm_2"))) {
    return;
  }

  const maintenanceButton = document.getElementById("maintenance-reset-eeprom-button");

  beginButtonFeedback(maintenanceButton, translate("maintenance.reset_eeprom_busy"));
  announceStatus(translate("maintenance.reset_eeprom_start"), "warn");

  try {
    await apiFetch(getMaintenanceResetEepromUrl());
    announceStatus(translate("maintenance.reset_eeprom_restart"), "warn");
    maintenanceButton.textContent = translate("maintenance.reset_eeprom_wait");
    await sleep(250);
    await apiFetch(getMaintenanceResetStm32Url());
    announceStatus(translate("maintenance.reset_eeprom_reload"), "warn");
    await waitForStm32ResetAndReload(30000, 1500);
  } catch (error) {
    announceStatus(translate("maintenance.reset_eeprom_failed"), "error");
    finishButtonFeedback(maintenanceButton, translate("maintenance.reset_eeprom"), "error", translate("common.error"));
  }
}

async function formatLittleFs() {
  await runConfirmedButtonAction(
    "maintenance-format-fs-button",
    translate("maintenance.format_fs_confirm"),
    {
      busyText: translate("common.running"),
      idleText: translate("maintenance.format_fs_button"),
      successText: translate("common.ready"),
      errorText: translate("maintenance.format_fs_failed"),
      successStatusText: translate("maintenance.format_fs_done"),
      reloadDelayMs: 1200,
      request: () => apiFetch(getMaintenanceFormatFsUrl())
    }
  );
}

async function formatLittleFsFromFiles() {
  const didRun = await runConfirmedButtonAction(
    "files-format-fs-button",
    translate("maintenance.format_fs_confirm"),
    {
      busyText: translate("common.running"),
      idleText: translate("maintenance.format_fs_button"),
      successText: translate("common.ready"),
      errorText: translate("maintenance.format_fs_failed"),
      successStatusText: translate("maintenance.format_fs_done"),
      reloadDelayMs: 1200,
      request: () => apiFetch(getMaintenanceFormatFsUrl())
    }
  );
  if (didRun) {
    setFsActionStatus(translate("maintenance.format_fs_done"));
  }
}

async function showFsFile(fileName) {
  if (!fileName) {
    return;
  }

  try {
    const response = await apiFetch(getFsShowBaseUrl() + encodeURIComponent(fileName));
    const text = await response.text();
    document.getElementById("fs-preview-content").textContent = text || translate("maintenance.preview_empty");
    setFsActionStatus(translateFormat("maintenance.fs_showing", { file: fileName }));
    announceStatus(fileName + " " + translate("common.loaded"), "ok");
  } catch (error) {
    announceStatus(translate("maintenance.file_load_failed"), "error");
  }
}

async function deleteFsFile(fileName) {
  if (!fileName) {
    return;
  }

  if (!window.confirm(translateFormat("maintenance.fs_delete_confirm", { file: fileName }))) {
    return;
  }

  try {
    await apiFetch(getFsRemoveBaseUrl() + encodeURIComponent(fileName));
    document.getElementById("fs-preview-content").textContent = translate("maintenance.preview_placeholder");
    setFsActionStatus(translateFormat("maintenance.fs_deleted", { file: fileName }));
    await loadData();
  } catch (error) {
    announceStatus(translate("maintenance.file_delete_failed"), "error");
  }
}

function handleProgressFrameLoad() {
  const note = document.getElementById("update-progress-note");

  try {
    if (pendingProgressAction === "esp-update") {
      note.textContent = translate("maintenance.esp_update_waiting");
      return;
    }

    if (pendingProgressAction === "stm32-flash" && !stm32AutoResetStarted) {
      const text = readUpdateProgressFrameText();
      const normalized = (text || "").replace(/\s+/g, " ").trim();

      if (normalized.indexOf("Flash failed") >= 0 || normalized.indexOf("Check failed") >= 0 || normalized.indexOf("verify failed") >= 0) {
        failStm32Update(translate("maintenance.stm32_flash_failed"));
        return;
      }

      beginStm32AutoReset(translate("maintenance.stm32_flash_done_reset"));
    }
  } catch (error) {
    note.textContent = translate("maintenance.stm32_update_response_received");
  }
}

function setProgressActionContext(actionType, buttonId, buttonText) {
  pendingProgressAction = actionType || "";
  pendingProgressButtonId = buttonId || "";

  if (buttonId) {
    const button = document.getElementById(buttonId);
    if (button) {
      button.dataset.restoreText = buttonText || button.textContent;
    }
  }
}

function getProgressShellMeta(actionType, buttonId, message) {
  const isEspLikeAction = actionType === "esp-update" || actionType === "esp-local-update";
  return {
    actionType: actionType || "",
    buttonId: buttonId || "",
    message: message || "",
    keepFrameActiveInBackground: actionType === "stm32-flash" || isEspLikeAction || actionType === "app-file-install" || actionType === "app-local-install",
    showVisualProgress: actionType === "stm32-flash"
  };
}

function buildRemoteUpdateRequestState(meta, payload) {
  const url = meta.buildUrl(payload);
  const progress = getProgressShellMeta(meta.actionType, meta.buttonId, meta.startMessage);

  return {
    ...meta,
    url,
    payload,
    progress
  };
}

function showRemoteUpdateProgressShell(actionType, message, buttonId) {
  const progressMeta = getProgressShellMeta(actionType, buttonId, message);
  const progressShell = document.getElementById("update-progress-shell");
  const progressFrame = document.getElementById("update-progress-frame");

  progressShell.classList.remove("is-hidden");
  progressFrame.classList.toggle("progress-frame-hidden", progressMeta.keepFrameActiveInBackground);
  progressFrame.classList.remove("is-hidden");
  document.getElementById("update-progress-visual").classList.toggle("is-hidden", !progressMeta.showVisualProgress);
  document.getElementById("update-progress-note").textContent = progressMeta.message;
  document.getElementById("updated-at").textContent = progressMeta.message;
  setBusyButton(progressMeta.buttonId, translate("common.running"));
  if (progressMeta.showVisualProgress) {
    beginStm32Progress();
  } else {
    stopStm32Progress();
  }
  rememberProgressReturnScrollPosition();
  scrollElementBelowStickyNav(progressShell);
}

function startProgressAction(url, message, actionType, buttonId, buttonText) {
  const progressMeta = getProgressShellMeta(actionType, buttonId, message);
  setProgressActionContext(actionType, buttonId, buttonText);
  showRemoteUpdateProgressShell(progressMeta.actionType, progressMeta.message, progressMeta.buttonId);
  window.setTimeout(() => {
    if (progressMeta.actionType === "esp-update") {
      submitProgressFrameRequest(url, "update-progress-frame");
      return;
    }

    const progressFrame = document.getElementById("update-progress-frame");
    const separator = url.indexOf("?") >= 0 ? "&" : "?";
    progressFrame.src = url + separator + "_ts=" + Date.now();
  }, 0);
}

function startRemoteUpdateAction(meta, payload) {
  const requestState = buildRemoteUpdateRequestState(meta, payload);

  if (requestState.kind === "stm32") {
    startStm32RemoteStreamingRequest(requestState);
    return;
  }

  startProgressAction(requestState.url, requestState.progress.message, requestState.actionType, requestState.buttonId, requestState.buttonText);
}

function getProgressRequestForm(targetFrameName) {
  const formId = "progress-request-form-" + targetFrameName;
  let form = document.getElementById(formId);

  if (form) {
    return form;
  }

  form = document.createElement("form");
  form.id = formId;
  form.method = "GET";
  form.target = targetFrameName;
  form.style.display = "none";
  document.body.appendChild(form);

  return form;
}

function submitProgressFrameRequest(url, targetFrameName) {
  const absoluteUrl = new URL(url, window.location.origin);
  const form = getProgressRequestForm(targetFrameName);

  form.action = absoluteUrl.pathname;
  form.innerHTML = "";

  absoluteUrl.searchParams.set("_ts", String(Date.now()));
  absoluteUrl.searchParams.forEach((value, key) => {
    const input = document.createElement("input");
    input.type = "hidden";
    input.name = key;
    input.value = value;
    form.appendChild(input);
  });

  form.submit();
}

function startStm32RemoteStreamingRequest(requestState) {
  setProgressActionContext(requestState.actionType, requestState.buttonId, requestState.buttonText);
  stm32AutoResetStarted = false;
  stm32RemoteStreamOffset = 0;
  stm32RemoteRequestInFlight = true;
  stm32RemoteResultOkSeen = false;
  stopUpdateProgressPolling();
  showRemoteUpdateProgressShell(requestState.progress.actionType, requestState.progress.message, requestState.progress.buttonId);

  const xhr = new XMLHttpRequest();
  xhr.open("GET", requestState.url, true);
  startUpdateProgressPolling(200);

  xhr.onprogress = () => {
    consumeStm32RemoteProgressStream(xhr.responseText || "");
  };

  xhr.onload = async () => {
    consumeStm32RemoteProgressStream(xhr.responseText || "");
    stm32RemoteRequestInFlight = false;

    if (xhr.status < 200 || xhr.status >= 300) {
      failStm32Update(translate("maintenance.remote_stm32_start_failed"));
      return;
    }

    if (stm32RemoteResultOkSeen && !stm32AutoResetStarted) {
      beginStm32AutoReset(translate("maintenance.stm32_flash_done_reset"));
      return;
    }

    if (!stm32AutoResetStarted) {
      const progress = await settleFetchJson(getUpdateProgressUrl(), { ok: false }, 1000);
      applyUpdateProgressStatus(progress);
    }

    if (!stm32AutoResetStarted) {
      failStm32Update(translate("maintenance.stm32_flash_end_unclear"));
      return;
    }
  };

  xhr.onerror = () => {
    stm32RemoteRequestInFlight = false;
    failStm32Update(translate("maintenance.remote_stm32_start_failed"));
  };

  xhr.send();
}

function consumeStm32RemoteProgressStream(text) {
  const pending = text.slice(stm32RemoteStreamOffset);
  const lastNewline = pending.lastIndexOf("\n");

  if (lastNewline < 0) {
    return;
  }

  const chunk = pending.slice(0, lastNewline);
  stm32RemoteStreamOffset += lastNewline + 1;

  chunk.split("\n").forEach((line) => {
    const trimmed = line.trim();

    if (!trimmed) {
      return;
    }

    try {
      applyStm32RemoteProgressEvent(JSON.parse(trimmed));
    } catch (_) {
    }
  });
}

function applyStm32RemoteProgressEvent(event) {
  if (!event || event.ok !== true) {
    return;
  }

  if (event.state && event.type === "stm32") {
    applyUpdateProgressStatus({
      ok: true,
      active: event.active,
      type: event.type,
      state: event.state,
      message: event.message,
      progress_current: event.progress_current,
      progress_total: event.progress_total,
      error_code: event.error_code,
      started_at: event.started_at,
      updated_at: event.updated_at,
      finished_at: event.finished_at
    });
  }

  if (event.event === "result" && event.result_ok === false) {
    stm32RemoteRequestInFlight = false;
    failStm32Update(event.message || translate("maintenance.stm32_flash_failed"));
    return;
  }

  if (event.event === "result" && event.result_ok === true) {
    stm32RemoteResultOkSeen = true;

    if (!stm32AutoResetStarted && !stm32RemoteRequestInFlight) {
      beginStm32AutoReset(translate("maintenance.stm32_flash_done_reset"));
    }
  }
}

function startStm32StreamingUpload(file, buttonId, buttonText) {
  return new Promise((resolve, reject) => {
    pendingProgressAction = "stm32-flash";
    pendingProgressButtonId = buttonId || "";
    stm32AutoResetStarted = false;
    stopUpdateProgressPolling();
    const progressShell = document.getElementById("update-progress-shell");
    const progressFrame = document.getElementById("update-progress-frame");
    const button = document.getElementById(buttonId);

    progressShell.classList.remove("is-hidden");
    progressFrame.classList.add("is-hidden");
    progressFrame.classList.remove("progress-frame-hidden");
    document.getElementById("update-progress-visual").classList.remove("is-hidden");
    document.getElementById("update-progress-note").textContent = translate("maintenance.stm32_local_flash_starting");
    document.getElementById("updated-at").textContent = translate("maintenance.stm32_local_flash_starting");
    setBusyButton(buttonId, translate("common.running"));

    if (button) {
      button.dataset.restoreText = buttonText || button.textContent;
    }

    beginStm32Progress();
    rememberProgressReturnScrollPosition();
    scrollElementBelowStickyNav(progressShell);

    uploadRawFile(
      buildUploadUrl(getLocalStm32UploadUrl(), file.name),
      file,
      (loaded, total) => {
        const percent = total ? Math.min(100, Math.round((loaded / total) * 100)) : 0;
        document.getElementById("local-update-note").textContent = translateFormat("maintenance.stm32_local_upload_progress", { percent });
        document.getElementById("update-progress-note").textContent = translateFormat("maintenance.stm32_local_upload_progress", { percent });
      },
      () => {
        document.getElementById("local-update-note").textContent = translate("maintenance.stm32_local_upload_done");
        document.getElementById("update-progress-note").textContent = translate("maintenance.stm32_local_upload_done");
      }
    ).then(() => {
      const xhr = new XMLHttpRequest();
      xhr.open("GET", getLocalStm32FlashUrl(), true);
      startUpdateProgressPolling();

      xhr.onprogress = () => {
        const text = xhr.responseText || "";
        syncStm32ProgressFromText(text);

        if (!stm32AutoResetStarted && hasStm32FlashFinished(text)) {
          beginStm32AutoReset(translate("maintenance.stm32_flash_done_reset"));
        }
      };

      xhr.onload = () => {
        const text = xhr.responseText || "";
        syncStm32ProgressFromText(text);

        if (xhr.status < 200 || xhr.status >= 300) {
          document.getElementById("local-update-note").textContent = translate("maintenance.stm32_local_flash_start_failed");
          failStm32Update(translate("maintenance.stm32_local_flash_start_failed"));
          reject(new Error("stm32 local failed"));
          return;
        }

        if (!stm32AutoResetStarted && hasStm32FlashFinished(text)) {
          beginStm32AutoReset(translate("maintenance.stm32_flash_done_reset"));
        }

        resolve();
      };

      xhr.onerror = () => {
        document.getElementById("local-update-note").textContent = translate("maintenance.stm32_local_flash_start_failed");
        failStm32Update(translate("maintenance.stm32_local_flash_start_failed"));
        reject(new Error("stm32 local failed"));
      };

      xhr.send();
    }).catch(() => {
      document.getElementById("local-update-note").textContent = translate("maintenance.stm32_local_upload_failed");
      failStm32Update(translate("maintenance.stm32_local_upload_failed"));
      reject(new Error("stm32 local upload failed"));
    });
  });
}

function failStm32Update(message) {
  document.getElementById("update-progress-note").textContent = message;
  stopStm32Progress();
  stopUpdateProgressPolling();
  resetProgressButton();
  // Mit 0 ms verschwindet die Fortschrittsflaeche samt Fehlermeldung im selben Frame,
  // in dem sie gesetzt wird — ein fehlgeschlagener Flash sieht dann aus wie nichts.
  // Gleiche Standzeit wie beim Erfolgsfall. R2-5.
  finishProgressUi(2200);
  clearProgressReturnScrollPosition();
}

function beginStm32AutoReset(message) {
  if (stm32AutoResetStarted) {
    return;
  }

  stm32AutoResetStarted = true;
  setStm32ProgressStage(5);
  document.getElementById("update-progress-note").textContent = message;
  autoResetStm32AfterFlash();
}

async function autoResetStm32AfterFlash() {
  try {
    // Rohes fetch() hatte hier weder Zeitgrenze noch Statuspruefung: Ein haengender
    // ESP liess den Aufruf unbegrenzt offen, ein 403 aus der Subresource-Abwehr galt
    // als Erfolg, und seit L39 waere auch ein {"ok":false} unbemerkt geblieben.
    // apiFetch bringt alles drei mit. Bewusst OHNE "attempts": Der Endpunkt antwortet
    // zuerst und setzt den STM32 erst danach zurueck — ein zweiter Anlauf traefe die
    // Uhr mitten im Hochlauf. R2-4.
    await apiFetch(getMaintenanceResetStm32Url());
    setStm32ProgressStage(6);
    document.getElementById("updated-at").textContent = translate("maintenance.stm32_auto_reset_wait");
    document.getElementById("update-progress-note").textContent = translate("maintenance.stm32_auto_reset_running");
    await sleep(4000);
    setStm32ProgressStage(8);
    document.getElementById("updated-at").textContent = translate("maintenance.stm32_reset_after_flash");
    document.getElementById("update-progress-note").textContent = translate("maintenance.stm32_flash_success");
    stopStm32Progress();
    resetProgressButton();
    finishProgressUi(2200);
    scheduleProgressReturnScroll(2300);
    try {
      await loadData();
    } catch (error) {
      announceStatus(translate("maintenance.reload_after_stm32_failed"), "warn");
    }
  } catch (error) {
    document.getElementById("update-progress-note").textContent = describeApiError(error, translate("maintenance.stm32_flash_auto_reset_failed"));
    stopStm32Progress();
    stopUpdateProgressPolling();
    resetProgressButton();
    // Ohne diesen Aufruf bleibt die Fortschrittsflaeche nach einem gescheiterten
    // Auto-Reset dauerhaft stehen und verdeckt die Bedienung. Spiegelbild von R2-5:
    // dort war sie einen Frame lang zu sehen, hier fuer immer.
    finishProgressUi(2200);
  }
}

async function waitForDeviceReady(timeoutMs, initialDelayMs, readyMessage, reloadPage, options) {
  const deadline = Date.now() + timeoutMs;
  const config = options || {};
  const forcedReloadAt = Date.now() + Math.min(timeoutMs, config.forcedReloadAfterMs || 25000);
  const note = document.getElementById("update-progress-note");
  const probes = Array.isArray(config.probes) && config.probes.length ? config.probes : getDefaultReconnectProbes();
  const waitingMessage = config.waitingMessage || translate("maintenance.remote_esp_waiting");
  const requireReconnectCycle = !!config.requireReconnectCycle;
  const requiredStableSuccesses = Math.max(1, Number(config.requiredStableSuccesses || (requireReconnectCycle ? 2 : 1)));
  const requireProgressClearForType = config.requireProgressClearForType || "";
  const progressClearImpliesReconnect = !!requireProgressClearForType;
  let reconnectCycleObserved = !requireReconnectCycle;
  let stableSuccessCount = 0;

  async function isProgressTypeStillActive() {
    if (!requireProgressClearForType) {
      return false;
    }

    try {
      const response = await fetchWithTimeout(getUpdateProgressUrl() + "?_ts=" + Date.now(), {
        cache: "no-store"
      }, 1200);

      if (!response.ok) {
        return false;
      }

      const progress = await response.json();
      return !!(progress &&
        progress.ok &&
        progress.type === requireProgressClearForType &&
        (progress.active || (progress.state && progress.state !== "done" && progress.state !== "error")));
    } catch (error) {
      return false;
    }
  }

  async function handleSuccessfulProbe() {
    const progressStillActive = await isProgressTypeStillActive();

    if (progressStillActive) {
      stableSuccessCount = 0;
      return false;
    }

    if (!reconnectCycleObserved && !progressClearImpliesReconnect) {
      stableSuccessCount = 0;
      return false;
    }

    stableSuccessCount += 1;

    if (stableSuccessCount < requiredStableSuccesses) {
      note.textContent = waitingMessage;
      return false;
    }

    clearEspReloadWatchdog();
    note.textContent = readyMessage;
    document.getElementById("updated-at").textContent = readyMessage;
    resetProgressButton();
    finishProgressUi(900);

    if (reloadPage) {
      setTimeout(reloadAppPage, 1200);
    } else {
      scheduleProgressReturnScroll(1000);
      setTimeout(loadData, 1200);
    }
    return true;
  }

  async function probeViaFetch(probe) {
    const separator = probe.url.indexOf("?") >= 0 ? "&" : "?";
    const response = await fetchWithTimeout(probe.url + separator + "_ts=" + Date.now(), {
      cache: "no-store"
    }, probe.timeoutMs || 1800);

    if (!response.ok) {
      return false;
    }

    if (probe.mode === "json") {
      const data = await response.json();
      return probe.validate ? !!probe.validate(data) : true;
    }

    if (probe.mode === "text") {
      const text = await response.text();
      return probe.validate ? !!probe.validate(text) : !!text;
    }

    return probe.validate ? !!probe.validate(response) : true;
  }

  if (reloadPage) {
    scheduleEspReloadWatchdog(Math.min(timeoutMs, config.reloadWatchdogDelayMs || 30000));
  }

  scrollUpdateProgressIntoView();

  await sleep(initialDelayMs || 0);

  while (Date.now() < deadline) {
    for (const probe of probes) {
      try {
        if (await probeViaFetch(probe)) {
          if (await handleSuccessfulProbe()) {
            return;
          }
        }
      } catch (error) {
        reconnectCycleObserved = true;
        stableSuccessCount = 0;
      }
    }

    for (const probe of probes) {
      if (probe.mode && probe.mode !== "response") {
        continue;
      }

      try {
        const separator = probe.url.indexOf("?") >= 0 ? "&" : "?";
        const frameLoaded = await probeDeviceReadyViaFrame(probe.url + separator + "_ts=" + Date.now(), probe.timeoutMs || CONNECTION_STABILITY.frameProbeTimeoutMs);
        if (frameLoaded) {
          if (await handleSuccessfulProbe()) {
            return;
          }
        } else if (requireReconnectCycle) {
          reconnectCycleObserved = true;
          stableSuccessCount = 0;
        }
      } catch (error) {
        reconnectCycleObserved = true;
        stableSuccessCount = 0;
      }
    }

    note.textContent = waitingMessage;
    scrollUpdateProgressIntoView();

    if (reloadPage && Date.now() >= forcedReloadAt) {
      clearEspReloadWatchdog();
      note.textContent = translate("maintenance.device_ready_reload");
      document.getElementById("updated-at").textContent = note.textContent;
      resetProgressButton();
      finishProgressUi(900);
      setTimeout(reloadAppPage, 900);
      return;
    }

    await sleep(2000);
  }

  if (reloadPage) {
    clearEspReloadWatchdog();
    note.textContent = translate("maintenance.no_reconnect_reload");
    document.getElementById("updated-at").textContent = note.textContent;
    resetProgressButton();
    finishProgressUi(900);
    setTimeout(reloadAppPage, 900);
    return;
  }

  note.textContent = translate("maintenance.esp_not_ready");
  clearEspReloadWatchdog();
  resetProgressButton();
}

async function reloadAppPage() {
  clearEspReloadWatchdog();
  try {
    if (appServiceWorkerRegistration) {
      await appServiceWorkerRegistration.update().catch(() => {});

      if (appServiceWorkerRegistration.waiting) {
        triggerWaitingServiceWorker(appServiceWorkerRegistration.waiting);
      }
    }

    if ("caches" in window) {
      const cacheKeys = await caches.keys();
      await Promise.all(
        cacheKeys
          .filter((key) => key.indexOf("wordclock-app-") === 0)
          .map((key) => caches.delete(key))
      );
    }

    if ("serviceWorker" in navigator) {
      const registrations = await navigator.serviceWorker.getRegistrations();
      await Promise.all(registrations.map((registration) => registration.update().catch(() => {})));
    }
  } catch (error) {
    // Ignore cache/service worker cleanup errors and continue with the reload.
  }

  const url = new URL(window.location.href);
  const nextPath = "/app/" + (url.hash || "");
  window.location.replace(nextPath);
}

function clearReloadQueryMarker() {
  // Reload query marker removed intentionally; kept as a no-op for compatibility.
}

function manualReloadApp() {
  if (hasUnsavedEdits && !window.confirm(translate("maintenance.unsaved_reload_confirm"))) {
    return;
  }

  const button = document.getElementById("reload-button");
  beginButtonFeedback(button, translate("common.reloading"));
  announceStatus(translate("maintenance.reloading"), "warn");
  clearProgressReturnScrollPosition();
  window.setTimeout(reloadAppPage, 180);
}

function scheduleEspReloadWatchdog(delayMs) {
  clearEspReloadWatchdog();
  espReloadWatchdogId = window.setTimeout(() => {
    const note = document.getElementById("update-progress-note");

    if (note) {
      note.textContent = translate("maintenance.forced_reload");
    }
    document.getElementById("updated-at").textContent = note ? note.textContent : translate("maintenance.reloading");
    resetProgressButton();
    finishProgressUi(300);
    window.setTimeout(reloadAppPage, 600);
  }, Math.max(8000, delayMs || 0));
}

function clearEspReloadWatchdog() {
  if (!espReloadWatchdogId) {
    return;
  }

  window.clearTimeout(espReloadWatchdogId);
  espReloadWatchdogId = 0;
}

async function fetchWithTimeout(url, options, timeoutMs) {
  const controller = new AbortController();
  const timer = window.setTimeout(() => controller.abort(), timeoutMs || CONNECTION_STABILITY.frameProbeTimeoutMs);

  try {
    return await fetch(url, {
      ...(options || {}),
      signal: controller.signal
    });
  } finally {
    window.clearTimeout(timer);
  }
}

function settleWithTimeout(promise, fallbackValue, timeoutMs) {
  return Promise.race([
    Promise.resolve(promise).catch(() => fallbackValue),
    new Promise((resolve) => {
      window.setTimeout(() => resolve(fallbackValue), timeoutMs || CONNECTION_STABILITY.frameProbeTimeoutMs);
    })
  ]);
}

function settleFetchText(url, fallbackValue, timeoutMs, attempts) {
  return settleWithTimeout(
    fetchWithRetry(url, { cache: "no-store" }, timeoutMs, attempts || CONNECTION_STABILITY.fastReadAttempts)
      .then((response) => response.ok ? response.text() : fallbackValue),
    fallbackValue,
    timeoutMs
  );
}

function settleFetchJson(url, fallbackValue, timeoutMs, attempts) {
  return settleWithTimeout(
    fetchWithRetry(url, { cache: "no-store" }, timeoutMs, attempts || CONNECTION_STABILITY.fastReadAttempts)
      .then((response) => response.ok ? response.json() : fallbackValue),
    fallbackValue,
    timeoutMs
  );
}

function isRetryableFetchError(error) {
  if (!error) {
    return false;
  }

  if (error.name === "AbortError" || error.message === "Failed to fetch") {
    return true;
  }

  return /^http-5\d\d$/.test(String(error.message || ""));
}

async function fetchWithRetry(url, options, timeoutMs, attempts) {
  const maxAttempts = Math.max(1, Number(attempts || 1));
  let lastError = null;

  for (let attempt = 1; attempt <= maxAttempts; attempt += 1) {
    try {
      const response = await fetchWithTimeout(url, options, timeoutMs);
      if (!response.ok && response.status >= 500 && attempt < maxAttempts) {
        await sleep(CONNECTION_STABILITY.retryDelayMs * attempt);
        continue;
      }
      return response;
    } catch (error) {
      lastError = error;
      if (attempt >= maxAttempts || !isRetryableFetchError(error)) {
        throw error;
      }
      await sleep(CONNECTION_STABILITY.retryDelayMs * attempt);
    }
  }

  throw lastError || new Error("fetch-failed");
}

function stopUpdateProgressPolling() {
  if (!updateProgressPollTimer) {
    return;
  }

  window.clearInterval(updateProgressPollTimer);
  updateProgressPollTimer = 0;
}

function syncStm32ProgressFromStatus(progress) {
  const note = document.getElementById("update-progress-note");
  const message = progress && progress.message ? progress.message : "";

  if (!progress || progress.type !== "stm32") {
    return;
  }

  if (progress.state === "bootloader" && stm32ProgressStage < 2) {
    setStm32ProgressStage(2);
  } else if (progress.state === "verify" && stm32ProgressStage < 3) {
    setStm32ProgressStage(3);
  } else if (progress.state === "erase" && stm32ProgressStage < 4) {
    setStm32ProgressStage(4);
  } else if ((progress.state === "write" || progress.state === "reset_wait" || progress.state === "done") && stm32ProgressStage < 5) {
    setStm32ProgressStage(5);
  } else if (progress.state === "reset" && stm32ProgressStage < 6) {
    setStm32ProgressStage(6);
  }

  if (message) {
    if ((progress.state === "write" || progress.state === "reset_wait") &&
        Number(progress.progress_total || 0) > 0 &&
        Number(progress.progress_current || 0) > 0) {
      note.textContent = message + " Seiten: " + String(progress.progress_current) + "/" + String(progress.progress_total);
    } else {
      note.textContent = message;
    }
  }
}

function applyUpdateProgressStatus(progress) {
  const note = document.getElementById("update-progress-note");

  if (!progress || !progress.ok) {
    return;
  }

  if (pendingProgressAction === "esp-update" && progress.type === "esp" && progress.message) {
    note.textContent = progress.message;
    document.getElementById("updated-at").textContent = progress.message;
    return;
  }

  if (pendingProgressAction !== "stm32-flash" || progress.type !== "stm32") {
    return;
  }

  syncStm32ProgressFromStatus(progress);
  if (progress.message) {
    document.getElementById("updated-at").textContent = progress.message;
  }

  if (progress.state === "error") {
    stopUpdateProgressPolling();
    stopStm32Progress();
    note.textContent = progress.message || translate("maintenance.stm32_flash_failed");
    resetProgressButton();
    clearProgressReturnScrollPosition();
    return;
  }

  if ((progress.state === "reset_wait" || progress.state === "done") && !stm32AutoResetStarted) {
    if (stm32RemoteRequestInFlight && progress.state === "reset_wait") {
      note.textContent = progress.message || translate("maintenance.stm32_flash_done_confirming");
      return;
    }

    stm32AutoResetStarted = true;
    setStm32ProgressStage(5);
    note.textContent = progress.message || translate("maintenance.stm32_flash_done_reset");
    autoResetStm32AfterFlash();
  }
}

function startUpdateProgressPolling(initialDelayMs) {
  stopUpdateProgressPolling();

  const poll = async () => {
    const progress = await settleFetchJson(getUpdateProgressUrl(), { ok: false }, CONNECTION_STABILITY.progressPollTimeoutMs);
    applyUpdateProgressStatus(progress);
  };

  const start = () => {
    void poll();
    updateProgressPollTimer = window.setInterval(() => {
      void poll();
    }, CONNECTION_STABILITY.progressPollIntervalMs);
  };

  if (initialDelayMs && initialDelayMs > 0) {
    updateProgressPollTimer = window.setTimeout(() => {
      updateProgressPollTimer = 0;
      start();
    }, initialDelayMs);
    return;
  }

  start();
}

function probeDeviceReadyViaFrame(url, timeoutMs) {
  return new Promise((resolve) => {
    const frame = document.createElement("iframe");
    let done = false;

    frame.className = "progress-frame-hidden";
    frame.setAttribute("aria-hidden", "true");

    const finish = (result) => {
      if (done) {
        return;
      }
      done = true;
      frame.remove();
      resolve(result);
    };

    const timer = window.setTimeout(() => {
      finish(false);
    }, timeoutMs || CONNECTION_STABILITY.frameProbeTimeoutMs);

    bindDomEvent(frame, "load", () => {
      window.clearTimeout(timer);
      finish(true);
    }, { once: true });

    bindDomEvent(frame, "error", () => {
      window.clearTimeout(timer);
      finish(false);
    }, { once: true });

    document.body.appendChild(frame);
    frame.src = url;
  });
}

function scrollUpdateProgressIntoView() {
  const shell = document.getElementById("update-progress-shell");

  if (shell && !shell.classList.contains("is-hidden")) {
    scrollElementBelowStickyNav(shell);
  }
}

function scrollElementBelowStickyNav(element) {
  if (!element) {
    return;
  }

  const navShell = document.querySelector(".module-nav-shell");
  const stickyHeight = navShell ? Math.ceil(navShell.getBoundingClientRect().height) : 0;
  const extraGap = 12;
  const targetTop = Math.max(0, window.scrollY + element.getBoundingClientRect().top - stickyHeight - extraGap);

  window.scrollTo({ top: targetTop, behavior: "smooth" });
}

function rememberProgressReturnScrollPosition() {
  progressReturnScrollY = window.scrollY || window.pageYOffset || 0;
  try {
    window.sessionStorage.setItem(PROGRESS_SCROLL_RESTORE_KEY, String(progressReturnScrollY));
  } catch (_) {
  }
}

function clearProgressReturnScrollPosition() {
  progressReturnScrollY = null;
  try {
    window.sessionStorage.removeItem(PROGRESS_SCROLL_RESTORE_KEY);
  } catch (_) {
  }
}

function restoreProgressReturnScrollPosition() {
  let target = progressReturnScrollY;

  if (target === null || target === undefined) {
    try {
      const stored = window.sessionStorage.getItem(PROGRESS_SCROLL_RESTORE_KEY);
      if (stored !== null && stored !== "") {
        target = Number(stored);
      }
    } catch (_) {
    }
  }

  if (!Number.isFinite(target)) {
    clearProgressReturnScrollPosition();
    return;
  }

  window.scrollTo({ top: Math.max(0, target), behavior: "smooth" });
  window.setTimeout(clearProgressReturnScrollPosition, 900);
}

function scheduleProgressReturnScroll(delayMs) {
  window.setTimeout(restoreProgressReturnScrollPosition, delayMs || 0);
}

function sleep(ms) {
  return new Promise((resolve) => window.setTimeout(resolve, ms));
}

function setBusyButton(buttonId, busyText) {
  if (!buttonId) {
    return;
  }

  const button = document.getElementById(buttonId);

  if (button) {
    if (!button.dataset.restoreText) {
      button.dataset.restoreText = button.textContent;
    }
    button.disabled = true;
    button.textContent = busyText;
  }
}

function resetProgressButton() {
  if (!pendingProgressButtonId) {
    return;
  }

  const button = document.getElementById(pendingProgressButtonId);

  if (button) {
    button.disabled = false;
    button.textContent = button.dataset.restoreText || button.textContent;
  }

  pendingProgressButtonId = "";
}

function finishProgressUi(delayMs) {
  window.setTimeout(() => {
    stopUpdateProgressPolling();
    document.getElementById("update-progress-shell").classList.add("is-hidden");
    document.getElementById("update-progress-frame").src = "about:blank";
    document.getElementById("update-progress-frame").classList.remove("progress-frame-hidden");
    document.getElementById("update-progress-visual").classList.add("is-hidden");
    pendingProgressAction = "";
  }, delayMs || 0);
}

function beginStm32Progress() {
  const visual = document.getElementById("update-progress-visual");
  visual.classList.remove("is-hidden");
  stopStm32Progress();
  stm32AutoResetStarted = false;
  setStm32ProgressStage(1);
  document.getElementById("update-progress-note").textContent = translate("maintenance.stm32_prepare_note");
  stm32ProgressAdvanceTimer = window.setTimeout(() => {
    if (stm32ProgressStage === 1) {
      document.getElementById("update-progress-note").textContent = translate("maintenance.stm32_wait_bootloader");
    }
  }, 1200);
  stm32ProgressTimer = window.setInterval(() => {
    const note = document.getElementById("update-progress-note");

    if (stm32ProgressStage === 4) {
      note.textContent = translate("maintenance.stm32_flash_writing");
    }
  }, 3000);
}

function stopStm32Progress() {
  if (stm32ProgressAdvanceTimer) {
    window.clearTimeout(stm32ProgressAdvanceTimer);
    stm32ProgressAdvanceTimer = 0;
  }
  if (stm32ProgressTimer) {
    window.clearInterval(stm32ProgressTimer);
    stm32ProgressTimer = 0;
  }
}

function readUpdateProgressFrameText() {
  try {
    const frame = document.getElementById("update-progress-frame");
    const body = frame.contentWindow && frame.contentWindow.document && frame.contentWindow.document.body;
    return body ? (body.textContent || "") : "";
  } catch (error) {
    return "";
  }
}

function setStm32ProgressStage(stage) {
  stm32ProgressStage = stage;
  const steps = [
    { title: translate("maintenance.progress_prepare_title"), note: translate("maintenance.progress_prepare_note") },
    { title: translate("maintenance.progress_bootloader_title"), note: translate("maintenance.progress_bootloader_note") },
    { title: translate("maintenance.progress_hex_check_title"), note: translate("maintenance.progress_hex_check_note") },
    { title: translate("maintenance.progress_flash_erase_title"), note: translate("maintenance.progress_flash_erase_note") },
    { title: translate("maintenance.progress_flash_write_title"), note: translate("maintenance.progress_flash_write_note") },
    { title: translate("maintenance.progress_reset_title"), note: translate("maintenance.progress_reset_note") },
    { title: translate("maintenance.progress_finish_title"), note: translate("maintenance.progress_finish_note") }
  ];
  const progressBar = document.getElementById("update-progress-bar");
  const progressSteps = document.getElementById("update-progress-steps");
  const widths = [0, 8, 20, 34, 52, 76, 90, 100, 100];

  progressBar.style.width = (widths[stage] || 0) + "%";
  progressSteps.innerHTML = steps.map((step, index) => {
    const stepNumber = index + 1;
    const stateClass = stepNumber < stage ? "is-done" : (stepNumber === stage ? "is-active" : "");
    const badgeText = stepNumber < stage ? "✓" : String(stepNumber);
    return '<div class="progress-step ' + stateClass + '">' +
      '<div class="progress-step-badge">' + badgeText + '</div>' +
      '<div><div class="progress-step-title">' + step.title + '</div><div class="progress-step-note">' + step.note + '</div></div>' +
      '</div>';
  }).join("");
}

function syncStm32ProgressFromText(text) {
  const normalized = (text || "").replace(/\s+/g, " ").trim();
  const note = document.getElementById("update-progress-note");

  if (!normalized) {
    return;
  }

  if (normalized.indexOf("Trying to enter bootloader mode") >= 0 || normalized.indexOf("Bootloader version:") >= 0) {
    if (stm32ProgressStage < 2) {
      setStm32ProgressStage(2);
    }
    note.textContent = translate("maintenance.stm32_bootloader_reached");
  }

  if (normalized.indexOf("Checking HEX file") >= 0 || normalized.indexOf("Check successful") >= 0) {
    if (stm32ProgressStage < 3) {
      setStm32ProgressStage(3);
    }
    note.textContent = translate("maintenance.stm32_hex_check_running");
  }

  if (normalized.indexOf("Erasing flash") >= 0) {
    if (stm32ProgressStage < 4) {
      setStm32ProgressStage(4);
    }
    note.textContent = translate("maintenance.stm32_flash_erasing");
  }

  if (normalized.indexOf("Flashing STM32") >= 0 || normalized.indexOf("Pages flashed:") >= 0 || normalized.indexOf("Flash successful") >= 0) {
    if (stm32ProgressStage < 5) {
      setStm32ProgressStage(5);
    }
    note.textContent = translate("maintenance.stm32_flash_writing");
  }
}

function hasStm32FlashFinished(text) {
  const normalized = (text || "").replace(/\s+/g, " ").trim();

  if (!normalized) {
    return false;
  }

  return normalized.indexOf("Please Reset your STM32 now") >= 0 ||
    normalized.indexOf("Done.") >= 0;
}

async function saveRtcTemperatureCorrection() {
  await saveTemperatureCorrection("temperature-rtc-correction-input", "temperature-rtc-correction-save-button", getTemperatureRtcCorrectionSetUrl(), translate("climate.save_rtc_correction"), translate("climate.rtc_correction_save_failed"));
}

async function saveDs18xxTemperatureCorrection() {
  await saveTemperatureCorrection("temperature-ds18xx-correction-input", "temperature-ds18xx-correction-save-button", getTemperatureDs18xxCorrectionSetUrl(), translate("climate.save_ds18xx_correction"), translate("climate.ds18xx_correction_save_failed"));
}

async function saveTemperatureCorrection(inputId, buttonId, endpoint, buttonText, errorText) {
  const input = document.getElementById(inputId);
  const number = readNumberInputOrReport(input, -20, 20);

  if (number === null) {
    return;
  }

  const value = number;
  input.value = String(value);
  await runValueSave(buttonId, endpoint, value, buttonText, errorText);
}

async function displayTemperatureNow() {
  await runSimpleAction("temperature-display-button", getTemperatureDisplayUrl(), translate("climate.show_temperature"), translate("climate.temperature_display_failed"), translate("climate.temperature_display_started"));
}

async function setLdrMinValue() {
  await runSimpleAction("ldr-min-button", getLdrMinSetUrl(), translate("climate.set_min_ldr"), translate("climate.ldr_min_set_failed"), translate("climate.ldr_min_saved"));
}

async function setLdrMaxValue() {
  await runSimpleAction("ldr-max-button", getLdrMaxSetUrl(), translate("climate.set_max_ldr"), translate("climate.ldr_max_set_failed"), translate("climate.ldr_max_saved"));
}

async function saveAnimationMode() {
  await runSelectSave("animation-mode-select", "animation-mode-save-button", getAnimationModeSetUrl(), translate("animations.save_display_animation"), translate("animations.display_animation_save_failed"));
}

async function saveColorAnimationMode() {
  await runSelectSave("color-animation-mode-select", "color-animation-mode-save-button", getColorAnimationModeSetUrl(), translate("animations.save_color_animation"), translate("animations.color_animation_save_failed"));
}

async function runSelectSave(selectId, buttonId, endpoint, buttonText, errorText) {
  const value = document.getElementById(selectId).value;
  await runValueSave(buttonId, endpoint, value, buttonText, errorText);
}

async function saveAnimationProfile(idx) {
  // Der Regler kann von sich aus nichts Unzulaessiges liefern -- ein ueber die
  // Tastatur oder aus einem Fremdskript gesetzter Wert aber schon, und seit Runde 1
  // weist der ESP ihn ab (http.cpp: "deceleration out of range (1..15)"). Die Grenzen
  // stehen hier so, wie das Geraet sie fuehrt; 0 ist bei DIESEM Setter verboten.
  // Massnahme 17.
  const deceleration = readNumberInputOrReport(document.getElementById("an-dec-" + idx), 1, 15);

  if (deceleration === null) {
    return;
  }

  const favourite = document.getElementById("an-fav-" + idx).checked ? "on" : "off";
  await runIndexedQueryButtonRequest('[data-an-save="%idx%"]', idx, {
    endpoint: getAnimationProfileSetUrl(),
    query: { idx, deceleration, favourite },
    busyText: translate("common.saving"),
    idleText: translate("animations.profile_save"),
    successText: translate("common.saved"),
    errorText: translate("animations.profile_save_failed"),
    reload: true
  });
}

async function resetAnimationProfileDefault(idx) {
  await runIndexedQueryButtonRequest('[data-an-default="%idx%"]', idx, {
    endpoint: getAnimationProfileDefaultUrl(),
    query: { idx },
    busyText: translate("common.setting"),
    idleText: translate("animations.default"),
    successText: translate("common.set"),
    errorText: translate("display.default_set_failed"),
    reload: true
  });
}

async function saveColorAnimationProfile(idx) {
  // Anders als beim Display-Animationsprofil ist 0 hier gueltig (vars.h:330). Die
  // beiden Geschwister haben verschiedene Untergrenzen, und genau deshalb steht die
  // Zahl an jeder Stelle einzeln und nicht in einer gemeinsamen Konstanten.
  const deceleration = readNumberInputOrReport(document.getElementById("can-dec-" + idx), 0, 15);

  if (deceleration === null) {
    return;
  }

  await runIndexedQueryButtonRequest('[data-can-save="%idx%"]', idx, {
    endpoint: getColorAnimationProfileSetUrl(),
    query: { idx, deceleration },
    busyText: translate("common.saving"),
    idleText: translate("animations.profile_save"),
    successText: translate("common.saved"),
    errorText: translate("animations.color_profile_save_failed"),
    reload: true
  });
}

async function resetColorAnimationProfileDefault(idx) {
  await runIndexedQueryButtonRequest('[data-can-default="%idx%"]', idx, {
    endpoint: getColorAnimationProfileDefaultUrl(),
    query: { idx },
    busyText: translate("common.setting"),
    idleText: translate("animations.default"),
    successText: translate("common.set"),
    errorText: translate("display.default_set_failed"),
    reload: true
  });
}

async function saveDimCurve(prefix) {
  const button = document.getElementById(prefix === "ambi" ? "ambilight-dim-save-button" : "display-dim-save-button");
  const buttonText = prefix === "ambi" ? translate("display.save_ambilight_dim_curve") : translate("display.dim_curve_save");
  await persistDimCurve(prefix, button, buttonText);
}

async function persistDimCurve(prefix, button, buttonText) {
  const endpoint = prefix === "ambi" ? getAmbilightDimLevelSetUrl() : getDisplayDimLevelSetUrl();

  // Erst alle sechzehn Stufen einsammeln, dann senden. Die Schleife kostet den STM
  // sechzehn Kommandos am Stueck — rund vier Sekunden Hauptloop-Stillstand. Bricht sie
  // erst bei Stufe neun ab, bleibt die Kurve halb geschrieben auf dem Geraet zurueck.
  const values = [];

  for (let idx = 0; idx <= 15; idx += 1) {
    const input = document.getElementById(prefix + "-dim-" + idx);
    const number = readNumberInputOrReport(input, 0, 15);

    if (number === null) {
      return;
    }

    const value = number;
    input.value = String(value);
    syncDimCurveValue(prefix, idx);
    values.push(value);
  }

  beginButtonFeedback(button, translate("common.saving"));

  try {
    for (let idx = 0; idx <= 15; idx += 1) {
      await apiFetch(endpoint + "?idx=" + idx + "&value=" + encodeURIComponent(values[idx]));
    }
    markEditsPersisted();
    await loadData();
    finishButtonFeedback(button, buttonText, "success", translate("common.saved"));
  } catch (error) {
    // Dieselbe Behandlung wie in runButtonRequest: traegt der Fehler eine Begruendung
    // vom Geraet, ist sie konkreter als "Dimmkurve konnte nicht gespeichert werden".
    // Hier besonders wertvoll, weil die Reihe mitten in sechzehn Kommandos abbrechen
    // kann — welche Stufen schon stehen, zeigt der anschliessende Aktualisierungslauf.
    announceStatus(describeApiError(error, translate("display.dim_curve_save_failed")), "error");
    finishButtonFeedback(button, buttonText, "error", translate("common.error"));
  }
}

async function saveTftFlags() {
  const rgb = document.getElementById("tft-rgb-checkbox").checked ? "on" : "off";
  const hflip = document.getElementById("tft-hflip-checkbox").checked ? "on" : "off";
  const vflip = document.getElementById("tft-vflip-checkbox").checked ? "on" : "off";
  await runQueryButtonRequest(document.getElementById("tft-save-button"), {
    endpoint: getTftFlagsSetUrl(),
    query: { rgb, hflip, vflip },
    busyText: translate("common.saving"),
    idleText: translate("display.save_tft_options"),
    successText: translate("common.saved"),
    errorText: translate("display.tft_save_failed"),
    reload: true
  });
}

async function runTextSave(buttonId, endpoint, value, buttonText, errorText) {
  await runValueSave(buttonId, endpoint, value, buttonText, errorText);
}

async function refreshUpdateServerAvailability() {
  announceStatus(translate("maintenance.server_recheck_running"), "warn");

  try {
    await loadData();
    announceStatus(translate("maintenance.server_recheck_done"), "ok");
  } catch (error) {
    announceStatus(translate("maintenance.server_recheck_failed"), "error");
  }
}

async function runSimpleAction(buttonId, endpoint, buttonText, errorText, successText) {
  await runTriggerAction(buttonId, endpoint, buttonText, errorText, successText, {
    busyText: translate("common.running"),
    successText: translate("common.ready")
  });
}

async function runButtonRequestById(buttonId, options) {
  return runButtonRequest(document.getElementById(buttonId), options);
}

async function runQueryButtonRequestById(buttonId, options) {
  return runQueryButtonRequest(document.getElementById(buttonId), options);
}

function buildQueryString(params) {
  return Object.entries(params || {})
    .map(([key, value]) => key + "=" + encodeURIComponent(value))
    .join("&");
}

function parseTimeInput(value) {
  const parts = String(value || "00:00").split(":");
  return {
    hour: Number(parts[0] || 0),
    minute: Number(parts[1] || 0)
  };
}

async function runQueryButtonRequest(button, options) {
  const {
    endpoint,
    query = {},
    request,
    ...requestOptions
  } = options || {};

  return runButtonRequest(button, {
    ...requestOptions,
    request: request || (() => {
      const queryString = buildQueryString(query);
      return apiFetch(endpoint + (queryString ? "?" + queryString : ""));
    })
  });
}

async function runIndexedButtonRequest(selector, idx, options) {
  return runButtonRequest(document.querySelector(selector.replace("%idx%", String(idx))), options);
}

async function runIndexedQueryButtonRequest(selector, idx, options) {
  return runQueryButtonRequest(document.querySelector(selector.replace("%idx%", String(idx))), options);
}

async function runValueSave(buttonId, endpoint, value, buttonText, errorText, options) {
  return runQueryButtonRequestById(buttonId, {
    endpoint,
    query: { value },
    busyText: translate("common.saving"),
    idleText: buttonText,
    successText: translate("common.saved"),
    errorText,
    reload: true,
    ...(options || {})
  });
}

async function runQuerySave(buttonId, endpoint, query, buttonText, errorText, options) {
  return runQueryButtonRequestById(buttonId, {
    endpoint,
    query,
    busyText: translate("common.saving"),
    idleText: buttonText,
    successText: translate("common.saved"),
    errorText,
    reload: true,
    ...(options || {})
  });
}

async function runTriggerAction(buttonId, endpoint, buttonText, errorText, successStatusText, options) {
  return runButtonRequestById(buttonId, {
    busyText: (options && options.busyText) || translate("common.starting"),
    idleText: buttonText,
    successText: (options && options.successText) || translate("common.started"),
    errorText,
    successStatusText: successStatusText || "",
    reloadDelayMs: (options && options.reloadDelayMs) || 1200,
    preserveCurrentText: !!(options && options.preserveCurrentText),
    request: (options && options.request) || (() => apiFetch(endpoint))
  });
}

async function runStateToggleButton(button, endpoint, options) {
  const current = options && options.currentValue ? options.currentValue() : (button.dataset.state === "on" ? "on" : "off");
  const next = current === "on" ? "off" : "on";
  const idleText = options && options.idleText ? options.idleText : (button.dataset.restoreText || button.textContent);
  const successText = options && options.successText ? options.successText(next) : (next === "on" ? translate("flags.enabled") : translate("flags.disabled"));

  await runQueryButtonRequest(button, {
    endpoint,
    query: { value: next },
    busyText: translate("common.running"),
    idleText,
    successText,
    errorText: options && options.errorText ? options.errorText : translate("common.toggle_failed"),
    reload: true,
    preserveCurrentText: options && options.preserveCurrentText !== undefined ? options.preserveCurrentText : true
  });
}

async function runButtonRequest(button, options) {
  const {
    busyText = translate("common.running"),
    idleText = button && (button.dataset.restoreText || button.textContent) ? (button.dataset.restoreText || button.textContent) : "",
    successText = translate("common.started"),
    errorText = translate("common.action_failed"),
    successStatusText = "",
    reload = false,
    reloadDelayMs = 0,
    preserveCurrentText = false,
    request
  } = options || {};

  beginButtonFeedback(button, busyText);

  try {
    await request();

    // Nach einem erfolgreichen Schreibvorgang mit anschliessendem Neulesen sind die
    // Felder gleich darauf wieder Geraetestand — das Dirty-Flag haette nichts mehr zu
    // schuetzen. Blieb es stehen, legte es ueber "opts.auto && hasUnsavedEdits" in
    // loadData() die Selbstaktualisierung fuer den Rest der Sitzung still. L33,
    // Massnahme 4. Reine Ausloeseaktionen (reloadDelayMs) ruehren es bewusst nicht an.
    if (reload) {
      markEditsPersisted();
      await loadData();
    } else if (reloadDelayMs > 0) {
      setTimeout(loadData, reloadDelayMs);
    }
    if (successStatusText) {
      announceStatus(successStatusText, "ok");
    }
    finishButtonFeedback(button, idleText, "success", successText, preserveCurrentText);
  } catch (error) {
    // Hat das Geraet die Eingabe begruendet abgewiesen, gehoert SEIN Grund angezeigt
    // und nicht "Aktion konnte nicht ausgefuehrt werden".
    announceStatus(describeApiError(error, errorText), "error");
    finishButtonFeedback(button, idleText, "error", translate("common.error"), preserveCurrentText);
  }
}

async function saveAmbilightModeProfile(idx) {
  // Hier stand die letzte stille Klemmung der Oberflaeche: Math.max(0, Math.min(15, ...))
  // zog jeden Wert auf die Grenze, und danach meldete die Maske "gespeichert". Das ist
  // genau der Fall aus Massnahme 17 -- die Oberflaeche log. Jetzt wird abgewiesen.
  const deceleration = readNumberInputOrReport(document.getElementById("alm-dec-" + idx), 0, 15);

  if (deceleration === null) {
    return;
  }

  await runIndexedQueryButtonRequest('[data-alm-save="%idx%"]', idx, {
    endpoint: getAmbilightModeProfileSetUrl(),
    query: { idx, deceleration },
    busyText: translate("common.saving"),
    idleText: translate("animations.profile_save"),
    successText: translate("common.saved"),
    errorText: translate("display.ambilight_profile_save_failed"),
    reload: true
  });
}

async function resetAmbilightModeProfile(idx) {
  await runIndexedQueryButtonRequest('[data-alm-default="%idx%"]', idx, {
    endpoint: getAmbilightModeProfileDefaultUrl(),
    query: { idx },
    busyText: translate("common.setting"),
    idleText: translate("animations.default"),
    successText: translate("common.set"),
    errorText: translate("display.ambilight_default_set_failed"),
    reload: true
  });
}

async function saveAmbilightBrightness() {
  const value = document.getElementById("ambilight-brightness-slider").value;
  await runButtonRequestById("ambilight-brightness-save-button", {
    busyText: translate("common.saving"),
    idleText: translate("display.save_ambilight_brightness"),
    successText: translate("common.saved"),
    errorText: translate("display.ambilight_brightness_save_failed"),
    reload: true,
    request: () => apiFetch(getAmbilightBrightnessSetUrl() + "?value=" + encodeURIComponent(value))
  });
}

async function saveAmbilightMode() {
  const value = document.getElementById("ambilight-mode-select").value;
  await runButtonRequestById("ambilight-mode-save-button", {
    busyText: translate("common.saving"),
    idleText: translate("display.save_ambilight_mode"),
    successText: translate("common.saved"),
    errorText: translate("display.ambilight_mode_save_failed"),
    reload: true,
    request: () => apiFetch(getAmbilightModeSetUrl() + "?value=" + encodeURIComponent(value))
  });
}

async function saveAmbilightLeds() {
  const input = document.getElementById("ambilight-leds-input");
  const number = readNumberInputOrReport(input, 0, 999);

  if (number === null) {
    return;
  }

  const value = number;
  input.value = String(value);
  await runButtonRequestById("ambilight-leds-save-button", {
    busyText: translate("common.saving"),
    idleText: translate("display.save_ambilight_leds"),
    successText: translate("common.saved"),
    errorText: translate("display.ambilight_leds_save_failed"),
    reload: true,
    request: () => apiFetch(getAmbilightLedsSetUrl() + "?value=" + encodeURIComponent(value))
  });
}

async function saveAmbilightOffset() {
  const input = document.getElementById("ambilight-offset-input");
  const number = readNumberInputOrReport(input, 0, 999);

  if (number === null) {
    return;
  }

  const value = number;
  input.value = String(value);
  await runButtonRequestById("ambilight-offset-save-button", {
    busyText: translate("common.saving"),
    idleText: translate("display.save_ambilight_offset"),
    successText: translate("common.saved"),
    errorText: translate("display.ambilight_offset_save_failed"),
    reload: true,
    request: () => apiFetch(getAmbilightOffsetSetUrl() + "?value=" + encodeURIComponent(value))
  });
}

async function saveColor(prefix) {
  const rgbHex = document.getElementById(prefix + "-color-rgb").value;
  const white = Number(document.getElementById(prefix + "-color-white").value || 0);
  const rgb = hexToRgb63(rgbHex);
  const endpoint = {
    display: getDisplayColorSetUrl(),
    ambilight: getAmbilightColorSetUrl(),
    marker: getMarkerColorSetUrl()
  }[prefix];

  const idleText = {
    display: translate("display.save_color"),
    ambilight: translate("display.save_ambilight_color"),
    marker: translate("display.save_marker_color")
  }[prefix];
  await runQueryButtonRequest(document.getElementById(prefix + "-color-save-button"), {
    endpoint,
    query: { red: rgb.red, green: rgb.green, blue: rgb.blue, white },
    busyText: translate("common.saving"),
    idleText,
    successText: translate("common.saved"),
    errorText: translate("display.color_save_failed"),
    reload: true
  });
}

async function saveDfplayerVolume() {
  const value = document.getElementById("dfplayer-volume-slider").value;
  await runQueryButtonRequest(document.getElementById("dfplayer-volume-save-button"), {
    endpoint: getDfplayerVolumeSetUrl(),
    query: { value },
    busyText: translate("common.saving"),
    idleText: translate("dfplayer.volume_save"),
    successText: translate("common.saved"),
    errorText: translate("dfplayer.volume_save_failed"),
    reload: true
  });
}

async function saveDfplayerMode() {
  const value = document.getElementById("dfplayer-mode-select").value;
  await runQueryButtonRequest(document.getElementById("dfplayer-mode-save-button"), {
    endpoint: getDfplayerModeSetUrl(),
    query: { value },
    busyText: translate("common.saving"),
    idleText: translate("dfplayer.mode_save"),
    successText: translate("common.saved"),
    errorText: translate("dfplayer.mode_save_failed"),
    reload: true
  });
}

async function saveDfplayerBellFlags() {
  const m15 = document.getElementById("dfplayer-bell-15").checked ? "on" : "off";
  const m30 = document.getElementById("dfplayer-bell-30").checked ? "on" : "off";
  const m45 = document.getElementById("dfplayer-bell-45").checked ? "on" : "off";
  await runQueryButtonRequest(document.getElementById("dfplayer-bell-save-button"), {
    endpoint: getDfplayerBellFlagsSetUrl(),
    query: { m15, m30, m45 },
    busyText: translate("common.saving"),
    idleText: translate("dfplayer.bell_save"),
    successText: translate("common.saved"),
    errorText: translate("dfplayer.bell_save_failed"),
    reload: true
  });
}

async function saveDfplayerSpeakCycle() {
  const input = document.getElementById("dfplayer-speak-cycle-input");
  const number = readNumberInputOrReport(input, 0, 255);

  if (number === null) {
    return;
  }

  const value = number;
  input.value = String(value);
  await runQueryButtonRequest(document.getElementById("dfplayer-speak-save-button"), {
    endpoint: getDfplayerSpeakCycleSetUrl(),
    query: { value },
    busyText: translate("common.saving"),
    idleText: translate("dfplayer.speak_cycle_save"),
    successText: translate("common.saved"),
    errorText: translate("dfplayer.speak_cycle_save_failed"),
    reload: true
  });
}

async function saveDfplayerSilenceStart() {
  await saveDfplayerSilenceTime("dfplayer-silence-start-input", "dfplayer-silence-start-save-button", getDfplayerSilenceStartSetUrl(), translate("dfplayer.silence_start_save"), translate("dfplayer.silence_start_save_failed"));
}

async function saveDfplayerSilenceStop() {
  await saveDfplayerSilenceTime("dfplayer-silence-stop-input", "dfplayer-silence-stop-save-button", getDfplayerSilenceStopSetUrl(), translate("dfplayer.silence_stop_save"), translate("dfplayer.silence_stop_save_failed"));
}

async function saveDfplayerSilenceTime(inputId, buttonId, endpoint, buttonText, errorText) {
  const { hour, minute } = parseTimeInput(document.getElementById(inputId).value || "00:00");
  await runQueryButtonRequest(document.getElementById(buttonId), {
    endpoint,
    query: { hour, minute },
    busyText: translate("common.saving"),
    idleText: buttonText,
    successText: translate("common.saved"),
    errorText,
    reload: true
  });
}

async function playDfplayerTrack() {
  const values = readNumberFieldsOrReport([
    { name: "folder", id: "dfplayer-folder-input", min: 0, max: 255 },
    { name: "track", id: "dfplayer-track-input", min: 0, max: 255 }
  ]);

  if (!values) {
    return;
  }

  const { folder, track } = values;
  await runQueryButtonRequest(document.getElementById("dfplayer-play-button"), {
    endpoint: getDfplayerPlayUrl(),
    query: { folder, track },
    busyText: translate("common.running"),
    idleText: translate("dfplayer.play_track"),
    successText: translate("common.ready"),
    errorText: translate("dfplayer.play_failed"),
    successStatusText: translate("dfplayer.play_started")
  });
}

async function saveDfplayerAlarm(idx) {
  const active = document.getElementById("df-alarm-active-" + idx).checked ? "on" : "off";
  const from = document.getElementById("df-alarm-from-" + idx).value;
  const to = document.getElementById("df-alarm-to-" + idx).value;
  const { hour, minute } = parseTimeInput(document.getElementById("df-alarm-time-" + idx).value || "00:00");
  const idleText = translate("dfplayer.alarm_title") + " " + String(idx + 1).padStart(3, "0") + " " + translate("common.save").toLowerCase();
  await runIndexedQueryButtonRequest('[data-alarm-save="%idx%"]', idx, {
    endpoint: getDfplayerAlarmSetUrl(),
    query: { idx, active, from, to, hour, minute },
    busyText: translate("common.saving"),
    idleText,
    successText: translate("common.saved"),
    errorText: translate("dfplayer.alarm_save_failed"),
    reload: true
  });
}

async function saveOverlay(idx) {
  const active = document.getElementById("ov-active-" + idx).checked ? "on" : "off";
  const type = document.getElementById("ov-type-" + idx).value;
  const typeNumber = Number(type);
  const typeNames = getOverlayTypeNames();

  // L205: Am Geraet stand overlay[0].type auf 14, gueltig sind 0..10. Das Auswahlfeld
  // zeigte deshalb den ERSTEN Eintrag ("Keins"), und wer das Overlay dann speicherte,
  // schrieb still type=0 und loeschte die Einstellung des Nutzers -- dieselbe Klasse
  // wie die stillen Klemmungen im ESP, nur in der Gegenrichtung.
  //
  // buildNamedOptions() macht den unbekannten Wert seither als eigenen, markierten
  // Eintrag sichtbar und haelt ihn fest. Hier wird er abgewiesen statt gesendet: Das
  // Geraet wuerde ihn seit Runde 1 ohnehin abweisen, die Meldung steht so aber am Feld
  // und nennt den Rohwert. Entscheidend ist, was NICHT passiert -- der Geraetewert wird
  // nicht ueberschrieben.
  if (!Number.isInteger(typeNumber) || typeNumber < 0 || typeNumber >= typeNames.length) {
    announceStatus(translateFormat("overlays.type_unknown_save", {
      value: String(type),
      min: 0,
      max: typeNames.length - 1
    }), "error");
    return;
  }

  // Die beiden MP3-Felder gingen als rohe .value hinaus und wurden lediglich mit
  // padStart auf Breite gebracht -- "abc" wurde zu "abc/000" (L64, B1i). Geprueft wird
  // nur, wenn sie ueberhaupt gelten: bei jedem anderen Typ sind sie ausgeblendet und
  // duerfen ein Speichern nicht blockieren.
  const mp3Numbers = typeNumber === 7
    ? readNumberFieldsOrReport([
      { name: "folder", id: "ov-folder-" + idx, min: 0, max: 99 },
      { name: "track", id: "ov-track-" + idx, min: 0, max: 999 }
    ])
    : {};

  if (!mp3Numbers) {
    return;
  }

  const value = typeNumber === 1
    ? (document.getElementById("ov-icon-" + idx).value || "")
    : typeNumber === 7
      ? formatOverlayMp3Value(mp3Numbers.folder, mp3Numbers.track)
      : (typeNumber === 6 ? document.getElementById("ov-value-" + idx).value : "");
  const dateCode = document.getElementById("ov-datecode-" + idx).value;
  const month = document.getElementById("ov-month-" + idx).value;
  const day = document.getElementById("ov-day-" + idx).value;

  // Tag und Monat sind seit Runde 1 ein Paar. Die Haelfte davon hat das Geraet frueher
  // still verworfen und "gespeichert" gemeldet -- die Maske sagt es jetzt selbst,
  // statt den Nutzer in eine Fehlermeldung des Geraets laufen zu lassen.
  if (overlayStartDateIsPartial(month, day)) {
    announceStatus(translate("overlays.start_date_incomplete"), "error");
    return;
  }

  // 0 war fuer interval und days die stille Abkuerzung auf die Vorgabe, duration trug
  // sogar einen Doppelvertrag. Beides weist das Geraet jetzt ab; abgefangen wird es
  // hier, damit die Meldung am Feld steht und den erlaubten Bereich nennt.
  const numbers = readNumberFieldsOrReport([
    { name: "interval", id: "ov-interval-" + idx, min: 1, max: 255 },
    { name: "duration", id: "ov-duration-" + idx, min: 5, max: 9 },
    { name: "days", id: "ov-days-" + idx, min: 1, max: 255 }
  ]);

  if (!numbers) {
    return;
  }

  await runIndexedQueryButtonRequest('[data-overlay-save="%idx%"]', idx, {
    endpoint: getOverlaySetUrl(),
    query: {
      idx,
      active,
      type,
      value,
      interval: numbers.interval,
      duration: numbers.duration,
      date_code: dateCode,
      month,
      day,
      days: numbers.days
    },
    busyText: translate("common.saving"),
    idleText: translate("overlays.save"),
    successText: translate("common.saved"),
    errorText: translate("overlays.save_failed"),
    reload: true
  });
}

function isOverlayDraftRow(idx) {
  const card = document.querySelector('.overlay-card[data-overlay-idx="' + idx + '"]');
  return !!(card && card.getAttribute("data-overlay-new") === "1");
}

function getOverlayDraftCard(idx) {
  return document.querySelector('.overlay-card[data-overlay-idx="' + idx + '"]');
}

function serializeOverlayDraftState(idx) {
  return JSON.stringify({
    active: !!document.getElementById("ov-active-" + idx)?.checked,
    type: String(document.getElementById("ov-type-" + idx)?.value || ""),
    icon: String(document.getElementById("ov-icon-" + idx)?.value || ""),
    value: String(document.getElementById("ov-value-" + idx)?.value || ""),
    folder: String(document.getElementById("ov-folder-" + idx)?.value || ""),
    track: String(document.getElementById("ov-track-" + idx)?.value || ""),
    interval: String(document.getElementById("ov-interval-" + idx)?.value || ""),
    duration: String(document.getElementById("ov-duration-" + idx)?.value || ""),
    dateCode: String(document.getElementById("ov-datecode-" + idx)?.value || ""),
    month: String(document.getElementById("ov-month-" + idx)?.value || ""),
    day: String(document.getElementById("ov-day-" + idx)?.value || ""),
    days: String(document.getElementById("ov-days-" + idx)?.value || "")
  });
}

function captureOverlayDraftBaseline(idx) {
  const card = getOverlayDraftCard(idx);

  if (!card || card.getAttribute("data-overlay-new") !== "1") {
    return;
  }

  card.dataset.overlayBaseline = serializeOverlayDraftState(idx);
}

function isOverlayDraftDirty(idx) {
  const card = getOverlayDraftCard(idx);

  if (!card || card.getAttribute("data-overlay-new") !== "1") {
    return false;
  }

  return serializeOverlayDraftState(idx) !== String(card.dataset.overlayBaseline || "");
}

function updateOverlayDraftActions(idx) {
  const cancelButton = document.querySelector('[data-overlay-cancel="' + idx + '"]');

  if (!cancelButton) {
    return;
  }

  cancelButton.classList.toggle("is-hidden", !isOverlayDraftDirty(idx));
}

async function displayOverlay(idx) {
  await runIndexedQueryButtonRequest('[data-overlay-display="%idx%"]', idx, {
    endpoint: getOverlayDisplayUrl(),
    query: { idx },
    busyText: translate("common.showing"),
    idleText: translate("overlays.display"),
    successText: translate("common.started"),
    errorText: translate("overlays.show_failed"),
    successStatusText: translateFormat("overlays.show_status", { idx })
  });
}

async function deleteOverlay(idx) {
  await runIndexedQueryButtonRequest('[data-overlay-delete="%idx%"]', idx, {
    endpoint: getOverlayDeleteUrl(),
    query: { idx },
    busyText: translate("common.deleting"),
    idleText: translate("overlays.delete"),
    successText: translate("common.deleted"),
    errorText: translate("overlays.delete_failed"),
    successStatusText: translate("overlays.deleted"),
    reload: true
  });
}

function cancelOverlayEdit(idx) {
  const settings = getCurrentSettingsSnapshot();
  const button = document.querySelector('[data-overlay-cancel="' + idx + '"]');

  if (!settings) {
    return;
  }

  if (button) {
    button.disabled = true;
    button.classList.remove("is-hidden");
    button.classList.add("is-success");
    button.textContent = translate("common.canceled");
  }

  window.setTimeout(() => {
    markEditsPersisted();
    renderOverlayRows(settings);
    announceStatus("Neues Overlay verworfen", "info");
  }, 140);
}

async function saveTimerRow(idx, isAmbilight, options) {
  const opts = options || {};
  const prefix = isAmbilight ? "at" : "t";
  const active = document.getElementById(prefix + "-active-" + idx).checked ? "on" : "off";
  const switchOn = document.getElementById(prefix + "-action-" + idx).value || "off";
  const from = document.getElementById(prefix + "-from-" + idx).value;
  const to = document.getElementById(prefix + "-to-" + idx).value;
  const { hour, minute } = parseTimeInput(document.getElementById(prefix + "-time-" + idx).value || "00:00");
  const endpoint = isAmbilight ? getAmbilightTimerSetUrl() : getTimerSetUrl();
  await runIndexedQueryButtonRequest('[data-' + prefix + '-save="%idx%"]', idx, {
    endpoint,
    query: { idx, active, switch_on: switchOn, from, to, hour, minute },
    busyText: translate("common.saving"),
    idleText: translate("common.save"),
    successText: translate("common.saved"),
    errorText: translate("timers.save_failed"),
    reload: opts.reload !== false
  });
}

async function clearTimerRow(idx, isAmbilight) {
  const prefix = isAmbilight ? "at" : "t";
  const endpoint = isAmbilight ? getAmbilightTimerSetUrl() : getTimerSetUrl();
  await runIndexedQueryButtonRequest('[data-' + prefix + '-clear="%idx%"]', idx, {
    endpoint,
    query: { idx, active: "off", switch_on: "off", from: 0, to: 0, hour: 0, minute: 0 },
    busyText: translate("common.clearing"),
    idleText: translate("timers.clear_slot"),
    successText: translate("timers.cleared"),
    errorText: translate("timers.clear_failed"),
    reload: true
  });
}

async function saveAllTimerRows(isAmbilight) {
  const button = document.getElementById(isAmbilight ? "ambilight-timer-save-all-button" : "timer-save-all-button");
  const root = document.getElementById(isAmbilight ? "ambilight-timer-list" : "timer-list");
  const prefix = isAmbilight ? "at" : "t";
  const buttons = Array.from(root.querySelectorAll('[data-' + prefix + '-save]'));
  const originalText = button.textContent;

  beginButtonFeedback(button, translate("common.saving"));

  try {
    for (const slotButton of buttons) {
      const idx = Number(slotButton.getAttribute('data-' + prefix + '-save'));
      await saveTimerRow(idx, isAmbilight, { reload: false });
    }
    markEditsPersisted();
    await loadData();
    announceStatus(translate("timers.saved_all"), "ok");
    finishButtonFeedback(button, originalText, "success", translate("common.saved"));
  } catch (error) {
    announceStatus(translate("timers.save_all_failed"), "error");
    finishButtonFeedback(button, originalText, "error", translate("common.error"));
  }
}

async function toggleFlagButton(id, endpoint) {
  const button = document.getElementById(id);
  const current = button.dataset.state === "on" ? "on" : "off";
  const next = current === "on" ? "off" : "on";

  beginButtonFeedback(button, "schaltet...");

  try {
    await apiFetch(endpoint + "?value=" + next);
    markEditsPersisted();
    await loadData();
    finishButtonFeedback(button, button.dataset.restoreText || button.textContent, "success", next === "on" ? "aktiviert" : "deaktiviert", true);
  } catch (error) {
    // Eigener Pfad neben runButtonRequest, deshalb braucht er dieselbe Behandlung:
    // sonst meldete gerade dieser Schalter eine Abweisung des Geraets nur als
    // allgemeinen Fehler. (Die hartcodierten Texte hier gehoeren zu Massnahme 18.)
    announceStatus(describeApiError(error, translate("common.toggle_failed")), "error");
    finishButtonFeedback(button, button.dataset.restoreText || button.textContent, "error", "Fehler", true);
  }
}

function renderList(id, items) {
  const root = document.getElementById(id);
  root.innerHTML = items.map(([label, value]) => (
    '<div class="info-item"><span class="label">' + escapeHtml(label) + '</span><strong>' + escapeHtml(value) + "</strong></div>"
  )).join("");
}

function renderHealthList(settings, ambilightOnline, dfplayerOnline) {
  const meta = getHealthUiMeta(settings, ambilightOnline, dfplayerOnline);
  const root = document.getElementById("health-list");
  root.innerHTML = [
    '<div class="info-item"><span class="label">RTC</span><strong>' + escapeHtml(meta.rtcOnline) + "</strong></div>",
    '<div class="info-item"><span class="label">EEPROM</span><strong>' + escapeHtml(meta.eepromOnline) + "</strong></div>",
    '<div class="info-item"><span class="label">EEPROM Version</span><strong>' + escapeHtml(meta.eepromVersion) + "</strong></div>",
    '<div class="info-item"><span class="label">Ambilight</span><select id="health-ambilight-select" class="inline-select"><option value="on">Online</option><option value="off">Offline</option></select></div>',
    '<div class="info-item"><span class="label">DFPlayer</span><strong>' + escapeHtml(meta.dfplayerOnline) + "</strong></div>",
    '<div class="info-item"><span class="label">DFPlayer Version</span><strong>' + escapeHtml(meta.dfplayerVersion) + "</strong></div>"
  ].join("");

  const select = document.getElementById("health-ambilight-select");
  if (select) {
    select.value = meta.ambilightValue;
  }
}

function renderWordclock(active, settings, layoutPreview) {
  const meta = getWordclockRenderMeta(active, settings, layoutPreview);
  const root = document.getElementById("wordclock-grid");

  if (meta.renderSignature === lastWordclockRenderSignature) {
    scheduleWordclockSizing();
    return;
  }

  lastWordclockRenderSignature = meta.renderSignature;
  ensureWordclockResizeObserver();
  root.innerHTML = "";
  const columnCount = Math.max(...meta.rows.map((row) => row.length), 1);
  const activeSet = buildActiveWordSet(settings, meta.preview, meta.rows);

  root.style.gridTemplateColumns = "repeat(" + columnCount + ", minmax(0, 1fr))";
  root.style.gridTemplateRows = "";
  root.dataset.rows = String(meta.rows.length);
  root.dataset.columns = String(columnCount);
  root.classList.toggle("is-wide-layout", columnCount > 12);
  root.classList.toggle("is-dense-layout", columnCount > 16);

  meta.rows.forEach((row, rowIndex) => {
    row.split("").forEach((char, colIndex) => {
      const cell = document.createElement("span");
      cell.textContent = char === "*" || char === "#" ? " " : char;
      if (active && activeSet.has(rowIndex + "-" + colIndex)) {
        cell.className = "active";
      }
      root.appendChild(cell);
    });
  });

  renderWordclockCorners(settings, meta.preview);
  scheduleWordclockSizing();
}

function scheduleWordclockSizing() {
  if (wordclockSizingFrame) {
    window.cancelAnimationFrame(wordclockSizingFrame);
  }
  if (wordclockSizingTimeout) {
    window.clearTimeout(wordclockSizingTimeout);
    wordclockSizingTimeout = 0;
  }
  wordclockSizingFrame = window.requestAnimationFrame(() => {
    wordclockSizingFrame = 0;
    updateWordclockSizing();
  });
  wordclockSizingTimeout = window.setTimeout(() => {
    wordclockSizingTimeout = 0;
    updateWordclockSizing();
  }, 140);
}

function ensureWordclockResizeObserver() {
  if (wordclockResizeObserver || typeof ResizeObserver === "undefined") {
    return;
  }

  const panel = document.querySelector(".wordclock-panel");
  if (!panel) {
    return;
  }

  wordclockResizeObserver = new ResizeObserver(() => {
    scheduleWordclockSizing();
  });
  wordclockResizeObserver.observe(panel);
}

function updateWordclockSizing() {
  const root = document.getElementById("wordclock-grid");
  const panel = root ? root.closest(".wordclock-panel") : null;
  if (!root || !panel) {
    return;
  }

  const rowCount = Number(root.dataset.rows || 0);
  const columnCount = Number(root.dataset.columns || 0);
  if (!rowCount || !columnCount) {
    return;
  }

  const panelSize = Math.min(panel.clientWidth, panel.clientHeight);
  const contentInset = 60;
  const contentSize = Math.max(panelSize - contentInset * 2, panelSize * 0.74);
  const contentWidth = contentSize;
  const contentHeight = contentSize;

  let columnGap = 8;
  if (columnCount > 16) {
    columnGap = 3;
  } else if (columnCount > 12) {
    columnGap = 5;
  }

  const usableWidth = contentWidth - (columnCount - 1) * columnGap;
  const fontSizeFromWidth = (usableWidth / columnCount) * 0.78;
  const maxFontSizeFromHeight = (contentHeight / rowCount) * 0.82;
  const fontSize = Math.max(10, Math.min(fontSizeFromWidth, maxFontSizeFromHeight));
  let rowGap = rowCount > 1 ? (contentHeight - rowCount * fontSize) / (rowCount - 1) : 0;
  if (rowCount > 14) {
    rowGap = Math.max(2, rowGap);
  } else if (rowCount > 10) {
    rowGap = Math.max(4, rowGap);
  } else {
    rowGap = Math.max(6, rowGap);
  }

  root.style.setProperty("--wc-content-width", contentWidth.toFixed(2) + "px");
  root.style.setProperty("--wc-content-height", contentHeight.toFixed(2) + "px");
  root.style.setProperty("--wc-content-inset", contentInset + "px");
  root.style.setProperty("--wc-column-gap", columnGap + "px");
  root.style.setProperty("--wc-row-gap", rowGap + "px");
  root.style.setProperty("--wc-font-size", fontSize.toFixed(2) + "px");
}

function buildActiveWordSet(settings, layoutPreview, rows) {
  if (layoutPreview && layoutPreview.table) {
    return buildLayoutActiveWordSet(settings, layoutPreview.table);
  }

  const current = settings && settings.tmvars ? settings.tmvars[0] : null;
  const hour = current ? Number(current.hour || 0) : new Date().getHours();
  const minute = current ? Number(current.minute || 0) : new Date().getMinutes();
  const set = new Set(["0-0", "0-1", "0-3", "0-4", "0-5"]);

  if (minute >= 45) {
    ["3-0", "3-1", "3-2"].forEach((key) => set.add(key));
  } else if (minute >= 30) {
    ["4-0", "4-1", "4-2", "4-3"].forEach((key) => set.add(key));
  } else if (minute >= 15) {
    ["2-0", "2-1", "2-2", "2-3", "2-4", "2-5", "2-6"].forEach((key) => set.add(key));
  } else {
    ["9-7", "9-8", "9-9", "9-10"].forEach((key) => set.add(key));
  }

  const words = ["ZWOLF", "EINS", "ZWEI", "DREI", "VIER", "FUNF", "SECHS", "SIEBEN", "ACHT", "NEUN", "ZEHN", "ELF"];
  const label = words[hour % 12];
  const pos = findWord(label, rows || fallbackWordclockRows);
  pos.forEach((key) => set.add(key));
  return set;
}

function findWord(word, rows) {
  const lines = rows || fallbackWordclockRows;
  for (let row = 0; row < lines.length; row += 1) {
    const index = lines[row].indexOf(word);
    if (index >= 0) {
      return word.split("").map((_, offset) => row + "-" + (index + offset));
    }
  }
  return [];
}

async function loadWordclockLayoutPreview(updateTableInfo, settings) {
  const currentTable = normalizeLayoutFileName(getUpdateTableCurrentFile(updateTableInfo));
  const fallbackPreview = getDefaultLayoutPreview(settings);

  if (!currentTable) {
    return getCurrentLayoutPreview(settings) || fallbackPreview;
  }

  const cachedPreview = getCachedLayoutPreview(currentTable);
  if (cachedPreview) {
    return cachedPreview;
  }

  try {
    const response = await fetchWithTimeout(getFsShowBaseUrl() + encodeURIComponent(currentTable), { cache: "no-store" }, 8000);
    const text = await response.text();
    if (!text || !text.trim()) {
      return getCurrentLayoutPreview(settings) || null;
    }
    const table = parseLayoutTable(text);
    const rows = await getLayoutPreviewRows(currentTable);
    const preview = { file: currentTable, table, rows: rows.length ? rows : fallbackPreview.rows };

    setCachedLayoutPreview(currentTable, preview);
    return preview;
  } catch (error) {
    return getCurrentLayoutPreview(settings) || null;
  }
}

function restoreStoredLayoutPreview() {
  try {
    const raw = window.localStorage.getItem(LAYOUT_PREVIEW_STORAGE_KEY);
    if (!raw) {
      return null;
    }

    const parsed = JSON.parse(raw);
    if (!parsed || !Array.isArray(parsed.rows) || !parsed.rows.length) {
      return null;
    }

    if (parsed.table && (
      !Array.isArray(parsed.table.illumination) ||
      !Array.isArray(parsed.table.modes) ||
      !Array.isArray(parsed.table.hours) ||
      !Array.isArray(parsed.table.minutes)
    )) {
      parsed.table = null;
    }

    return parsed;
  } catch (error) {
    return null;
  }
}

function storeLayoutPreview(preview) {
  try {
    if (!preview || !Array.isArray(preview.rows) || !preview.rows.length) {
      return;
    }

    window.localStorage.setItem(LAYOUT_PREVIEW_STORAGE_KEY, JSON.stringify({
      file: preview.file || "",
      rows: preview.rows,
      table: preview.table || null
    }));
  } catch (error) {
    // Ignore storage failures; the current session preview still works.
  }
}

function parseLayoutTable(text) {
  const normalizedHex = String(text || "").replace(/[^0-9a-fA-F]/g, "").toLowerCase();
  const bytes = [];

  for (let index = 0; index < normalizedHex.length - 1; index += 2) {
    bytes.push(parseInt(normalizedHex.slice(index, index + 2), 16));
  }

  let cursor = 0;
  const readByte = () => bytes[cursor++];
  const readCString = () => {
    const chars = [];
    let value = readByte();

    while (value !== 0 && value !== undefined) {
      chars.push(String.fromCharCode(value));
      value = readByte();
    }

    return chars.join("");
  };

  const versionMagic = readByte();
  const version = versionMagic === TABLES_VERSION_MAGIC ? readByte() : 0;
  const rows = versionMagic === TABLES_VERSION_MAGIC ? readByte() : versionMagic;
  const columns = readByte();
  const wordCount = readByte();
  const illumination = [];

  for (let idx = 0; idx < wordCount; idx += 1) {
    illumination.push({
      row: readByte(),
      col: readByte(),
      len: readByte()
    });
  }

  if (version === 0) {
    readByte();
    readByte();
  }

  const displayModesCount = readByte();
  const modes = [];

  for (let idx = 0; idx < displayModesCount; idx += 1) {
    modes.push({
      hour_idx: readByte(),
      minute_idx: readByte(),
      description: readCString()
    });
  }

  const hourModesCount = readByte();
  const hourCount = readByte();
  readByte();
  const hours = [];

  for (let mode = 0; mode < hourModesCount; mode += 1) {
    const modeEntries = [];

    for (let hour = 0; hour < hourCount; hour += 1) {
      const words = [];
      let value = readByte();

      while (value !== undefined) {
        words.push(value);
        if (value === 0) {
          break;
        }
        value = readByte();
      }

      modeEntries.push(words);
    }

    hours.push(modeEntries);
  }

  const minuteModesCount = readByte();
  const minuteCount = readByte();
  readByte();
  const minutes = [];

  for (let mode = 0; mode < minuteModesCount; mode += 1) {
    const modeEntries = [];

    for (let minute = 0; minute < minuteCount; minute += 1) {
      const flags = readByte();
      const words = [];
      let value = readByte();

      while (value !== undefined) {
        words.push(value);
        if (value === 0) {
          break;
        }
        value = readByte();
      }

      modeEntries.push({ flags, words });
    }

    minutes.push(modeEntries);
  }

  return { normalizedHex, rows, columns, illumination, modes, hours, minutes, hourCount, minuteCount };
}

function normalizeLayoutFileName(fileName) {
  return String(fileName || "")
    .split("/")
    .pop()
    .trim()
    .toLowerCase();
}

function getDefaultLayoutPreview(settings) {
  const fileName = getDefaultLayoutPreviewFile(settings);
  const rows = Array.isArray(DEFAULT_LAYOUT_PREVIEW_ROWS[fileName])
    ? DEFAULT_LAYOUT_PREVIEW_ROWS[fileName].slice()
    : fallbackWordclockRows.slice();

  return {
    file: fileName,
    table: null,
    rows
  };
}

function buildLayoutActiveWordSet(settings, table) {
  if (!table || !Array.isArray(table.modes) || !table.modes.length) {
    return new Set();
  }

  const current = settings && settings.tmvars ? settings.tmvars[0] : {};
  const displayFlags = settings && settings.numvars ? (settings.numvars[NUM.DISPLAY_FLAGS] || 0) : 0;
  let mode = settings && settings.numvars ? Number(settings.numvars[NUM.DISPLAY_MODE] || 0) : 0;
  let hour = Number(current.hour || 0);
  let minute = Number(current.minute || 0);

  if (mode >= table.modes.length) {
    mode = 0;
  }

  const modeInfo = table.modes[mode];
  const minuteMode = table.minutes[modeInfo.minute_idx] || [];
  const hourMode = table.hours[modeInfo.hour_idx] || [];
  const minuteIndex = table.minuteCount === 12 ? Math.floor(minute / 5) : minute;
  const minuteEntry = minuteMode[Math.min(minuteIndex, Math.max(minuteMode.length - 1, 0))];

  if (!minuteEntry) {
    return new Set();
  }

  const activeWords = new Set();
  const showItIs = !!(displayFlags & 0x01);
  let pmMode = 0;
  let isMidnight = false;

  minuteEntry.words.forEach((wordIdx) => {
    if (wordIdx > 0) {
      activeWords.add(wordIdx);
    }
  });

  if (hour >= 12) {
    pmMode = 1;
  }

  if (minuteEntry.flags & MDF_HOUR_OFFSET_1) {
    hour += 1;
  } else if (minuteEntry.flags & MDF_HOUR_OFFSET_2) {
    hour += 2;
  }

  if (hour === 0 || hour === 24) {
    isMidnight = true;
  }

  while (hour >= table.hourCount) {
    hour -= table.hourCount;
  }

  const hourWords = hourMode[hour] || [];

  for (let idx = 0; idx < hourWords.length && hourWords[idx] !== 0; idx += 1) {
    if (hourWords[idx] === WP_IF_MINUTE_IS_0) {
      activeWords.add(hourWords[minute === 0 ? idx + 1 : idx + 2]);
      idx += 2;
    } else if (hourWords[idx] === WP_IF_HOUR_IS_0) {
      activeWords.add(hourWords[isMidnight ? idx + 1 : idx + 2]);
      idx += 2;
    } else {
      activeWords.add(hourWords[idx]);
    }
  }

  const activeCells = new Set();
  const fullOrHalfHour = table.minuteCount === 12
    ? (minuteIndex === 0 || minuteIndex === Math.floor(table.minuteCount / 2))
    : (minute === 0 || minute === 30);

  activeWords.forEach((wordIdx) => {
    const illumination = table.illumination[wordIdx];

    if (!illumination) {
      return;
    }

    const isItIsWord = !!(illumination.len & ILLUMINATION_FLAG_IT_IS);
    const isAmWord = !!(illumination.len & ILLUMINATION_FLAG_AM);
    const isPmWord = !!(illumination.len & ILLUMINATION_FLAG_PM);
    const length = illumination.len & ILLUMINATION_LEN_MASK;
    let doShow = true;

    if (!fullOrHalfHour && isItIsWord && !showItIs) {
      doShow = false;
    } else if (!pmMode && isPmWord) {
      doShow = false;
    } else if (pmMode && isAmWord) {
      doShow = false;
    }

    if (!doShow) {
      return;
    }

    for (let offset = 0; offset < length; offset += 1) {
      activeCells.add(illumination.row + "-" + (illumination.col + offset));
    }
  });

  return activeCells;
}

function renderWordclockCorners(settings, layoutPreview) {
  const corners = document.getElementById("wordclock-corners");

  if (!corners) {
    return;
  }

  const previewMeta = getLayoutPreviewMeta(settings, layoutPreview);

  corners.classList.toggle("is-hidden", !previewMeta.is12hLayout);

  corners.querySelectorAll(".wordclock-corner").forEach((corner, index) => {
    corner.classList.toggle("is-active", index < previewMeta.activeCornerCount);
  });
}

function isTwelveHourLayout(layoutPreview, config) {
  const backendValue = getResolvedLayoutMeta(null, null, null).isTwelveHour;
  if (backendValue !== null) {
    return backendValue;
  }

  if (layoutPreview && typeof layoutPreview.file === "string" && layoutPreview.file.indexOf("wc12h-") === 0) {
    return true;
  }

  return (config & HW.WC_MASK) === HW.WC_12H;
}

function decodeHardware(config) {
  const backendLabels = getResolvedHardwareMeta(null, null, null).labels;
  if (backendLabels) {
    return {
      hardware: backendLabels.hardware || "unbekannt",
      processor: backendLabels.processor || "unbekannt",
      board: backendLabels.board || "unbekannt",
      frequency: backendLabels.frequency || "unbekannt",
      oscillator: backendLabels.oscillator || "unbekannt",
      display: backendLabels.display || "unbekannt"
    };
  }

  const wc = config & HW.WC_MASK;
  const stm32 = config & HW.STM32_MASK;
  const led = config & HW.LED_MASK;
  const osc = config & HW.OSC_MASK;

  return {
    hardware: ({ 0: "WC24h", 8: "WC12h", 16: "uClock" }[wc] || "unbekannt"),
    processor: ({
      0: "STM32F103C8",
      1: "STM32F401RE",
      2: "STM32F411RE",
      3: "STM32F446RE",
      4: "STM32F407VE",
      5: "STM32F401CC",
      6: "STM32F411CE"
    }[stm32] || "unbekannt"),
    board: ({
      0: "BluePill",
      1: "Nucleo",
      2: "Nucleo",
      3: "Nucleo",
      4: "BlackBoard",
      5: "BlackPill",
      6: "BlackPill"
    }[stm32] || "unbekannt"),
    frequency: ({
      0: "72 MHz",
      1: "84 MHz",
      2: "100 MHz",
      3: "180 MHz",
      4: "168 MHz",
      5: "84 MHz",
      6: "100 MHz"
    }[stm32] || "unbekannt"),
    oscillator: ({ 0: "8 MHz", 512: "25 MHz" }[osc] || "unbekannt"),
    display: ({
      0: "WS2812 GRB",
      64: "WS2812 RGB",
      128: "APA102 RGB",
      192: "SK6812 RGB",
      256: "SK6812 RGBW",
      320: "TFT RGB"
    }[led] || "unbekannt")
  };
}

function isAmbilightOnline(settings, debugOverrides) {
  const persistedState = getPersistedAmbilightState();

  if (debugOverrides.ambilight === "on") {
    return true;
  }

  if (persistedState) {
    return persistedState === "on";
  }

  return !!settings.numvars[NUM.AMBILIGHT_IS_UP];
}

function isDfplayerOnline(settings, debugOverrides) {
  return debugOverrides.dfplayer === "on" ? true : !!settings.numvars[NUM.DFPLAYER_IS_UP];
}

function getModuleStateUiMeta(settings, debugOverrides) {
  return {
    ambilightOnline: isAmbilightOnline(settings, debugOverrides),
    dfplayerOnline: isDfplayerOnline(settings, debugOverrides)
  };
}

function getDisplayFeatureUiMeta(config, settings, debugOverrides) {
  const ledCapabilities = getLedCapabilities(config, debugOverrides);
  const hasTft = hasTftDisplay(config, debugOverrides);
  const useRgbw = isRgbwUiActive(settings, ledCapabilities);

  return {
    ledCapabilities,
    hasTft,
    useRgbw
  };
}

function getUiFeatureState(settings, debugOverrides) {
  const config = settings && settings.numvars ? (settings.numvars[NUM.HARDWARE_CONFIGURATION] || 0) : 0;
  const moduleState = getModuleStateUiMeta(settings, debugOverrides);
  const displayFeatureMeta = getDisplayFeatureUiMeta(config, settings, debugOverrides);

  return {
    config,
    moduleState,
    ledCapabilities: displayFeatureMeta.ledCapabilities,
    hasTft: displayFeatureMeta.hasTft,
    useRgbw: displayFeatureMeta.useRgbw
  };
}

function getFeatureUiMeta(settings, debugOverrides) {
  return getUiFeatureState(settings, debugOverrides);
}

function getHealthUiMeta(settings, ambilightOnline, dfplayerOnline) {
  const displayMeta = getDisplayBackupMeta(settings);

  return {
    rtcOnline: onOff(settings.numvars[NUM.RTC_IS_UP]),
    eepromOnline: onOff(settings.numvars[NUM.EEPROM_IS_UP]),
    eepromVersion: displayMeta.eepromVersion || "-",
    ambilightValue: ambilightOnline ? "on" : "off",
    dfplayerOnline: onOff(dfplayerOnline ? 1 : 0),
    dfplayerVersion: toHex4(settings.numvars[NUM.DFPLAYER_VERSION] || 0)
  };
}

function getOverviewUiMeta(settings, displayPower, ambilightPower, debugOverrides, updateStatus) {
  const featureMeta = getFeatureUiMeta(settings, debugOverrides);
  const displayMeta = getDisplayBackupMeta(settings);
  const networkMeta = getNetworkBackupMeta(settings, getCurrentEepromSettings());
  const climateMeta = getClimateBackupMeta(settings);
  const ambilightMeta = getAmbilightBackupMeta(settings);
  const dfplayerMeta = getDfplayerBackupMeta(settings);
  const configItems = buildOverviewConfigItems(settings, featureMeta, displayMeta, networkMeta, climateMeta, ambilightMeta, dfplayerMeta);

  return {
    hardware: decodeHardware(featureMeta.config || 0),
    ledCapabilities: featureMeta.ledCapabilities,
    ambilightOnline: featureMeta.moduleState.ambilightOnline,
    dfplayerOnline: featureMeta.moduleState.dfplayerOnline,
    displayPowerLabel: displayPower === "on" ? translate("climate.status_on") : translate("climate.status_off"),
    displayPowerState: displayPower === "on" ? "on" : "off",
    ambilightPowerLabel: featureMeta.moduleState.ambilightOnline ? (ambilightPower === "on" ? translate("climate.status_on") : translate("climate.status_off")) : translate("common.offline"),
    ambilightPowerState: featureMeta.moduleState.ambilightOnline ? (ambilightPower === "on" ? "on" : "off") : "offline",
    firmwareVersion: displayMeta.firmwareVersion || "-",
    espVersion: getUpdateAvailableVersion(updateStatus, "esp_version") || displayMeta.espVersion || "-",
    lastStartLabel: formatLastStartFromSettings(settings),
    configItems
  };
}

function buildOverviewConfigItems(settings, featureMeta, displayMeta, networkMeta, climateMeta, ambilightMeta, dfplayerMeta) {
  const configItems = [
    [translate("overview.display_mode"), getDisplayModeName(settings, displayMeta.mode)],
    [translate("overview.brightness"), String(displayMeta.brightness)],
    [translate("overview.auto_brightness"), displayMeta.automaticBrightness ? "on" : "off"],
    [translate("overview.led_capabilities"), featureMeta.ledCapabilities.label],
    [translate("overview.timeserver"), networkMeta.timeserver || "-"],
    [translate("overview.ticker_delay"), String(displayMeta.tickerDeceleration || 0)]
  ];

  if (settings.strvars[STR.RESET_CAUSE]) {
    configItems.unshift([translate("overview.last_stm32_restart"), settings.strvars[STR.RESET_CAUSE]]);
  }

  const lastStartLabel = formatLastStartFromSettings(settings);
  configItems.unshift([translate("overview.last_start"), lastStartLabel]);

  const weatherLocation = climateMeta.weatherCity
    ? climateMeta.weatherCity
    : ((climateMeta.weatherLon || climateMeta.weatherLat)
      ? (climateMeta.weatherLon || "-") + " / " + (climateMeta.weatherLat || "-")
      : "");

  if (weatherLocation) {
    configItems.splice(configItems.length - 1, 0, [translate("overview.weather_location"), weatherLocation]);
  }

  if (displayMeta.tickerText) {
    configItems.splice(configItems.length - 1, 0, [translate("overview.ticker"), displayMeta.tickerText]);
  }

  if (displayMeta.dateTickerFormat) {
    configItems.splice(configItems.length - 1, 0, [translate("overview.date_format"), displayMeta.dateTickerFormat]);
  }

  if (featureMeta.moduleState.ambilightOnline) {
    configItems.splice(4, 0,
      [translate("overview.ambilight_mode"), getAmbilightModeName(settings)],
      [translate("overview.ambilight_brightness"), String(ambilightMeta.brightness || 0)],
      [translate("overview.ambilight_leds"), String(ambilightMeta.leds || 0)],
      [translate("overview.ambilight_offset"), String(ambilightMeta.offset || 0)]
    );
  }

  if (featureMeta.moduleState.dfplayerOnline) {
    configItems.push(
      [translate("overview.dfplayer_mode"), getDfplayerModeName(dfplayerMeta.mode || 0)],
      [translate("overview.dfplayer_volume"), String(dfplayerMeta.volume || 0)],
      [translate("overview.speak_cycle"), String(dfplayerMeta.speakCycle || 0)]
    );
  }

  return configItems;
}

function getColorUiMeta(settings, ambilightOnline, debugOverrides) {
  const featureMeta = getFeatureUiMeta(settings, debugOverrides);
  const colorAnimationMode = Number(settings.numvars[NUM.COLOR_ANIMATION_MODE] || 0);

  return {
    capabilities: featureMeta.ledCapabilities,
    useRgbw: featureMeta.useRgbw,
    colorAnimationMode,
    persistedLiveColor: colorAnimationMode !== 0 ? restoreStoredLiveDisplayColor(colorAnimationMode) : null,
    displayColor: settings.dspcolors[0] || { red: 63, green: 45, blue: 18, white: 0 },
    ambilightColor: settings.dspcolors[1] || { red: 63, green: 45, blue: 18, white: 0 },
    markerColor: settings.dspcolors[2] || { red: 63, green: 45, blue: 18, white: 0 },
    ambilightOnline
  };
}

function getPersistedAmbilightState() {
  try {
    const value = localStorage.getItem(AMBILIGHT_STORAGE_KEY);
    return value === "on" || value === "off" ? value : "";
  } catch (error) {
    return "";
  }
}

function setPersistedAmbilightState(state) {
  try {
    if (state === "on" || state === "off") {
      localStorage.setItem(AMBILIGHT_STORAGE_KEY, state);
    } else {
      localStorage.removeItem(AMBILIGHT_STORAGE_KEY);
    }
  } catch (error) {
  }
}

function applyPersistedAmbilightState(settings) {
  const persistedState = getPersistedAmbilightState();

  if (!persistedState) {
    return;
  }

  const persistedValue = persistedState === "on" ? 1 : 0;
  const currentValue = settings.numvars[NUM.AMBILIGHT_IS_UP] ? 1 : 0;

  if (currentValue !== persistedValue) {
    syncPersistedAmbilightState(persistedState);
  }

  settings.numvars[NUM.AMBILIGHT_IS_UP] = persistedValue;
}

async function syncPersistedAmbilightState(state) {
  const desired = state === "on" ? "on" : "off";

  try {
    await apiFetch(getAmbilightOnlineSetUrl() + "?value=" + desired);
  } catch (error) {
  }
}

function hasTftDisplay(config, debugOverrides) {
  if (debugOverrides.tft === "on") {
    return true;
  }

  const displayInfo = getResolvedHardwareMeta(null, null, null).display;
  if (displayInfo) {
    return displayInfo.hasTft;
  }

  return isTftDisplayFromConfig(config);
}

function getLedCapabilities(config, debugOverrides) {
  if (debugOverrides.color === "rgbw") {
    return {
      hasColor: true,
      whiteChannel: true,
      mode: "rgbw",
      label: "RGBW (Debug Override)",
      note: translate("display.led_note_debug_rgbw")
    };
  }

  if (debugOverrides.color === "rgb") {
    return {
      hasColor: true,
      whiteChannel: false,
      mode: "rgb",
      label: "RGB (Debug Override)",
      note: translate("display.led_note_debug_rgb")
      };
  }

  const displayInfo = getResolvedHardwareMeta(null, null, null).display;
  if (displayInfo && displayInfo.mode) {
    const mode = displayInfo.mode;
    const hasTft = displayInfo.hasTft;
    const whiteChannel = displayInfo.hasWhiteChannel;

    if (mode === "rgbw") {
      return {
        hasColor: true,
        whiteChannel: true,
        mode: "rgbw",
        label: displayInfo.label || "RGBW",
        note: translate("display.led_note_rgbw")
      };
    }

    if (mode === "rgb" || mode === "tft") {
      return {
        hasColor: true,
        whiteChannel,
        mode: whiteChannel ? "rgbw" : "rgb",
        label: displayInfo.label || (hasTft ? "TFT RGB" : "RGB"),
        note: hasTft
          ? translate("display.led_note_tft")
          : translate("display.led_note_rgb")
      };
    }

    if (mode === "none") {
      return {
        hasColor: false,
        whiteChannel: false,
        mode: "none",
        label: displayInfo.label || translate("display.led_label_none"),
        note: translate("display.led_note_none")
      };
    }
  }

  const led = getDisplayLedMask(config);

  switch (led) {
    case HW.LED_SK6812_RGBW:
      return {
        hasColor: true,
        whiteChannel: true,
        mode: "rgbw",
        label: "RGBW",
        note: translate("display.led_note_rgbw")
      };
    case HW.LED_WS2812_GRB:
    case HW.LED_WS2812_RGB:
    case HW.LED_APA102_RGB:
    case HW.LED_SK6812_RGB:
    case HW.LED_TFT_RGB:
      return {
        hasColor: true,
        whiteChannel: false,
        mode: "rgb",
        label: led === HW.LED_TFT_RGB ? "TFT RGB" : "RGB",
        note: led === HW.LED_TFT_RGB
          ? translate("display.led_note_tft")
          : translate("display.led_note_rgb")
      };
    default:
      return {
        hasColor: false,
        whiteChannel: false,
        mode: "none",
        label: translate("display.led_label_none"),
        note: translate("display.led_note_none")
      };
  }
}

function isRgbwUiActive(settings, capabilities) {
  return !!(capabilities && capabilities.whiteChannel && settings && settings.numvars && settings.numvars[NUM.DISPLAY_USE_RGBW]);
}

function getFsUploadTargets(config) {
  const assetPrefix = getAssetPrefixFromHardwareConfig(config);
  const hasTft = isTftDisplayFromConfig(config);
  const targets = {
    icon: "",
    weather: "",
    tables: "",
    display: ""
  };

  switch (assetPrefix) {
    case "wc24h":
      targets.icon = "wc24h-icon.txt";
      targets.weather = "wc24h-weather.txt";
      targets.tables = "wc24h-tables-local.txt";
      targets.display = hasTft ? "wc24h-display-local.txt" : "";
      break;
    case "wc12h":
      targets.icon = "wc12h-icon.txt";
      targets.weather = "wc12h-weather.txt";
      targets.tables = "wc12h-tables-local.txt";
      targets.display = hasTft ? "wc12h-display-local.txt" : "";
      break;
    case "uc":
      targets.icon = "uc-icon.txt";
      targets.weather = "uc-weather.txt";
      targets.tables = "";
      targets.display = "";
      break;
    default:
      break;
  }

  return targets;
}

// L208 (1): Hier stand eine fest verdrahtete Namenstabelle mit fuenf Eintraegen
// (Normal/Sekunden/Datum/Temperatur/Ticker), die mit den LAYOUT-Modi des Geraets
// nichts zu tun hat -- ab Index 5 gab sie die blosse Zahl aus. Am Geraet gemessen:
// Die Uebersicht zeigte "Normal", das Auswahlfeld direkt darunter
// "SCHWEIZERDEUTSCH 1". Zwei Angaben derselben Sache, die sich widersprechen.
//
// Die Schwesterfunktion getAmbilightModeName() darunter macht es seit jeher richtig
// und gleicht gegen die Geraeteliste ab; genau das passiert jetzt auch hier, ueber
// dieselbe Quelle, aus der das Auswahlfeld seine Eintraege nimmt
// (getDisplayUiMeta().displayModes). Damit koennen die beiden nicht mehr auseinander-
// laufen. Kennt die Liste den Wert nicht, wird er als unbekannter Geraetewert benannt
// statt als erster Listeneintrag ausgegeben -- dieselbe Linie wie L205.
function getDisplayModeName(settings, mode) {
  const modes = getDisplayUiMeta(settings).displayModes;
  const match = (modes || []).find((entry) => entry.idx === mode);

  return match
    ? localizeDisplayModeName(match.name || String(match.idx))
    : translateFormat("common.unknown_device_value", { value: String(mode) });
}

function getAmbilightModeName(settings) {
  const mode = settings.numvars[NUM.AMBILIGHT_MODE] || 0;
  const match = (settings.almodes || []).find((entry) => entry.idx === mode);
  return match ? localizeAmbilightModeName(match.name) : String(mode);
}

function getDfplayerModeName(mode) {
  return ({
    0: translate("common.none"),
    1: translate("dfplayer.mode_bell"),
    2: translate("dfplayer.mode_speech")
  }[mode] || String(mode || 0));
}

function formatRgbwColor(color) {
  if (!color || (
    typeof color.red !== "number" &&
    typeof color.green !== "number" &&
    typeof color.blue !== "number" &&
    typeof color.white !== "number"
  )) {
    return "-";
  }

  return "R " + String(Number(color.red || 0)) +
    " / G " + String(Number(color.green || 0)) +
    " / B " + String(Number(color.blue || 0)) +
    " / W " + String(Number(color.white || 0));
}

function localizeDisplayModeName(name) {
  return ({
    Normal: translate("display.mode_normal"),
    Seconds: translate("display.mode_seconds"),
    Date: translate("display.mode_date"),
    Temperature: translate("display.mode_temperature"),
    Ticker: translate("display.mode_ticker")
  }[name] || name);
}

function localizeAmbilightModeName(name) {
  return ({
    Clock: translate("animations.name_clock"),
    Rainbow: translate("animations.name_rainbow")
  }[name] || name);
}

function localizeAnimationName(name) {
  return ({
    None: translate("animations.name_none"),
    Normal: translate("animations.name_normal"),
    Clock: translate("animations.name_clock"),
    Rainbow: translate("animations.name_rainbow"),
    Temperature: translate("animations.name_temperature"),
    Ticker: translate("animations.name_ticker"),
    Date: translate("animations.name_date"),
    Seconds: translate("animations.name_seconds")
  }[name] || name);
}

function getDebugOverrides() {
  try {
    const parsed = JSON.parse(localStorage.getItem(DEBUG_STORAGE_KEY) || "{}");
    return {
      ambilight: parsed.ambilight || "auto",
      dfplayer: parsed.dfplayer || "auto",
      color: parsed.color || "auto",
      tft: parsed.tft || "auto"
    };
  } catch (error) {
    return {
      ambilight: "auto",
      dfplayer: "auto",
      color: "auto",
      tft: "auto"
    };
  }
}

function saveDebugOverrides(overrides) {
  localStorage.setItem(DEBUG_STORAGE_KEY, JSON.stringify(overrides));
}

function loadDebugOverridesIntoUi() {
  const overrides = getDebugOverrides();
  const ambilight = document.getElementById("debug-ambilight-select");
  const dfplayer = document.getElementById("debug-dfplayer-select");
  const color = document.getElementById("debug-color-select");
  const tft = document.getElementById("debug-tft-select");

  if (!ambilight || !dfplayer || !color || !tft) {
    return;
  }

  ambilight.value = overrides.ambilight;
  dfplayer.value = overrides.dfplayer;
  color.value = overrides.color;
  tft.value = overrides.tft;
}

async function applyDebugOverrides() {
  const button = document.getElementById("debug-apply-button");
  const overrides = {
    ambilight: document.getElementById("debug-ambilight-select").value,
    dfplayer: document.getElementById("debug-dfplayer-select").value,
    color: document.getElementById("debug-color-select").value,
    tft: document.getElementById("debug-tft-select").value
  };

  beginButtonFeedback(button, translate("debug.apply_busy"));
  try {
    saveDebugOverrides(overrides);
    markEditsPersisted();
    document.getElementById("updated-at").textContent = translate("debug.active");
    await loadData();
    finishButtonFeedback(button, translate("system.apply_overrides"), "success", translate("debug.active_short"));
  } catch (error) {
    announceStatus(translate("debug.apply_failed"), "error");
    finishButtonFeedback(button, translate("system.apply_overrides"), "error", translate("common.error"));
  }
}

async function resetDebugOverrides() {
  const button = document.getElementById("debug-reset-button");
  const overrides = {
    ambilight: "auto",
    dfplayer: "auto",
    color: "auto",
    tft: "auto"
  };

  beginButtonFeedback(button, translate("debug.reset_busy"));
  try {
    saveDebugOverrides(overrides);
    loadDebugOverridesIntoUi();
    markEditsPersisted();
    document.getElementById("updated-at").textContent = translate("debug.reset_done");
    await loadData();
    finishButtonFeedback(button, translate("system.reset_overrides"), "success", translate("debug.reset_short"));
  } catch (error) {
    announceStatus(translate("debug.reset_failed"), "error");
    finishButtonFeedback(button, translate("system.reset_overrides"), "error", translate("common.error"));
  }
}

function onOff(value) {
  return value ? "online" : "offline";
}

function formatHalfDegreeValue(value) {
  if (value === null || value === undefined) {
    return translate("common.offline");
  }

  if (value === 0xFF) {
    return translate("common.invalid_value");
  }

  const integer = Math.floor(value / 2);
  const fraction = value % 2 ? ".5" : ".0";
  return integer + fraction + " °C";
}

function decodeTimezone(raw) {
  let offset = raw & 0xff;

  if (raw & 0x100) {
    offset = -offset;
  }

  return {
    offset,
    summertime: !!(raw & 0x200)
  };
}

function toHex4(value) {
  return String(value).toString(16).padStart(4, "0");
}

function rgb63ToHex(color) {
  const red = Math.round(((color.red || 0) / 63) * 255);
  const green = Math.round(((color.green || 0) / 63) * 255);
  const blue = Math.round(((color.blue || 0) / 63) * 255);
  return "#" + [red, green, blue].map((value) => value.toString(16).padStart(2, "0")).join("");
}

function hexToRgb63(hex) {
  const clean = String(hex || "#000000").replace("#", "");
  const red = Math.round((parseInt(clean.slice(0, 2), 16) / 255) * 63);
  const green = Math.round((parseInt(clean.slice(2, 4), 16) / 255) * 63);
  const blue = Math.round((parseInt(clean.slice(4, 6), 16) / 255) * 63);
  return { red, green, blue };
}

function buildColorPreview(color, useRgbw) {
  const rgb = rgb63ToHex(color);
  const white = Math.round(((color.white || 0) / 63) * 255);
  return "linear-gradient(90deg, " + rgb + ", rgb(" + white + ", " + white + ", " + white + "))";
}

function applyWordclockTheme(color, useRgbw, colorAnimationMode) {
  const panel = document.querySelector(".wordclock-panel");

  if (!panel) {
    return;
  }

  const animationMode = Number(colorAnimationMode || 0);
  const hasExplicitColor = !!(color && typeof color.red === "number" && typeof color.green === "number" && typeof color.blue === "number");
  const ledColor = animationMode !== 0 && !hasExplicitColor
    ? "rgb(255, 255, 255)"
    : mixRgbwToCss(color || { red: 63, green: 45, blue: 18, white: 0 }, useRgbw);
  const isRainbow = Number(colorAnimationMode || 0) === 1;
  const activeColor = ledColor;
  const glowStrong = colorWithAlpha(activeColor, 0.42);
  const glowSoft = colorWithAlpha(activeColor, 0.2);
  const inactive = "rgba(0, 0, 0, 0.7)";
  const cornerIdle = "rgba(0, 0, 0, 0.78)";
  const cornerBorder = "rgba(255, 255, 255, 0.08)";
  const themeSignature = [
    String(activeColor),
    String(glowStrong),
    String(glowSoft),
    String(inactive),
    String(cornerIdle),
    String(cornerBorder),
    isRainbow ? "1" : "0"
  ].join("::");

  if (themeSignature === lastWordclockThemeSignature) {
    return;
  }

  lastWordclockThemeSignature = themeSignature;

  panel.classList.toggle("is-rainbow-preview", isRainbow);
  panel.style.setProperty("--wc-active-color", activeColor);
  panel.style.setProperty("--wc-glow-strong", glowStrong);
  panel.style.setProperty("--wc-glow-soft", glowSoft);
  panel.style.setProperty("--wc-inactive-color", inactive);
  panel.style.setProperty("--wc-corner-idle", cornerIdle);
  panel.style.setProperty("--wc-corner-border", cornerBorder);
}

function shouldUseLiveDisplayColor(settings) {
  return !!(
    settings &&
    settings.numvars &&
    Number(settings.numvars[NUM.COLOR_ANIMATION_MODE] || 0) !== 0 &&
    (getActiveModuleName() === "main" || getActiveModuleName() === "system")
  );
}

function getDaylightPreviewColor(settings) {
  const current = settings && settings.tmvars ? (settings.tmvars[0] || {}) : {};
  let hour = Number(current.hour || 0);

  if (!Number.isFinite(hour) || hour < 0) {
    hour = 0;
  }

  hour %= 24;

  return {
    red: DAYLIGHT_RED[hour] || 0,
    green: DAYLIGHT_GREEN[hour] || 0,
    blue: DAYLIGHT_BLUE[hour] || 0,
    white: 0
  };
}

function restoreStoredLiveDisplayColor(mode) {
  try {
    const raw = window.localStorage.getItem(LIVE_DISPLAY_COLOR_STORAGE_KEY);
    if (!raw) {
      return null;
    }

    const parsed = JSON.parse(raw);
    if (!parsed || Number(parsed.mode || 0) !== Number(mode || 0)) {
      return null;
    }

    if (typeof parsed.red !== "number" || typeof parsed.green !== "number" || typeof parsed.blue !== "number") {
      return null;
    }

    return {
      red: Number(parsed.red || 0),
      green: Number(parsed.green || 0),
      blue: Number(parsed.blue || 0),
      white: Number(parsed.white || 0)
    };
  } catch (error) {
    return null;
  }
}

function storeLiveDisplayColor(mode, color) {
  try {
    if (!color) {
      return;
    }

    window.localStorage.setItem(LIVE_DISPLAY_COLOR_STORAGE_KEY, JSON.stringify({
      mode: Number(mode || 0),
      red: Number(color.red || 0),
      green: Number(color.green || 0),
      blue: Number(color.blue || 0),
      white: Number(color.white || 0)
    }));
  } catch (error) {
    // Ignore storage failures; live polling still works.
  }
}

async function refreshLiveDisplayColor() {
  if (isBackgroundPauseActive()) {
    return;
  }

  const settings = getCurrentSettingsSnapshot();
  if (!shouldUseLiveDisplayColor(settings)) {
    return;
  }

  const response = await settleFetchJson(getLiveDisplayColorUrl(), null, 3000);

  if (!response || response.ok !== true) {
    return;
  }

  currentLiveDisplayColor = {
    red: Number(response.red || 0),
    green: Number(response.green || 0),
    blue: Number(response.blue || 0),
    white: Number(response.white || 0)
  };
  storeLiveDisplayColor(settings && settings.numvars
    ? Number(settings.numvars[NUM.COLOR_ANIMATION_MODE] || 0)
    : 0,
  currentLiveDisplayColor);

  if (settings) {
    renderPreviewDebug(settings);
  }

  if (!settings || !shouldUseLiveDisplayColor(settings)) {
    return;
  }

  const colorMeta = getColorUiMeta(settings, !!(settings.numvars && settings.numvars[NUM.AMBILIGHT_IS_UP]), getDebugOverrides());
  const useRgbw = colorMeta.useRgbw;
  const colorAnimationMode = Number(settings.numvars[NUM.COLOR_ANIMATION_MODE] || 0);

  applyWordclockTheme(currentLiveDisplayColor, useRgbw, colorAnimationMode);
}

function syncLiveDisplayColorPolling(settings) {
  // document.hidden mit in der Abbruchbedingung: siehe die Begruendung am
  // visibilitychange-Hoerer. R2-11.
  if (settingsImportInProgress || isBackgroundPauseActive() || document.hidden) {
    if (liveDisplayColorTimer) {
      window.clearInterval(liveDisplayColorTimer);
      liveDisplayColorTimer = 0;
    }
    return;
  }

  const enabled = shouldUseLiveDisplayColor(settings);
  const nextMode = settings && settings.numvars ? Number(settings.numvars[NUM.COLOR_ANIMATION_MODE] || 0) : 0;
  const modeChanged = enabled && nextMode !== lastLiveDisplayColorMode;

  if (!enabled) {
    if (liveDisplayColorTimer) {
      window.clearInterval(liveDisplayColorTimer);
      liveDisplayColorTimer = 0;
    }
    currentLiveDisplayColor = null;
    lastLiveDisplayColorMode = 0;
    return;
  }

  if (modeChanged) {
    currentLiveDisplayColor = restoreStoredLiveDisplayColor(nextMode);
  }

  lastLiveDisplayColorMode = nextMode;

  if (!liveDisplayColorTimer) {
    void refreshLiveDisplayColor();
    liveDisplayColorTimer = window.setInterval(() => {
      void refreshLiveDisplayColor();
    }, LIVE_DISPLAY_COLOR_POLL_INTERVAL_MS);
  } else if (modeChanged || !currentLiveDisplayColor) {
    void refreshLiveDisplayColor();
  }
}

function mixRgbwToCss(color, useRgbw) {
  const base = rgb63ToRgb255(color || {});
  const white = Math.round((((color && color.white) || 0) / 63) * 255);
  const whiteRatio = Math.max(0, Math.min(1, white / 255));
  const mixedRed = Math.round(base.red + (255 - base.red) * whiteRatio);
  const mixedGreen = Math.round(base.green + (255 - base.green) * whiteRatio);
  const mixedBlue = Math.round(base.blue + (255 - base.blue) * whiteRatio);

  return "rgb(" +
    String(mixedRed) + ", " +
    String(mixedGreen) + ", " +
    String(mixedBlue) +
    ")";
}

function rgb63ToRgb255(color) {
  return {
    red: Math.round((((color && color.red) || 0) / 63) * 255),
    green: Math.round((((color && color.green) || 0) / 63) * 255),
    blue: Math.round((((color && color.blue) || 0) / 63) * 255)
  };
}

function colorWithAlpha(rgb, alpha) {
  const match = String(rgb || "").match(/rgb\((\d+), (\d+), (\d+)\)/);

  if (!match) {
    return "rgba(255, 209, 102, " + String(alpha) + ")";
  }

  return "rgba(" + match[1] + ", " + match[2] + ", " + match[3] + ", " + String(alpha) + ")";
}

function minutesToTimeValue(totalMinutes) {
  const hour = Math.floor(totalMinutes / 60) % 24;
  const minute = totalMinutes % 60;
  return String(hour).padStart(2, "0") + ":" + String(minute).padStart(2, "0");
}

function buildWeekdayOptions(selected) {
  const days = ["So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"];
  return days.map((day, idx) => (
    '<option value="' + idx + '"' + (idx === selected ? " selected" : "") + ">" + day + "</option>"
  )).join("");
}

// Ein Geraetewert ausserhalb der Liste darf nicht still zum ersten Listeneintrag
// werden. Genau das ist am 04.10.2026 gemessen worden (L205): overlay[0].type stand
// auf 14, gueltig sind 0..10, das Auswahlfeld zeigte "Keins" (0) -- und ein Speichern
// haette die Einstellung des Nutzers mit 0 ueberschrieben, ohne dass irgendetwas
// darauf hingewiesen haette.
//
// Der unbekannte Wert bekommt deshalb einen eigenen, benannten Eintrag und bleibt
// ausgewaehlt. Zwei Dinge folgen daraus, und beide sind gewollt: Der Nutzer SIEHT,
// dass dort etwas Fremdes steht, und das Auswahlfeld traegt weiterhin den Rohwert --
// es kann ihn also nicht mehr durch 0 ersetzen. Das Speichern selbst weist ihn ab
// (siehe saveOverlay), statt ihn ans Geraet zu schicken.
function buildUnknownDeviceValueOption(selected) {
  return '<option value="' + escapeHtml(String(selected)) + '" selected>' +
    escapeHtml(translateFormat("common.unknown_device_value", { value: String(selected) })) +
    "</option>";
}

function isKnownListIndex(values, selected) {
  const index = Number(selected);
  return Number.isInteger(index) && index >= 0 && index < (values || []).length;
}

function buildNamedOptions(values, selected) {
  const options = values.map((value, idx) => (
    '<option value="' + idx + '"' + (idx === selected ? " selected" : "") + ">" + escapeHtml(value) + "</option>"
  ));

  if (!isKnownListIndex(values, selected)) {
    options.push(buildUnknownDeviceValueOption(selected));
  }

  return options.join("");
}

function buildMonthOptions(selected) {
  return MONTH_OPTIONS.map((value, idx) => (
    '<option value="' + idx + '"' + (idx === selected ? " selected" : "") + ">" + escapeHtml(value) + "</option>"
  )).join("");
}

function buildDayOptions(selected) {
  const options = ['<option value="0"' + (selected === 0 ? " selected" : "") + '></option>'];

  for (let day = 1; day <= 31; day += 1) {
    const label = String(day).padStart(2, "0");
    options.push('<option value="' + day + '"' + (day === selected ? " selected" : "") + ">" + label + "</option>");
  }

  return options.join("");
}

function buildIconOptions(selected) {
  const cachedIcons = getOverlayIconsCache();
  const icons = cachedIcons.length ? cachedIcons : (selected ? [selected] : []);
  if (!icons.length) {
    return '<option value="">Keine Icons gefunden</option>';
  }
  return icons.map((iconName) => (
    '<option value="' + escapeHtml(iconName) + '"' + (iconName === selected ? " selected" : "") + ">" + escapeHtml(iconName) + "</option>"
  )).join("");
}

function parseOverlayMp3Value(value) {
  const parts = String(value || "").split("/");
  return {
    folder: parts[0] || "",
    track: parts[1] || ""
  };
}

function formatOverlayMp3Value(folder, track) {
  const f = String(folder || "").padStart(2, "0");
  const t = String(track || "").padStart(3, "0");
  return f + "/" + t;
}

function updateOverlayRowVisibility(idx) {
  const type = Number(document.getElementById("ov-type-" + idx).value || 0);
  const dateCode = Number(document.getElementById("ov-datecode-" + idx).value || 0);
  const month = Number(document.getElementById("ov-month-" + idx).value || 0);
  const day = Number(document.getElementById("ov-day-" + idx).value || 0);
  const hasDateStart = month > 0 && day > 0;
  const useDateCode = dateCode !== 0 && !hasDateStart;

  toggleHidden("ov-icon-wrap-" + idx, type !== 1);
  toggleHidden("ov-value-wrap-" + idx, type !== 6);
  toggleHidden("ov-mp3-wrap-" + idx, type !== 7);
  toggleHidden("ov-duration-wrap-" + idx, !(type === 1 || type === 4 || type === 8));
  toggleHidden("ov-datecode-wrap-" + idx, hasDateStart);
  toggleHidden("ov-month-wrap-" + idx, useDateCode);
  toggleHidden("ov-day-wrap-" + idx, useDateCode);
  toggleHidden("ov-days-wrap-" + idx, !useDateCode && !hasDateStart);
}

async function ensureOverlayIconsLoaded(forceRefresh) {
  const cachedIcons = getOverlayIconsCache();
  if (!forceRefresh && cachedIcons.length) {
    return cachedIcons;
  }

  return setOverlayIconsCache(await settleFetchJson(getOverlayIconsUrl(), cachedIcons || [], 3000));
}

function refreshOverlayIconSelect(idx) {
  const select = document.getElementById("ov-icon-" + idx);

  if (!select) {
    return;
  }

  const selected = select.value || "";
  select.innerHTML = buildIconOptions(selected);
  if (selected) {
    select.value = selected;
  }
}

async function handleOverlayTypeChange(idx) {
  updateOverlayRowVisibility(idx);

  const type = Number(document.getElementById("ov-type-" + idx).value || 0);
  if (type !== 1) {
    return;
  }

  try {
    await ensureOverlayIconsLoaded(false);
    refreshOverlayIconSelect(idx);
  } catch (error) {
    announceStatus(translate("overlays.icon_list_failed"), "error");
  }
}

function toggleHidden(id, hidden) {
  const element = document.getElementById(id);
  if (element) {
    element.classList.toggle("is-hidden", hidden);
  }
}

// Ein leeres oder nicht numerisches Feld ist KEIN Wert. Einen Rueckfallwert
// einzusetzen waere nur eine andere stille Verfaelschung — deshalb wird abgewiesen und
// gar nicht erst gesendet. Der ESP antwortet seit seiner Haelfte der Korrektur ebenso
// mit {"ok":false}; die PWA zieht die Grenze nur frueher. Die Meldung nennt den
// erlaubten Bereich, damit sie spaeter auch Massnahme 17 traegt. L29.
//
// Seit Massnahme 17 traegt sie ihn: Ein Wert ausserhalb des Bereichs wird hier
// ebenfalls abgewiesen, statt an der Aufrufstelle still auf die Grenze gezogen zu
// werden. Wer 16 in ein Feld mit Maximum 15 tippte, bekam "gespeichert" und hatte 15 —
// die Oberflaeche log. Die Klammerung im ESP bleibt davon unberuehrt; sie ist das Netz
// fuer die Legacy-Oberflaeche und fuer direkte API-Aufrufe.
//
// Gerundet wird hier bewusst NICHT: die Temperaturkorrektur rechnet in halben Grad,
// ein Abrunden wuerde sie still zerstoeren. Geprueft wird nur die Lage im Bereich.
function readNumberInputOrReport(input, min, max) {
  const raw = input && input.value !== null && input.value !== undefined ? String(input.value).trim() : "";
  const number = Number(raw);

  if (!raw || !Number.isFinite(number)) {
    announceStatus(translateFormat("input.number_required", { min, max }), "error");
    if (input && input.focus) {
      input.focus();
    }
    return null;
  }

  if (number < min || number > max) {
    announceStatus(translateFormat("input.number_range", { value: raw, min, max }), "error");
    if (input && input.focus) {
      input.focus();
    }
    return null;
  }

  return number;
}

// Mehrere Zahlenfelder einer Maske zusammen einlesen. Abgebrochen wird beim ERSTEN
// ungueltigen Feld, damit genau eine Meldung erscheint und der Fokus dort landet, wo
// der Fehler steckt. Gesendet wird erst, wenn alle Felder gueltig sind — ein halb
// geschriebener Zustand auf dem Geraet kann so gar nicht entstehen. Massnahme 17.
function readNumberFieldsOrReport(fields) {
  const result = {};

  for (let idx = 0; idx < fields.length; idx += 1) {
    const field = fields[idx];
    const number = readNumberInputOrReport(document.getElementById(field.id), field.min, field.max);

    if (number === null) {
      return null;
    }

    result[field.name] = number;
  }

  return result;
}

// clampNumber() ist mit Massnahme 17 entfallen und wurde NICHT durch eine stillere
// Variante ersetzt: Die Funktion hat Eingaben ausserhalb des Bereichs auf die Grenze
// gezogen, und die Oberflaeche meldete danach "gespeichert". Wer einen Wert begrenzen
// will, prueft ihn stattdessen mit readNumberInputOrReport() und weist ihn ab. Die
// Klammerung im ESP bleibt als Netz fuer Legacy und direkte API-Aufrufe bestehen.

function formatDateTimePreview(current) {
  if (!current.year || !current.month || !current.day) {
    return currentLanguage === "en" ? "Device time is currently unavailable." : "Gerätezeit ist derzeit nicht verfügbar.";
  }

  const weekday = (currentLanguage === "en"
    ? ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
    : ["Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"])[current.wday] || (currentLanguage === "en" ? "Unknown" : "Unbekannt");
  return weekday + ", " +
    pad2(current.day) + "." + pad2(current.month) + "." + current.year +
    " " + pad2(current.hour || 0) + ":" + pad2(current.minute || 0);
}

function getUptimeSeconds(settings) {
  if (!settings || !settings.numvars) {
    return 0;
  }

  const low = Number(settings.numvars[NUM.UPTIME_SECONDS_LO] || 0) & 0xFFFF;
  const high = Number(settings.numvars[NUM.UPTIME_SECONDS_HI] || 0) & 0xFFFF;

  return (high * 65536) + low;
}

function formatLastStartFromSettings(settings) {
  const current = settings && settings.tmvars ? (settings.tmvars[0] || null) : null;
  const uptimeSeconds = getUptimeSeconds(settings);

  if (!current || !current.year || !current.month || !current.day) {
    return currentLanguage === "en" ? "not available" : "nicht verfügbar";
  }

  if (uptimeSeconds <= 0) {
    return currentLanguage === "en" ? "not available" : "nicht verfügbar";
  }

  const currentDate = new Date(
    current.year,
    Math.max(0, Number(current.month || 1) - 1),
    current.day,
    current.hour || 0,
    current.minute || 0,
    current.second || 0
  );

  if (Number.isNaN(currentDate.getTime())) {
    return currentLanguage === "en" ? "not available" : "nicht verfügbar";
  }

  const startDate = new Date(currentDate.getTime() - (uptimeSeconds * 1000));

  return pad2(startDate.getDate()) + "." +
    pad2(startDate.getMonth() + 1) + "." +
    startDate.getFullYear() + " " +
    pad2(startDate.getHours()) + ":" +
    pad2(startDate.getMinutes());
}

function setText(id, text) {
  document.getElementById(id).textContent = text;
}

function pad2(value) {
  return String(value).padStart(2, "0");
}

function escapeHtml(value) {
  return String(value)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}
