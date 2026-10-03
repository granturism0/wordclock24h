CMAKE ?= /Applications/CMake.app/Contents/bin/cmake
ARDUINO_CLI ?= /Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli
BUILD_DIR ?= build/stm-rgbw-12h
ARM_TOOLCHAIN_ROOT ?=
CONFIGURE_ARGS = -S . -B $(BUILD_DIR) -G "Unix Makefiles" -DCMAKE_MAKE_PROGRAM=/usr/bin/make -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$(CURDIR)/cmake/toolchain-arm-none-eabi.cmake
ifneq ($(strip $(ARM_TOOLCHAIN_ROOT)),)
CONFIGURE_ARGS += -DARM_NONE_EABI_ROOT=$(ARM_TOOLCHAIN_ROOT)
endif
ESP_BUILD_DIR ?= build/esp8266
ESP_SKETCH_DIR ?= ESP8266/ESP-uclock
ESP_FQBN ?= esp8266:esp8266:generic:baud=115200,xtal=80,CrystalFreq=26,FlashFreq=40,FlashMode=dout,eesz=4M1M,ip=lm2f,vt=flash,exception=disabled,stacksmash=disabled,wipe=none,ssl=all,mmu=3232,non32xfer=fast,sdk=nonosdk_190703,led=2,dbg=Disabled,lvl=None____,ResetMethod=nodemcu
ESP_OUTPUT_BASENAME ?= ESP-WordClock-4M
# Warnstufe des ESP-Builds. Vorgabe "none", damit der normale Build leise bleibt --
# so hat arduino-cli es ohne diesen Schalter ohnehin getan. Der Compile-Smoke in
# tools/guardrails.sh setzt ESP_WARNINGS=all und ist damit erst wirksam: Bis zum
# 03.10.2026 lief er mit der Vorgabe und konnte deshalb GAR KEINE Warnung melden.
# Er meldete "uebersetzt" und prueffte nichts. Aufgefallen beim Bauen von F1.
ESP_WARNINGS ?= none
APP_VERSION_SOURCE ?= ESP8266/ESP-uclock/data/app/app.js
APP_VERSION_FILE ?= $(ESP_BUILD_DIR)/app-version.txt
APP_DIR ?= ESP8266/ESP-uclock/data/app
# Die Sprachdateien stehen als Platzhalter, nicht einzeln: Seit der Auslagerung (B15)
# ist jede zusaetzliche Sprache eine eigene i18n/<code>.json. Eine feste Liste muesste
# bei jeder neuen Sprache angefasst werden -- und wer sie vergisst, merkt es erst am
# Geraet, weil die Datei dann ungepackt bleibt und der ESP nur .gz ausliefert.
GZIP_SOURCES ?= $(APP_DIR)/app.js $(APP_DIR)/styles.css $(APP_DIR)/index.html $(APP_DIR)/sw.js $(APP_DIR)/manifest.webmanifest $(APP_DIR)/layout-previews.json $(wildcard $(APP_DIR)/i18n/*.json) $(APP_DIR)/icons/icon-192.svg $(APP_DIR)/icons/icon-512.svg $(APP_DIR)/icons/icon-192.png $(APP_DIR)/icons/icon-512.png $(APP_DIR)/icons/icon-180.png $(APP_DIR)/icons/icon-mask.png
RELEASE_DIR ?= build/releases
# Sekundengenau statt minutengenau: Das Release-Ziel macht ein "rm -f" auf diesen
# Namen. Zwei Builds in derselben Minute haben sich vorher KOMMENTARLOS
# ueberschrieben, und beide meldeten Erfolg (BEFUNDE.md, L2).
#
# Der Zeitstempel steht in einer EIGENEN Variablen mit ":=", und das ist hier
# Pflicht, nicht Geschmack: "?=" erzeugt eine rekursiv expandierte Variable, deren
# $(shell date) bei JEDER Verwendung neu laeuft. RELEASE_ZIP wird vier Mal benutzt
# (rm, zip, zip -@, echo) -- mit Minutengenauigkeit fiel das kaum auf, mit Sekunden
# haette das Archiv regelmaessig unter einem anderen Namen gestanden als dem, der
# geloescht und gemeldet wird. ":=" expandiert genau einmal beim Einlesen.
RELEASE_STAMP := $(shell date +"%Y-%m-%d-%H%M%S")
RELEASE_ZIP ?= $(RELEASE_DIR)/wordclock-release-$(RELEASE_STAMP).zip
STM_VERSION_FILE ?= $(BUILD_DIR)/wc.txt
ESP_VERSION_FILE ?= $(ESP_BUILD_DIR)/ESP-WordClock.txt

.PHONY: configure f103 f411 all esp release-zip stm-version-file esp-version-file app-version-file app-gz clean clean-stm clean-esp clean-release clean-app-gz say-stm say-esp say-versions

configure:
	$(CMAKE) $(CONFIGURE_ARGS)

f103: configure
	$(CMAKE) --build $(BUILD_DIR) --target wordclock_f103_rgbw -j4
	$(MAKE) stm-version-file
	@$(MAKE) --no-print-directory say-stm

f411: configure
	$(CMAKE) --build $(BUILD_DIR) --target wordclock_f411_rgbw -j4
	$(MAKE) stm-version-file
	@$(MAKE) --no-print-directory say-stm

all: configure
	$(CMAKE) --build $(BUILD_DIR) -j4
	$(MAKE) stm-version-file
	@$(MAKE) --no-print-directory say-stm

esp:
	"$(ARDUINO_CLI)" compile --fqbn '$(ESP_FQBN)' --warnings $(ESP_WARNINGS) --build-path $(ESP_BUILD_DIR) $(ESP_SKETCH_DIR)
	cp $(ESP_BUILD_DIR)/ESP-uclock.ino.bin $(ESP_BUILD_DIR)/$(ESP_OUTPUT_BASENAME).bin
	cp $(ESP_BUILD_DIR)/ESP-uclock.ino.elf $(ESP_BUILD_DIR)/$(ESP_OUTPUT_BASENAME).elf
	cp $(ESP_BUILD_DIR)/ESP-uclock.ino.map $(ESP_BUILD_DIR)/$(ESP_OUTPUT_BASENAME).map
	$(MAKE) esp-version-file
	@$(MAKE) --no-print-directory say-esp

stm-version-file:
	mkdir -p $(BUILD_DIR)
	grep '^#define VERSION' src/main.h | head -n 1 | cut -d'"' -f2 > $(STM_VERSION_FILE)

esp-version-file:
	mkdir -p $(ESP_BUILD_DIR)
	grep '^#define ESP_VERSION' ESP8266/ESP-uclock/version.h | head -n 1 | cut -d'"' -f2 > $(ESP_VERSION_FILE)

app-version-file:
	mkdir -p $(ESP_BUILD_DIR)
	grep '^const APP_VERSION' $(APP_VERSION_SOURCE) | head -n 1 | cut -d'"' -f2 > $(APP_VERSION_FILE)

# arduino-cli meldet im Protokoll die PLATTFORM ("esp8266:esp8266 3.1.2") -- das ist
# der Framework-Stand, nicht unsere Firmware. Beide fangen mit "3." an und stehen zwei
# Zeilen auseinander; das hat schon zur Verwechslung gefuehrt. Deshalb sagt jedes
# Build-Ziel am Ende ausdruecklich, WELCHE unserer Versionen entstanden ist.
say-stm: stm-version-file
	@printf '\n  gebaut: STM-Firmware %s\n\n' "$$(cat $(STM_VERSION_FILE))"

say-esp: esp-version-file
	@printf '\n  gebaut: ESP-Firmware %s   (esp8266-Plattform oben ist der Framework-Stand)\n\n' "$$(cat $(ESP_VERSION_FILE))"

say-versions: stm-version-file esp-version-file app-version-file
	@printf '\n  STM-Firmware: %s\n  ESP-Firmware: %s\n  PWA:          %s\n\n' \
		"$$(cat $(STM_VERSION_FILE))" "$$(cat $(ESP_VERSION_FILE))" "$$(cat $(APP_VERSION_FILE))"

app-gz:
	@for f in $(GZIP_SOURCES); do \
		if [ -f "$$f" ]; then \
			gzip -9 -k -f "$$f" && echo "  gzip $$f -> $$f.gz"; \
		fi; \
	done

clean-app-gz:
	@for f in $(GZIP_SOURCES); do \
		if [ -f "$$f.gz" ]; then \
			rm "$$f.gz" && echo "  removed $$f.gz"; \
		fi; \
	done

release-zip: app-gz app-version-file f103 f411 esp
	mkdir -p $(RELEASE_DIR)
	rm -f $(RELEASE_ZIP)
	zip -j $(RELEASE_ZIP) \
		$(APP_VERSION_FILE) \
		$(BUILD_DIR)/wc12h-stm32f103-sk6812-rgbw.hex \
		$(BUILD_DIR)/wc12h-stm32f411ce-25-sk6812-rgbw.hex \
		$(STM_VERSION_FILE) \
		$(ESP_BUILD_DIR)/$(ESP_OUTPUT_BASENAME).bin \
		$(ESP_VERSION_FILE)
	cd $(ESP_SKETCH_DIR)/data && find app -name "*.gz" | sort | zip $(CURDIR)/$(RELEASE_ZIP) -@
	@echo
	@echo "Release ZIP ready:"
	@echo "  $(RELEASE_ZIP)"
	@$(MAKE) --no-print-directory say-versions

clean:
	rm -rf $(BUILD_DIR) $(ESP_BUILD_DIR) $(RELEASE_DIR)

clean-stm:
	rm -rf $(BUILD_DIR)

clean-esp:
	rm -rf $(ESP_BUILD_DIR)

clean-release:
	rm -rf $(RELEASE_DIR)
