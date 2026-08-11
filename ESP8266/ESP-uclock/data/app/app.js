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
const APP_VERSION = "1.4.69";
const DEFAULT_LANGUAGE = "de";
const LANGUAGE_STORAGE_KEY = "wordclock-language";
const I18N = {
  de: {
    "app.title": "WordClock",
    "app.version_label": "App-Version",
    "language.de": "Deutsch",
    "language.en": "English",
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
    "system.debug_eyebrow": "Debug",
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
    "climate.ds18xx_correction": "DS18xx-Korrektur (-20 bis +20 in 0,5 °C Schritten)",
    "climate.save_ds18xx_correction": "DS18xx-Korrektur speichern",
    "climate.rtc_correction": "RTC-Korrektur (-20 bis +20 in 0,5 °C Schritten)",
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
    "system.logs_clear_failed": "STM32-Logbuch konnte nicht geleert werden",
    "system.logs_cleared_status": "STM32-Logbuch wurde geleert",
    "system.logs_clear_confirm": "STM32-Logbuch wirklich leeren?",
    "backup.export_button": "Einstellungen exportieren",
    "backup.import_button": "Einstellungen importieren",
    "backup.exported": "exportiert",
    "backup.imported": "importiert",
    "backup.export_success": "Einstellungen wurden exportiert.",
    "backup.export_failed": "Einstellungen konnten nicht exportiert werden.",
    "backup.choose_file_first": "Bitte zuerst eine Sicherungsdatei auswählen.",
    "backup.invalid_format": "Ungültiges Dateiformat – keine gültige WordClock-Sicherungsdatei.",
    "backup.incompatible_version": "Inkompatible Backup-Version – Datei mit einer neueren App erstellt.",
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
    "debug.apply_busy": "übernimmt...",
    "debug.active": "Overrides aktiv",
    "debug.active_short": "aktiv",
    "debug.apply_failed": "Overrides konnten nicht angewendet werden",
    "debug.reset_busy": "setzt zurück...",
    "debug.reset_done": "Overrides zurückgesetzt",
    "debug.reset_short": "zurückgesetzt",
    "debug.reset_failed": "Overrides konnten nicht zurückgesetzt werden",
    "common.error": "Fehler",
    "common.saving": "speichert...",
    "common.loading": "lädt...",
    "common.running": "läuft...",
    "common.connecting": "verbindet...",
    "common.starting": "startet...",
    "common.uploading": "lädt hoch...",
    "common.deleting": "löscht...",
    "common.clearing": "leert...",
    "common.showing": "zeigt...",
    "common.saved": "gespeichert",
    "common.loaded": "geladen",
    "common.started": "gestartet",
    "common.switched_on": "eingeschaltet",
    "common.switched_off": "ausgeschaltet",
    "common.applied": "gesetzt",
    "common.canceled": "verworfen",
    "common.active": "Aktiv",
    "common.save": "Speichern",
    "common.deleted": "gelöscht",
    "common.cleared": "geleert",
    "common.none": "Keins",
    "common.offline": "offline",
    "common.invalid_value": "ungültig",
    "common.file": "Datei",
    "common.display": "Anzeigen",
    "common.delete": "Löschen",
    "common.file_upload": "Datei hochladen",
    "common.loading_short": "wird geladen...",
    "common.reloading": "lädt neu...",
    "common.ready": "fertig",
    "common.invalid_file_extension": "Ungültige Dateiendung"
  },
  en: {
    "app.title": "WordClock",
    "app.version_label": "App version",
    "language.de": "German",
    "language.en": "English",
    "hero.eyebrow": "Alongside the legacy page",
    "hero.text": "First real PWA alongside the existing ESP web interface. This view reads real device data and stays intentionally lean while the legacy page remains available.",
    "hero.language_label": "Language",
    "hero.legacy_button": "Legacy page",
    "hero.reload_button": "Reload",
    "nav.aria_label": "Sections",
    "nav.main": "Overview",
    "nav.system": "System",
    "nav.network": "Network",
    "nav.climate": "Climate",
    "nav.display": "Display",
    "nav.animations": "Animations",
    "nav.overlays": "Overlays",
    "nav.ambilight": "Ambilight",
    "nav.timers": "Timers",
    "nav.dfplayer": "DFPlayer",
    "nav.maintenance": "Maintenance",
    "nav.hint": "Swipe sideways to reveal more sections.",
    "section.eyebrow": "Section",
    "section.main.title": "Overview",
    "section.main.hint": "Live status, quick actions and the central device time at a glance.",
    "section.system.title": "System",
    "section.system.hint": "Hardware data, device time, detected subsystems and debug views for the interface.",
    "section.network.title": "Network",
    "section.network.hint": "Wi-Fi client, built-in access point and time services clearly separated for connectivity and setup.",
    "section.climate.title": "Climate",
    "section.climate.hint": "Weather, temperature sensors and the brightness sensor combined in one climate area.",
    "section.display.title": "Display",
    "section.display.hint": "Display, brightness, ticker, dim curves and TFT-specific options.",
    "section.animations.title": "Animations",
    "section.animations.hint": "Display and color animations including profiles and delays.",
    "section.overlays.title": "Overlays",
    "section.overlays.hint": "Overlay inserts for icons, ticker, date, weather and DFPlayer content.",
    "section.ambilight.title": "Ambilight",
    "section.ambilight.hint": "Ambilight control, colors, profiles and its dedicated dim curve.",
    "section.timers.title": "Timers",
    "section.timers.hint": "Schedules for display and Ambilight in one shared module.",
    "section.dfplayer.title": "DFPlayer",
    "section.dfplayer.hint": "Audio, bell, speech and the DFPlayer's stored track blocks.",
    "section.maintenance.title": "Maintenance",
    "section.maintenance.hint": "Update source, LittleFS, local files and service actions for the device.",
    "main.live_eyebrow": "Live",
    "main.status_title": "Current status",
    "main.waiting": "waiting for data",
    "main.label.display": "Display",
    "main.label.ambilight": "Ambilight",
    "main.label.firmware": "Firmware",
    "main.label.last_start": "Last start",
    "main.display_toggle": "Toggle display",
    "main.ambilight_toggle": "Toggle Ambilight",
    "main.display_turn_on": "Turn display on",
    "main.display_turn_off": "Turn display off",
    "main.ambilight_turn_on": "Turn Ambilight on",
    "main.ambilight_turn_off": "Turn Ambilight off",
    "main.display_toggle_failed": "Display could not be switched",
    "main.ambilight_toggle_failed": "Ambilight could not be switched",
    "common.auto": "Auto",
    "system.hardware_eyebrow": "System",
    "system.hardware_title": "Hardware",
    "system.subsystems_eyebrow": "Subsystems",
    "system.subsystems_title": "Availability",
    "system.time_eyebrow": "Time",
    "system.time_title": "Date, time and IR",
    "system.time_loading": "Device time is loading...",
    "system.date_label": "Date",
    "system.day_label": "Day",
    "system.month_label": "Month",
    "system.year_label": "Year",
    "system.clock_label": "Time",
    "system.hour_label": "Hour",
    "system.minute_label": "Minute",
    "system.datetime_save": "Save date and time",
    "system.learn_ir": "Learn IR remote",
    "system.debug_eyebrow": "Debug",
    "system.debug_overrides_title": "View overrides",
    "system.debug_overrides_hint": "Only for local app tests. Device messages themselves are not changed.",
    "system.debug_ambilight_ui": "Ambilight UI",
    "system.debug_dfplayer_ui": "DFPlayer UI",
    "system.debug_color_ui": "Color LED UI",
    "system.debug_tft_ui": "TFT UI",
    "system.force_online": "Force online",
    "system.force_rgb": "Force RGB",
    "system.force_rgbw": "Force RGBW",
    "system.force_visible": "Force visible",
    "system.apply_overrides": "Apply overrides",
    "system.reset_overrides": "Reset overrides",
    "system.preview_hint": "Shows the stored display color, the current live color from the device, and the preview layout in use.",
    "system.logs_hint": "Shows deliberately marked STM32 log lines that are mirrored into the web UI over the ESP UART.",
    "system.debug_eyebrow": "Debug",
    "system.preview_colors": "Preview colors",
    "system.logbook_title": "STM32 logbook",
    "network.client_eyebrow": "Client",
    "network.client_title": "Connect to Wi-Fi",
    "network.status_loading": "Network status is loading...",
    "network.found_ssids": "Detected Wi-Fi networks",
    "network.wifi_password": "Wi-Fi password",
    "network.wifi_password_placeholder": "Password for Wi-Fi client",
    "network.scan": "Reload Wi-Fi list",
    "network.connect_client": "Connect as Wi-Fi client",
    "network.connect_client_error": "Wi-Fi client could not be set",
    "network.connect_client_started": "Wi-Fi client connection was triggered",
    "network.ap_eyebrow": "Access point",
    "network.ap_title": "Provide your own Wi-Fi",
    "network.ap_hint": "Useful for first-time setup or when no existing Wi-Fi should be used.",
    "network.ap_ssid": "AP SSID",
    "network.ap_ssid_placeholder": "WordClock access point",
    "network.ap_password": "AP password",
    "network.ap_password_placeholder": "At least 10 characters",
    "network.start_ap": "Start access point",
    "network.start_ap_error": "Access point could not be set",
    "network.start_ap_started": "Access point start was triggered",
    "network.time_eyebrow": "Time",
    "network.time_title": "Time server and clock",
    "network.timeserver": "Time server",
    "network.save_timeserver": "Save time server",
    "network.timezone": "Timezone (GMT +/-)",
    "network.save_timezone": "Save timezone",
    "network.summertime": "Apply daylight saving time",
    "network.summertime_disable": "Disable daylight saving time",
    "network.fetch_network_time": "Fetch network time",
    "climate.weather_eyebrow": "Weather",
    "climate.weather_title": "Weather and location",
    "climate.api_key": "API key",
    "climate.save_api_key": "Save API key",
    "climate.choose_location": "Choose location",
    "climate.city": "City",
    "climate.save_city": "Save city",
    "climate.longitude": "Longitude",
    "climate.latitude": "Latitude",
    "climate.save_coordinates": "Save coordinates",
    "climate.pick_on_map": "Pick location on map",
    "climate.location_ready": "Map and search are ready for location selection.",
    "climate.load_weather": "Load weather",
    "climate.weather_hint": "Use the selected location and fetch current values or the forecast.",
    "climate.fetch_weather": "Fetch weather",
    "climate.fetch_forecast": "Fetch forecast",
    "climate.temperature_eyebrow": "Temperature",
    "climate.temperature_title": "Sensors and correction",
    "climate.ds18xx_correction": "DS18xx correction (-20 to +20 in 0.5 °C steps)",
    "climate.save_ds18xx_correction": "Save DS18xx correction",
    "climate.rtc_correction": "RTC correction (-20 to +20 in 0.5 °C steps)",
    "climate.save_rtc_correction": "Save RTC correction",
    "climate.show_temperature": "Show temperature",
    "climate.ldr_title": "Brightness sensor",
    "climate.toggle_auto_brightness": "Toggle automatic brightness",
    "climate.set_min_ldr": "Set current value as minimum",
    "climate.set_max_ldr": "Set current value as maximum",
    "climate.enable_auto_brightness": "Enable automatic brightness",
    "climate.disable_auto_brightness": "Disable automatic brightness",
    "climate.auto_brightness": "Automatic brightness",
    "climate.current_ldr_value": "Current LDR value",
    "climate.minimum": "Minimum",
    "climate.maximum": "Maximum",
    "climate.status_on": "on",
    "climate.status_off": "off",
    "display.config_eyebrow": "Configuration",
    "display.config_title": "Key settings",
    "display.control_eyebrow": "Display",
    "display.control_title": "Additional controls",
    "display.brightness": "Display brightness",
    "display.save_brightness": "Save brightness",
    "display.color": "Display color",
    "display.color_hint": "Display color can be selected when no color animation is active.",
    "display.white_channel": "White channel",
    "display.save_color": "Save display color",
    "display.color_save_failed": "Color could not be saved",
    "display.mode": "Display mode",
    "display.save_mode": "Save display mode",
    "display.mode_normal": "Normal",
    "display.mode_seconds": "Seconds",
    "display.mode_date": "Date",
    "display.mode_temperature": "Temperature",
    "display.mode_ticker": "Ticker",
    "display.keep_it_is": "Keep “IT IS” visible",
    "display.keep_it_is_disable": "Disable “IT IS”",
    "display.ticker_text": "Ticker text",
    "display.ticker_placeholder": "WordClock ready",
    "display.save_ticker": "Save ticker",
    "display.date_format": "Ticker date format",
    "display.save_date_format": "Save date format",
    "display.ticker_delay": "Ticker delay",
    "display.save_ticker_delay": "Save ticker delay",
    "display.diagnostics": "Diagnostics",
    "display.diagnostics_hint": "Temporary LED and color test for pure display function verification.",
    "display.run_test": "Start display test",
    "display.dim_curve_eyebrow": "Brightness curve",
    "display.dim_curve_title": "Display",
    "display.dim_curve_hint": "Dim values for the 16 brightness levels.",
    "display.dim_level": "Level {idx}",
    "display.dim_curve_ambilight_title": "Ambilight",
    "display.dim_curve_ambilight_hint": "Dim values for the 16 Ambilight steps.",
    "display.presets": "Presets",
    "display.curve_preset": "Curve preset",
    "display.apply_preset_save": "Apply preset and save",
    "display.dim_curve_save": "Save display dim curve",
    "display.save_ambilight_dim_curve": "Save Ambilight dim curve",
    "display.tft_eyebrow": "TFT",
    "display.tft_panel_title": "Panel options",
    "display.ambilight_control_eyebrow": "Ambilight",
    "display.ambilight_control_title": "Additional controls",
    "display.ambilight_brightness": "Ambilight brightness",
    "display.save_ambilight_brightness": "Save Ambilight brightness",
    "display.ambilight_mode": "Ambilight mode",
    "display.save_ambilight_mode": "Save Ambilight mode",
    "display.ambilight_mode_profiles": "Mode profiles",
    "display.ambilight_leds": "Ambilight LEDs",
    "display.save_ambilight_leds": "Save LED count",
    "display.ambilight_offset": "Offset at second 0",
    "display.save_ambilight_offset": "Save offset",
    "display.colors_eyebrow": "Colors",
    "display.colors_title": "RGBW and synchronization",
    "display.color_detecting": "Detecting hardware...",
    "display.marker_color": "Marker color",
    "display.save_ambilight_color": "Save Ambilight color",
    "display.save_marker_color": "Save marker color",
    "display.manual_adjustment": "Manual adjustment",
    "display.sync_ambilight": "Synchronize Ambilight",
    "display.unsync_ambilight": "Disable Ambilight synchronization",
    "display.sync_markers": "Synchronize markers",
    "display.unsync_markers": "Disable marker synchronization",
    "display.fade_clock_seconds": "Fade out seconds softly",
    "display.fade_clock_seconds_disable": "Disable soft fade-out",
    "display.five_second_markers": "5-second markers",
    "display.ambilight_modes_unavailable": "No configurable Ambilight modes are available for the detected hardware.",
    "display.tft_rgb_order": "RGB order",
    "display.tft_flip_horizontal": "Flip horizontally",
    "display.tft_flip_vertical": "Flip vertically",
    "display.save_tft_options": "Save TFT options",
    "animations.current_eyebrow": "Animations",
    "animations.current_title": "Current selection",
    "animations.current_hint": "Choose and save the base mode for display and colors directly.",
    "animations.display_animation": "Display animation",
    "animations.save_display_animation": "Save display animation",
    "animations.color_animation": "Color animation",
    "animations.save_color_animation": "Save color animation",
    "animations.display_profiles_eyebrow": "Display animations",
    "animations.profiles_title": "Profiles",
    "animations.display_profiles_hint": "Adjust delay and favorites per profile.",
    "animations.color_profiles_eyebrow": "Color animations",
    "animations.color_profiles_hint": "Adjust the delay per profile.",
    "animations.favorite": "Favorite",
    "animations.delay": "Delay",
    "animations.profile_save": "Save profile",
    "animations.default": "Default",
    "animations.name_fade": "Fade",
    "animations.name_roll": "Roll",
    "animations.name_explode": "Explode",
    "animations.name_snake": "Snake",
    "animations.name_cube": "Cube",
    "animations.name_teletype": "Teletype",
    "animations.name_none": "None",
    "animations.name_normal": "Normal",
    "animations.name_clock": "Clock",
    "animations.name_rainbow": "Rainbow",
    "animations.name_temperature": "Temperature",
    "animations.name_ticker": "Ticker",
    "animations.name_date": "Date",
    "animations.name_seconds": "Seconds",
    "overlays.eyebrow": "Overlays",
    "overlays.title": "Overlay entries",
    "overlays.hint": "Edit existing overlays or create a new overlay row.",
    "overlays.new_title": "New overlay",
    "overlays.new_badge": "New",
    "overlays.content": "Content",
    "overlays.type": "Type",
    "overlays.icon": "Icon",
    "overlays.value": "Value",
    "overlays.folder": "Folder",
    "overlays.track": "Track",
    "overlays.time_and_date": "Time and date",
    "overlays.interval": "Interval (min.)",
    "overlays.duration": "Duration (sec.)",
    "overlays.date_code": "Date code",
    "overlays.day": "Day",
    "overlays.month": "Month",
    "overlays.days": "Days",
    "overlays.create": "Create overlay",
    "overlays.save": "Save overlay",
    "overlays.cancel": "Cancel",
    "overlays.display": "Display",
    "overlays.delete": "Delete",
    "overlays.type_none": "None",
    "overlays.type_icon": "Icon",
    "overlays.type_date": "Date",
    "overlays.type_temperature": "Temperature",
    "overlays.type_weather_icon": "Weather icon",
    "overlays.type_weather_ticker": "Weather ticker",
    "overlays.type_ticker": "Ticker",
    "overlays.type_dfplayer": "DFPlayer",
    "overlays.type_forecast_icon": "Forecast icon",
    "overlays.type_forecast_ticker": "Forecast ticker",
    "overlays.type_temperature_digits": "Temperature as digits",
    "overlays.datecode_none": "----",
    "overlays.datecode_carnival": "Carnival",
    "overlays.datecode_easter": "Easter Sunday",
    "overlays.datecode_advent1": "1st Advent",
    "overlays.datecode_advent2": "2nd Advent",
    "overlays.datecode_advent3": "3rd Advent",
    "overlays.datecode_advent4": "4th Advent",
    "overlays.save_failed": "Overlay could not be saved",
    "timers.save_all": "Save all timers",
    "timers.ambilight_eyebrow": "Ambilight timers",
    "timers.ambilight_title": "Times for ambilight",
    "timers.save_all_ambilight": "Save all ambilight timers",
    "timers.slot": "Slot",
    "timers.slot_subline": "Timer",
    "timers.ambilight_slot_subline": "Ambilight timer",
    "timers.period": "Schedule",
    "timers.action": "Action",
    "timers.switch_on": "Switch on",
    "timers.switch_off": "Switch off",
    "timers.time": "Time",
    "timers.from_day": "From day",
    "timers.to_day": "To day",
    "timers.clear_slot": "Clear slot",
    "timers.main_eyebrow": "Timers",
    "timers.main_title": "Schedules",
    "dfplayer.volume": "Volume",
    "dfplayer.volume_save": "Save volume",
    "dfplayer.volume_save_failed": "DFPlayer volume could not be saved",
    "dfplayer.panel_eyebrow": "DFPlayer",
    "dfplayer.panel_title": "Audio and bell",
    "dfplayer.note_online": "DFPlayer is online.",
    "dfplayer.note_offline": "DFPlayer is offline and hidden.",
    "dfplayer.mode": "Mode",
    "dfplayer.mode_none": "None",
    "dfplayer.mode_bell": "Bell",
    "dfplayer.mode_speech": "Speech",
    "dfplayer.mode_save": "Save mode",
    "dfplayer.mode_save_failed": "DFPlayer mode could not be saved",
    "dfplayer.bell_times": "Bell times",
    "dfplayer.bell_save": "Save bell times",
    "dfplayer.bell_save_failed": "Bell times could not be saved",
    "dfplayer.speech": "Speech",
    "dfplayer.speak_cycle": "Speech cycle",
    "dfplayer.speak_cycle_save": "Save speech cycle",
    "dfplayer.speak_cycle_save_failed": "Speech cycle could not be saved",
    "dfplayer.silence_start": "Silence start",
    "dfplayer.silence_stop": "Silence end",
    "dfplayer.silence_start_save": "Save silence start",
    "dfplayer.silence_stop_save": "Save silence end",
    "dfplayer.silence_start_save_failed": "Silence start could not be saved",
    "dfplayer.silence_stop_save_failed": "Silence end could not be saved",
    "dfplayer.test_folder": "Test folder",
    "dfplayer.test_track": "Test track",
    "dfplayer.play_track": "Play track",
    "dfplayer.play_started": "DFPlayer track started",
    "dfplayer.play_failed": "DFPlayer track could not be started",
    "dfplayer.saved_titles": "Titles 001-008",
    "dfplayer.alarm_title": "Title",
    "dfplayer.alarm_subline": "Schedule",
    "dfplayer.from": "From",
    "dfplayer.to": "To",
    "dfplayer.time": "Time",
    "dfplayer.alarm_save_failed": "DFPlayer track could not be saved",
    "maintenance.update_eyebrow": "Update",
    "maintenance.source_versions_title": "Source and versions",
    "maintenance.source_versions_hint": "Overview of the update source, detected versions and available server files.",
    "maintenance.server": "Server",
    "maintenance.update_host": "Update host",
    "maintenance.save_update_host": "Save update host",
    "maintenance.update_path": "Update path",
    "maintenance.save_update_path": "Save update path",
    "maintenance.server_selection": "Server selection",
    "maintenance.stm32_server_firmware": "STM32 firmware from server",
    "maintenance.layout_server_table": "Layout table from server",
    "maintenance.versions": "Versions",
    "maintenance.update_progress": "Update progress",
    "maintenance.waiting_for_action": "Waiting for action...",
    "maintenance.progress_prepare_title": "Prepare",
    "maintenance.progress_prepare_note": "Update is starting.",
    "maintenance.progress_bootloader_title": "Bootloader",
    "maintenance.progress_bootloader_note": "Contacting STM32 bootloader.",
    "maintenance.progress_hex_check_title": "Check HEX",
    "maintenance.progress_hex_check_note": "Firmware file is being checked.",
    "maintenance.progress_flash_erase_title": "Erase flash",
    "maintenance.progress_flash_erase_note": "STM32 flash is being erased.",
    "maintenance.progress_flash_write_title": "Write flash",
    "maintenance.progress_flash_write_note": "Firmware is written and verified.",
    "maintenance.progress_reset_title": "Reset",
    "maintenance.progress_reset_note": "STM32 is restarted automatically.",
    "maintenance.progress_finish_title": "Finish",
    "maintenance.progress_finish_note": "Data is reloaded.",
    "maintenance.stm32_prepare_note": "Preparing STM32 update...",
    "maintenance.stm32_wait_bootloader": "Waiting for STM32 bootloader response...",
    "maintenance.stm32_bootloader_reached": "STM32 bootloader reached.",
    "maintenance.stm32_hex_check_running": "Firmware file is being checked.",
    "maintenance.stm32_flash_erasing": "STM32 flash is being erased.",
    "maintenance.stm32_flash_writing": "STM32 firmware is being written and verified...",
    "maintenance.esp_update_waiting": "ESP updated. Waiting for the device to become ready again.",
    "maintenance.stm32_flash_failed": "STM32 flashing failed.",
    "maintenance.stm32_flash_done_reset": "STM32 flashing finished. STM32 is now reset automatically.",
    "maintenance.stm32_local_flash_starting": "Local STM32 flash is starting.",
    "maintenance.stm32_local_flash_start_failed": "Local STM32 flash could not be started.",
    "maintenance.stm32_local_upload_failed": "Local STM32 upload failed.",
    "maintenance.stm32_local_upload_progress": "Uploading STM32 firmware: {percent}%",
    "maintenance.stm32_local_upload_done": "STM32 firmware uploaded. Flash is starting...",
    "maintenance.stm32_flash_auto_reset_failed": "STM32 flash finished, automatic reset failed.",
    "maintenance.stm32_update_response_received": "Update response received.",
    "maintenance.server_recheck_running": "Rechecking server files with the new update settings...",
    "maintenance.server_recheck_done": "Server availability rechecked",
    "maintenance.server_recheck_failed": "Server availability could not be rechecked",
    "maintenance.weather_file": "Weather file",
    "maintenance.icon_file": "Icon file",
    "maintenance.layout_table_file": "Layout table",
    "maintenance.tft_display_file": "TFT display file",
    "maintenance.install_app_files_direct": "Install app files directly",
    "maintenance.remote_eyebrow": "Remote",
    "maintenance.remote_title": "Load and install from server",
    "maintenance.remote_hint": "Load available files directly from the update server. Progress and result are shown below.",
    "maintenance.update_esp": "Update ESP firmware",
    "maintenance.update_esp_confirm": "Update ESP firmware from the update server now? The device will restart.",
    "maintenance.update_esp_start": "Starting ESP update...",
    "maintenance.flash_stm32": "Flash STM32",
    "maintenance.flash_stm32_confirm": "Flash STM32 now with “{file}”?",
    "maintenance.flash_stm32_started": "STM32 flash started.",
    "maintenance.server_files": "Files from server",
    "maintenance.load_layout_table": "Load layout table",
    "maintenance.load_icon_files": "Load icon files",
    "maintenance.load_app_files": "Load app files",
    "maintenance.service_eyebrow": "Maintenance",
    "maintenance.service_title": "Service actions",
    "maintenance.service_hint": "Only for targeted service cases. These actions do not trigger a normal update or backup workflow.",
    "maintenance.reset_stm32": "Reset STM32",
    "maintenance.reset_stm32_failed": "STM32 could not be reset",
    "maintenance.reset_eeprom": "Reset EEPROM",
    "maintenance.release_notes": "Release notes",
    "maintenance.files_eyebrow": "Files",
    "maintenance.files_title": "LittleFS",
    "maintenance.files_hint": "View, delete, and upload directly to the known target files.",
    "maintenance.file_list": "File list",
    "maintenance.file_preview": "File preview",
    "maintenance.file_actions_status": "Actions and status",
    "maintenance.preview_placeholder": "The content of the selected file from the file list will be shown here via “Display”.",
    "maintenance.file_action_placeholder": "No file action has been executed yet.",
    "maintenance.no_files": "No files found in LittleFS yet.",
    "maintenance.special_display_target": "TFT special case",
    "maintenance.no_stm32_files": "no STM32 files found",
    "maintenance.no_release_notes": "No release notes were read from the server.",
    "maintenance.fs_total": "Total",
    "maintenance.fs_used": "Used",
    "maintenance.fs_block_size": "Block size",
    "maintenance.fs_page_size": "Page size",
    "maintenance.fs_max_open_files": "Max. open files",
    "maintenance.fs_max_path_length": "Max. path length",
    "maintenance.boot_mode": "Boot mode",
    "maintenance.boot_mode_ap": "Access point",
    "maintenance.boot_mode_client": "Wi-Fi client",
    "maintenance.no_data": "no data",
    "maintenance.version_flash": "ESP flash",
    "maintenance.version_ota": "OTA update",
    "maintenance.version_wc": "WordClock version",
    "maintenance.version_wc_available": "WordClock available",
    "maintenance.version_esp": "ESP version",
    "maintenance.version_esp_available": "ESP available",
    "maintenance.version_app": "App version",
    "maintenance.version_app_available": "App available",
    "maintenance.version_stm32_default": "Default STM32",
    "maintenance.version_available_yes": "possible",
    "maintenance.version_available_no": "not possible",
    "maintenance.local_update_unavailable": "Local update is not available with this ESP flash size.",
    "maintenance.choose_file_for_target": "Please select a file for {target} first.",
    "maintenance.file_expected_pattern": "Wrong file selected. Expected a matching table pattern like {pattern} for {target}.",
    "maintenance.file_expected_exact": "Wrong file selected. Expected {target}.",
    "maintenance.txt_required": "{target} must be a .txt file.",
    "maintenance.file_upload_failed": "{target} could not be uploaded",
    "maintenance.file_upload_failed_detail": "{target} could not be uploaded: {error}",
    "maintenance.local_uploads": "Upload local files",
    "maintenance.local_app_assets": "Local app files (.gz)",
    "maintenance.choose_local_app_folder": "Choose local app folder",
    "maintenance.local_update_eyebrow": "Local update",
    "maintenance.local_update_title": "Upload ESP and STM32 firmware",
    "maintenance.local_update_hint": "Upload local firmware files directly without using the update server.",
    "maintenance.local_update_partial_support": "Some local PWA update paths are not yet supported by this firmware.",
    "maintenance.local_update_select_file": "Select an ESP or STM32 file and upload it locally.",
    "maintenance.local_update_preparing": "Preparing local update...",
    "maintenance.local_esp_file": "ESP firmware file (.bin)",
    "maintenance.local_esp_update": "Update ESP locally",
    "maintenance.local_esp_choose_first": "Please choose an ESP firmware file first.",
    "maintenance.local_esp_expected": "Expected an ESP .bin file",
    "maintenance.local_esp_uploading": "Uploading ESP firmware locally...",
    "maintenance.local_esp_uploaded_wait": "ESP firmware was transferred. Waiting for restart.",
    "maintenance.local_stm32_file": "STM32 firmware file (.hex)",
    "maintenance.local_stm32_update": "Update STM32 locally",
    "maintenance.local_stm32_choose_first": "Please choose an STM32 firmware file first.",
    "maintenance.local_stm32_expected": "Expected a matching STM32 file",
    "maintenance.local_stm32_uploading": "Uploading STM32 firmware locally...",
    "maintenance.local_stm32_update_failed": "STM32 firmware could not be updated.",
    "maintenance.backup_eyebrow": "Backup",
    "maintenance.backup_title": "Export and import settings",
    "maintenance.backup_import_file": "Import backup file",
    "maintenance.backup_hint": "Saves the configurable settings as a JSON file and restores them on request. The file also contains Wi-Fi and AP data in plain text.",
    "maintenance.backup_idle": "No backup action has been executed yet.",
    "weather.map_modal_title": "Choose location on map",
    "weather.close": "Close",
    "weather.modal_eyebrow": "Weather",
    "weather.modal_hint": "Search for a place, tap the map, or use your current location.",
    "weather.search_place": "Search place",
    "weather.search_placeholder": "Zurich, Switzerland",
    "weather.use_current_location": "Use current location",
    "weather.map_aria": "Location map",
    "weather.waiting_for_map": "Waiting for map...",
    "weather.place": "Place",
    "weather.place_placeholder": "Taken from search or map",
    "weather.current_location_prefix": "Current:",
    "status.auto_refresh_paused": "Automatic refresh is paused until unsaved changes are stored",
    "status.waiting_for_data": "Waiting for data...",
    "status.updated_at": "Updated {time}",
    "status.data_load_failed": "Data could not be loaded",
    "overview.display_mode": "Display mode",
    "overview.brightness": "Brightness",
    "overview.auto_brightness": "Automatic brightness",
    "overview.led_capabilities": "LED capabilities",
    "overview.timeserver": "Time server",
    "overview.ticker_delay": "Ticker delay",
    "overview.last_stm32_restart": "Last STM32 restart",
    "overview.last_start": "Last start",
    "overview.weather_location": "Weather location",
    "overview.ticker": "Ticker",
    "overview.date_format": "Date format",
    "overview.ambilight_mode": "Ambilight mode",
    "overview.ambilight_brightness": "Ambilight brightness",
    "overview.ambilight_leds": "Ambilight LEDs",
    "overview.ambilight_offset": "Ambilight offset",
    "overview.dfplayer_mode": "DFPlayer mode",
    "overview.dfplayer_volume": "DFPlayer volume",
    "overview.speak_cycle": "Speech interval",
    "preview.color_animation": "Color animation",
    "preview.persisted_display_color": "Saved display color",
    "preview.live_device_color": "Live device color",
    "preview.layout_file": "Preview layout",
    "display.white_channel_inactive": "RGBW hardware detected, but the white channel is currently not active in firmware.",
    "display.color_direct_available": "The display color can be set directly while no color animation is active.",
    "display.color_direct_unavailable": "The display color can only be selected directly when color animation = none.",
    "system.logs_empty": "No STM32 logs available yet.",
    "system.logs_buffer": "{count} log line{suffix} in buffer.",
    "system.logs_reload": "Reload logs",
    "system.logs_clear": "Clear logs",
    "system.logs_jump_end": "Jump to end",
    "system.logs_reload_busy": "loading...",
    "system.logs_clear_busy": "clearing...",
    "system.logs_jump_done": "bottom",
    "system.logs_loaded": "loaded",
    "system.logs_cleared": "cleared",
    "system.logs_load_failed": "STM32 logs could not be loaded",
    "system.logs_clear_failed": "STM32 logbook could not be cleared",
    "system.logs_cleared_status": "STM32 logbook was cleared",
    "system.logs_clear_confirm": "Really clear the STM32 logbook?",
    "backup.export_button": "Export settings",
    "backup.import_button": "Import settings",
    "backup.exported": "exported",
    "backup.imported": "imported",
    "backup.export_success": "Settings were exported.",
    "backup.export_failed": "Settings could not be exported.",
    "backup.choose_file_first": "Please select a backup file first.",
    "backup.invalid_format": "Invalid file format – no valid WordClock backup file.",
    "backup.incompatible_version": "Incompatible backup version – file was created with a newer app.",
    "backup.import_failed": "Settings could not be imported.",
    "backup.import_confirm": "Import settings from “{file}” now?",
    "backup.import_start": "Starting backup restore...",
    "backup.import_validate": "Checking imported settings...",
    "backup.import_verify_sections": "Checking display, climate, overlays, and timers...",
    "backup.import_reload_data": "Reloading updated device data...",
    "backup.import_network_final": "Applying network settings as final step...",
    "backup.import_network_verify": "Checking network settings again...",
    "backup.import_network_wait": "Waiting for network settings to be applied...",
    "backup.import_sensor_final": "Applying sensor corrections as final step...",
    "backup.import_sensor_verify": "Checking sensor corrections again...",
    "backup.import_persist_critical": "Persisting critical settings...",
    "backup.import_persist_temperature": "Persisting temperature corrections...",
    "backup.import_wait_persist": "Waiting until settings are stored permanently...",
    "backup.import_restart_now": "Import complete. STM32 is now restarting automatically...",
    "backup.import_restart_reload_data": "STM32 restarted. Reloading data...",
    "backup.import_restart_refreshing": "Import complete. Clock is restarting, app will refresh...",
    "backup.import_service": "Applying maintenance settings...",
    "backup.import_assets": "Restoring files and assets...",
    "backup.import_display": "Applying display settings...",
    "backup.import_climate": "Applying climate and weather settings...",
    "backup.import_animations": "Applying animation settings...",
    "backup.import_tft": "Applying TFT settings...",
    "backup.import_ambilight": "Applying ambilight settings...",
    "backup.import_dfplayer": "Applying DFPlayer settings...",
    "backup.import_overlays": "Applying overlays...",
    "backup.import_timers": "Applying timers...",
    "backup.import_display_retry": "Reapplying display settings...",
    "backup.import_climate_retry": "Reapplying climate and weather settings...",
    "backup.import_overlays_retry": "Reapplying overlays...",
    "backup.import_timers_retry": "Reapplying timers...",
    "backup.import_temperature_retry": "Reapplying temperature corrections...",
    "backup.import_rtc_final": "Applying RTC correction as final step...",
    "backup.import_rtc_retry": "Reapplying RTC correction...",
    "backup.import_network_retry": "Reapplying network and time settings...",
    "backup.import_network_time_retry": "Reapplying time server and clock settings...",
    "backup.import_maintenance_retry": "Reapplying update host and update path...",
    "backup.import_network_time_final": "Applying time server and clock settings as final step...",
    "backup.import_maintenance_final": "Applying update host and update path as final step...",
    "backup.import_sensor_persist_final": "Applying sensor corrections as final persistence step...",
    "backup.import_restart": "Import finished. STM32 will restart automatically",
    "backup.import_reload": "Import finished. App will reload",
    "backup.import_reconnect": "Import finished. Connection will be restored after restart",
    "weather.map_loading": "Loading map service...",
    "weather.map_load_failed": "Map service could not be loaded.",
    "weather.map_hint": "Tap the map or search for a place.",
    "weather.enter_location_first": "Please enter a place first.",
    "weather.search_busy": "searching...",
    "weather.searching": "Searching location...",
    "weather.no_result": "No result found for this place.",
    "weather.search_button": "Search",
    "weather.found": "found",
    "weather.location_found": "Location found and placed on the map.",
    "weather.search_failed": "Location search could not be loaded.",
    "weather.current_location_busy": "reading...",
    "weather.current_location_label": "Use current location",
    "weather.current_location_reading": "Reading current location...",
    "weather.current_location_set": "Current location set.",
    "weather.approx_location_start": "Determining approximate location via internet connection...",
    "weather.approx_location_city": "Approximate location set: {city}.",
    "weather.approx_location_set": "Approximate location was set.",
    "weather.location_unavailable": "Even an approximate location could not be determined.",
    "weather.map_applied": "Location taken from map.",
    "weather.reverse_failed": "Coordinates set. Place name could not be resolved.",
    "weather.map_preview": "Chosen from map: {city} | {lon} / {lat}",
    "weather.apply_map_busy": "applying...",
    "weather.apply_map_idle": "Apply to weather",
    "weather.apply_map_done": "applied",
    "weather.apply_map_failed": "Weather location and coordinates could not be applied",
    "weather.apply_map_success": "Weather location and coordinates were applied",
    "network.scan_busy": "loading...",
    "network.scan_success": "Wi-Fi list was updated",
    "network.scan_failed": "Wi-Fi list could not be updated",
    "local_app.folder_read_failed": "The app folder could not be read",
    "local_app.folder_browser_failed": "The local app folder could not be opened through the browser.",
    "local_app.folder_checked": "Checked local app folder: {found}/{total} required files detected.",
    "local_app.folder_none": "A local app folder was chosen, but the browser did not provide matching app files under app/....",
    "local_app.note_empty": "No app folder selected yet. Please choose the folder that contains the compressed files (.gz) under app/....",
    "local_app.note_complete": "App folder fully recognized. {found}/{total} files are ready and can be installed directly.",
    "local_app.note_missing": "App folder checked. {found}/{total} files found. Missing: {missing}",
    "local_app.incomplete": "The app folder is not complete yet.",
    "local_app.missing_required": "Local app files cannot be installed yet. Required files are missing.",
    "local_app.upload_unsupported": "This firmware does not support local app file upload yet.",
    "local_app.install_confirm": "Write the local app files directly to the device now?",
    "local_app.installing": "Installing local app files...",
    "local_app.installing_fs": "Local app files are being written directly to LittleFS...",
    "local_app.progress": "Local app files: {step}/{total} {asset}",
    "local_app.installed_reload": "Local app files installed. Reloading app...",
    "local_app.installed_status": "Local app files installed. Page will reload.",
    "local_app.installed_fs": "Local app files were installed successfully.",
    "local_app.install_button": "Install local app files",
    "local_app.installed": "installed",
    "local_app.install_failed": "Local app files could not be installed",
    "local_app.install_failed_detail": "Local app files could not be installed: {error}",
    "maintenance.app_install_confirm": "Load and install app files directly from the server now?",
    "maintenance.app_install_button": "Load app files",
    "maintenance.app_install_loading": "Loading app files from server...",
    "maintenance.app_install_running": "Installing app files...",
    "maintenance.app_install_loaded": "App files were loaded and are being installed...",
    "maintenance.app_install_reload": "App files installed. Reloading app...",
    "maintenance.app_install_success": "App files installed. Page will reload.",
    "maintenance.app_install_failed": "App files could not be loaded",
    "maintenance.app_install_failed_detail": "App files could not be loaded or installed.",
    "maintenance.layout_choose_first": "Please select a layout table first",
    "maintenance.layout_confirm": "Load layout table “{file}” now?",
    "maintenance.layout_loading": "Loading layout table...",
    "maintenance.layout_loaded": "Layout table was loaded.",
    "maintenance.layout_load_failed": "Layout table could not be loaded",
    "maintenance.layout_button": "Load layout table",
    "maintenance.reset_stm32_ok": "STM32 was reset",
    "maintenance.reset_stm32_started_wait": "STM32 reset triggered. Waiting for completion...",
    "maintenance.reset_stm32_reconnect": "STM32 is ready again. App will reload...",
    "maintenance.reset_stm32_reconnect_unclear": "STM32 reconnect was not detected reliably. App will reload as a precaution...",
    "maintenance.esp_ready_reload": "ESP reachable again. Reloading page.",
    "maintenance.reset_eeprom_confirm_1": "Really reset EEPROM to factory defaults?",
    "maintenance.reset_eeprom_confirm_2": "Really reset all EEPROM values to factory defaults?",
    "maintenance.reset_eeprom_busy": "resetting...",
    "maintenance.reset_eeprom_wait": "waiting...",
    "maintenance.reset_eeprom_start": "Triggering EEPROM reset...",
    "maintenance.reset_eeprom_restart": "EEPROM reset triggered. STM32 will restart...",
    "maintenance.reset_eeprom_reload": "Waiting for STM32 restart. The app will reload afterwards...",
    "maintenance.reset_eeprom_failed": "EEPROM could not be reset",
    "maintenance.device_ready_reload": "Device should be ready again. App will reload as a precaution.",
    "maintenance.no_reconnect_reload": "No reliable reconnect signal received. App will reload as a precaution.",
    "maintenance.esp_not_ready": "ESP is not reachable again yet. Please reload manually if needed.",
    "maintenance.unsaved_reload_confirm": "There are unsaved changes. Reload the app anyway?",
    "maintenance.reloading": "Reloading app...",
    "maintenance.forced_reload": "Reload is being forced so the updated app becomes visible again.",
    "maintenance.assets_confirm": "Really load icon files from the server now?",
    "maintenance.assets_loaded": "Icon files loaded",
    "maintenance.assets_load_failed": "Icon files could not be loaded",
    "maintenance.stm32_file_choose_first": "Please choose an STM32 file first",
    "maintenance.reset_stm32_confirm": "Really reset STM32 now?",
    "maintenance.format_fs_confirm": "Really format LittleFS?",
    "maintenance.format_fs_button": "Format LittleFS",
    "maintenance.format_fs_done": "LittleFS was formatted.",
    "maintenance.local_esp_waiting": "Local ESP update is running. Waiting for restart and reconnect...",
    "maintenance.remote_esp_waiting": "ESP is updating right now. Waiting for restart and reconnect...",
    "maintenance.stm32_auto_reset_wait": "STM32 was reset automatically after flashing. Waiting for completion...",
    "maintenance.stm32_auto_reset_running": "STM32 is being reset automatically. Data will reload afterwards.",
    "maintenance.stm32_flash_success": "STM32 update completed successfully.",
    "maintenance.fs_showing": "Showing file “{file}”.",
    "maintenance.fs_deleted": "File “{file}” was deleted.",
    "maintenance.fs_delete_confirm": "Really delete file “{file}”?",
    "maintenance.file_load_failed": "File could not be loaded",
    "maintenance.file_delete_failed": "File could not be deleted",
    "overlays.discarded": "Discarded",
    "overlays.new_discarded": "New overlay discarded",
    "overlays.show_failed": "Overlay could not be displayed",
    "overlays.show_status": "Showing overlay {idx}",
    "overlays.delete_failed": "Overlay could not be deleted",
    "overlays.deleted": "Overlay was deleted",
    "overlays.icon_list_failed": "Icon list could not be loaded",
    "timers.saved_all": "All timers were saved",
    "timers.save_all_failed": "Timers could not be fully saved",
    "timers.save_failed": "Timer could not be saved",
    "timers.cleared": "Slot cleared",
    "timers.clear_failed": "Timer could not be cleared",
    "flags.toggle_failed": "Switch could not be updated",
    "flags.enabled": "enabled",
    "flags.disabled": "disabled",
    "debug.apply_busy": "applying...",
    "debug.active": "Overrides active",
    "debug.active_short": "active",
    "debug.apply_failed": "Overrides could not be applied",
    "debug.reset_busy": "resetting...",
    "debug.reset_done": "Overrides reset",
    "debug.reset_short": "reset",
    "debug.reset_failed": "Overrides could not be reset",
    "common.error": "Error",
    "common.saving": "saving...",
    "common.loading": "loading...",
    "common.running": "running...",
    "common.connecting": "connecting...",
    "common.starting": "starting...",
    "common.uploading": "uploading...",
    "common.deleting": "deleting...",
    "common.clearing": "clearing...",
    "common.showing": "showing...",
    "common.saved": "saved",
    "common.loaded": "loaded",
    "common.started": "started",
    "common.switched_on": "switched on",
    "common.switched_off": "switched off",
    "common.applied": "applied",
    "common.canceled": "discarded",
    "common.active": "Active",
    "common.save": "Save",
    "common.deleted": "deleted",
    "common.cleared": "cleared",
    "common.none": "None",
    "common.offline": "offline",
    "common.invalid_value": "invalid",
    "common.file": "File",
    "common.display": "View",
    "common.delete": "Delete",
    "common.file_upload": "Upload file",
    "common.loading_short": "loading...",
    "common.reloading": "reloading...",
    "common.ready": "done",
    "common.invalid_file_extension": "Invalid file extension"
  }
};
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
const BACKUP_VERSION = 2;

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
const PROGRESS_SCROLL_RESTORE_KEY = "wordclock-progress-scroll-restore";
const LEAFLET_CSS_URL = "https://unpkg.com/leaflet@1.9.4/dist/leaflet.css";
const LEAFLET_JS_URL = "https://unpkg.com/leaflet@1.9.4/dist/leaflet.js";
let currentLanguage = DEFAULT_LANGUAGE;

function normalizeLanguage(language) {
  return Object.prototype.hasOwnProperty.call(I18N, language) ? language : DEFAULT_LANGUAGE;
}

function getStoredLanguage() {
  try {
    return normalizeLanguage(window.localStorage.getItem(LANGUAGE_STORAGE_KEY));
  } catch (_) {
    return DEFAULT_LANGUAGE;
  }
}

function translate(key) {
  const active = I18N[currentLanguage] || I18N[DEFAULT_LANGUAGE];
  const fallback = I18N[DEFAULT_LANGUAGE] || {};
  return active[key] || fallback[key] || key;
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

function setCurrentLanguage(language, persist) {
  currentLanguage = normalizeLanguage(language);

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
  ["fs-upload-icon-form", "submit", (event) => uploadFsTargetFile(event, getFsUploadUrl("icon"), "Datei wurde hochgeladen.")],
  ["fs-upload-weather-form", "submit", (event) => uploadFsTargetFile(event, getFsUploadUrl("weather"), "Datei wurde hochgeladen.")],
  ["fs-upload-tables-form", "submit", (event) => uploadFsTargetFile(event, getFsUploadUrl("tables"), "Layout-Tabelle wurde hochgeladen.")],
  ["fs-upload-display-form", "submit", (event) => uploadFsTargetFile(event, getFsUploadUrl("display"), "TFT-Display-Datei wurde hochgeladen.")]
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
bindElementEvent("language-select", "change", (event) => {
  setCurrentLanguage(event.target && event.target.value ? event.target.value : DEFAULT_LANGUAGE);
});
setCurrentLanguage(getStoredLanguage(), false);
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
  setActiveModule(button.getAttribute("data-module-target") || "main");
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
  if (!document.hidden) {
    if (shouldDelayStartupRefresh() || shouldSkipLifecycleRefresh()) {
      return;
    }
    void refreshVisibleModuleData({ visibilityRefresh: true });
  }
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

async function registerAppServiceWorker() {
  if (!("serviceWorker" in navigator)) {
    return;
  }

  try {
    const registration = await navigator.serviceWorker.register("/app/sw.js", { scope: "/app/" });
    appServiceWorkerRegistration = registration;
    bindServiceWorkerLifecycle(registration);
  } catch (_) {
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

function startAlignedAutoRefresh() {
  const intervalMs = 15000;
  const phaseOffsetMs = 2000;

  if (alignedAutoRefreshTimeout) {
    window.clearTimeout(alignedAutoRefreshTimeout);
    alignedAutoRefreshTimeout = 0;
  }
  if (alignedAutoRefreshInterval) {
    window.clearInterval(alignedAutoRefreshInterval);
    alignedAutoRefreshInterval = 0;
  }

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

function announceStatus(message, tone) {
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

  if (settingsImportInProgress) {
    await sleep(180);
  }

  return response;
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
  } catch (_) {}
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
  } catch (_) {}
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
    meta.textContent = translate("system.logs_empty");
    output.textContent = translate("system.logs_empty");
    return;
  }

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

async function fetchStm32Log(silent) {
  if (stm32LogRefreshInFlight) {
    return;
  }

  stm32LogRefreshInFlight = true;

  try {
    const response = await apiFetch(getStm32LogUrl());
    const data = await response.json();
    updateStm32Log(data);
    return data;
  } catch (error) {
    if (!silent) {
      throw error;
    }
  } finally {
    stm32LogRefreshInFlight = false;
  }
}

function syncStm32LogPolling() {
  if (settingsImportInProgress) {
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
    await fetchStm32Log(false);
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
    try {
      await loadData();
    } catch (_) {
    }
    announceStatus("Ambilight-Status gespeichert", "ok");
  } catch (error) {
    select.value = previous;
    announceStatus("Ambilight-Status konnte nicht gesetzt werden", "error");
  } finally {
    select.disabled = false;
  }
}

async function saveBrightness() {
  const slider = document.getElementById("brightness-slider");
  const value = slider.value;
  await runValueSave("brightness-save-button", getDisplayBrightnessSetUrl(), value, translate("display.save_brightness"), "Helligkeit konnte nicht gespeichert werden");
}

async function toggleAutoBrightness() {
  const button = document.getElementById("auto-brightness-button");
  await runStateToggleButton(button, getAutoBrightnessSetUrl(), {
    idleText: button.dataset.restoreText || translate("climate.auto_brightness"),
    errorText: "Automatische Helligkeit konnte nicht geschaltet werden"
  });
}

async function togglePermanentItIs() {
  const button = document.getElementById("display-it-is-button");
  await runStateToggleButton(button, getDisplayItIsSetUrl(), {
    idleText: button.dataset.restoreText || translate("display.keep_it_is"),
    errorText: "„ES IST“ konnte nicht gesetzt werden"
  });
}

function parseSettings(xmlText) {
  const xml = new DOMParser().parseFromString(xmlText, "application/xml");
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

function updateNetworkControls(settings, networkInfo) {
  updateNetworkControlsFromMeta(getSettingsControlUiMeta(settings, networkInfo).network);
}

function updateNetworkControlsFromMeta(meta) {
  const select = document.getElementById("network-ssid-select");

  select.innerHTML = meta.networks.length
    ? meta.networks.map((ssid) => '<option value="' + escapeHtml(ssid) + '"' + (ssid === meta.currentSsid ? " selected" : "") + ">" + escapeHtml(ssid) + "</option>").join("")
    : '<option value="">Keine WLANs gefunden</option>';

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
  setSettingsBackupNote(file ? "Ausgewählt: " + file.name : "Noch keine Sicherungsdatei ausgewählt.");
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

async function parseSettingsBackupFile(file) {
  const backup = JSON.parse(await file.text());

  if (backup.format !== BACKUP_FORMAT) {
    throw new Error("invalid-backup-format");
  }

  if (Number(backup.version || 0) !== BACKUP_VERSION) {
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
  return error && error.message === "invalid-backup-format"
    ? translate("backup.invalid_format")
    : error && error.message === "unsupported-backup-version"
      ? translate("backup.incompatible_version")
      : translate("backup.import_failed");
}

async function prepareBackupExportDocument() {
  await ensureBackupExportState();
  return buildBackupExportDocument(getBackupExportState());
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
      interval: Number(item.interval || 0),
      duration: Number(item.duration || 0),
      date_code: Number(item.date_code || 0),
      month: item.date_start ? ((Number(item.date_start) >> 8) & 0xff) : 0,
      day: item.date_start ? (Number(item.date_start) & 0xff) : 0,
      days: Number(item.days || 0)
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

  if (!getUpdateTableCurrentFile(getCurrentUpdateTableInfo())) {
    setCurrentUpdateTableInfo(await settleFetchJson(getUpdateTableFilesUrl(), {}, 5000));
  }
}

async function exportSettingsBackup() {
  const button = document.getElementById("settings-export-button");

  beginButtonFeedback(button, "exportiert...");

  try {
    const backup = await prepareBackupExportDocument();
    const blob = new Blob([JSON.stringify(backup, null, 2)], { type: "application/json" });
    const link = document.createElement("a");
    const timestamp = new Date().toISOString().replace(/[:]/g, "-").replace(/\..+/, "");
    link.href = URL.createObjectURL(blob);
    link.download = "wordclock-settings-" + timestamp + ".json";
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    window.setTimeout(() => URL.revokeObjectURL(link.href), 1000);
    setSettingsBackupNote(translate("backup.export_success"), "ok");
    finishButtonFeedback(button, translate("backup.export_button"), "success", translate("backup.exported"));
  } catch (error) {
    setSettingsBackupNote(translate("backup.export_failed"), "error");
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

  beginButtonFeedback(button, "importiert...");

  try {
    const importedState = await loadImportedBackupState(file);

    if (!window.confirm(translateFormat("backup.import_confirm", { file: file.name }))) {
      finishButtonFeedback(button, translate("backup.import_button"));
      return;
    }

    setSettingsBackupNote(translate("backup.import_start"));
    hasUnsavedEdits = false;
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

  return String(network.timeserver || "") !== String(current.timeserver || "") ||
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

async function runImportStageList(stages) {
  for (const stage of (stages || [])) {
    if (!stage || stage.when === false) {
      continue;
    }

    if (stage.note) {
      setSettingsBackupNote(stage.note);
    }

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
    setSettingsBackupNote("Import abgeschlossen. App wird neu geladen...", "success");
    announceStatus(translate("backup.import_reload"), "ok");
  } catch (error) {
    setSettingsBackupNote(translate("backup.import_restart_refreshing"), "success");
    announceStatus(translate("backup.import_reconnect"), "ok");
  }
  setTimeout(reloadAppPage, 1200);
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
    { note: translate("backup.import_timers"), run: () => importTimerSettings(settings.timers), reload: true, pauseMs: 1200 }
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
    setSettingsBackupNote("Schreibe Temperatur-Korrekturen abschliessend...");
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

function overlayBackupItemsEqual(expectedItems, actualItems) {
  const expected = normalizeOverlayBackupItems(expectedItems || []);
  const actual = normalizeOverlayBackupItems(actualItems || []);

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
    importFieldMismatch(display, current, ["ticker_text", "date_ticker_format"]);
}

function climateImportNeedsRetry(climate, settings) {
  if (!climate || !settings) {
    return false;
  }

  const current = getImportRetryCurrentSections(settings).climate;

  return importFieldMismatch(climate, current, ["weather_city", "weather_lon", "weather_lat"]) ||
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

  return importFieldMismatch(maintenance, current, ["update_host", "update_path"]);
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

  setSettingsBackupNote("Stelle Layout-Tabelle wieder her...");

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

  setSettingsBackupNote("Stelle Icon- und Overlay-Dateien wieder her...");
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
  setSettingsBackupNote("Prüfe benötigte Dateien...");
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

async function importNetworkSettings(network) {
  if (!network) {
    return;
  }

  setSettingsBackupNote("Importiere Netzwerk- und EEPROM-Einstellungen...");

  await importNetworkTimeSettings(network);

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

  await apiFetchValue(getNetworkTimeserverSetUrl(), network.timeserver || "");
  await sleep(700);
  await apiFetchValue(getNetworkTimezoneSetUrl(), Number(network.timezone_offset || 0));
  await sleep(300);
  await apiFetchValue(getNetworkSummertimeSetUrl(), network.summertime ? "on" : "off");
  await sleep(300);
}

async function importDisplaySettings(display) {
  if (!display) {
    return;
  }

  setSettingsBackupNote("Importiere Display-Einstellungen...");

  await apiFetchValue(getDisplayPowerSetUrl(), display.power ? "on" : "off");
  await sleep(180);
  await apiFetchValue(getDisplayModeSetUrl(), Number(display.mode || 0));
  await sleep(180);
  await apiFetchValue(getDisplayUseRgbwSetUrl(), display.use_rgbw ? "on" : "off");
  await sleep(180);
  await apiFetchValue(getAutoBrightnessSetUrl(), display.automatic_brightness ? "on" : "off");
  await sleep(220);
  await apiFetchValue(getDisplayBrightnessSetUrl(), Number(display.brightness || 0));
  await sleep(220);
  await apiFetchValue(getDisplayItIsSetUrl(), display.permanent_it_is ? "on" : "off");
  await sleep(220);
  await apiFetchValue(getTickerSetUrl(), display.ticker_text || "");
  await sleep(350);
  await apiFetchValue(getDateTickerFormatSetUrl(), display.date_ticker_format || "");
  await sleep(900);
  await apiFetchValue(getTickerDecelerationSetUrl(), Number(display.ticker_deceleration || 0));
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

  setSettingsBackupNote("Importiere Wartungs- und Update-Einstellungen...");

  await apiFetchValue(getUpdateHostSetUrl(), maintenance.update_host || "");
  await sleep(900);
  await apiFetchValue(getUpdatePathSetUrl(), maintenance.update_path || "");
  await sleep(500);
}

async function importClimateSettings(climate) {
  if (!climate) {
    return;
  }

  setSettingsBackupNote("Importiere Klima- und Wetter-Einstellungen...");

  await apiFetchValue(getWeatherAppIdSetUrl(), climate.weather_appid || "");
  await sleep(250);
  await apiFetchValue(getWeatherCitySetUrl(), climate.weather_city || "");
  await sleep(250);
  await apiFetchQuery(getWeatherCoordinatesSetUrl(), {
    lon: climate.weather_lon || "",
    lat: climate.weather_lat || ""
  });
  await sleep(250);
  await apiFetchValue(getLdrMinValueSetUrl(), Number(climate.ldr_min || 0));
  await sleep(150);
  await apiFetchValue(getLdrMaxValueSetUrl(), Number(climate.ldr_max || 0));
}

async function importSensorCorrectionSettings(climate) {
  if (!climate) {
    return;
  }

  setSettingsBackupNote("Importiere Sensor-Korrekturen...");
  await apiFetchValue(getTemperatureRtcCorrectionSetUrl(), Math.max(-20, Math.min(20, Number(climate.rtc_temp_correction || 0))));
  await sleep(400);
  await apiFetchValue(getTemperatureDs18xxCorrectionSetUrl(), Math.max(-20, Math.min(20, Number(climate.ds18xx_temp_correction || 0))));
  await sleep(400);
}

async function importRtcCorrectionSetting(climate) {
  if (!climate) {
    return;
  }

  await apiFetchValue(getTemperatureRtcCorrectionSetUrl(), Math.max(-20, Math.min(20, Number(climate.rtc_temp_correction || 0))));
}

async function finalizeTemperatureCorrectionPersistence(climate) {
  if (!climate) {
    return;
  }

  const rtcCorrection = Math.max(-20, Math.min(20, Number(climate.rtc_temp_correction || 0)));
  const ds18xxCorrection = Math.max(-20, Math.min(20, Number(climate.ds18xx_temp_correction || 0)));
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

  setSettingsBackupNote("Importiere Animationen...");

  await apiFetchValue(getAnimationModeSetUrl(), Number(animations.display_mode || 0));
  await apiFetchValue(getColorAnimationModeSetUrl(), Number(animations.color_mode || 0));

  for (const entry of (animations.display_profiles || [])) {
    await apiFetchQuery(getAnimationProfileSetUrl(), {
      idx: Number(entry.idx || 0),
      deceleration: Number(entry.deceleration || 0),
      favourite: entry.favourite ? "on" : "off"
    });
  }

  for (const entry of (animations.color_profiles || [])) {
    await apiFetchQuery(getColorAnimationProfileSetUrl(), {
      idx: Number(entry.idx || 0),
      deceleration: Number(entry.deceleration || 0)
    });
  }
}

async function importTftSettings(tft) {
  if (!tft) {
    return;
  }

  setSettingsBackupNote("Importiere TFT-Einstellungen...");

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

  setSettingsBackupNote("Importiere Ambilight-Einstellungen...");

  const ambilightOnlineValue = ambilight.online ? "on" : "off";
  await apiFetchValue(getAmbilightOnlineSetUrl(), ambilightOnlineValue);
  setPersistedAmbilightState(ambilightOnlineValue);
  await apiFetchValue(getAmbilightPowerSetUrl(), ambilight.power ? "on" : "off");
  await apiFetchValue(getAmbilightModeSetUrl(), Number(ambilight.mode || 0));
  await apiFetchValue(getAmbilightLedsSetUrl(), Number(ambilight.leds || 0));
  await apiFetchValue(getAmbilightOffsetSetUrl(), Number(ambilight.offset || 0));
  await apiFetchValue(getAmbilightBrightnessSetUrl(), Number(ambilight.brightness || 0));
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
      deceleration: Number(entry.deceleration || 0)
    });
  }
}

async function importDfplayerSettings(dfplayer) {
  if (!dfplayer) {
    return;
  }

  setSettingsBackupNote("Importiere DFPlayer-Einstellungen...");

  await apiFetchValue(getDfplayerVolumeSetUrl(), Number(dfplayer.volume || 0));
  await apiFetchValue(getDfplayerModeSetUrl(), Number(dfplayer.mode || 0));
  await apiFetchQuery(getDfplayerBellFlagsSetUrl(), {
    m15: (Number(dfplayer.bell_flags || 0) & 0x01) ? "on" : "off",
    m30: (Number(dfplayer.bell_flags || 0) & 0x02) ? "on" : "off",
    m45: (Number(dfplayer.bell_flags || 0) & 0x04) ? "on" : "off"
  });
  await apiFetchValue(getDfplayerSpeakCycleSetUrl(), Number(dfplayer.speak_cycle || 0));
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
      from: Number(entry.from || 0),
      to: Number(entry.to || 0),
      hour: Number(entry.hour || 0),
      minute: Number(entry.minute || 0)
    })
  );
}

async function importOverlaySettings(overlays) {
  if (!overlays) {
    return;
  }

  setSettingsBackupNote("Importiere Overlays...");

  const items = Array.isArray(overlays.items) ? overlays.items.slice().sort((a, b) => a.idx - b.idx) : [];
  for (let idx = 31; idx >= 0; idx -= 1) {
    try {
      await apiFetchQuery(getOverlayDeleteUrl(), { idx });
    } catch (_) {}
  }
  await sleep(1500);

  for (let idx = 0; idx < items.length; idx += 1) {
    const entry = items[idx] || {};
    const entryFlags = Number(entry.flags || 0);
    const entryDateStart = Number(entry.date_start || 0);
    const entryValue = entry.value !== undefined ? entry.value : entry.text;
    await apiFetchQuery(getOverlaySetUrl(), {
      idx,
      active: (entry.active !== undefined ? entry.active : !!(entryFlags & 0x01)) ? "on" : "off",
      type: Number(entry.type || 0),
      value: entryValue || "",
      interval: Number(entry.interval || 0),
      duration: Number(entry.duration || 0),
      date_code: Number(entry.date_code || 0),
      month: Number(entry.month || ((entryDateStart >> 8) & 0xff) || 0),
      day: Number(entry.day || (entryDateStart & 0xff) || 0),
      days: Number(entry.days || 1)
    });
    await sleep(idx === 0 ? 1100 : 700);
  }
  await sleep(1200);
}

async function importTimerSettings(timers) {
  if (!timers) {
    return;
  }

  setSettingsBackupNote("Importiere Timer...");

  const importTimerGroup = async (endpoint, entries) => {
    const entryMap = new Map((entries || []).map((entry) => [Number(entry.idx || 0), entry]));

    for (let idx = 0; idx < 8; idx += 1) {
      const entry = entryMap.get(idx) || { idx, active: false, switch_on: false, from: 0, to: 0, hour: 0, minute: 0 };
      await apiFetchQuery(endpoint, {
        idx: Number(entry.idx || 0),
        active: entry.active ? "on" : "off",
        switch_on: entry.switch_on ? "on" : "off",
        from: Number(entry.from || 0),
        to: Number(entry.to || 0),
        hour: Number(entry.hour || 0),
        minute: Number(entry.minute || 0)
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

  await apiFetchQuery(endpoint, {
    red: Number(color.red || 0),
    green: Number(color.green || 0),
    blue: Number(color.blue || 0),
    white: Number(color.white || 0)
  });
}

async function saveImportedDimCurve(endpoint, values) {
  if (!Array.isArray(values)) {
    return;
  }

  for (let idx = 0; idx < values.length && idx <= 15; idx += 1) {
    await apiFetchQuery(endpoint, { idx, value: Number(values[idx] || 0) });
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
          '<button class="button primary" type="button" data-alarm-save="' + idx + '">' + escapeHtml(translate("common.save")) + '</button>' +
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
          '<button class="button primary" type="button" data-an-save="' + item.idx + '">' + escapeHtml(translate("animations.profile_save")) + '</button>' +
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
          '<button class="button primary" type="button" data-can-save="' + item.idx + '">' + escapeHtml(translate("animations.profile_save")) + '</button>' +
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
          '<button class="button primary" type="button" data-alm-save="' + item.idx + '">' + escapeHtml(translate("animations.profile_save")) + '</button>' +
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
  setUploadFormSupported("fs-upload-icon-form", uploadMeta.targetUploadsSupported, "PWA-Zieluploads werden von dieser Firmware noch nicht unterstützt.");
  setUploadFormSupported("fs-upload-weather-form", uploadMeta.targetUploadsSupported, "PWA-Zieluploads werden von dieser Firmware noch nicht unterstützt.");
  setUploadFormSupported("fs-upload-tables-form", uploadMeta.targetUploadsSupported, "PWA-Zieluploads werden von dieser Firmware noch nicht unterstützt.");
  setUploadFormSupported("fs-upload-display-form", uploadMeta.targetUploadsSupported, "PWA-Zieluploads werden von dieser Firmware noch nicht unterstützt.");
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
    : '<option value="">keine Layout-Tabellen gefunden</option>';
  tableButton.classList.toggle("is-hidden", !serverFilesMeta.tableActionSupported);
  assetsButton.classList.toggle("is-hidden", !serverFilesMeta.assetsActionSupported);
  appFilesButton.classList.toggle("is-hidden", !serverFilesMeta.appFilesActionSupported);
  serverFilesBlock.classList.toggle("is-hidden", !serverFilesMeta.anyActionSupported);

  document.getElementById("update-release-notes").innerHTML = view.releaseNotes || "<p>" + escapeHtml(translate("maintenance.no_release_notes")) + "</p>";
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

function getNetworkUiMeta(settings, networkInfo) {
  const timezone = decodeTimezone(settings.numvars[NUM.TIMEZONE] || 0);
  return {
    networks: Array.isArray(networkInfo && networkInfo.networks) ? networkInfo.networks : [],
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
    network: getNetworkUiMeta(settings, networkInfo),
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
  const clockMode = (settings.almodes || []).find((entry) => entry.idx === 0) || null;
  const markersEnabled = !!(((clockMode && clockMode.flags) || 0) & 0x02);

  return {
    syncAmbilight: ambilightOnline && !!(flags & 0x02),
    syncMarkers: ambilightOnline && !!(flags & 0x04),
    fadeClockSeconds: ambilightOnline && !!(flags & 0x08),
    ambilightMarkers: ambilightOnline && markersEnabled
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
    announceStatus("Preset konnte nicht angewendet werden", "error");
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
    const showIcon = type === 1;
    const showText = type === 6;
    const showMp3 = type === 7;
    const showDuration = type === 1 || type === 4 || type === 8;
    const showDateStart = overlay.date_code === 0;
    const showDays = !(overlay.date_code === 0 && !overlay.date_start);
    const idx = overlay.idx;
    const title = overlay.isNew ? translate("overlays.new_title") : "Overlay " + String(idx);
    const overlayTypeNames = getOverlayTypeNames();
    const overlayTypeName = overlayTypeNames[type] || translate("common.none");

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
            '<label class="field"><span class="label">' + escapeHtml(translate("overlays.type")) + '</span><select id="ov-type-' + idx + '">' + buildNamedOptions(overlayTypeNames, overlay.type) + "</select></label>" +
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
              '<label class="field"><span class="label">' + escapeHtml(translate("overlays.interval")) + '</span><input id="ov-interval-' + idx + '" type="number" min="1" max="99" step="1" value="' + escapeHtml(String(overlay.interval || 5)) + '"></label>' +
              '<label id="ov-duration-wrap-' + idx + '" class="field' + (showDuration ? '' : ' is-hidden') + '"><span class="label">' + escapeHtml(translate("overlays.duration")) + '</span><input id="ov-duration-' + idx + '" type="number" min="5" max="9" step="1" value="' + escapeHtml(String(overlay.duration || 5)) + '"></label>' +
            '</div>' +
            '<label id="ov-datecode-wrap-' + idx + '" class="field"><span class="label">' + escapeHtml(translate("overlays.date_code")) + '</span><select id="ov-datecode-' + idx + '">' + buildNamedOptions(getOverlayDateCodeNames(), overlay.date_code) + '</select></label>' +
            '<div class="overlay-date-grid">' +
              '<label id="ov-day-wrap-' + idx + '" class="field' + (showDateStart ? '' : ' is-hidden') + '"><span class="label">' + escapeHtml(translate("overlays.day")) + '</span><select id="ov-day-' + idx + '">' + buildDayOptions(day) + '</select></label>' +
              '<label id="ov-month-wrap-' + idx + '" class="field' + (showDateStart ? '' : ' is-hidden') + '"><span class="label">' + escapeHtml(translate("overlays.month")) + '</span><select id="ov-month-' + idx + '">' + buildMonthOptions(month) + '</select></label>' +
              '<label id="ov-days-wrap-' + idx + '" class="field' + (showDays ? '' : ' is-hidden') + '"><span class="label">' + escapeHtml(translate("overlays.days")) + '</span><input id="ov-days-' + idx + '" type="number" min="1" max="255" step="1" value="' + escapeHtml(String(overlay.days || 1)) + '"></label>' +
            '</div>' +
          '</div>' +
          '<div class="profile-actions overlay-actions">' +
            '<button class="button primary" type="button" data-overlay-save="' + idx + '">' + escapeHtml(overlay.isNew ? translate("overlays.create") : translate("overlays.save")) + "</button>" +
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

function renderTimerRowsFromMeta(items, isAmbilight) {
  const root = document.getElementById(isAmbilight ? "ambilight-timer-list" : "timer-list");

  root.innerHTML = items.map((item) => {
    const idx = item.idx;
    const time = minutesToTimeValue(item.minutes || 0);
    const fromDay = (item.flags & 0x38) >> 3;
    const toDay = item.flags & 0x07;
    const active = (item.flags & 0x80) ? "checked" : "";
    const switchOn = (item.flags & 0x40) ? "checked" : "";
    const prefix = isAmbilight ? "at" : "t";

    return (
      '<section class="alarm-card">' +
        '<div class="card-headline"><div><span class="label">' + escapeHtml(translate("timers.slot")) + ' ' + escapeHtml(String(idx)) + '</span><p class="card-subline">' + escapeHtml(isAmbilight ? translate("timers.ambilight_slot_subline") : translate("timers.slot_subline")) + '</p></div></div>' +
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
          '<button class="button primary" type="button" data-' + prefix + '-save="' + idx + '">' + escapeHtml(translate("common.save")) + '</button>' +
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

  setActionToggleButton("sync-ambilight-button", translate("display.unsync_ambilight"), translate("display.sync_ambilight"), meta.syncAmbilight);
  setActionToggleButton("sync-markers-button", translate("display.unsync_markers"), translate("display.sync_markers"), meta.syncMarkers);
  setActionToggleButton("fade-clock-seconds-button", translate("display.fade_clock_seconds_disable"), translate("display.fade_clock_seconds"), meta.fadeClockSeconds);
  setActionToggleButton("ambilight-markers-button", "5-Sekunden-Marker deaktivieren", "5-Sekunden-Marker aktivieren", meta.ambilightMarkers);
}

function setActionToggleButton(id, onText, offText, enabled) {
  const button = document.getElementById(id);
  button.dataset.state = enabled ? "on" : "off";
  button.textContent = enabled ? onText : offText;
  button.classList.toggle("primary", enabled);
}

function updateLiveColorPreview(prefix, syncTheme) {
  const preview = document.getElementById(prefix + "-color-preview");
  const rgbInput = document.getElementById(prefix + "-color-rgb");
  const whiteInput = document.getElementById(prefix + "-color-white");
  const white = Number(whiteInput.value || 0);
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
  await runValueSave("display-mode-save-button", getDisplayModeSetUrl(), value, translate("display.save_mode"), "Display-Modus konnte nicht gespeichert werden");
}

async function saveTickerText() {
  const value = document.getElementById("ticker-text-input").value;
  await runValueSave("ticker-save-button", getTickerSetUrl(), value, translate("display.save_ticker"), "Ticker konnte nicht gespeichert werden");
}

async function saveDateTickerFormat() {
  const value = document.getElementById("date-format-input").value;
  await runValueSave("date-format-save-button", getDateTickerFormatSetUrl(), value, translate("display.save_date_format"), "Datumsformat konnte nicht gespeichert werden");
}

async function saveTickerDeceleration() {
  const input = document.getElementById("ticker-deceleration-input");
  const value = Math.max(0, Math.min(255, Number(input.value || 0)));
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
  await runValueSave("weather-appid-save-button", getWeatherAppIdSetUrl(), value, translate("climate.save_api_key"), "API-Schlüssel konnte nicht gespeichert werden");
}

async function saveWeatherCity() {
  const value = document.getElementById("weather-city-input").value || "";
  await runValueSave("weather-city-save-button", getWeatherCitySetUrl(), value, translate("climate.save_city"), "Ort konnte nicht gespeichert werden");
}

async function saveWeatherCoordinates() {
  const lon = document.getElementById("weather-lon-input").value || "";
  const lat = document.getElementById("weather-lat-input").value || "";
  await runQuerySave("weather-coordinates-save-button", getWeatherCoordinatesSetUrl(), { lon, lat }, translate("climate.save_coordinates"), "Koordinaten konnten nicht gespeichert werden");
}

async function getWeatherNow() {
  await runWeatherAction("weather-now-button", getWeatherNowUrl(), translate("climate.fetch_weather"), "Wetter konnte nicht angefordert werden");
}

async function getWeatherForecast() {
  await runWeatherAction("weather-forecast-button", getWeatherForecastUrl(), translate("climate.fetch_forecast"), "Wettervorhersage konnte nicht angefordert werden");
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

async function openWeatherMapPicker() {
  const modal = document.getElementById("weather-map-modal");
  const status = document.getElementById("weather-map-status");
  modal.classList.remove("is-hidden");
  modal.setAttribute("aria-hidden", "false");
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
  modal.classList.add("is-hidden");
  modal.setAttribute("aria-hidden", "true");
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

  document.getElementById("weather-map-city-input").value = city;
  document.getElementById("weather-map-lon-input").value = Number.isFinite(lon) ? lon.toFixed(4) : "";
  document.getElementById("weather-map-lat-input").value = Number.isFinite(lat) ? lat.toFixed(4) : "";

  if (weatherMap && Number.isFinite(lat) && Number.isFinite(lon)) {
    setWeatherMapSelection(lat, lon, city);
    weatherMap.setView([lat, lon], 10);
  }
}

function setWeatherMapSelection(lat, lon, city) {
  if (!weatherMap || !window.L) {
    return;
  }

  const roundedLat = Number(lat);
  const roundedLon = Number(lon);
  selectedWeatherLocation = {
    city: city || "",
    lat: roundedLat,
    lon: roundedLon
  };

  document.getElementById("weather-map-city-input").value = city || "";
  document.getElementById("weather-map-lat-input").value = roundedLat.toFixed(4);
  document.getElementById("weather-map-lon-input").value = roundedLon.toFixed(4);

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
        useApproximateWeatherLocation("Standortfreigabe wurde abgelehnt. Näherungsstandort wird ermittelt...");
      } else if (error && error.code === 2) {
        useApproximateWeatherLocation("Standort ist derzeit nicht verfügbar. Näherungsstandort wird ermittelt...");
      } else if (error && error.code === 3) {
        useApproximateWeatherLocation("Standortabfrage lief in ein Zeitlimit. Näherungsstandort wird ermittelt...");
      } else {
        useApproximateWeatherLocation("Standort konnte nicht gelesen werden. Näherungsstandort wird ermittelt...");
      }
    },
    { enableHighAccuracy: true, timeout: 10000, maximumAge: 30000 }
  );
}

async function useApproximateWeatherLocation(initialMessage) {
  const button = document.getElementById("weather-current-location-button");
  const status = document.getElementById("weather-map-status");

  beginButtonFeedback(button, "ermittelt...");
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

    document.getElementById("weather-map-city-input").value = city;
    if (selectedWeatherLocation) {
      selectedWeatherLocation.city = city;
    }
    status.textContent = translate("weather.map_applied");
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
      await apiFetchValue(getWeatherCitySetUrl(), city);
      await apiFetchQuery(getWeatherCoordinatesSetUrl(), { lon, lat });
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
  const ssid = document.getElementById("network-ssid-select").value || "";
  const key = document.getElementById("network-key-input").value || "";
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
  await runTextSave("network-timeserver-save-button", getNetworkTimeserverSetUrl(), document.getElementById("network-timeserver-input").value || "", translate("network.save_timeserver"), "Zeitserver konnte nicht gespeichert werden");
}

async function saveTimezone() {
  const input = document.getElementById("network-timezone-input");
  const value = Math.max(-12, Math.min(14, Number(input.value || 0)));
  input.value = String(value);
  await runQuerySave("network-timezone-save-button", getNetworkTimezoneSetUrl(), { value }, translate("network.save_timezone"), "Zeitzone konnte nicht gespeichert werden", {
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
    errorText: "Sommerzeit konnte nicht gesetzt werden",
    preserveCurrentText: true
  });
}

async function saveDateTime() {
  const year = clampNumber(document.getElementById("datetime-year-input").value, 2000, 2999, 2026);
  const month = clampNumber(document.getElementById("datetime-month-input").value, 1, 12, 1);
  const day = clampNumber(document.getElementById("datetime-day-input").value, 1, 31, 1);
  const hour = clampNumber(document.getElementById("datetime-hour-input").value, 0, 23, 0);
  const minute = clampNumber(document.getElementById("datetime-minute-input").value, 0, 59, 0);

  document.getElementById("datetime-year-input").value = String(year);
  document.getElementById("datetime-month-input").value = String(month);
  document.getElementById("datetime-day-input").value = String(day);
  document.getElementById("datetime-hour-input").value = String(hour);
  document.getElementById("datetime-minute-input").value = String(minute);

  await runQuerySave("datetime-save-button", getDateTimeSetUrl(), { year, month, day, hour, minute }, translate("system.datetime_save"), "Datum und Uhrzeit konnten nicht gespeichert werden");
}

async function learnIrRemote() {
  await runSimpleAction("learn-ir-button", getLearnIrUrl(), translate("system.learn_ir"), "IR-Lernmodus konnte nicht gestartet werden", "IR-Lernmodus gestartet");
}

async function getNetTime() {
  await runSimpleAction("network-nettime-button", getNetworkGetTimeUrl(), translate("network.fetch_network_time"), "Netzzeit konnte nicht angefordert werden", "Netzzeit angefordert");
}

async function runWps() {
  await runSimpleAction("network-wps-button", getNetworkWpsUrl(), "WPS", "WPS konnte nicht gestartet werden", "WPS wurde gestartet");
}

async function saveUpdateHost() {
  await runTextSave("update-host-save-button", getUpdateHostSetUrl(), document.getElementById("update-host-input").value || "", translate("maintenance.save_update_host"), "Update-Host konnte nicht gespeichert werden");
  await refreshUpdateServerAvailability();
}

async function saveUpdatePath() {
  await runTextSave("update-path-save-button", getUpdatePathSetUrl(), document.getElementById("update-path-input").value || "", translate("maintenance.save_update_path"), "Update-Pfad konnte nicht gespeichert werden");
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

  if (!isBinFileName(file.name)) {
    document.getElementById("local-update-note").textContent = "Falsche ESP-Datei ausgewählt. Erwartet wird eine .bin-Datei.";
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
        document.getElementById("local-update-note").textContent = "ESP-Firmware wird hochgeladen: " + percent + "%";
        document.getElementById("update-progress-note").textContent = "ESP-Firmware wird hochgeladen: " + percent + "%";
        document.getElementById("updated-at").textContent = document.getElementById("update-progress-note").textContent;
      }
    );
  } catch (error) {
    stopUpdateProgressPolling();
    document.getElementById("local-update-note").textContent = "ESP-Firmware konnte nicht hochgeladen werden: " + (error.message || "unbekannter Fehler");
    button.disabled = false;
    button.textContent = translate("maintenance.local_esp_update");
    finishProgressUi(0);
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

  if (!isMatchingLocalStm32File(file.name)) {
    const expected = getExpectedLocalStm32Filename(getCurrentUpdateStatus()) || "passende STM32-.hex-Datei";
    document.getElementById("local-update-note").textContent = "Falsche STM32-Datei ausgewählt. Erwartet wird " + expected + ".";
    announceStatus(translate("maintenance.local_stm32_expected"), "error");
    finishButtonFeedback(button, translate("maintenance.local_stm32_update"), "error", translate("common.error"));
    return;
  }

  button.disabled = true;
  button.textContent = "lädt hoch...";
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

  beginButtonFeedback(button, "lädt hoch...");
  setFsActionStatus(startStatusText);
  if (successAnnounceText) {
    announceStatus(startStatusText, "warn");
  }

  await uploadRawFile(
    uploadUrl,
    file,
    (loaded, total) => {
      const percent = total ? Math.min(100, Math.round((loaded / total) * 100)) : 0;
      button.textContent = "lädt hoch... " + percent + "%";
      setFsActionStatus(onProgressText ? onProgressText(percent) : startStatusText);
    },
    () => {
      button.classList.add("is-busy");
      button.textContent = "wird installiert...";
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
  finishButtonFeedback(button, idleText || getUploadActionButtonText(button, "Datei hochladen"), "success", successText || "hochgeladen");

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
const getDisplayUseRgbwSetUrl = createConfiguredUrlGetter("display_use_rgbw_set_url");
const getTickerSetUrl = createConfiguredUrlGetter("ticker_set_url");
const getDateTickerFormatSetUrl = createConfiguredUrlGetter("date_ticker_format_set_url");
const getTickerDecelerationSetUrl = createConfiguredUrlGetter("ticker_deceleration_set_url");
const getAmbilightPowerUrl = createConfiguredUrlGetter("ambilight_power_url");
const getAmbilightPowerSetUrl = createConfiguredUrlGetter("ambilight_power_set_url");
const getAmbilightOnlineSetUrl = createConfiguredUrlGetter("ambilight_online_set_url");
const getPowerStatusUrl = createConfiguredUrlGetter("power_status_url");
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

function isTablesUploadUrl(url) {
  return normalizeUrlPath(url) === normalizeUrlPath(getFsUploadUrl("tables"));
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

  if (!isMatchingFsUploadFile(url, file.name, targetName)) {
    document.getElementById("fs-action-status").textContent =
      isTablesUploadUrl(url)
        ? translateFormat("maintenance.file_expected_pattern", { target: targetName, pattern: targetName.replace("local.txt", "*.txt") })
        : translateFormat("maintenance.file_expected_exact", { target: targetName });
    announceStatus(targetName + " erwartet", "error");
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
      startStatusText: targetName + " wird hochgeladen: " + file.name,
      installStatusText: targetName + " wurde hochgeladen und wird jetzt gespeichert...",
      successStatusText: successMessage,
      successAnnounceText: successMessage,
      idleText: translate("common.file_upload"),
      successText: translate("common.uploaded"),
      onProgressText: (percent) => targetName + " wird hochgeladen: " + percent + "%",
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
  if (!window.confirm("EEPROM wirklich auf Werkseinstellungen zurücksetzen?")) {
    return;
  }

  if (!window.confirm("Wirklich alle EEPROM-Werte auf Werkseinstellungen zurücksetzen?")) {
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
      errorText: "LittleFS konnte nicht formatiert werden",
      successStatusText: "LittleFS wurde formatiert",
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
      errorText: "LittleFS konnte nicht formatiert werden",
      successStatusText: "LittleFS wurde formatiert",
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
    document.getElementById("fs-preview-content").textContent = text || "(leer)";
    setFsActionStatus(translateFormat("maintenance.fs_showing", { file: fileName }));
    announceStatus(fileName + " geladen", "ok");
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
      failStm32Update("Remote STM32-Flash konnte nicht gestartet werden.");
      return;
    }

    if (stm32RemoteResultOkSeen && !stm32AutoResetStarted) {
      beginStm32AutoReset("STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.");
      return;
    }

    if (!stm32AutoResetStarted) {
      const progress = await settleFetchJson(getUpdateProgressUrl(), { ok: false }, 1000);
      applyUpdateProgressStatus(progress);
    }

    if (!stm32AutoResetStarted) {
      failStm32Update("STM32-Flash-Ende konnte nicht sicher erkannt werden.");
      return;
    }
  };

  xhr.onerror = () => {
    stm32RemoteRequestInFlight = false;
    failStm32Update("Remote STM32-Flash konnte nicht gestartet werden.");
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
      beginStm32AutoReset("STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.");
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
  finishProgressUi(0);
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
    await fetch(getMaintenanceResetStm32Url(), { cache: "no-store" });
    setStm32ProgressStage(6);
    document.getElementById("updated-at").textContent = translate("maintenance.stm32_auto_reset_wait");
    document.getElementById("update-progress-note").textContent = translate("maintenance.stm32_auto_reset_running");
    await sleep(4000);
    setStm32ProgressStage(8);
    document.getElementById("updated-at").textContent = "STM32 wurde nach dem Flash automatisch zurückgesetzt";
    document.getElementById("update-progress-note").textContent = translate("maintenance.stm32_flash_success");
    stopStm32Progress();
    resetProgressButton();
    finishProgressUi(2200);
    scheduleProgressReturnScroll(2300);
    try {
      await loadData();
    } catch (error) {
      announceStatus("Daten konnten nach dem STM32-Update nicht neu geladen werden", "warn");
    }
  } catch (error) {
    document.getElementById("update-progress-note").textContent = translate("maintenance.stm32_flash_auto_reset_failed");
    stopStm32Progress();
    stopUpdateProgressPolling();
    resetProgressButton();
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
      note.textContent = "Gerät sollte wieder bereit sein. App wird vorsorglich neu geladen.";
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
    note.textContent = "Kein sicheres Reconnect-Signal erhalten. App wird vorsorglich neu geladen.";
    document.getElementById("updated-at").textContent = note.textContent;
    resetProgressButton();
    finishProgressUi(900);
    setTimeout(reloadAppPage, 900);
    return;
  }

  note.textContent = "ESP ist noch nicht wieder erreichbar. Bitte Seite bei Bedarf manuell neu laden.";
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
  if (hasUnsavedEdits && !window.confirm("Es gibt ungespeicherte Änderungen. App trotzdem neu laden?")) {
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
      note.textContent = "Neuladen wird erzwungen, damit die aktualisierte App wieder angezeigt wird.";
    }
    document.getElementById("updated-at").textContent = note ? note.textContent : "App wird neu geladen.";
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
      note.textContent = progress.message || "STM32-Flash abgeschlossen. Abschluss wird bestätigt...";
      return;
    }

    stm32AutoResetStarted = true;
    setStm32ProgressStage(5);
    note.textContent = progress.message || "STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.";
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
  await saveTemperatureCorrection("temperature-rtc-correction-input", "temperature-rtc-correction-save-button", getTemperatureRtcCorrectionSetUrl(), "RTC-Korrektur speichern", "RTC-Korrektur konnte nicht gespeichert werden");
}

async function saveDs18xxTemperatureCorrection() {
  await saveTemperatureCorrection("temperature-ds18xx-correction-input", "temperature-ds18xx-correction-save-button", getTemperatureDs18xxCorrectionSetUrl(), translate("climate.save_ds18xx_correction"), "DS18xx-Korrektur konnte nicht gespeichert werden");
}

async function saveTemperatureCorrection(inputId, buttonId, endpoint, buttonText, errorText) {
  const input = document.getElementById(inputId);
  const value = Math.max(-20, Math.min(20, Number(input.value || 0)));
  input.value = String(value);
  await runValueSave(buttonId, endpoint, value, buttonText, errorText);
}

async function displayTemperatureNow() {
  await runSimpleAction("temperature-display-button", getTemperatureDisplayUrl(), translate("climate.show_temperature"), "Temperatur konnte nicht angezeigt werden", "Temperaturanzeige ausgelöst");
}

async function setLdrMinValue() {
  await runSimpleAction("ldr-min-button", getLdrMinSetUrl(), translate("climate.set_min_ldr"), "LDR-Minimum konnte nicht gesetzt werden", "LDR-Minimum gespeichert");
}

async function setLdrMaxValue() {
  await runSimpleAction("ldr-max-button", getLdrMaxSetUrl(), translate("climate.set_max_ldr"), "LDR-Maximum konnte nicht gesetzt werden", "LDR-Maximum gespeichert");
}

async function saveAnimationMode() {
  await runSelectSave("animation-mode-select", "animation-mode-save-button", getAnimationModeSetUrl(), translate("animations.save_display_animation"), "Anzeigeanimation konnte nicht gespeichert werden");
}

async function saveColorAnimationMode() {
  await runSelectSave("color-animation-mode-select", "color-animation-mode-save-button", getColorAnimationModeSetUrl(), translate("animations.save_color_animation"), "Farbanimation konnte nicht gespeichert werden");
}

async function runSelectSave(selectId, buttonId, endpoint, buttonText, errorText) {
  const value = document.getElementById(selectId).value;
  await runValueSave(buttonId, endpoint, value, buttonText, errorText);
}

async function saveAnimationProfile(idx) {
  const deceleration = document.getElementById("an-dec-" + idx).value;
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
  const deceleration = document.getElementById("can-dec-" + idx).value;
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

  beginButtonFeedback(button, translate("common.saving"));

  try {
    for (let idx = 0; idx <= 15; idx += 1) {
      const input = document.getElementById(prefix + "-dim-" + idx);
      const value = Math.max(0, Math.min(15, Number(input.value || 0)));
      input.value = String(value);
      syncDimCurveValue(prefix, idx);
      await apiFetch(endpoint + "?idx=" + idx + "&value=" + encodeURIComponent(value));
    }
    await loadData();
    finishButtonFeedback(button, buttonText, "success", translate("common.saved"));
  } catch (error) {
    announceStatus(translate("display.dim_curve_save_failed"), "error");
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
    errorText: options && options.errorText ? options.errorText : "Schalter konnte nicht gesetzt werden",
    reload: true,
    preserveCurrentText: options && options.preserveCurrentText !== undefined ? options.preserveCurrentText : true
  });
}

async function runButtonRequest(button, options) {
  const {
    busyText = translate("common.running"),
    idleText = button && (button.dataset.restoreText || button.textContent) ? (button.dataset.restoreText || button.textContent) : "",
    successText = translate("common.started"),
    errorText = "Aktion konnte nicht ausgeführt werden",
    successStatusText = "",
    reload = false,
    reloadDelayMs = 0,
    preserveCurrentText = false,
    request
  } = options || {};

  beginButtonFeedback(button, busyText);

  try {
    await request();
    if (reload) {
      await loadData();
    } else if (reloadDelayMs > 0) {
      setTimeout(loadData, reloadDelayMs);
    }
    if (successStatusText) {
      announceStatus(successStatusText, "ok");
    }
    finishButtonFeedback(button, idleText, "success", successText, preserveCurrentText);
  } catch (error) {
    announceStatus(errorText, "error");
    finishButtonFeedback(button, idleText, "error", translate("common.error"), preserveCurrentText);
  }
}

async function saveAmbilightModeProfile(idx) {
  const deceleration = Math.max(0, Math.min(15, Number(document.getElementById("alm-dec-" + idx).value || 0)));
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
  const value = Math.max(0, Math.min(999, Number(input.value || 0)));
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
  const value = Math.max(0, Math.min(999, Number(input.value || 0)));
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
  const value = Math.max(0, Math.min(255, Number(input.value || 0)));
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
  const folder = Math.max(0, Math.min(255, Number(document.getElementById("dfplayer-folder-input").value || 0)));
  const track = Math.max(0, Math.min(255, Number(document.getElementById("dfplayer-track-input").value || 0)));
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
  const value = typeNumber === 1
    ? (document.getElementById("ov-icon-" + idx).value || "")
    : typeNumber === 7
      ? formatOverlayMp3Value(document.getElementById("ov-folder-" + idx).value, document.getElementById("ov-track-" + idx).value)
      : (typeNumber === 6 ? document.getElementById("ov-value-" + idx).value : "");
  const interval = document.getElementById("ov-interval-" + idx).value;
  const duration = document.getElementById("ov-duration-" + idx).value;
  const dateCode = document.getElementById("ov-datecode-" + idx).value;
  const month = document.getElementById("ov-month-" + idx).value;
  const day = document.getElementById("ov-day-" + idx).value;
  const days = document.getElementById("ov-days-" + idx).value;
  await runIndexedQueryButtonRequest('[data-overlay-save="%idx%"]', idx, {
    endpoint: getOverlaySetUrl(),
    query: {
      idx,
      active,
      type,
      value,
      interval,
      duration,
      date_code: dateCode,
      month,
      day,
      days
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
    hasUnsavedEdits = false;
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
    await loadData();
    announceStatus("Alle Timer wurden gespeichert", "ok");
    finishButtonFeedback(button, originalText, "success", translate("common.saved"));
  } catch (error) {
    announceStatus("Timer konnten nicht vollständig gespeichert werden", "error");
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
    await loadData();
    finishButtonFeedback(button, button.dataset.restoreText || button.textContent, "success", next === "on" ? "aktiviert" : "deaktiviert", true);
  } catch (error) {
    announceStatus("Schalter konnte nicht gesetzt werden", "error");
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
    [translate("overview.display_mode"), getDisplayModeName(displayMeta.mode)],
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
      note: "Debug Override aktiv. Farb-LED UI wird als RGBW angezeigt."
    };
  }

  if (debugOverrides.color === "rgb") {
    return {
      hasColor: true,
      whiteChannel: false,
      mode: "rgb",
      label: "RGB (Debug Override)",
      note: "Debug Override aktiv. Farb-LED UI wird als RGB angezeigt."
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
        note: "RGBW-Hardware erkannt. RGB- und Weisskanal sind verfügbar."
      };
    }

    if (mode === "rgb" || mode === "tft") {
      return {
        hasColor: true,
        whiteChannel,
        mode: whiteChannel ? "rgbw" : "rgb",
        label: displayInfo.label || (hasTft ? "TFT RGB" : "RGB"),
        note: hasTft
          ? "TFT-Hardware erkannt. TFT-Optionen sind verfügbar, der Weisskanal bleibt ausgeblendet."
          : "RGB-Hardware erkannt. Der White-Channel ist daher ausgeblendet."
      };
    }

    if (mode === "none") {
      return {
        hasColor: false,
        whiteChannel: false,
        mode: "none",
        label: displayInfo.label || "keine Farb-LEDs erkannt",
        note: "Keine unterstützte Farb-LED-Hardware erkannt. Farbsteuerung ist deshalb ausgeblendet."
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
        note: "RGBW-Hardware erkannt. RGB- und Weisskanal sind verfügbar."
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
          ? "TFT-Hardware erkannt. TFT-Optionen sind verfügbar, der Weisskanal bleibt ausgeblendet."
          : "RGB-Hardware erkannt. Der White-Channel ist daher ausgeblendet."
      };
    default:
      return {
        hasColor: false,
        whiteChannel: false,
        mode: "none",
        label: "keine Farb-LEDs erkannt",
        note: "Keine unterstützte Farb-LED-Hardware erkannt. Farbsteuerung ist deshalb ausgeblendet."
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

function getDisplayModeName(mode) {
  const names = {
    0: translate("display.mode_normal"),
    1: translate("display.mode_seconds"),
    2: translate("display.mode_date"),
    3: translate("display.mode_temperature"),
    4: translate("display.mode_ticker")
  };
  return names[mode] || String(mode || 0);
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

  const colorMeta = getColorUiMeta(settings, !!(settings.numvars && settings.numvars[NUM.AMBILIGHT_IS_UP]), loadDebugOverrides());
  const useRgbw = colorMeta.useRgbw;
  const colorAnimationMode = Number(settings.numvars[NUM.COLOR_ANIMATION_MODE] || 0);

  applyWordclockTheme(currentLiveDisplayColor, useRgbw, colorAnimationMode);
}

function syncLiveDisplayColorPolling(settings) {
  if (settingsImportInProgress || isBackgroundPauseActive()) {
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

function buildNamedOptions(values, selected) {
  return values.map((value, idx) => (
    '<option value="' + idx + '"' + (idx === selected ? " selected" : "") + ">" + escapeHtml(value) + "</option>"
  )).join("");
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
    announceStatus("Icon-Liste konnte nicht geladen werden", "error");
  }
}

function toggleHidden(id, hidden) {
  const element = document.getElementById(id);
  if (element) {
    element.classList.toggle("is-hidden", hidden);
  }
}

function clampNumber(value, min, max, fallback) {
  const number = Number(value);

  if (!Number.isFinite(number)) {
    return fallback;
  }

  return Math.max(min, Math.min(max, Math.trunc(number)));
}

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
