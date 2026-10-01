# ESP: Absturz per URL und Auslieferung leerer Dateien

**Auslöser:** `REVIEW-2026-09-29.md`, Kernbefunde 3 und 4, Massnahmen 1 und 2.

## Befund 1 — ESP-Absturz aus dem ganzen LAN auslösbar

`http_set_params()` setzt `.name` immer, `.value` aber **nur**, wenn ein `=` gefunden
wird. Im `&`-Zweig beginnt ein neuer Name, ohne den Wert zurückzusetzen.
`normalize_http_parameters()` beginnt danach mit `while (*p)` — **ohne Null-Prüfung**.

Ein `GET /?a` genügt. Beim ersten Mal nach dem Start ist `.value` NULL; danach zeigt es
auf den Stackpuffer des **vorherigen** Requests, dessen Frame längst weg ist — und die
Funktion *schreibt* dort hinein. Kein Login, keine Hürde.

## Befund 2 — leere `.gz` wird mit 200 OK ausgeliefert

`http_send_fs_file()` und `http_find_stored_app_asset_filename()` prüfen nur
`LittleFS.exists()`. Eine 0-Byte-Datei existiert und wird ausgeliefert. Bei `app.js.gz`
heisst das weisser Bildschirm — und die PWA ist danach nicht mehr bedienbar, um es zu
korrigieren. Der passende Helfer `http_fs_file_exists_and_nonempty()` war vorhanden,
aber auf dem Auslieferungspfad nicht benutzt.

Die Invariante steht in `CLAUDE.md`: „Existenzprüfungen für Assets müssen zusätzlich
Dateigrösse > 0 prüfen. Eine leere `.gz` führt zu White-Screen/Crash — das ist real
passiert."

## Akzeptanzkriterien

- [ ] **AK1** — `.value` wird beim Anlegen jedes Parameternamens auf NULL gesetzt.
- [ ] **AK2** — `normalize_http_parameters()` kehrt bei NULL sofort zurück.
- [ ] **AK3, am Gerät** — `GET /?a` wird beantwortet, der ESP bleibt erreichbar.
      Mehrfach, auch direkt nach einem Neustart.
- [ ] **AK4** — Beide Fundstellen der Asset-Suche und die Dateiauslieferung nutzen
      `http_fs_file_exists_and_nonempty()`.
- [ ] **AK5, am Gerät** — Die PWA lädt unverändert; keine Regression bei der
      Auslieferung der `.gz`-Assets.

## Nicht in diesem Schritt

- **Destruktive Endpunkte auf POST umstellen** (Massnahme 14). Das berührt
  möglicherweise PWA und Legacy-Oberfläche gleichzeitig und braucht eine eigene Spec.
- Die Längenbegrenzung des Query-Strings (255 Zeichen, `http.cpp:10548`).
