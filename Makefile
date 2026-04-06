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
APP_BUNDLE_SCRIPT ?= ESP8266/ESP-uclock/tools/release-app-bundle.sh
APP_BUNDLE_FILE ?= ESP8266/ESP-uclock/data/app-bundle.txt
RELEASE_DIR ?= build/releases
RELEASE_ZIP ?= $(RELEASE_DIR)/wordclock-release-$(shell date +"%Y-%m-%d-%H%M").zip
STM_VERSION_FILE ?= $(BUILD_DIR)/wc.txt
ESP_VERSION_FILE ?= $(ESP_BUILD_DIR)/ESP-WordClock.txt

.PHONY: configure f103 f411 all esp app-bundle release-zip stm-version-file esp-version-file clean clean-stm clean-esp clean-release

configure:
	$(CMAKE) $(CONFIGURE_ARGS)

f103: configure
	$(CMAKE) --build $(BUILD_DIR) --target wordclock_f103_rgbw -j4
	$(MAKE) stm-version-file

f411: configure
	$(CMAKE) --build $(BUILD_DIR) --target wordclock_f411_rgbw -j4
	$(MAKE) stm-version-file

all: configure
	$(CMAKE) --build $(BUILD_DIR) -j4
	$(MAKE) stm-version-file

esp:
	"$(ARDUINO_CLI)" compile --fqbn '$(ESP_FQBN)' --build-path $(ESP_BUILD_DIR) $(ESP_SKETCH_DIR)
	cp $(ESP_BUILD_DIR)/ESP-uclock.ino.bin $(ESP_BUILD_DIR)/$(ESP_OUTPUT_BASENAME).bin
	cp $(ESP_BUILD_DIR)/ESP-uclock.ino.elf $(ESP_BUILD_DIR)/$(ESP_OUTPUT_BASENAME).elf
	cp $(ESP_BUILD_DIR)/ESP-uclock.ino.map $(ESP_BUILD_DIR)/$(ESP_OUTPUT_BASENAME).map
	$(MAKE) esp-version-file

stm-version-file:
	mkdir -p $(BUILD_DIR)
	grep '^#define VERSION' src/main.h | head -n 1 | cut -d'"' -f2 > $(STM_VERSION_FILE)

esp-version-file:
	mkdir -p $(ESP_BUILD_DIR)
	grep '^#define ESP_VERSION' ESP8266/ESP-uclock/version.h | head -n 1 | cut -d'"' -f2 > $(ESP_VERSION_FILE)

app-bundle:
	sh $(APP_BUNDLE_SCRIPT)

release-zip: app-bundle f103 f411 esp
	mkdir -p $(RELEASE_DIR)
	rm -f $(RELEASE_ZIP)
	zip -j $(RELEASE_ZIP) \
		$(APP_BUNDLE_FILE) \
		$(BUILD_DIR)/wc12h-stm32f103-sk6812-rgbw.hex \
		$(BUILD_DIR)/wc12h-stm32f411ce-25-sk6812-rgbw.hex \
		$(STM_VERSION_FILE) \
		$(ESP_BUILD_DIR)/$(ESP_OUTPUT_BASENAME).bin \
		$(ESP_VERSION_FILE)
	@echo
	@echo "Release ZIP ready:"
	@echo "  $(RELEASE_ZIP)"

clean:
	rm -rf $(BUILD_DIR) $(ESP_BUILD_DIR) $(RELEASE_DIR)

clean-stm:
	rm -rf $(BUILD_DIR)

clean-esp:
	rm -rf $(ESP_BUILD_DIR)

clean-release:
	rm -rf $(RELEASE_DIR)
