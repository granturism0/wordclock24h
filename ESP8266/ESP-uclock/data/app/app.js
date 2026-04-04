const APP_VERSION = "1.13.60";
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
  OVERLAY_N_OVERLAYS: 46
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
  DATE_TICKER_FORMAT: 11
};

const HW = {
  STM32_MASK: 0x07,
  WC_MASK: 0x07 << 3,
  LED_MASK: 0x07 << 6,
  OSC_MASK: 0x07 << 9,
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

const LAYOUT_PREVIEW_SOURCES = [
  {
    file: "wc12h-tables-ch1.txt",
    signature: "ff020a0b17000000000082000384000803010006010803020006020803030002030305040003040304040803050005050504060006060605070005070504080004080704090006090803010000534348",
    display: "ESKISCHAFÜFVIERTUBFZÄÄZWÄNZGSIVORABOHAUBIEPMEISZWÖISDRÜVIERIFÜFIQTSÄCHSISIBNIACHTINÜNIELZÄNIERBEUFIZWÖUFIAMUHR"
  },
  {
    file: "wc12h-tables-ch2.txt",
    signature: "ff020a0b17000000000082000384010004010506000803020006020803030002030305040003040304040803050005050605060006060605070005070604080004080704090006090803010000534348",
    display: "ESKESCHAZÄÄFÖIFCVIERTUZWÄNZGSIVORABOHAUBIEGEEISZWÖISDRÜVIERITFÖIFISÄCHSISEBNIACHTIENÜNILZÄNIERBELFIZWÖLFINAUHR"
  },
  {
    file: "wc12h-tables-de.txt",
    signature: "ff020a0b1c00000000008200038300070401000401040702000402040402040702000b030204030603040004040505050004050203050204050506060104060704070003070304070704080104080504",
    display: "ESKISTLFÜNFZEHNZWANZIGDREIVIERTELTGNACHVORJMHALBQZWÖLFPZWEINSIEBENKDREIRHFÜNFELFNEUNVIERWACHTZEHNRSBSECHSFMUHR"
  },
  {
    file: "wc12h-tables-de2.txt",
    signature: "ff020a0b1c00000000008200038300070401000401040702000402040402040702000b030003030704040004040503040704050003050004050704060004060704070005070704080006080605090004",
    display: "ESKISTAFÜNFZEHNZWANZIGDREIVIERTELVORFUNKNACHHALBAELFÜNFEINSXÄMZWEIDREIPMJVIERSECHSNLACHTSIEBENZWÖLFZEHNEUNKUHR"
  },
  {
    file: "wc12h-tables-en1.txt",
    signature: "ff020a0b19000000000082000382000742000922010207020006020604030004030503030902040004040704050003050303050605060004060404060803070005070506080005080506090003090506",
    display: "ITLISASAMPMACQUARTERDCTWENTYFIVEXHALFSTENFTOPASTERUNINEONESIXTHREEFOURFIVETWOEIGHTELEVENSEVENTWELVETENSEOCLOCK"
  },
  {
    file: "wc12h-tables-en2.txt",
    signature: "ff020a0b18000000000082000382010407020106020704000604010003030002030204040704060703080403050005030704040004040403070605060106070005050506080103090401090605010000",
    display: "ITKISGHALFETENYQUARTERDTWENTYFIVETOPASTEFOURFIVETWONINETHREETWELVEBELEVENONESSEVENWEIGHTITENSIXTIESTINEOICLOCK"
  },
  {
    file: "wc12h-tables-es.txt",
    signature: "ff020a0b1900000000008200018300058200058300080301000301040402000602060503000403050504000404040505020405070406000406050106060507010607070408000b080605090005090506",
    display: "ESONELASUNADOSITRESOAMCUATROCINCOSEISASIETENOCHONUEVEPMLADIEZSONCEDOCELYMENOSOVEINTEDIEZVEINTICINCOMEDIACUARTO"
  },
  {
    file: "wc12h-tables-fr.txt",
    signature: "ff020a0b1c00000000008200038300070401000601060502000402040302070403000403040303070404000404020304050605000405050505050606000506060206080307000207030508000508000a",
    display: "ILNESTODEUXQUATRETROISNEUFUNESEPTHUITSIXCINQMIDIXMINUITONZERHEURESMOINSOLEDIXETRQUARTPMDVINGT-CINQUETSDEMIEPAM"
  },
  {
    file: "wc12h-tables-it.txt",
    signature: "ff020a0b1b00000000008400058201008101028101030301070302000302030402070403000503050604000604060505000705080306000606070407000107020207050608000508000b080506090005",
    display: "SONORLEBOREERLUNASDUEZTREOTTONOVEDIECIUNDICIDODICISETTEQUATTROCSEICINQUEAMENOEKUNLQUARTOVENTICINQUEDIECILMEZZA"
  },
  {
    file: "wc12h-tables-se.txt",
    signature: "ff020a0b19000000000087000882010003010401010603010a01020005020601030005030601040004040704050003050803060003060704070003070803080003080304080803090003090304090704",
    display: "KLOCKANTÄRKFEMYISTIONIKVARTQIENZOTJUGOLIVIPMÖVERKAMHALVETTUSVLXTVATREMYKYFYRAFEMSFLORSEXSJUÄTTAINIOTIOELVATOLV"
  },
  {
    file: "wc24h-tables-de.txt",
    signature: "ff0210124c000000000082000383000707000e03000e04000f02010004010203010204010205010704010705010b04010b06020003020204020604020a04020e04030004030404030905030e04040003",
    display: "ES#IST#VIERTELEINSDREINERSECHSIEBEN#ELFÜNFNEUNVIERACHTNULLZWEI#ZWÖLFZEHNUND#ZWANZIGVIERZIGDREISSIGFÜNFZIGUHRMINUTEN#VORUNDNACHEINDREIVIERTELHALBSIEBENEUNULLZWEINEFÜNFSECHSNACHTVIERDREINSUND#ELF#ZEHNZWANZIGGRADREISSIGVIERZIGZWÖLFÜNFZIGMINUTENUHR#FRÜHVORABENDSMITTERNACHTSMORGENSWARMMITTAGS"
  },
  {
    file: "wc24h-tables-de2.txt",
    signature: "ff020f0f2d000000000082000383000704000c03010004010504010a04020104020804020805030004030006030904040004040604040c03050105050903060007060803060c03070003070004070504",
    display: "ESBISTMNULLWEINZWEILDREIJVIERKNFÜNFQHGSECHSVWSIEBENWCFACHTGKNEUNTDZEHNRPELFVZWÖLFMDFUNDKGSZWANZIGTUHRNUNDEINEYZWEIXDREIPVIERDFÜNFWSECHSGRSIEBENHACHTLOBNEUNAZEHNDELFSZHTZWÖLFKMUNDBEZWANZIGVDREIßIGIVIERZIGCFNULLFFÜNFZIGTMINUTEN"
  },
  {
    file: "wc24h-tables-pl.txt",
    signature: "ff02121229000000030a08030505000507020607030005000c06020006020d04010009010909040809040008070004080e040a00051004030908040f0c061000040d0c051100060f00050a0a08090008",
    display: "JEST*TRZECIASZOSTADZIEWIATADZIESIATASIODMACZWARTAOSMA*PIATADRUGAPIERWSZADWUNASTAJEDENASTA**PIECDZIESIAT********DWADZIESCIA***ZEROCZTERDZIESCI***TRZYDZIESCI**ZERODZIESIECTRZYNASCIEJEDENASCIEDZIEWIEC*****SZESNASCIE******DZIEWIETNASCIE*CZTERNASCIE*SZESC******PIETNASCIE**OSIEMNASCIE*CZTERYPIECDWANASCIE*****SIEDEMNASCIE*PM*AM"
  },
  {
    file: "wc24h-tables-pl2.txt",
    signature: "ff02141431000000000084080008030005030d07020107070f05030606010006011004090009010609060009040b0804000908090a050b09060a0900050b05000a07010d02090a0010040b00040d1004",
    display: "JEST*SIEDEMNASTAZEROSIODMADZIESIATA*OSMA*CZWARTA*DWUDZIESTA*DRUGA*SZOSTA*TRZECIATRZYNASTA**DWUNASTA*OSIEMNASTA*PIETNASTAJEDENASTA*SZESNASTA**DZIEWIETNASTA*PIATAPIERWSZA*CZTERNASTA*DZIEWIATA*SZESNASCIETRZYDZIESCI*DZIESIECZERO***DWADZIESCIA**PIETNASCIEJEDENASCIE***CZTERDZIESCI*ZERO*PIECDZIESIAT**JEDENOSIEMNASCIEPIECSZESC*CZTERY***TRZYNASCIESIEDEMNASCIEDZIEWIECCZTERNASCIEDWANASCIE***DZIEWIETNASCIE*C*"
  }
];

const OVERLAY_TYPE_NAMES = [
  "Keins",
  "Icon",
  "Datum",
  "Temperatur",
  "Wetter-Icon",
  "Wetter-Ticker",
  "Ticker",
  "DFPlayer",
  "Wettervorhersage-Icon",
  "Wettervorhersage-Ticker",
  "Temperatur als Ziffern"
];

const OVERLAY_DATE_CODE_NAMES = [
  "----",
  "Karneval",
  "Ostersonntag",
  "1. Advent",
  "2. Advent",
  "3. Advent",
  "4. Advent"
];

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
const layoutPreviewCache = {};
let pendingProgressAction = "";
let pendingProgressButtonId = "";
let stm32ProgressTimer = 0;
let stm32ProgressStage = 0;
let stm32ProgressAdvanceTimer = 0;
let stm32ProgressMonitorTimer = 0;
let stm32AutoResetStarted = false;
let hasUnsavedEdits = false;

const DEBUG_STORAGE_KEY = "wordclock-app-debug-overrides";
const MODULE_STORAGE_KEY = "wordclock-app-active-module";
const AMBILIGHT_STORAGE_KEY = "wordclock-app-ambilight-online";

document.getElementById("reload-button").addEventListener("click", manualReloadApp);
document.getElementById("display-toggle-button").addEventListener("click", toggleDisplayPower);
document.getElementById("ambilight-toggle-button").addEventListener("click", toggleAmbilightPower);
document.getElementById("brightness-slider").addEventListener("input", syncBrightnessLabel);
document.getElementById("brightness-save-button").addEventListener("click", saveBrightness);
document.getElementById("auto-brightness-button").addEventListener("click", toggleAutoBrightness);
document.getElementById("display-mode-save-button").addEventListener("click", saveDisplayMode);
document.getElementById("display-it-is-button").addEventListener("click", togglePermanentItIs);
document.getElementById("ticker-save-button").addEventListener("click", saveTickerText);
document.getElementById("date-format-save-button").addEventListener("click", saveDateTickerFormat);
document.getElementById("ticker-deceleration-save-button").addEventListener("click", saveTickerDeceleration);
document.getElementById("test-display-button").addEventListener("click", testDisplay);
document.getElementById("weather-appid-save-button").addEventListener("click", saveWeatherAppId);
document.getElementById("weather-city-save-button").addEventListener("click", saveWeatherCity);
document.getElementById("weather-coordinates-save-button").addEventListener("click", saveWeatherCoordinates);
document.getElementById("weather-map-button").addEventListener("click", openWeatherMapPicker);
document.getElementById("weather-now-button").addEventListener("click", getWeatherNow);
document.getElementById("weather-forecast-button").addEventListener("click", getWeatherForecast);
document.getElementById("weather-map-close-button").addEventListener("click", closeWeatherMapPicker);
document.getElementById("weather-map-search-button").addEventListener("click", searchWeatherLocation);
document.getElementById("weather-current-location-button").addEventListener("click", useCurrentWeatherLocation);
document.getElementById("weather-map-apply-button").addEventListener("click", applyWeatherMapSelection);
document.getElementById("network-scan-button").addEventListener("click", loadData);
document.getElementById("network-client-save-button").addEventListener("click", saveNetworkClient);
document.getElementById("network-ap-save-button").addEventListener("click", saveNetworkAp);
document.getElementById("network-timeserver-save-button").addEventListener("click", saveTimeServer);
document.getElementById("network-timezone-save-button").addEventListener("click", saveTimezone);
document.getElementById("network-summertime-button").addEventListener("click", toggleSummertime);
document.getElementById("network-nettime-button").addEventListener("click", getNetTime);
document.getElementById("network-wps-button").addEventListener("click", runWps);
document.getElementById("update-host-save-button").addEventListener("click", saveUpdateHost);
document.getElementById("update-path-save-button").addEventListener("click", saveUpdatePath);
document.getElementById("update-assets-button").addEventListener("click", downloadUpdateAssets);
document.getElementById("update-app-bundle-button").addEventListener("click", downloadUpdateAppBundle);
document.getElementById("update-esp-button").addEventListener("click", triggerEspUpdate);
document.getElementById("update-stm32-button").addEventListener("click", triggerStm32Update);
document.getElementById("update-table-button").addEventListener("click", triggerTableUpdate);
document.getElementById("maintenance-reset-stm32-button").addEventListener("click", resetStm32);
document.getElementById("maintenance-reset-eeprom-button").addEventListener("click", resetEeprom);
document.getElementById("files-format-fs-button").addEventListener("click", formatLittleFsFromFiles);
document.getElementById("local-update-reset-button").addEventListener("click", resetStm32);
document.getElementById("local-update-esp-form").addEventListener("submit", uploadLocalEspUpdate);
document.getElementById("local-update-stm32-form").addEventListener("submit", uploadLocalStm32Update);
document.getElementById("temperature-rtc-correction-save-button").addEventListener("click", saveRtcTemperatureCorrection);
document.getElementById("temperature-ds18xx-correction-save-button").addEventListener("click", saveDs18xxTemperatureCorrection);
document.getElementById("temperature-display-button").addEventListener("click", displayTemperatureNow);
document.getElementById("ldr-min-button").addEventListener("click", setLdrMinValue);
document.getElementById("ldr-max-button").addEventListener("click", setLdrMaxValue);
document.getElementById("animation-mode-save-button").addEventListener("click", saveAnimationMode);
document.getElementById("color-animation-mode-save-button").addEventListener("click", saveColorAnimationMode);
document.getElementById("tft-save-button").addEventListener("click", saveTftFlags);
document.getElementById("display-dim-save-button").addEventListener("click", () => saveDimCurve("disp"));
document.getElementById("display-dim-preset-select").addEventListener("change", () => applyDimPreset("disp"));
document.getElementById("display-dim-preset-apply-button").addEventListener("click", () => applyDimPresetAndSave("disp"));
document.getElementById("ambilight-brightness-slider").addEventListener("input", syncAmbilightBrightnessLabel);
document.getElementById("ambilight-brightness-save-button").addEventListener("click", saveAmbilightBrightness);
document.getElementById("ambilight-mode-save-button").addEventListener("click", saveAmbilightMode);
document.getElementById("ambilight-leds-save-button").addEventListener("click", saveAmbilightLeds);
document.getElementById("ambilight-offset-save-button").addEventListener("click", saveAmbilightOffset);
document.getElementById("ambilight-dim-save-button").addEventListener("click", () => saveDimCurve("ambi"));
document.getElementById("ambilight-dim-preset-select").addEventListener("change", () => applyDimPreset("ambi"));
document.getElementById("ambilight-dim-preset-apply-button").addEventListener("click", () => applyDimPresetAndSave("ambi"));
document.getElementById("display-color-save-button").addEventListener("click", () => saveColor("display"));
document.getElementById("ambilight-color-save-button").addEventListener("click", () => saveColor("ambilight"));
document.getElementById("marker-color-save-button").addEventListener("click", () => saveColor("marker"));
document.getElementById("sync-ambilight-button").addEventListener("click", () => toggleFlagButton("sync-ambilight-button", "/api/sync_ambilight_set"));
document.getElementById("sync-markers-button").addEventListener("click", () => toggleFlagButton("sync-markers-button", "/api/sync_markers_set"));
document.getElementById("fade-clock-seconds-button").addEventListener("click", () => toggleFlagButton("fade-clock-seconds-button", "/api/fade_clock_seconds_set"));
document.getElementById("ambilight-markers-button").addEventListener("click", () => toggleFlagButton("ambilight-markers-button", "/api/ambilight_markers_set"));
document.getElementById("timer-save-all-button").addEventListener("click", () => saveAllTimerRows(false));
document.getElementById("ambilight-timer-save-all-button").addEventListener("click", () => saveAllTimerRows(true));
["display", "ambilight", "marker"].forEach((prefix) => {
  document.getElementById(prefix + "-color-rgb").addEventListener("input", () => updateLiveColorPreview(prefix));
  document.getElementById(prefix + "-color-white").addEventListener("input", () => updateLiveColorPreview(prefix));
});
document.getElementById("dfplayer-volume-slider").addEventListener("input", syncDfplayerVolumeLabel);
document.getElementById("dfplayer-volume-save-button").addEventListener("click", saveDfplayerVolume);
document.getElementById("dfplayer-mode-save-button").addEventListener("click", saveDfplayerMode);
document.getElementById("dfplayer-bell-save-button").addEventListener("click", saveDfplayerBellFlags);
document.getElementById("dfplayer-speak-save-button").addEventListener("click", saveDfplayerSpeakCycle);
document.getElementById("dfplayer-silence-start-save-button").addEventListener("click", saveDfplayerSilenceStart);
document.getElementById("dfplayer-silence-stop-save-button").addEventListener("click", saveDfplayerSilenceStop);
document.getElementById("dfplayer-play-button").addEventListener("click", playDfplayerTrack);
document.getElementById("debug-apply-button").addEventListener("click", applyDebugOverrides);
document.getElementById("debug-reset-button").addEventListener("click", resetDebugOverrides);
document.getElementById("datetime-save-button").addEventListener("click", saveDateTime);
document.getElementById("learn-ir-button").addEventListener("click", learnIrRemote);
document.getElementById("app-version").textContent = "App-Version " + APP_VERSION;
document.getElementById("app-version-card").textContent = APP_VERSION;
document.getElementById("legacy-action-frame").addEventListener("load", handleLegacyFrameLoad);
document.getElementById("update-progress-frame").addEventListener("load", handleProgressFrameLoad);
document.querySelectorAll(".upload-form").forEach((form) => {
  form.addEventListener("submit", handleUploadSubmit);
});
document.querySelectorAll(".module-chip").forEach((button) => {
  button.addEventListener("click", () => setActiveModule(button.getAttribute("data-module-target") || "main"));
});
document.addEventListener("input", handleDirtyFormInteraction, true);
document.addEventListener("change", handleDirtyFormInteraction, true);

if ("serviceWorker" in navigator) {
  window.addEventListener("load", () => {
    navigator.serviceWorker.register("/app/sw.js").catch(() => {});
  });
}

loadDebugOverridesIntoUi();
restoreActiveModule();
loadData();
setInterval(() => loadData({ auto: true }), 15000);
window.addEventListener("resize", scheduleWordclockSizing);

let statusToneResetTimer = 0;
let buttonFeedbackTimers = new WeakMap();

function handleDirtyFormInteraction(event) {
  const target = event.target;
  if (!target || !(target instanceof HTMLElement)) {
    return;
  }
  if (!target.closest(".module-section, #weather-map-modal")) {
    return;
  }
  if (target.matches('button, iframe, [type="hidden"]')) {
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
  const response = await fetch(url, {
    cache: "no-store",
    ...(options || {})
  });

  if (!response.ok) {
    throw new Error("http-" + response.status);
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
  const visibility = {
    ambilight: isAmbilightOnline(settings, debugOverrides),
    dfplayer: isDfplayerOnline(settings, debugOverrides)
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
}

async function loadData(options) {
  const opts = options || {};
  if (opts.auto && hasUnsavedEdits) {
    announceStatus("Automatische Aktualisierung pausiert, bis ungespeicherte Änderungen gespeichert sind", "warn");
    return;
  }
  try {
    const [settingsText, displayPower, ambilightPower, overlayIcons, networkInfo, fsInfo, fsList, updateStatus, updateTableInfo] = await Promise.all([
      fetch("/get_settings", { cache: "no-store" }).then((response) => response.text()),
      fetch("/display_power", { cache: "no-store" }).then((response) => response.text()),
      fetch("/ambilight_power", { cache: "no-store" }).then((response) => response.text()),
      fetch("/api/overlay_icons", { cache: "no-store" })
        .then((response) => response.ok ? response.json() : [])
        .catch(() => []),
      fetch("/api/network_scan", { cache: "no-store" })
        .then((response) => response.ok ? response.json() : { networks: [] })
        .catch(() => ({ networks: [] })),
      fetch("/api/fs_info", { cache: "no-store" })
        .then((response) => response.ok ? response.json() : {})
        .catch(() => ({})),
      fetch("/api/fs_list", { cache: "no-store" })
        .then((response) => response.ok ? response.json() : { files: [] })
        .catch(() => ({ files: [] })),
      fetch("/api/update_status", { cache: "no-store" })
        .then((response) => response.ok ? response.json() : {})
        .catch(() => ({})),
      fetch("/api/update_table_files", { cache: "no-store" })
        .then((response) => response.ok ? response.json() : {})
        .catch(() => ({}))
    ]);

    overlayIconsCache = Array.isArray(overlayIcons) ? overlayIcons : [];
    currentFsFiles = Array.isArray(fsList.files) ? fsList.files : [];
    const settings = parseSettings(settingsText);
    const debugOverrides = getDebugOverrides();
    applyPersistedAmbilightState(settings);
    currentLayoutPreview = await loadWordclockLayoutPreview(updateTableInfo, settings);
    const ambilightOnline = isAmbilightOnline(settings, debugOverrides);
    renderOverview(settings, displayPower.trim(), ambilightPower.trim(), debugOverrides, updateStatus);
    renderWordclock(displayPower.trim() === "on", settings, currentLayoutPreview);
    updateDisplayButton(displayPower.trim());
    updateAmbilightButton(ambilightPower.trim(), ambilightOnline);
    updateAmbilightOnlineButton(ambilightOnline ? "on" : "off");
    updateAmbilightAvailability(ambilightOnline ? "on" : "off");
    updateBrightnessControl(
      settings.numvars[NUM.DISPLAY_BRIGHTNESS] || 0,
      settings.numvars[NUM.DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE] ? "on" : "off"
    );
    updateDisplayModeControl(settings);
    updateDisplayFlagControls(settings);
    updateTextControls(settings);
    updateWeatherControls(settings);
    updateNetworkControls(settings, networkInfo);
    updateMaintenanceControls(settings);
    updateDateTimeControls(settings);
    updateTemperatureControls(settings);
    updateLdrControls(settings);
    updateAnimationControls(settings);
    updateTftVisibility(settings, debugOverrides);
    updateTftControls(settings);
    updateAmbilightBrightnessControl(settings.numvars[NUM.AMBILIGHT_BRIGHTNESS] || 0);
    updateAmbilightModeControl(settings);
    updateAmbilightNumberControls(settings);
    updateColorControls(settings, ambilightOnline, debugOverrides);
    updateFlagControls(settings, ambilightOnline);
    updateDfplayerControls(settings, debugOverrides);
    renderAnimationProfiles(settings);
    renderColorAnimationProfiles(settings);
    renderAmbilightModeProfiles(settings);
    renderDimCurves(settings);
    renderDfplayerAlarmRows(settings);
    renderOverlayRows(settings);
    renderTimerRows(settings, false);
    renderTimerRows(settings, true);
    renderFileSystem(fsInfo, currentFsFiles, settings);
    updateUpdateStatus(updateStatus, updateTableInfo, settings);
    updateLocalUpdateControls(updateStatus);
    hasUnsavedEdits = false;
    announceStatus("Aktualisiert " + new Date().toLocaleTimeString("de-CH"));
  } catch (error) {
    announceStatus("Daten konnten nicht geladen werden", "error");
  }
}

async function toggleDisplayPower() {
  const current = document.getElementById("display-power").textContent.trim() === "an" ? "on" : "off";
  const next = current === "on" ? "off" : "on";
  const button = document.getElementById("display-toggle-button");

  beginButtonFeedback(button, "schaltet...");

  try {
    await apiFetch("/api/display_power_set?value=" + next);
    await loadData();
    finishButtonFeedback(button, button.dataset.restoreText || "Display umschalten", "success", next === "on" ? "eingeschaltet" : "ausgeschaltet", true);
  } catch (error) {
    announceStatus("Display konnte nicht geschaltet werden", "error");
    finishButtonFeedback(button, button.dataset.restoreText || "Display umschalten", "error", "Fehler");
  }
}

async function toggleAmbilightPower() {
  const current = document.getElementById("ambilight-power").textContent.trim() === "an" ? "on" : "off";
  const next = current === "on" ? "off" : "on";
  const button = document.getElementById("ambilight-toggle-button");

  beginButtonFeedback(button, "schaltet...");

  try {
    await apiFetch("/api/ambilight_power_set?value=" + next);
    await loadData();
    finishButtonFeedback(button, button.dataset.restoreText || "Ambilight umschalten", "success", next === "on" ? "eingeschaltet" : "ausgeschaltet", true);
  } catch (error) {
    announceStatus("Ambilight konnte nicht geschaltet werden", "error");
    finishButtonFeedback(button, button.dataset.restoreText || "Ambilight umschalten", "error", "Fehler");
  }
}

async function saveAmbilightOnlineState() {
  const select = document.getElementById("health-ambilight-select");
  const next = select.value === "on" ? "on" : "off";

  select.disabled = true;

  try {
    await apiFetch("/api/ambilight_online_set?value=" + next);
    setPersistedAmbilightState(next);
    await loadData();
    announceStatus("Ambilight-Status gespeichert", "ok");
  } catch (error) {
    select.value = next === "on" ? "off" : "on";
    announceStatus("Ambilight-Status konnte nicht gesetzt werden", "error");
  } finally {
    select.disabled = false;
  }
}

async function saveBrightness() {
  const slider = document.getElementById("brightness-slider");
  const button = document.getElementById("brightness-save-button");
  const value = slider.value;

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/display_brightness_set?value=" + value);
    await loadData();
    finishButtonFeedback(button, "Helligkeit speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Helligkeit konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Helligkeit speichern", "error", "Fehler");
  }
}

async function toggleAutoBrightness() {
  const button = document.getElementById("auto-brightness-button");
  const current = button.dataset.state === "on" ? "on" : "off";
  const next = current === "on" ? "off" : "on";

  beginButtonFeedback(button, "schaltet...");

  try {
    await apiFetch("/api/auto_brightness_set?value=" + next);
    await loadData();
    finishButtonFeedback(button, button.dataset.restoreText || "Automatische Helligkeit", "success", next === "on" ? "aktiviert" : "deaktiviert", true);
  } catch (error) {
    announceStatus("Automatische Helligkeit konnte nicht geschaltet werden", "error");
    finishButtonFeedback(button, button.dataset.restoreText || "Automatische Helligkeit", "error", "Fehler");
  }
}

async function togglePermanentItIs() {
  const button = document.getElementById("display-it-is-button");
  const next = button.dataset.state === "on" ? "off" : "on";

  beginButtonFeedback(button, "schaltet...");

  try {
    await apiFetch("/api/display_it_is_set?value=" + next);
    await loadData();
    finishButtonFeedback(button, button.dataset.restoreText || "„ES IST“ dauerhaft anzeigen", "success", next === "on" ? "aktiviert" : "deaktiviert", true);
  } catch (error) {
    announceStatus("„ES IST“ konnte nicht gesetzt werden", "error");
    finishButtonFeedback(button, button.dataset.restoreText || "„ES IST“ dauerhaft anzeigen", "error", "Fehler");
  }
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
  const hw = decodeHardware(settings.numvars[NUM.HARDWARE_CONFIGURATION] || 0);
  const ledCapabilities = getLedCapabilities(settings.numvars[NUM.HARDWARE_CONFIGURATION] || 0, debugOverrides);
  const ambilightOnline = isAmbilightOnline(settings, debugOverrides);
  const dfplayerOnline = isDfplayerOnline(settings, debugOverrides);
  const displayMode = getDisplayModeName(settings.numvars[NUM.DISPLAY_MODE]);
  const brightness = settings.numvars[NUM.DISPLAY_BRIGHTNESS] || 0;
  const automatic = settings.numvars[NUM.DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE] ? "an" : "aus";

  setText("display-power", displayPower === "on" ? "an" : "aus");
  setText("ambilight-power", ambilightOnline ? (ambilightPower === "on" ? "an" : "aus") : "offline");
  setText("firmware-version", settings.strvars[STR.VERSION] || "-");
  setText("esp-version", (updateStatus && updateStatus.esp_version) || settings.strvars[STR.ESP8266_VERSION] || "-");

  renderList("system-list", [
    ["Board", hw.board],
    ["Processor", hw.processor],
    ["Oscillator", hw.oscillator],
    ["Frequency", hw.frequency],
    ["Hardware", hw.hardware],
    ["Display", hw.display]
  ]);

  renderHealthList(settings, ambilightOnline, dfplayerOnline);

  const configItems = [
    ["Display-Modus", displayMode],
    ["Helligkeit", String(brightness)],
    ["Automatische Helligkeit", automatic],
    ["LED-Fähigkeiten", ledCapabilities.label],
    ["Zeitserver", settings.strvars[STR.TIMESERVER] || "-"],
    ["Wetter-Ort", settings.strvars[STR.WEATHER_CITY] || "-"],
    ["Ticker", settings.strvars[STR.TICKER_TEXT] || "-"],
    ["Datumsformat", settings.strvars[STR.DATE_TICKER_FORMAT] || "-"],
    ["Ticker-Verzögerung", String(settings.numvars[NUM.TICKER_DECELERATION] || 0)]
  ];

  if (ambilightOnline) {
    configItems.splice(4, 0,
      ["Ambilight-Modus", getAmbilightModeName(settings)],
      ["Ambilight-Helligkeit", String(settings.numvars[NUM.AMBILIGHT_BRIGHTNESS] || 0)],
      ["Ambilight LEDs", String(settings.numvars[NUM.AMBILIGHT_LEDS] || 0)],
      ["Ambilight Offset", String(settings.numvars[NUM.AMBILIGHT_OFFSET] || 0)]
    );
  }

  if (dfplayerOnline) {
    configItems.push(
      ["DFPlayer-Modus", getDfplayerModeName(settings.numvars[NUM.DFPLAYER_MODE] || 0)],
      ["DFPlayer-Lautstärke", String(settings.numvars[NUM.DFPLAYER_VOLUME] || 0)],
      ["Sprechintervall", String(settings.numvars[NUM.DFPLAYER_SPEAK_CYCLE] || 0)]
    );
  }

  renderList("config-list", configItems);
  updateModuleAvailability(settings, debugOverrides);
}

function updateDisplayButton(displayPower) {
  const button = document.getElementById("display-toggle-button");
  button.textContent = displayPower === "on" ? "Display ausschalten" : "Display einschalten";
}

function updateAmbilightButton(ambilightPower, ambilightOnline) {
  const button = document.getElementById("ambilight-toggle-button");
  button.classList.toggle("is-hidden", !ambilightOnline);
  button.textContent = ambilightPower === "on" ? "Ambilight ausschalten" : "Ambilight einschalten";
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
  const flags = settings.numvars[NUM.DISPLAY_FLAGS] || 0;
  const active = !!(flags & 0x01);
  setActionToggleButton("display-it-is-button", "„ES IST“ deaktivieren", "„ES IST“ dauerhaft anzeigen", active);
}

function updateBrightnessControl(value, autoState) {
  const slider = document.getElementById("brightness-slider");
  const saveButton = document.getElementById("brightness-save-button");
  const autoButton = document.getElementById("auto-brightness-button");
  slider.value = value;
  slider.disabled = autoState === "on";
  saveButton.disabled = autoState === "on";
  setActionToggleButton("auto-brightness-button", "Automatische Helligkeit deaktivieren", "Automatische Helligkeit aktivieren", autoState === "on");
  syncBrightnessLabel();
}

function updateDisplayModeControl(settings) {
  const select = document.getElementById("display-mode-select");
  const currentMode = settings.numvars[NUM.DISPLAY_MODE] || 0;
  const options = settings.dispmodes.length ? settings.dispmodes : [
    { idx: 0, name: "Normal" },
    { idx: 1, name: "Sekunden" },
    { idx: 2, name: "Datum" },
    { idx: 3, name: "Temperatur" },
    { idx: 4, name: "Ticker" }
  ];

  select.innerHTML = options.map((mode) => (
    '<option value="' + mode.idx + '">' + escapeHtml(localizeDisplayModeName(mode.name || String(mode.idx))) + "</option>"
  )).join("");
  select.value = String(currentMode);
}

function updateTextControls(settings) {
  document.getElementById("ticker-text-input").value = settings.strvars[STR.TICKER_TEXT] || "";
  document.getElementById("date-format-input").value = settings.strvars[STR.DATE_TICKER_FORMAT] || "";
  document.getElementById("ticker-deceleration-input").value = String(settings.numvars[NUM.TICKER_DECELERATION] || 0);
}

function updateWeatherControls(settings) {
  document.getElementById("weather-appid-input").value = settings.strvars[STR.WEATHER_APPID] || "";
  document.getElementById("weather-city-input").value = settings.strvars[STR.WEATHER_CITY] || "";
  document.getElementById("weather-lon-input").value = settings.strvars[STR.WEATHER_LON] || "";
  document.getElementById("weather-lat-input").value = settings.strvars[STR.WEATHER_LAT] || "";

  const parts = [];
  if (settings.strvars[STR.WEATHER_CITY]) {
    parts.push(settings.strvars[STR.WEATHER_CITY]);
  }
  if (settings.strvars[STR.WEATHER_LON] || settings.strvars[STR.WEATHER_LAT]) {
    parts.push((settings.strvars[STR.WEATHER_LON] || "-") + " / " + (settings.strvars[STR.WEATHER_LAT] || "-"));
  }
  document.getElementById("weather-location-preview").textContent = parts.length
    ? "Aktuell: " + parts.join(" | ")
    : "Karte und Suche stehen für die Standortwahl bereit.";
}

function updateNetworkControls(settings, networkInfo) {
  const select = document.getElementById("network-ssid-select");
  const networks = Array.isArray(networkInfo.networks) ? networkInfo.networks : [];
  const currentSsid = networkInfo.ssid || "";
  const timezone = decodeTimezone(settings.numvars[NUM.TIMEZONE] || 0);
  const summertimeButton = document.getElementById("network-summertime-button");

  select.innerHTML = networks.length
    ? networks.map((ssid) => '<option value="' + escapeHtml(ssid) + '"' + (ssid === currentSsid ? " selected" : "") + ">" + escapeHtml(ssid) + "</option>").join("")
    : '<option value="">Keine WLANs gefunden</option>';

  document.getElementById("network-timeserver-input").value = settings.strvars[STR.TIMESERVER] || "";
  document.getElementById("network-timezone-input").value = String(timezone.offset);
  setActionToggleButton("network-summertime-button", "Sommerzeit-Berücksichtigung deaktivieren", "Sommerzeit berücksichtigen", timezone.summertime);

  document.getElementById("network-status-note").textContent =
    "SSID: " + (currentSsid || "-") +
    " | IP: " + (networkInfo.ip || "-") +
    " | Modus: " + (networkInfo.mode || "-");
}

function updateMaintenanceControls(settings) {
  document.getElementById("update-host-input").value = settings.strvars[STR.UPDATE_HOST] || "";
  document.getElementById("update-path-input").value = settings.strvars[STR.UPDATE_PATH] || "";
}

function updateDateTimeControls(settings) {
  const current = settings.tmvars[0] || {};
  document.getElementById("datetime-year-input").value = current.year ? String(current.year) : "";
  document.getElementById("datetime-month-input").value = current.month ? String(current.month) : "";
  document.getElementById("datetime-day-input").value = current.day ? String(current.day) : "";
  document.getElementById("datetime-hour-input").value = current.hour !== undefined ? String(current.hour) : "";
  document.getElementById("datetime-minute-input").value = current.minute !== undefined ? String(current.minute) : "";
  document.getElementById("datetime-preview").textContent = formatDateTimePreview(current);
}

function updateTemperatureControls(settings) {
  renderList("temperature-list", [
    ["DS18xx", formatHalfDegreeValue(settings.numvars[NUM.DS18XX_IS_UP] ? settings.numvars[NUM.DS18XX_TEMP_INDEX] : null)],
    ["DS18xx online", onOff(settings.numvars[NUM.DS18XX_IS_UP])],
    ["RTC", formatHalfDegreeValue(settings.numvars[NUM.RTC_IS_UP] ? settings.numvars[NUM.RTC_TEMP_INDEX] : null)],
    ["RTC online", onOff(settings.numvars[NUM.RTC_IS_UP])]
  ]);

  document.getElementById("temperature-ds18xx-correction-input").value = String(settings.numvars[NUM.DS18XX_TEMP_CORRECTION] || 0);
  document.getElementById("temperature-rtc-correction-input").value = String(settings.numvars[NUM.RTC_TEMP_CORRECTION] || 0);
}

function updateLdrControls(settings) {
  const autoBrightness = !!settings.numvars[NUM.DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE];

  renderList("ldr-list", [
    ["Automatische Helligkeit", autoBrightness ? "ein" : "aus"],
    ["Aktueller LDR-Wert", String(settings.numvars[NUM.LDR_RAW_VALUE] || 0)],
    ["Minimum", String(settings.numvars[NUM.LDR_MIN_VALUE] || 0)],
    ["Maximum", String(settings.numvars[NUM.LDR_MAX_VALUE] || 0)]
  ]);

  document.getElementById("ldr-min-button").disabled = !autoBrightness;
  document.getElementById("ldr-max-button").disabled = !autoBrightness;
}

function updateAnimationControls(settings) {
  const animationSelect = document.getElementById("animation-mode-select");
  const colorAnimationSelect = document.getElementById("color-animation-mode-select");

  animationSelect.innerHTML = (settings.dispanims || []).map((entry) => (
    '<option value="' + entry.idx + '">' + escapeHtml(localizeAnimationName(entry.name || String(entry.idx))) + "</option>"
  )).join("");
  animationSelect.value = String(settings.numvars[NUM.ANIMATION_MODE] || 0);

  colorAnimationSelect.innerHTML = (settings.coloranims || []).map((entry) => (
    '<option value="' + entry.idx + '">' + escapeHtml(localizeAnimationName(entry.name || String(entry.idx))) + "</option>"
  )).join("");
  colorAnimationSelect.value = String(settings.numvars[NUM.COLOR_ANIMATION_MODE] || 0);
}

function updateTftControls(settings) {
  const flags = settings.numvars[NUM.SSD1963_FLAGS] || 0;
  document.getElementById("tft-rgb-checkbox").checked = !!(flags & 0x01);
  document.getElementById("tft-hflip-checkbox").checked = !!(flags & 0x02);
  document.getElementById("tft-vflip-checkbox").checked = !!(flags & 0x04);
}

function updateTftVisibility(settings, debugOverrides) {
  const isVisible = hasTftDisplay(settings.numvars[NUM.HARDWARE_CONFIGURATION] || 0, debugOverrides);
  document.getElementById("tft-panel").classList.toggle("is-hidden", !isVisible);
}

function updateAmbilightBrightnessControl(value) {
  const slider = document.getElementById("ambilight-brightness-slider");
  slider.value = value;
  syncAmbilightBrightnessLabel();
}

function updateAmbilightModeControl(settings) {
  const select = document.getElementById("ambilight-mode-select");
  const currentMode = settings.numvars[NUM.AMBILIGHT_MODE] || 0;
  const options = settings.almodes.length ? settings.almodes : [
    { idx: 0, name: "Uhr" },
    { idx: 1, name: "Regenbogen" }
  ];

  select.innerHTML = options.map((mode) => (
    '<option value="' + mode.idx + '">' + escapeHtml(localizeAmbilightModeName(mode.name || String(mode.idx))) + "</option>"
  )).join("");
  select.value = String(currentMode);
}

function updateAmbilightNumberControls(settings) {
  document.getElementById("ambilight-leds-input").value = String(settings.numvars[NUM.AMBILIGHT_LEDS] || 0);
  document.getElementById("ambilight-offset-input").value = String(settings.numvars[NUM.AMBILIGHT_OFFSET] || 0);
}

function updateColorControls(settings, ambilightOnline, debugOverrides) {
  const capabilities = getLedCapabilities(settings.numvars[NUM.HARDWARE_CONFIGURATION] || 0, debugOverrides);
  const useRgbw = capabilities.whiteChannel && !!settings.numvars[NUM.DISPLAY_USE_RGBW];

  applyColorCapabilities(capabilities, useRgbw, ambilightOnline);
  applyWordclockTheme(settings.dspcolors[0], useRgbw);

  setColorControl("display", settings.dspcolors[0], useRgbw);
  setColorControl("ambilight", settings.dspcolors[1], useRgbw);
  setColorControl("marker", settings.dspcolors[2], useRgbw);
}

function applyColorCapabilities(capabilities, useRgbw, ambilightOnline) {
  const note = document.getElementById("color-capability-note");
  const displayColorNote = document.getElementById("display-color-note");
  const colorAnimationMode = Number(document.getElementById("color-animation-mode-select").value || 0);
  const canEditDisplayColor = capabilities.hasColor && colorAnimationMode === 0;

  note.textContent = capabilities.whiteChannel && !useRgbw
    ? "RGBW-Hardware erkannt, aber der White-Channel ist aktuell firmwareseitig nicht aktiv."
    : capabilities.note;
  if (displayColorNote) {
    displayColorNote.textContent = canEditDisplayColor
      ? "Die Display-Farbe kann direkt gesetzt werden, solange keine Farbanimation aktiv ist."
      : "Die Display-Farbe ist nur direkt wählbar, wenn Farbanimation = Keine ist.";
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
  const isUp = isDfplayerOnline(settings, debugOverrides);
  const note = document.getElementById("dfplayer-note");
  const mode = settings.numvars[NUM.DFPLAYER_MODE] || 0;

  document.getElementById("dfplayer-panel").classList.toggle("is-hidden", !isUp);
  note.textContent = isUp ? "DFPlayer ist online." : "DFPlayer ist offline und wird ausgeblendet.";

  if (!isUp) {
    return;
  }

  document.getElementById("dfplayer-volume-slider").value = settings.numvars[NUM.DFPLAYER_VOLUME] || 0;
  syncDfplayerVolumeLabel();
  document.getElementById("dfplayer-mode-select").value = String(mode);
  document.getElementById("dfplayer-bell-15").checked = !!(settings.numvars[NUM.DFPLAYER_BELL_FLAGS] & 0x01);
  document.getElementById("dfplayer-bell-30").checked = !!(settings.numvars[NUM.DFPLAYER_BELL_FLAGS] & 0x02);
  document.getElementById("dfplayer-bell-45").checked = !!(settings.numvars[NUM.DFPLAYER_BELL_FLAGS] & 0x04);
  document.getElementById("dfplayer-speak-cycle-input").value = String(settings.numvars[NUM.DFPLAYER_SPEAK_CYCLE] || 0);
  document.getElementById("dfplayer-silence-start-input").value = minutesToTimeValue(settings.numvars[NUM.DFPLAYER_SILENCE_START] || 0);
  document.getElementById("dfplayer-silence-stop-input").value = minutesToTimeValue(settings.numvars[NUM.DFPLAYER_SILENCE_STOP] || 0);
  document.getElementById("dfplayer-bell-section").classList.toggle("is-hidden", mode !== 1);
  document.getElementById("dfplayer-speak-section").classList.toggle("is-hidden", mode !== 2);
}

function renderDfplayerAlarmRows(settings) {
  const root = document.getElementById("dfplayer-alarm-list");
  const alarms = (settings.alarmtimes || []).slice().sort((a, b) => a.idx - b.idx);

  root.innerHTML = alarms.map((alarm) => {
    const time = minutesToTimeValue(alarm.minutes || 0);
    const fromDay = (alarm.flags & 0x38) >> 3;
    const toDay = alarm.flags & 0x07;
    const active = (alarm.flags & 0x80) ? "checked" : "";
    const idx = alarm.idx;

    return (
      '<section class="alarm-card">' +
        '<div class="card-headline"><div><span class="label">Titel ' + escapeHtml(String(idx + 1).padStart(3, "0")) + '</span><p class="card-subline">Zeitplan</p></div></div>' +
        '<div class="chip-row">' +
          '<label class="chip-toggle"><input type="checkbox" id="df-alarm-active-' + idx + '" ' + active + '> Aktiv</label>' +
        '</div>' +
        '<div class="form-section">' +
          '<p class="section-label">Zeitraum</p>' +
          '<div class="timer-fields-grid">' +
            '<label class="field"><span class="label">Von</span><select id="df-alarm-from-' + idx + '">' + buildWeekdayOptions(fromDay) + "</select></label>" +
            '<label class="field"><span class="label">Bis</span><select id="df-alarm-to-' + idx + '">' + buildWeekdayOptions(toDay) + "</select></label>" +
            '<label class="field"><span class="label">Zeit</span><input id="df-alarm-time-' + idx + '" type="time" value="' + escapeHtml(time) + '"></label>' +
          '</div>' +
        '</div>' +
        '<div class="profile-actions">' +
          '<button class="button primary" type="button" data-alarm-save="' + idx + '">Speichern</button>' +
        '</div>' +
      "</section>"
    );
  }).join("");

  root.querySelectorAll("[data-alarm-save]").forEach((button) => {
    button.addEventListener("click", () => saveDfplayerAlarm(Number(button.getAttribute("data-alarm-save"))));
  });
}

function renderAnimationProfiles(settings) {
  const root = document.getElementById("animation-profile-list");
  const items = (settings.dispanims || []).filter((entry) => entry.flags & 0x01).sort((a, b) => a.idx - b.idx);

  root.innerHTML = items.map((item) => (
    '<section class="profile-card">' +
      '<div class="panel-head compact"><div><span class="label">' + escapeHtml(item.name || String(item.idx)) + "</span></div></div>" +
      '<div class="control-stack">' +
        '<div class="slider-row"><label class="label" for="an-dec-' + item.idx + '">Verzögerung</label><strong id="an-dec-value-' + item.idx + '" class="value-pill">' + escapeHtml(String(item.deceleration || 1)) + '</strong></div>' +
        '<input id="an-dec-' + item.idx + '" type="range" min="1" max="15" value="' + escapeHtml(String(item.deceleration || 1)) + '">' +
        '<label class="checkbox-line"><input type="checkbox" id="an-fav-' + item.idx + '"' + ((item.flags & 0x02) ? " checked" : "") + '> Favorit</label>' +
        '<div class="profile-actions">' +
          '<button class="button" type="button" data-an-default="' + item.idx + '">Standard</button>' +
          '<button class="button primary" type="button" data-an-save="' + item.idx + '">Profil speichern</button>' +
        "</div>" +
      "</div>" +
    "</section>"
  )).join("");

  root.querySelectorAll("[data-an-save]").forEach((button) => {
    button.addEventListener("click", () => saveAnimationProfile(Number(button.getAttribute("data-an-save"))));
  });
  root.querySelectorAll("[data-an-default]").forEach((button) => {
    button.addEventListener("click", () => resetAnimationProfileDefault(Number(button.getAttribute("data-an-default"))));
  });
  root.querySelectorAll('input[id^="an-dec-"]').forEach((input) => {
    input.addEventListener("input", () => syncProfileRangeValue("an", Number(input.id.split("-").pop())));
  });
}

function renderColorAnimationProfiles(settings) {
  const root = document.getElementById("color-animation-profile-list");
  const items = (settings.coloranims || []).filter((entry) => entry.flags & 0x01).sort((a, b) => a.idx - b.idx);

  root.innerHTML = items.map((item) => (
    '<section class="profile-card">' +
      '<div class="panel-head compact"><div><span class="label">' + escapeHtml(item.name || String(item.idx)) + "</span></div></div>" +
      '<div class="control-stack">' +
        '<div class="slider-row"><label class="label" for="can-dec-' + item.idx + '">Verzögerung</label><strong id="can-dec-value-' + item.idx + '" class="value-pill">' + escapeHtml(String(item.deceleration || 0)) + '</strong></div>' +
        '<input id="can-dec-' + item.idx + '" type="range" min="0" max="15" value="' + escapeHtml(String(item.deceleration || 0)) + '">' +
        '<div class="profile-actions">' +
          '<button class="button" type="button" data-can-default="' + item.idx + '">Standard</button>' +
          '<button class="button primary" type="button" data-can-save="' + item.idx + '">Profil speichern</button>' +
        "</div>" +
      "</div>" +
    "</section>"
  )).join("");

  root.querySelectorAll("[data-can-save]").forEach((button) => {
    button.addEventListener("click", () => saveColorAnimationProfile(Number(button.getAttribute("data-can-save"))));
  });
  root.querySelectorAll("[data-can-default]").forEach((button) => {
    button.addEventListener("click", () => resetColorAnimationProfileDefault(Number(button.getAttribute("data-can-default"))));
  });
  root.querySelectorAll('input[id^="can-dec-"]').forEach((input) => {
    input.addEventListener("input", () => syncProfileRangeValue("can", Number(input.id.split("-").pop())));
  });
}

function syncProfileRangeValue(prefix, idx) {
  const input = document.getElementById(prefix + "-dec-" + idx);
  const value = document.getElementById(prefix + "-dec-value-" + idx);
  if (input && value) {
    value.textContent = input.value;
  }
}

function renderAmbilightModeProfiles(settings) {
  const root = document.getElementById("ambilight-profile-list");
  const items = (settings.almodes || []).filter((entry) => entry.flags & 0x01).sort((a, b) => a.idx - b.idx);

  root.innerHTML = items.length ? items.map((item) => (
    '<section class="alarm-card">' +
      '<div class="panel-head compact"><div><span class="label">' + escapeHtml(localizeAmbilightModeName(item.name || String(item.idx))) + "</span></div></div>" +
      '<div class="alarm-grid">' +
        '<label class="field"><span class="label">Verzögerung</span><input id="alm-dec-' + item.idx + '" type="range" min="0" max="15" value="' + escapeHtml(String(item.deceleration || 0)) + '"></label>' +
        '<div class="hero-actions">' +
          '<button class="button" type="button" data-alm-default="' + item.idx + '">Standard</button>' +
          '<button class="button primary" type="button" data-alm-save="' + item.idx + '">Profil speichern</button>' +
        "</div>" +
      "</div>" +
    "</section>"
  )).join("") : '<p class="hint">Für die erkannte Hardware gibt es keine konfigurierbaren Ambilight-Modi.</p>';

  root.querySelectorAll("[data-alm-save]").forEach((button) => {
    button.addEventListener("click", () => saveAmbilightModeProfile(Number(button.getAttribute("data-alm-save"))));
  });
  root.querySelectorAll("[data-alm-default]").forEach((button) => {
    button.addEventListener("click", () => resetAmbilightModeProfile(Number(button.getAttribute("data-alm-default"))));
  });
}

function renderFileSystem(fsInfo, files, settings) {
  const infoItems = [];

  if (fsInfo.total !== undefined) {
    infoItems.push(
      ["Gesamt", formatBytes(fsInfo.total)],
      ["Belegt", formatBytes(fsInfo.used)],
      ["Blockgröße", formatBytes(fsInfo.block_size)],
      ["Seitengröße", formatBytes(fsInfo.page_size)],
      ["Max. offene Dateien", String(fsInfo.max_open_files)],
      ["Max. Pfadlänge", String(fsInfo.max_path_length)]
    );
  }

  renderList("fs-info-list", infoItems.length ? infoItems : [["LittleFS", "keine Daten"]]);

  const root = document.getElementById("fs-file-list");
  root.innerHTML = (files || []).length ? files.map((file) => (
    '<section class="file-row">' +
      '<div class="file-row-head">' +
        '<div class="file-name">' + escapeHtml(file.name || "-") + '</div>' +
        '<strong class="file-size">' + escapeHtml(formatBytes(file.size ?? 0)) + '</strong>' +
      '</div>' +
      '<div class="file-actions">' +
          '<button class="button" type="button" data-fs-show="' + escapeHtml(file.name || "") + '">Anzeigen</button>' +
          '<button class="button" type="button" data-fs-delete="' + escapeHtml(file.name || "") + '">Löschen</button>' +
      "</div>" +
    "</section>"
  )).join("") : '<p class="hint">Noch keine Dateien im LittleFS gefunden.</p>';

  root.querySelectorAll("[data-fs-show]").forEach((button) => {
    button.addEventListener("click", () => showFsFile(button.getAttribute("data-fs-show") || ""));
  });
  root.querySelectorAll("[data-fs-delete]").forEach((button) => {
    button.addEventListener("click", () => deleteFsFile(button.getAttribute("data-fs-delete") || ""));
  });

  updateFsUploadTargets(settings);
}

function formatBytes(bytes) {
  const value = Number(bytes || 0);
  if (value >= 1024 * 1024) {
    return (value / (1024 * 1024)).toFixed(value >= 10 * 1024 * 1024 ? 0 : 1) + " MB";
  }
  if (value >= 1024) {
    return (value / 1024).toFixed(value >= 10 * 1024 ? 0 : 1) + " kB";
  }
  return String(value) + " Bytes";
}

function updateFsUploadTargets(settings) {
  const targets = getFsUploadTargets(settings.numvars[NUM.HARDWARE_CONFIGURATION] || 0);

  updateUploadFormVisibility("fs-upload-icon-form", "fs-upload-icon-label", targets.icon);
  updateUploadFormVisibility("fs-upload-weather-form", "fs-upload-weather-label", targets.weather);
  updateUploadFormVisibility("fs-upload-tables-form", "fs-upload-tables-label", targets.tables);
  updateUploadFormVisibility("fs-upload-display-form", "fs-upload-display-label", targets.display);
}

function updateUploadFormVisibility(formId, labelId, fileName) {
  const form = document.getElementById(formId);
  const label = document.getElementById(labelId);
  form.classList.toggle("is-hidden", !fileName);
  if (fileName) {
    label.textContent = fileName;
  }
}

function updateUpdateStatus(updateStatus, updateTableInfo, settings) {
  const flashSize = updateStatus.flash_size || 0;
  const statusItems = [
    ["ESP-Flash", flashSize ? String(flashSize) + " Bytes" : "-"],
    ["OTA-Update", updateStatus.can_update ? "möglich" : "nicht möglich"],
    ["ESP-Version", updateStatus.esp_version || (settings.strvars[STR.ESP8266_VERSION] || "-")],
    ["ESP verfügbar", updateStatus.esp_available || "-"],
    ["WordClock-Version", updateStatus.wc_version || (settings.strvars[STR.VERSION] || "-")],
    ["WordClock verfügbar", updateStatus.wc_available || "-"],
    ["Standard STM32", updateStatus.stm32_default || "-"]
  ];

  renderList("update-status-list", statusItems);

  const select = document.getElementById("update-stm32-select");
  const files = Array.isArray(updateStatus.stm32_files) ? updateStatus.stm32_files : [];
  select.innerHTML = files.length
    ? files.map((file) => '<option value="' + escapeHtml(file) + '"' + (file === updateStatus.stm32_default ? " selected" : "") + ">" + escapeHtml(file) + "</option>").join("")
    : '<option value="">keine STM32-Dateien gefunden</option>';

  const tableField = document.getElementById("update-table-field");
  const tableSelect = document.getElementById("update-table-select");
  const tableButton = document.getElementById("update-table-button");
  const assetsButton = document.getElementById("update-assets-button");
  const appBundleButton = document.getElementById("update-app-bundle-button");
  const serverFilesBlock = document.getElementById("update-server-files-block");
  const tableFiles = Array.isArray(updateTableInfo.table_files) ? updateTableInfo.table_files : [];
  const currentTable = updateTableInfo.current_table || "";
  const tableAvailable = tableFiles.length > 0;
  const assetsAvailable = !! updateStatus.assets_available;
  const appBundleAvailable = !! updateStatus.app_bundle_available;
  const anyServerFilesAvailable = tableAvailable || assetsAvailable || appBundleAvailable;

  tableField.classList.toggle("is-hidden", !tableAvailable);
  tableSelect.innerHTML = tableFiles.length
    ? tableFiles.map((file) => '<option value="' + escapeHtml(file) + '"' + (file === currentTable ? " selected" : "") + ">" + escapeHtml(file) + "</option>").join("")
    : '<option value="">keine Layout-Tabellen gefunden</option>';
  tableButton.classList.toggle("is-hidden", !tableAvailable);
  assetsButton.classList.toggle("is-hidden", !assetsAvailable);
  appBundleButton.classList.toggle("is-hidden", !appBundleAvailable);
  serverFilesBlock.classList.toggle("is-hidden", !anyServerFilesAvailable);

  document.getElementById("update-release-notes").innerHTML = updateStatus.release_notes || "<p>Keine Release Notes vom Server gelesen.</p>";
  document.getElementById("update-esp-button").disabled = !updateStatus.can_update;
  document.getElementById("update-stm32-button").disabled = !files.length;
  tableButton.disabled = !tableAvailable;
  assetsButton.disabled = !assetsAvailable;
  appBundleButton.disabled = !appBundleAvailable;
}

function updateLocalUpdateControls(updateStatus) {
  const supported = updateStatus.local_update_supported !== false;
  const note = document.getElementById("local-update-note");
  note.textContent = supported
    ? "ESP- oder STM32-Datei auswählen und direkt lokal hochladen."
    : "Lokales Update ist bei dieser ESP-Flashgröße nicht verfügbar.";
  document.getElementById("local-update-esp-file-input").disabled = !supported;
  document.getElementById("local-update-esp-submit-button").disabled = !supported;
  document.getElementById("local-update-stm32-file-input").disabled = !supported;
  document.getElementById("local-update-stm32-submit-button").disabled = !supported;
}

function renderDimCurves(settings) {
  populateDimPresetSelect("display-dim-preset-select");
  populateDimPresetSelect("ambilight-dim-preset-select");
  renderDimCurveList("display-dim-list", settings.num8arrays[0] || {}, "disp");
  renderDimCurveList("ambilight-dim-list", settings.num8arrays[1] || {}, "ambi");
  syncDimPresetSelection("disp");
  syncDimPresetSelection("ambi");
}

function renderDimCurveList(rootId, values, prefix) {
  const root = document.getElementById(rootId);
  const rows = [];

  for (let idx = 0; idx <= 15; idx += 1) {
    const value = values[idx] ?? 0;
    rows.push(
      '<section class="dim-card">' +
        '<div class="slider-row">' +
          '<span class="label">Stufe ' + idx + '</span>' +
          '<input id="' + prefix + '-dim-' + idx + '" type="range" min="0" max="15" value="' + escapeHtml(String(value)) + '">' +
          '<strong id="' + prefix + '-dim-value-' + idx + '" class="value-pill">' + escapeHtml(String(value)) + '</strong>' +
        '</div>' +
      '</section>'
    );
  }

  root.innerHTML = rows.join("");
  root.querySelectorAll('input[id^="' + prefix + '-dim-"]').forEach((input) => {
    input.addEventListener("input", () => {
      syncDimCurveValue(prefix, Number(input.id.split("-").pop()));
      syncDimPresetSelection(prefix);
    });
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
  applyDimPreset(prefix);
  await saveDimCurve(prefix);
}

function renderOverlayRows(settings) {
  const root = document.getElementById("overlay-list");
  const count = settings.numvars[NUM.OVERLAY_N_OVERLAYS] || 0;
  const activeOverlays = (settings.overlays || []).filter((overlay) => overlay.idx < count);
  const items = activeOverlays.slice();

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
    const title = overlay.isNew ? "Neues Overlay" : "Overlay " + String(idx);
    const overlayTypeName = OVERLAY_TYPE_NAMES[type] || "Keins";

    return (
      '<section class="overlay-card">' +
        '<div class="card-headline">' +
          '<div><span class="label">' + escapeHtml(title) + '</span><p class="card-subline">' + escapeHtml(overlayTypeName) + '</p></div>' +
          (overlay.isNew ? '<span class="state-pill">Neu</span>' : '') +
        '</div>' +
        '<div class="chip-row">' +
          '<label class="chip-toggle"><input type="checkbox" id="ov-active-' + idx + '"' + ((overlay.flags & 0x01) ? " checked" : "") + '> Aktiv</label>' +
        '</div>' +
        '<div class="overlay-layout">' +
          '<div class="form-section">' +
            '<p class="section-label">Inhalt</p>' +
            '<label class="field"><span class="label">Typ</span><select id="ov-type-' + idx + '">' + buildNamedOptions(OVERLAY_TYPE_NAMES, overlay.type) + "</select></label>" +
            '<label id="ov-icon-wrap-' + idx + '" class="field' + (showIcon ? '' : ' is-hidden') + '"><span class="label">Icon</span><select id="ov-icon-' + idx + '">' + buildIconOptions(overlay.text || "") + '</select></label>' +
            '<label id="ov-value-wrap-' + idx + '" class="field' + (showText ? '' : ' is-hidden') + '"><span class="label">Wert</span><input id="ov-value-' + idx + '" type="text" maxlength="32" value="' + escapeHtml(overlay.text || "") + '"></label>' +
            '<div id="ov-mp3-wrap-' + idx + '" class="time-grid' + (showMp3 ? '' : ' is-hidden') + '">' +
              '<label class="field"><span class="label">Ordner</span><input id="ov-folder-' + idx + '" type="number" min="0" max="99" step="1" value="' + escapeHtml(mp3.folder) + '"></label>' +
              '<label class="field"><span class="label">Track</span><input id="ov-track-' + idx + '" type="number" min="0" max="999" step="1" value="' + escapeHtml(mp3.track) + '"></label>' +
            '</div>' +
          '</div>' +
          '<div class="form-section">' +
            '<p class="section-label">Zeit und Datum</p>' +
            '<div class="overlay-time-grid">' +
              '<label class="field"><span class="label">Intervall (Min.)</span><input id="ov-interval-' + idx + '" type="number" min="1" max="99" step="1" value="' + escapeHtml(String(overlay.interval || 5)) + '"></label>' +
              '<label id="ov-duration-wrap-' + idx + '" class="field' + (showDuration ? '' : ' is-hidden') + '"><span class="label">Dauer (Sek.)</span><input id="ov-duration-' + idx + '" type="number" min="5" max="9" step="1" value="' + escapeHtml(String(overlay.duration || 5)) + '"></label>' +
            '</div>' +
            '<label id="ov-datecode-wrap-' + idx + '" class="field"><span class="label">Datums-Code</span><select id="ov-datecode-' + idx + '">' + buildNamedOptions(OVERLAY_DATE_CODE_NAMES, overlay.date_code) + '</select></label>' +
            '<div class="overlay-date-grid">' +
              '<label id="ov-day-wrap-' + idx + '" class="field' + (showDateStart ? '' : ' is-hidden') + '"><span class="label">Tag</span><select id="ov-day-' + idx + '">' + buildDayOptions(day) + '</select></label>' +
              '<label id="ov-month-wrap-' + idx + '" class="field' + (showDateStart ? '' : ' is-hidden') + '"><span class="label">Monat</span><select id="ov-month-' + idx + '">' + buildMonthOptions(month) + '</select></label>' +
              '<label id="ov-days-wrap-' + idx + '" class="field' + (showDays ? '' : ' is-hidden') + '"><span class="label">Tage</span><input id="ov-days-' + idx + '" type="number" min="1" max="255" step="1" value="' + escapeHtml(String(overlay.days || 1)) + '"></label>' +
            '</div>' +
          '</div>' +
          '<div class="profile-actions overlay-actions">' +
            '<button class="button primary" type="button" data-overlay-save="' + idx + '">' + (overlay.isNew ? "Overlay anlegen" : "Overlay speichern") + "</button>" +
            (overlay.isNew ? "" : '<button class="button" type="button" data-overlay-display="' + idx + '">Anzeigen</button><button class="button" type="button" data-overlay-delete="' + idx + '">Löschen</button>') +
          '</div>' +
        "</div>" +
      "</section>"
    );
  }).join("");

  root.querySelectorAll("[data-overlay-save]").forEach((button) => {
    button.addEventListener("click", () => saveOverlay(Number(button.getAttribute("data-overlay-save"))));
  });

  root.querySelectorAll("[data-overlay-display]").forEach((button) => {
    button.addEventListener("click", () => displayOverlay(Number(button.getAttribute("data-overlay-display"))));
  });

  root.querySelectorAll("[data-overlay-delete]").forEach((button) => {
    button.addEventListener("click", () => deleteOverlay(Number(button.getAttribute("data-overlay-delete"))));
  });

  root.querySelectorAll("[id^='ov-type-']").forEach((select) => {
    select.addEventListener("change", () => updateOverlayRowVisibility(Number(select.id.split("-").pop())));
  });

  root.querySelectorAll("[id^='ov-datecode-']").forEach((select) => {
    select.addEventListener("change", () => updateOverlayRowVisibility(Number(select.id.split("-").pop())));
  });

  root.querySelectorAll("[id^='ov-month-'], [id^='ov-day-']").forEach((input) => {
    input.addEventListener("input", () => updateOverlayRowVisibility(Number(input.id.split("-").pop())));
  });
}

function renderTimerRows(settings, isAmbilight) {
  const root = document.getElementById(isAmbilight ? "ambilight-timer-list" : "timer-list");
  const items = (isAmbilight ? settings.ambinighttimes : settings.nighttimes || []).slice().sort((a, b) => a.idx - b.idx);

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
        '<div class="card-headline"><div><span class="label">Slot ' + escapeHtml(String(idx)) + '</span><p class="card-subline">' + (isAmbilight ? 'Ambilight-Timer' : 'Timer') + '</p></div></div>' +
        '<div class="chip-row">' +
          '<label class="chip-toggle"><input type="checkbox" id="' + prefix + '-active-' + idx + '" ' + active + '> Aktiv</label>' +
          '<label class="field timer-action-field"><span class="label">Aktion</span><select id="' + prefix + '-action-' + idx + '"><option value="on"' + ((item.flags & 0x40) ? ' selected' : '') + '>Einschalten</option><option value="off"' + (!(item.flags & 0x40) ? ' selected' : '') + '>Ausschalten</option></select></label>' +
        '</div>' +
        '<div class="form-section">' +
          '<p class="section-label">Zeitraum</p>' +
          '<div class="timer-fields-grid">' +
            '<label class="field"><span class="label">Von</span><select id="' + prefix + '-from-' + idx + '">' + buildWeekdayOptions(fromDay) + "</select></label>" +
            '<label class="field"><span class="label">Bis</span><select id="' + prefix + '-to-' + idx + '">' + buildWeekdayOptions(toDay) + "</select></label>" +
            '<label class="field"><span class="label">Zeit</span><input id="' + prefix + '-time-' + idx + '" type="time" value="' + escapeHtml(time) + '"></label>' +
          '</div>' +
        '</div>' +
        '<div class="profile-actions">' +
          '<button class="button primary" type="button" data-' + prefix + '-save="' + idx + '">Speichern</button>' +
          '<button class="button" type="button" data-' + prefix + '-clear="' + idx + '">Leeren</button>' +
        '</div>' +
      "</section>"
    );
  }).join("");

  root.querySelectorAll("[data-" + (isAmbilight ? "at" : "t") + "-save]").forEach((button) => {
    button.addEventListener("click", () => saveTimerRow(Number(button.getAttribute("data-" + (isAmbilight ? "at" : "t") + "-save")), isAmbilight));
  });
  root.querySelectorAll("[data-" + (isAmbilight ? "at" : "t") + "-clear]").forEach((button) => {
    button.addEventListener("click", () => clearTimerRow(Number(button.getAttribute("data-" + (isAmbilight ? "at" : "t") + "-clear")), isAmbilight));
  });
}

function setColorControl(prefix, color, useRgbw) {
  const current = color || { red: 0, green: 0, blue: 0, white: 0 };
  const rgbInput = document.getElementById(prefix + "-color-rgb");
  const whiteInput = document.getElementById(prefix + "-color-white");

  rgbInput.value = rgb63ToHex(current);
  whiteInput.value = String(current.white || 0);
  whiteInput.disabled = !useRgbw;
  updateLiveColorPreview(prefix);
}

function updateFlagControls(settings, ambilightOnline) {
  const flags = settings.numvars[NUM.DISPLAY_FLAGS] || 0;
  const ambilightModes = settings.almodes || [];
  const clockMode = ambilightModes.find((entry) => entry.idx === 0);
  const clockModeData = clockMode ? clockMode : null;

  setActionToggleButton("sync-ambilight-button", "Ambilight-Synchronisierung deaktivieren", "Ambilight synchronisieren", ambilightOnline && !!(flags & 0x02));
  setActionToggleButton("sync-markers-button", "Marker-Synchronisierung deaktivieren", "Marker synchronisieren", ambilightOnline && !!(flags & 0x04));
  setActionToggleButton("fade-clock-seconds-button", "Weiches Ausblenden deaktivieren", "Sekunden weich ausblenden", ambilightOnline && !!(flags & 0x08));

  const markersEnabled = !!(((clockModeData && clockModeData.flags) || 0) & 0x02);
  setActionToggleButton("ambilight-markers-button", "5-Sekunden-Marker deaktivieren", "5-Sekunden-Marker aktivieren", ambilightOnline && markersEnabled);
}

function setActionToggleButton(id, onText, offText, enabled) {
  const button = document.getElementById(id);
  button.dataset.state = enabled ? "on" : "off";
  button.textContent = enabled ? onText : offText;
  button.classList.toggle("primary", enabled);
}

function updateLiveColorPreview(prefix) {
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

  if (prefix === "display") {
    applyWordclockTheme({
      red: rgb.red,
      green: rgb.green,
      blue: rgb.blue,
      white
    }, !whiteInput.disabled);
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

async function saveDisplayMode() {
  const button = document.getElementById("display-mode-save-button");
  const value = document.getElementById("display-mode-select").value;

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/display_mode_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Display-Modus speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Display-Modus konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Display-Modus speichern", "error", "Fehler");
  }
}

async function saveTickerText() {
  const button = document.getElementById("ticker-save-button");
  const value = document.getElementById("ticker-text-input").value;

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/ticker_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Ticker speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Ticker konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Ticker speichern", "error", "Fehler");
  }
}

async function saveDateTickerFormat() {
  const button = document.getElementById("date-format-save-button");
  const value = document.getElementById("date-format-input").value;

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/date_ticker_format_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Datumsformat speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Datumsformat konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Datumsformat speichern", "error", "Fehler");
  }
}

async function saveTickerDeceleration() {
  const button = document.getElementById("ticker-deceleration-save-button");
  const input = document.getElementById("ticker-deceleration-input");
  const value = Math.max(0, Math.min(255, Number(input.value || 0)));
  input.value = String(value);

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/ticker_deceleration_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Ticker-Verzögerung speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Ticker-Verzögerung konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Ticker-Verzögerung speichern", "error", "Fehler");
  }
}

async function testDisplay() {
  const button = document.getElementById("test-display-button");

  beginButtonFeedback(button, "startet...");

  try {
    await apiFetch("/api/test_display");
    announceStatus("Displaytest gestartet", "ok");
    setTimeout(loadData, 1200);
    finishButtonFeedback(button, "Displaytest starten", "success", "gestartet");
  } catch (error) {
    announceStatus("Displaytest konnte nicht gestartet werden", "error");
    finishButtonFeedback(button, "Displaytest starten", "error", "Fehler");
  }
}

async function saveWeatherAppId() {
  const button = document.getElementById("weather-appid-save-button");
  const value = document.getElementById("weather-appid-input").value || "";

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/weather_appid_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "API-Schlüssel speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("API-Schlüssel konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "API-Schlüssel speichern", "error", "Fehler");
  }
}

async function saveWeatherCity() {
  const button = document.getElementById("weather-city-save-button");
  const value = document.getElementById("weather-city-input").value || "";

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/weather_city_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Ort speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Ort konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Ort speichern", "error", "Fehler");
  }
}

async function saveWeatherCoordinates() {
  const button = document.getElementById("weather-coordinates-save-button");
  const lon = document.getElementById("weather-lon-input").value || "";
  const lat = document.getElementById("weather-lat-input").value || "";

  beginButtonFeedback(button, "speichert...");

  try {
    const query = "?lon=" + encodeURIComponent(lon) + "&lat=" + encodeURIComponent(lat);
    await apiFetch("/api/weather_coordinates_set" + query);
    await loadData();
    finishButtonFeedback(button, "Koordinaten speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Koordinaten konnten nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Koordinaten speichern", "error", "Fehler");
  }
}

async function getWeatherNow() {
  await runWeatherAction("weather-now-button", "/api/weather_get_now", "Wetter abrufen", "Wetter konnte nicht angefordert werden");
}

async function getWeatherForecast() {
  await runWeatherAction("weather-forecast-button", "/api/weather_get_forecast", "Wettervorhersage abrufen", "Wettervorhersage konnte nicht angefordert werden");
}

async function runWeatherAction(buttonId, endpoint, buttonText, errorText) {
  const button = document.getElementById(buttonId);

  beginButtonFeedback(button, "läuft...");

  try {
    await apiFetch(endpoint);
    await loadData();
    finishButtonFeedback(button, buttonText, "success", "angefragt");
  } catch (error) {
    announceStatus(errorText, "error");
    finishButtonFeedback(button, buttonText, "error", "Fehler");
  }
}

function openWeatherMapPicker() {
  const modal = document.getElementById("weather-map-modal");
  modal.classList.remove("is-hidden");
  modal.setAttribute("aria-hidden", "false");
  initializeWeatherMap();
}

function closeWeatherMapPicker() {
  const modal = document.getElementById("weather-map-modal");
  modal.classList.add("is-hidden");
  modal.setAttribute("aria-hidden", "true");
}

function initializeWeatherMap() {
  const status = document.getElementById("weather-map-status");

  if (!window.L) {
    status.textContent = "Kartendienst konnte nicht geladen werden.";
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
  status.textContent = "Tippe auf die Karte oder suche einen Ort.";
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
    status.textContent = "Bitte zuerst einen Ort eingeben.";
    return;
  }

  button.disabled = true;
  button.textContent = "sucht...";
  status.textContent = "Ort wird gesucht...";

  try {
    const url = "https://nominatim.openstreetmap.org/search?format=jsonv2&limit=1&q=" + encodeURIComponent(query);
    const response = await fetch(url, {
      headers: { Accept: "application/json" },
      cache: "no-store"
    });
    const results = await response.json();

    if (!Array.isArray(results) || !results.length) {
      status.textContent = "Kein Treffer für diesen Ort gefunden.";
      return;
    }

    const result = results[0];
    const lat = Number(result.lat);
    const lon = Number(result.lon);
    const city = result.display_name || query;

    setWeatherMapSelection(lat, lon, city);
    weatherMap.setView([lat, lon], 11);
    status.textContent = "Ort gefunden und auf der Karte gesetzt.";
  } catch (error) {
    status.textContent = "Ortssuche konnte nicht geladen werden.";
  } finally {
    button.disabled = false;
    button.textContent = "Suchen";
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

  button.disabled = true;
  button.textContent = "liest...";
  status.textContent = "Aktueller Standort wird gelesen...";

  navigator.geolocation.getCurrentPosition(
    (position) => {
      const lat = position.coords.latitude;
      const lon = position.coords.longitude;

      setWeatherMapSelection(lat, lon, document.getElementById("weather-map-city-input").value || "");
      weatherMap.setView([lat, lon], 12);
      status.textContent = "Aktueller Standort gesetzt.";
      reverseLookupWeatherLocation(lat, lon);
      button.disabled = false;
      button.textContent = "Aktuellen Standort verwenden";
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
      button.disabled = false;
      button.textContent = "Aktuellen Standort verwenden";
    },
    { enableHighAccuracy: true, timeout: 10000, maximumAge: 30000 }
  );
}

async function useApproximateWeatherLocation(initialMessage) {
  const button = document.getElementById("weather-current-location-button");
  const status = document.getElementById("weather-map-status");

  button.disabled = true;
  button.textContent = "ermittelt...";
  status.textContent = initialMessage || "Näherungsstandort über Internetverbindung wird ermittelt...";

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
      ? "Näherungsstandort gesetzt: " + city + "."
      : "Näherungsstandort wurde gesetzt.";
  } catch (error) {
    status.textContent = "Standort konnte auch näherungsweise nicht ermittelt werden.";
  } finally {
    button.disabled = false;
    button.textContent = "Aktuellen Standort verwenden";
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
    status.textContent = "Standort aus Karte übernommen.";
  } catch (error) {
    status.textContent = "Koordinaten gesetzt. Ortsname konnte nicht aufgelöst werden.";
  }
}

function applyWeatherMapSelection() {
  const city = document.getElementById("weather-map-city-input").value || "";
  const lon = document.getElementById("weather-map-lon-input").value || "";
  const lat = document.getElementById("weather-map-lat-input").value || "";

  document.getElementById("weather-city-input").value = city;
  document.getElementById("weather-lon-input").value = lon;
  document.getElementById("weather-lat-input").value = lat;
  document.getElementById("weather-location-preview").textContent = "Aus Karte gewählt: " + (city || "-") + " | " + (lon || "-") + " / " + (lat || "-");
  closeWeatherMapPicker();
}

async function saveNetworkClient() {
  const button = document.getElementById("network-client-save-button");
  const ssid = document.getElementById("network-ssid-select").value || "";
  const key = document.getElementById("network-key-input").value || "";

  beginButtonFeedback(button, "verbindet...");

  try {
    const query = "?ssid=" + encodeURIComponent(ssid) + "&key=" + encodeURIComponent(key);
    await apiFetch("/api/network_client_set" + query);
    announceStatus("WLAN-Client-Verbindung wurde angestoßen", "ok");
    setTimeout(loadData, 1500);
    finishButtonFeedback(button, "Als WLAN-Client verbinden", "success", "gestartet");
  } catch (error) {
    announceStatus("WLAN-Client konnte nicht gesetzt werden", "error");
    finishButtonFeedback(button, "Als WLAN-Client verbinden", "error", "Fehler");
  }
}

async function saveNetworkAp() {
  const button = document.getElementById("network-ap-save-button");
  const ssid = document.getElementById("network-ap-ssid-input").value || "";
  const key = document.getElementById("network-ap-key-input").value || "";

  beginButtonFeedback(button, "startet...");

  try {
    const query = "?ssid=" + encodeURIComponent(ssid) + "&key=" + encodeURIComponent(key);
    await apiFetch("/api/network_ap_set" + query);
    announceStatus("Start des Zugangspunkts wurde angestoßen", "ok");
    setTimeout(loadData, 1500);
    finishButtonFeedback(button, "Zugangspunkt starten", "success", "gestartet");
  } catch (error) {
    announceStatus("Zugangspunkt konnte nicht gesetzt werden", "error");
    finishButtonFeedback(button, "Zugangspunkt starten", "error", "Fehler");
  }
}

async function saveTimeServer() {
  await runTextSave("network-timeserver-save-button", "/api/network_timeserver_set", document.getElementById("network-timeserver-input").value || "", "Zeitserver speichern", "Zeitserver konnte nicht gespeichert werden");
}

async function saveTimezone() {
  const button = document.getElementById("network-timezone-save-button");
  const input = document.getElementById("network-timezone-input");
  const value = Math.max(-12, Math.min(14, Number(input.value || 0)));
  input.value = String(value);

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/network_timezone_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Zeitzone speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Zeitzone konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Zeitzone speichern", "error", "Fehler");
  }
}

async function toggleSummertime() {
  const button = document.getElementById("network-summertime-button");
  const next = button.dataset.state === "on" ? "off" : "on";

  beginButtonFeedback(button, "schaltet...");

  try {
    await apiFetch("/api/network_summertime_set?value=" + next);
    await loadData();
    finishButtonFeedback(button, button.dataset.restoreText || "Sommerzeit berücksichtigen", "success", next === "on" ? "aktiviert" : "deaktiviert", true);
  } catch (error) {
    announceStatus("Sommerzeit konnte nicht gesetzt werden", "error");
    finishButtonFeedback(button, button.dataset.restoreText || "Sommerzeit berücksichtigen", "error", "Fehler", true);
  }
}

async function saveDateTime() {
  const button = document.getElementById("datetime-save-button");
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

  beginButtonFeedback(button, "speichert...");

  try {
    const query = "?year=" + encodeURIComponent(year) +
      "&month=" + encodeURIComponent(month) +
      "&day=" + encodeURIComponent(day) +
      "&hour=" + encodeURIComponent(hour) +
      "&minute=" + encodeURIComponent(minute);
    await apiFetch("/api/datetime_set" + query);
    await loadData();
    finishButtonFeedback(button, "Datum und Uhrzeit speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Datum und Uhrzeit konnten nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Datum und Uhrzeit speichern", "error", "Fehler");
  }
}

async function learnIrRemote() {
  await runSimpleAction("learn-ir-button", "/api/learn_ir", "IR-Fernbedienung lernen", "IR-Lernmodus konnte nicht gestartet werden", "IR-Lernmodus gestartet");
}

async function getNetTime() {
  await runSimpleAction("network-nettime-button", "/api/network_get_time", "Netzzeit abrufen", "Netzzeit konnte nicht angefordert werden", "Netzzeit angefordert");
}

async function runWps() {
  await runSimpleAction("network-wps-button", "/api/network_wps", "WPS", "WPS konnte nicht gestartet werden", "WPS wurde gestartet");
}

async function saveUpdateHost() {
  await runTextSave("update-host-save-button", "/api/update_host_set", document.getElementById("update-host-input").value || "", "Update-Host speichern", "Update-Host konnte nicht gespeichert werden");
  await refreshUpdateServerAvailability();
}

async function saveUpdatePath() {
  await runTextSave("update-path-save-button", "/api/update_path_set", document.getElementById("update-path-input").value || "", "Update-Pfad speichern", "Update-Pfad konnte nicht gespeichert werden");
  await refreshUpdateServerAvailability();
}

async function uploadLocalEspUpdate(event) {
  event.preventDefault();

  const fileInput = document.getElementById("local-update-esp-file-input");
  const button = document.getElementById("local-update-esp-submit-button");
  const file = fileInput.files && fileInput.files[0];

  if (!file) {
    document.getElementById("local-update-note").textContent = "Bitte zuerst eine ESP-Firmwaredatei auswählen.";
    return;
  }

  button.disabled = true;
  button.textContent = "lädt hoch...";
  document.getElementById("updated-at").textContent = "ESP-Firmware wird lokal hochgeladen...";
  document.getElementById("local-update-note").textContent = "ESP-Firmware wird lokal hochgeladen...";

  try {
    await uploadRawFile("/api/local_esp_update", file, (loaded, total) => {
      const percent = total ? Math.min(100, Math.round((loaded / total) * 100)) : 0;
      button.textContent = "lädt hoch... " + percent + "%";
      document.getElementById("local-update-note").textContent = "ESP-Firmware wird hochgeladen: " + percent + "%";
    });
  } catch (error) {
    document.getElementById("local-update-note").textContent = "ESP-Firmware konnte nicht hochgeladen werden: " + (error.message || "unbekannter Fehler");
    button.disabled = false;
    button.textContent = "ESP lokal aktualisieren";
    return;
  }

  button.textContent = "läuft...";
  try {
    await fetch("/api/local_esp_restart", { cache: "no-store" });
  } catch (error) {
  }
  document.getElementById("update-progress-shell").classList.remove("is-hidden");
  document.getElementById("update-progress-visual").classList.add("is-hidden");
  document.getElementById("update-progress-frame").classList.add("is-hidden");
  document.getElementById("update-progress-note").textContent = "ESP-Firmware wurde übertragen. Es wird auf den Neustart gewartet.";
  document.getElementById("updated-at").textContent = "ESP-Firmware wurde übertragen. Es wird auf den Neustart gewartet.";
  pendingProgressAction = "esp-local-update";
  pendingProgressButtonId = "local-update-esp-submit-button";
  button.dataset.restoreText = "ESP lokal aktualisieren";
  waitForDeviceReady(90000, 3000, "ESP wieder erreichbar. Seite wird neu geladen.", true);
}

async function uploadLocalStm32Update(event) {
  event.preventDefault();

  const fileInput = document.getElementById("local-update-stm32-file-input");
  const button = document.getElementById("local-update-stm32-submit-button");
  const file = fileInput.files && fileInput.files[0];

  if (!file) {
    document.getElementById("local-update-note").textContent = "Bitte zuerst eine STM32-Firmwaredatei auswählen.";
    return;
  }

  button.disabled = true;
  button.textContent = "lädt hoch...";
  document.getElementById("updated-at").textContent = "STM32-Firmware wird lokal hochgeladen...";
  document.getElementById("local-update-note").textContent = "STM32-Firmware wird lokal hochgeladen...";

  try {
    await startStm32StreamingUpload(file, "local-update-stm32-submit-button", "STM32 lokal aktualisieren");
  } catch (error) {
    document.getElementById("local-update-note").textContent = "STM32-Firmware konnte nicht aktualisiert werden.";
    button.disabled = false;
    button.textContent = "STM32 lokal aktualisieren";
    return;
  }
}

function uploadRawFile(url, file, onProgress) {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();

    xhr.open("POST", url, true);
    xhr.setRequestHeader("Content-Type", "application/octet-stream");

    xhr.upload.addEventListener("progress", (event) => {
      if (onProgress) {
        onProgress(event.loaded, event.total || file.size);
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

async function downloadUpdateAssets() {
  await runSimpleAction("update-assets-button", "/api/update_download_assets", "Icon-Dateien laden", "Icon-Dateien konnten nicht geladen werden", "Icon-Dateien geladen");
}

async function downloadUpdateAppBundle() {
  if (!window.confirm("App-Paket jetzt wirklich vom Server laden und installieren?")) {
    return;
  }

  const button = document.getElementById("update-app-bundle-button");

  button.disabled = true;
  button.textContent = "läuft...";

  try {
    await fetch("/api/update_download_app_bundle", { cache: "no-store" });
    document.getElementById("updated-at").textContent = "App-Paket geladen. Seite wird neu geladen.";
    setTimeout(() => window.location.reload(), 1200);
  } catch (error) {
    document.getElementById("updated-at").textContent = "App-Paket konnte nicht geladen werden";
    button.disabled = false;
    button.textContent = "App-Paket laden";
  }
}

function triggerEspUpdate() {
  if (!window.confirm("ESP-Firmware jetzt vom Update-Server aktualisieren? Das Gerät startet dabei neu.")) {
    return;
  }

  startProgressAction("/update?action=update", "ESP-Update wird gestartet...", "esp-update", "update-esp-button", "ESP-Firmware aktualisieren");
  waitForDeviceReady(90000, 5000, "ESP wieder erreichbar. Seite wird neu geladen.", true);
}

function triggerStm32Update() {
  const fileName = document.getElementById("update-stm32-select").value || "";

  if (!fileName) {
    document.getElementById("updated-at").textContent = "Bitte zuerst eine STM32-Datei auswählen";
    return;
  }

  if (!window.confirm("STM32 jetzt mit „" + fileName + "“ flashen?")) {
    return;
  }

  startStm32StreamingAction("/update?action=flash&stm32_filenames=" + encodeURIComponent(fileName), "STM32-Flash wurde gestartet.", "update-stm32-button", "STM32 flashen");
}

function triggerTableUpdate() {
  const fileName = document.getElementById("update-table-select").value || "";

  if (!fileName) {
    document.getElementById("updated-at").textContent = "Bitte zuerst eine Layout-Tabelle auswählen";
    return;
  }

  if (!window.confirm("Layout-Tabelle „" + fileName + "“ jetzt laden?")) {
    return;
  }

  startProgressAction("/fs?action=dwntable&tablefile=" + encodeURIComponent(fileName), "Layout-Tabelle wird geladen.", "layout-table", "update-table-button", "Layout-Tabelle laden");
}

async function resetStm32() {
  if (!window.confirm("STM32 jetzt wirklich resetten?")) {
    return;
  }

  const maintenanceButton = document.getElementById("maintenance-reset-stm32-button");
  const localUpdateButton = document.getElementById("local-update-reset-button");

  maintenanceButton.disabled = true;
  localUpdateButton.disabled = true;
  maintenanceButton.textContent = "läuft...";
  localUpdateButton.textContent = "läuft...";

  try {
    await fetch("/api/maintenance_reset_stm32", { cache: "no-store" });
    document.getElementById("updated-at").textContent = "STM32-Reset wurde ausgelöst. Warte auf Abschluss...";
    await sleep(4000);
    document.getElementById("updated-at").textContent = "STM32 wurde zurückgesetzt";
    await loadData();
  } catch (error) {
    document.getElementById("updated-at").textContent = "STM32 konnte nicht zurückgesetzt werden";
  } finally {
    maintenanceButton.disabled = false;
    localUpdateButton.disabled = false;
    maintenanceButton.textContent = "STM32 zurücksetzen";
    localUpdateButton.textContent = "STM32 zurücksetzen";
  }
}

async function resetEeprom() {
  if (!window.confirm("EEPROM wirklich auf Werkseinstellungen zurücksetzen?")) {
    return;
  }

  await runSimpleAction("maintenance-reset-eeprom-button", "/api/maintenance_reset_eeprom", "EEPROM zurücksetzen", "EEPROM konnte nicht zurückgesetzt werden", "EEPROM-Reset wurde ausgelöst");
}

async function formatLittleFs() {
  if (!window.confirm("LittleFS wirklich formatieren?")) {
    return;
  }

  await runSimpleAction("maintenance-format-fs-button", "/api/maintenance_format_fs", "LittleFS formatieren", "LittleFS konnte nicht formatiert werden", "LittleFS wurde formatiert");
}

async function formatLittleFsFromFiles() {
  if (!window.confirm("LittleFS wirklich formatieren?")) {
    return;
  }

  await runSimpleAction("files-format-fs-button", "/api/maintenance_format_fs", "LittleFS formatieren", "LittleFS konnte nicht formatiert werden", "LittleFS wurde formatiert");
  document.getElementById("fs-action-status").textContent = "LittleFS wurde formatiert.";
}

async function showFsFile(fileName) {
  if (!fileName) {
    return;
  }

  try {
    const response = await fetch("/api/fs_show?filename=" + encodeURIComponent(fileName), { cache: "no-store" });
    const text = await response.text();
    document.getElementById("fs-preview-content").textContent = text || "(leer)";
    document.getElementById("fs-action-status").textContent = "Datei „" + fileName + "“ wird angezeigt.";
    document.getElementById("updated-at").textContent = fileName + " geladen";
  } catch (error) {
    document.getElementById("updated-at").textContent = "Datei konnte nicht geladen werden";
  }
}

async function deleteFsFile(fileName) {
  if (!fileName) {
    return;
  }

  if (!window.confirm("Datei „" + fileName + "“ wirklich löschen?")) {
    return;
  }

  try {
    await fetch("/api/fs_remove?filename=" + encodeURIComponent(fileName), { cache: "no-store" });
    document.getElementById("fs-preview-content").textContent = "Mit „Anzeigen“ aus der Dateiliste wird hier der Inhalt der gewählten Datei eingeblendet.";
    document.getElementById("fs-action-status").textContent = "Datei „" + fileName + "“ wurde gelöscht.";
    await loadData();
  } catch (error) {
    document.getElementById("updated-at").textContent = "Datei konnte nicht gelöscht werden";
  }
}

function handleUploadSubmit(event) {
  const form = event.currentTarget;
  const button = form.querySelector('button[type="submit"]');
  const label = form.querySelector(".label");
  const fileInput = form.querySelector('input[type="file"]');

  if (button) {
    if (!button.dataset.restoreText) {
      button.dataset.restoreText = button.textContent;
    }
    button.disabled = true;
    button.textContent = "läuft...";
  }

  const targetName = label ? label.textContent : "Datei";
  const fileName = fileInput && fileInput.files && fileInput.files[0] ? fileInput.files[0].name : "";
  document.getElementById("fs-action-status").textContent =
    "Upload gestartet: " + targetName + (fileName ? " <- " + fileName : "");
  document.getElementById("updated-at").textContent = "Upload wurde gestartet...";
}

function handleLegacyFrameLoad() {
  document.querySelectorAll(".upload-form button[type='submit']").forEach((button) => {
    button.disabled = false;
    button.textContent = button.dataset.restoreText || "Hochladen";
  });
  resetProgressButton();
  try {
    const frame = document.getElementById("legacy-action-frame");
    const body = frame.contentWindow && frame.contentWindow.document && frame.contentWindow.document.body;
    const text = body ? (body.textContent || "") : "";
    const compact = text.replace(/\s+/g, " ").trim();

    if (compact) {
      if (compact.indexOf("WordClock App bundle installed successfully") >= 0) {
        document.getElementById("fs-action-status").textContent = "App-Paket wurde erfolgreich installiert.";
      } else if (compact.indexOf("Failed to upload file") >= 0 || compact.indexOf("Wrong http header") >= 0) {
        document.getElementById("fs-action-status").textContent = "Upload fehlgeschlagen: " + compact;
      } else {
        document.getElementById("fs-action-status").textContent = compact;
      }
    }
  } catch (error) {
  }
  document.getElementById("updated-at").textContent = "Legacy-Aktion beantwortet";
  setTimeout(loadData, 1500);
}

function handleProgressFrameLoad() {
  const frame = document.getElementById("update-progress-frame");
  const note = document.getElementById("update-progress-note");

  try {
    const body = frame.contentWindow && frame.contentWindow.document && frame.contentWindow.document.body;
    const text = body ? (body.textContent || "") : "";

    if (pendingProgressAction === "layout-table") {
      note.textContent = "Layout-Tabelle wurde geladen.";
      resetProgressButton();
      finishProgressUi(1800);
      setTimeout(loadData, 1500);
      return;
    }

    if (pendingProgressAction === "esp-update") {
      note.textContent = "ESP aktualisiert. Es wird gewartet, bis das Gerät wieder bereit ist.";
    }
  } catch (error) {
    note.textContent = "Update-Antwort empfangen.";
  }
}

function triggerLegacyAction(url, message, reloadDelay) {
  document.getElementById("updated-at").textContent = message;
  document.getElementById("legacy-action-frame").src = url;
  setTimeout(loadData, reloadDelay || 2000);
}

function startProgressAction(url, message, actionType, buttonId, buttonText) {
  pendingProgressAction = actionType || "";
  pendingProgressButtonId = buttonId || "";
  const progressShell = document.getElementById("update-progress-shell");
  const progressFrame = document.getElementById("update-progress-frame");
  const keepFrameActiveInBackground = actionType === "stm32-flash" || actionType === "layout-table";
  const hideFrame = actionType === "esp-update" || actionType === "layout-table";

  progressShell.classList.remove("is-hidden");
  progressFrame.classList.toggle("progress-frame-hidden", keepFrameActiveInBackground);
  progressFrame.classList.toggle("is-hidden", hideFrame);
  document.getElementById("update-progress-visual").classList.toggle("is-hidden", actionType !== "stm32-flash");
  document.getElementById("update-progress-note").textContent = message;
  document.getElementById("updated-at").textContent = message;
  setBusyButton(buttonId, "läuft...");
  if (buttonId) {
    const button = document.getElementById(buttonId);
    if (button) {
      button.dataset.restoreText = buttonText || button.textContent;
    }
  }
  if (actionType === "stm32-flash") {
    beginStm32Progress();
  } else {
    stopStm32Progress();
  }
  progressFrame.src = "about:blank";
  progressShell.scrollIntoView({ behavior: "smooth", block: "start" });
  window.setTimeout(() => {
    const separator = url.indexOf("?") >= 0 ? "&" : "?";
    progressFrame.src = url + separator + "_ts=" + Date.now();
  }, 0);
}

function startStm32StreamingAction(url, message, buttonId, buttonText) {
  pendingProgressAction = "stm32-flash";
  pendingProgressButtonId = buttonId || "";
  stm32AutoResetStarted = false;
  const progressShell = document.getElementById("update-progress-shell");
  const progressFrame = document.getElementById("update-progress-frame");

  progressShell.classList.remove("is-hidden");
  progressFrame.classList.add("is-hidden");
  progressFrame.classList.remove("progress-frame-hidden");
  document.getElementById("update-progress-visual").classList.remove("is-hidden");
  document.getElementById("update-progress-note").textContent = message;
  document.getElementById("updated-at").textContent = message;
  setBusyButton(buttonId, "läuft...");

  if (buttonId) {
    const button = document.getElementById(buttonId);
    if (button) {
      button.dataset.restoreText = buttonText || button.textContent;
    }
  }

  beginStm32Progress();
  progressShell.scrollIntoView({ behavior: "smooth", block: "start" });

  const xhr = new XMLHttpRequest();
  const separator = url.indexOf("?") >= 0 ? "&" : "?";
  xhr.open("GET", url + separator + "_ts=" + Date.now(), true);

  xhr.onprogress = () => {
    const text = xhr.responseText || "";
    syncStm32ProgressFromText(text);

    if (!stm32AutoResetStarted && text.indexOf("Please Reset your STM32 now") >= 0) {
      stm32AutoResetStarted = true;
      setStm32ProgressStage(5);
      document.getElementById("update-progress-note").textContent = "STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.";
      autoResetStm32AfterFlash();
    }
  };

  xhr.onload = () => {
    const text = xhr.responseText || "";
    syncStm32ProgressFromText(text);

    if (xhr.status < 200 || xhr.status >= 300) {
      document.getElementById("update-progress-note").textContent = "STM32-Flash konnte nicht gestartet werden.";
      stopStm32Progress();
      resetProgressButton();
      return;
    }

    if (!stm32AutoResetStarted && text.indexOf("Please Reset your STM32 now") >= 0) {
      stm32AutoResetStarted = true;
      setStm32ProgressStage(5);
      document.getElementById("update-progress-note").textContent = "STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.";
      autoResetStm32AfterFlash();
    }
  };

  xhr.onerror = () => {
    document.getElementById("update-progress-note").textContent = "STM32-Flash konnte nicht gestartet werden.";
    stopStm32Progress();
    resetProgressButton();
  };

  xhr.send();
}

function startStm32StreamingUpload(file, buttonId, buttonText) {
  return new Promise((resolve, reject) => {
    pendingProgressAction = "stm32-flash";
    pendingProgressButtonId = buttonId || "";
    stm32AutoResetStarted = false;
    const progressShell = document.getElementById("update-progress-shell");
    const progressFrame = document.getElementById("update-progress-frame");
    const button = document.getElementById(buttonId);
    const formData = new FormData();
    formData.append("fileField", file, file.name);

    progressShell.classList.remove("is-hidden");
    progressFrame.classList.add("is-hidden");
    progressFrame.classList.remove("progress-frame-hidden");
    document.getElementById("update-progress-visual").classList.remove("is-hidden");
    document.getElementById("update-progress-note").textContent = "Lokaler STM32-Flash wird gestartet.";
    document.getElementById("updated-at").textContent = "Lokaler STM32-Flash wird gestartet.";
    setBusyButton(buttonId, "läuft...");

    if (button) {
      button.dataset.restoreText = buttonText || button.textContent;
    }

    beginStm32Progress();
    progressShell.scrollIntoView({ behavior: "smooth", block: "start" });

    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/flash_stm32_local", true);

    xhr.upload.addEventListener("progress", (event) => {
      if (event.lengthComputable) {
        const percent = Math.min(100, Math.round((event.loaded / event.total) * 100));
        document.getElementById("local-update-note").textContent = "STM32-Firmware wird hochgeladen: " + percent + "%";
      }
    });

    xhr.onprogress = () => {
      const text = xhr.responseText || "";
      syncStm32ProgressFromText(text);

      if (!stm32AutoResetStarted && text.indexOf("Please Reset your STM32 now") >= 0) {
        stm32AutoResetStarted = true;
        setStm32ProgressStage(5);
        document.getElementById("update-progress-note").textContent = "STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.";
        autoResetStm32AfterFlash();
      }
    };

    xhr.onload = () => {
      const text = xhr.responseText || "";
      syncStm32ProgressFromText(text);

      if (xhr.status < 200 || xhr.status >= 300) {
        document.getElementById("update-progress-note").textContent = "Lokaler STM32-Flash konnte nicht gestartet werden.";
        stopStm32Progress();
        resetProgressButton();
        reject(new Error("stm32 local failed"));
        return;
      }

      if (!stm32AutoResetStarted && text.indexOf("Please Reset your STM32 now") >= 0) {
        stm32AutoResetStarted = true;
        setStm32ProgressStage(5);
        document.getElementById("update-progress-note").textContent = "STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.";
        autoResetStm32AfterFlash();
      }

      resolve();
    };

    xhr.onerror = () => {
      document.getElementById("update-progress-note").textContent = "Lokaler STM32-Flash konnte nicht gestartet werden.";
      stopStm32Progress();
      resetProgressButton();
      reject(new Error("stm32 local failed"));
    };

    xhr.send(formData);
  });
}

async function autoResetStm32AfterFlash() {
  try {
    await fetch("/api/maintenance_reset_stm32", { cache: "no-store" });
    setStm32ProgressStage(6);
    document.getElementById("updated-at").textContent = "STM32 wurde nach dem Flash automatisch zurückgesetzt. Warte auf Abschluss...";
    document.getElementById("update-progress-note").textContent = "STM32 wird automatisch zurückgesetzt. Daten werden danach neu geladen.";
    await sleep(4000);
    setStm32ProgressStage(8);
    document.getElementById("updated-at").textContent = "STM32 wurde nach dem Flash automatisch zurückgesetzt";
    document.getElementById("update-progress-note").textContent = "STM32-Update erfolgreich abgeschlossen.";
    await loadData();
    stopStm32Progress();
    resetProgressButton();
    finishProgressUi(2200);
  } catch (error) {
    document.getElementById("update-progress-note").textContent = "STM32-Flash fertig, automatischer Reset ist fehlgeschlagen.";
    stopStm32Progress();
    resetProgressButton();
  }
}

async function waitForDeviceReady(timeoutMs, initialDelayMs, readyMessage, reloadPage) {
  const deadline = Date.now() + timeoutMs;
  const forcedReloadAt = Date.now() + Math.min(timeoutMs, 25000);
  const note = document.getElementById("update-progress-note");
  const probeUrls = ["/display_power", "/api/status", "/get_settings", "/"];

  if (reloadPage) {
    scheduleEspReloadWatchdog(Math.min(timeoutMs, 30000));
  }

  scrollUpdateProgressIntoView();

  await sleep(initialDelayMs || 0);

  while (Date.now() < deadline) {
    for (const probeUrl of probeUrls) {
      try {
        const separator = probeUrl.indexOf("?") >= 0 ? "&" : "?";
        const response = await fetchWithTimeout(probeUrl + separator + "_ts=" + Date.now(), {
          cache: "no-store"
        }, 1800);

        if (response.ok) {
          clearEspReloadWatchdog();
          note.textContent = readyMessage;
          document.getElementById("updated-at").textContent = readyMessage;
          resetProgressButton();
          finishProgressUi(900);

          if (reloadPage) {
            setTimeout(reloadAppPage, 1200);
          } else {
            setTimeout(loadData, 1200);
          }
          return;
        }
      } catch (error) {
      }
    }

    for (const probeUrl of probeUrls) {
      try {
        const separator = probeUrl.indexOf("?") >= 0 ? "&" : "?";
        const frameLoaded = await probeDeviceReadyViaFrame(probeUrl + separator + "_ts=" + Date.now(), 1800);
        if (frameLoaded) {
          clearEspReloadWatchdog();
          note.textContent = readyMessage;
          document.getElementById("updated-at").textContent = readyMessage;
          resetProgressButton();
          finishProgressUi(900);

          if (reloadPage) {
            setTimeout(reloadAppPage, 1200);
          } else {
            setTimeout(loadData, 1200);
          }
          return;
        }
      } catch (error) {
      }
    }

    note.textContent = "Warte auf Neustart des ESP...";
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

function reloadAppPage() {
  clearEspReloadWatchdog();
  const url = new URL(window.location.href);
  url.searchParams.set("_reload", String(Date.now()));
  window.location.replace(url.toString());
}

function manualReloadApp() {
  if (hasUnsavedEdits && !window.confirm("Es gibt ungespeicherte Änderungen. App trotzdem neu laden?")) {
    return;
  }

  const button = document.getElementById("reload-button");
  beginButtonFeedback(button, "lädt neu...");
  announceStatus("App wird neu geladen...", "warn");
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
  const timer = window.setTimeout(() => controller.abort(), timeoutMs || 1500);

  try {
    return await fetch(url, {
      ...(options || {}),
      signal: controller.signal
    });
  } finally {
    window.clearTimeout(timer);
  }
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
    }, timeoutMs || 1500);

    frame.addEventListener("load", () => {
      window.clearTimeout(timer);
      finish(true);
    }, { once: true });

    frame.addEventListener("error", () => {
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
    shell.scrollIntoView({ behavior: "smooth", block: "start" });
  }
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
  document.getElementById("update-progress-note").textContent = "STM32-Update wird vorbereitet...";
  stm32ProgressAdvanceTimer = window.setTimeout(() => {
    if (stm32ProgressStage === 1) {
      document.getElementById("update-progress-note").textContent = "Warte auf Rückmeldung vom STM32-Bootloader...";
    }
  }, 1200);
  stm32ProgressTimer = window.setInterval(() => {
    const note = document.getElementById("update-progress-note");

    if (stm32ProgressStage === 4) {
      note.textContent = "STM32-Firmware wird geschrieben und verifiziert...";
    }
  }, 3000);
}

function stopStm32Progress() {
  if (stm32ProgressAdvanceTimer) {
    window.clearTimeout(stm32ProgressAdvanceTimer);
    stm32ProgressAdvanceTimer = 0;
  }
  if (stm32ProgressMonitorTimer) {
    window.clearInterval(stm32ProgressMonitorTimer);
    stm32ProgressMonitorTimer = 0;
  }
  if (stm32ProgressTimer) {
    window.clearInterval(stm32ProgressTimer);
    stm32ProgressTimer = 0;
  }
}

function setStm32ProgressStage(stage) {
  stm32ProgressStage = stage;
  const steps = [
    { title: "Vorbereiten", note: "Update wird gestartet." },
    { title: "Bootloader", note: "STM32-Bootloader wird angesprochen." },
    { title: "HEX prüfen", note: "Firmwaredatei wird geprüft." },
    { title: "Flash löschen", note: "STM32-Flash wird gelöscht." },
    { title: "Flash schreiben", note: "Firmware wird geschrieben und verifiziert." },
    { title: "Zurücksetzen", note: "STM32 wird automatisch neu gestartet." },
    { title: "Abschließen", note: "Daten werden neu geladen." }
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
    note.textContent = "STM32-Bootloader wurde erreicht.";
  }

  if (normalized.indexOf("Checking HEX file") >= 0 || normalized.indexOf("Check successful") >= 0) {
    if (stm32ProgressStage < 3) {
      setStm32ProgressStage(3);
    }
    note.textContent = "Firmwaredatei wird geprüft.";
  }

  if (normalized.indexOf("Erasing flash") >= 0) {
    if (stm32ProgressStage < 4) {
      setStm32ProgressStage(4);
    }
    note.textContent = "STM32-Flash wird gelöscht.";
  }

  if (normalized.indexOf("Flashing STM32") >= 0 || normalized.indexOf("Pages flashed:") >= 0 || normalized.indexOf("Flash successful") >= 0) {
    if (stm32ProgressStage < 5) {
      setStm32ProgressStage(5);
    }
    note.textContent = "STM32-Firmware wird geschrieben und verifiziert.";
  }
}

async function saveRtcTemperatureCorrection() {
  await saveTemperatureCorrection("temperature-rtc-correction-input", "temperature-rtc-correction-save-button", "/api/temperature_rtc_correction_set", "RTC-Korrektur speichern", "RTC-Korrektur konnte nicht gespeichert werden");
}

async function saveDs18xxTemperatureCorrection() {
  await saveTemperatureCorrection("temperature-ds18xx-correction-input", "temperature-ds18xx-correction-save-button", "/api/temperature_ds18xx_correction_set", "DS18xx-Korrektur speichern", "DS18xx-Korrektur konnte nicht gespeichert werden");
}

async function saveTemperatureCorrection(inputId, buttonId, endpoint, buttonText, errorText) {
  const button = document.getElementById(buttonId);
  const input = document.getElementById(inputId);
  const value = Math.max(0, Math.min(255, Number(input.value || 0)));
  input.value = String(value);

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch(endpoint + "?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, buttonText, "success", "gespeichert");
  } catch (error) {
    announceStatus(errorText, "error");
    finishButtonFeedback(button, buttonText, "error", "Fehler");
  }
}

async function displayTemperatureNow() {
  await runSimpleAction("temperature-display-button", "/api/temperature_display", "Temperatur anzeigen", "Temperatur konnte nicht angezeigt werden", "Temperaturanzeige ausgelöst");
}

async function setLdrMinValue() {
  await runSimpleAction("ldr-min-button", "/api/ldr_min_set", "Aktuellen Wert als Minimum setzen", "LDR-Minimum konnte nicht gesetzt werden", "LDR-Minimum gespeichert");
}

async function setLdrMaxValue() {
  await runSimpleAction("ldr-max-button", "/api/ldr_max_set", "Aktuellen Wert als Maximum setzen", "LDR-Maximum konnte nicht gesetzt werden", "LDR-Maximum gespeichert");
}

async function saveAnimationMode() {
  await runSelectSave("animation-mode-select", "animation-mode-save-button", "/api/animation_mode_set", "Anzeigeanimation speichern", "Anzeigeanimation konnte nicht gespeichert werden");
}

async function saveColorAnimationMode() {
  await runSelectSave("color-animation-mode-select", "color-animation-mode-save-button", "/api/color_animation_mode_set", "Farbanimation speichern", "Farbanimation konnte nicht gespeichert werden");
}

async function runSelectSave(selectId, buttonId, endpoint, buttonText, errorText) {
  const button = document.getElementById(buttonId);
  const value = document.getElementById(selectId).value;

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch(endpoint + "?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, buttonText, "success", "gespeichert");
  } catch (error) {
    announceStatus(errorText, "error");
    finishButtonFeedback(button, buttonText, "error", "Fehler");
  }
}

async function saveAnimationProfile(idx) {
  const button = document.querySelector('[data-an-save="' + idx + '"]');
  const deceleration = document.getElementById("an-dec-" + idx).value;
  const favourite = document.getElementById("an-fav-" + idx).checked ? "on" : "off";

  beginButtonFeedback(button, "speichert...");

  try {
    const query = "?idx=" + idx + "&deceleration=" + encodeURIComponent(deceleration) + "&favourite=" + favourite;
    await apiFetch("/api/animation_profile_set" + query);
    await loadData();
    finishButtonFeedback(button, "Profil speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Animationsprofil konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Profil speichern", "error", "Fehler");
  }
}

async function resetAnimationProfileDefault(idx) {
  const button = document.querySelector('[data-an-default="' + idx + '"]');

  beginButtonFeedback(button, "setzt...");

  try {
    await apiFetch("/api/animation_profile_default?idx=" + idx);
    await loadData();
    finishButtonFeedback(button, "Standard", "success", "gesetzt");
  } catch (error) {
    announceStatus("Standardwert konnte nicht gesetzt werden", "error");
    finishButtonFeedback(button, "Standard", "error", "Fehler");
  }
}

async function saveColorAnimationProfile(idx) {
  const button = document.querySelector('[data-can-save="' + idx + '"]');
  const deceleration = document.getElementById("can-dec-" + idx).value;

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/color_animation_profile_set?idx=" + idx + "&deceleration=" + encodeURIComponent(deceleration));
    await loadData();
    finishButtonFeedback(button, "Profil speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Farbanimationsprofil konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Profil speichern", "error", "Fehler");
  }
}

async function resetColorAnimationProfileDefault(idx) {
  const button = document.querySelector('[data-can-default="' + idx + '"]');

  beginButtonFeedback(button, "setzt...");

  try {
    await apiFetch("/api/color_animation_profile_default?idx=" + idx);
    await loadData();
    finishButtonFeedback(button, "Standard", "success", "gesetzt");
  } catch (error) {
    announceStatus("Standardwert konnte nicht gesetzt werden", "error");
    finishButtonFeedback(button, "Standard", "error", "Fehler");
  }
}

async function saveDimCurve(prefix) {
  const button = document.getElementById(prefix === "ambi" ? "ambilight-dim-save-button" : "display-dim-save-button");
  const endpoint = prefix === "ambi" ? "/api/ambilight_dim_level_set" : "/api/display_dim_level_set";
  const buttonText = prefix === "ambi" ? "Ambilight-Dimmkurve speichern" : "Display-Dimmkurve speichern";

  beginButtonFeedback(button, "speichert...");

  try {
    for (let idx = 0; idx <= 15; idx += 1) {
      const input = document.getElementById(prefix + "-dim-" + idx);
      const value = Math.max(0, Math.min(15, Number(input.value || 0)));
      input.value = String(value);
      syncDimCurveValue(prefix, idx);
      await apiFetch(endpoint + "?idx=" + idx + "&value=" + encodeURIComponent(value));
    }
    await loadData();
    finishButtonFeedback(button, buttonText, "success", "gespeichert");
  } catch (error) {
    announceStatus("Dimmkurve konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, buttonText, "error", "Fehler");
  }
}

async function saveTftFlags() {
  const button = document.getElementById("tft-save-button");
  const rgb = document.getElementById("tft-rgb-checkbox").checked ? "on" : "off";
  const hflip = document.getElementById("tft-hflip-checkbox").checked ? "on" : "off";
  const vflip = document.getElementById("tft-vflip-checkbox").checked ? "on" : "off";

  beginButtonFeedback(button, "speichert...");

  try {
    const query = "?rgb=" + rgb + "&hflip=" + hflip + "&vflip=" + vflip;
    await apiFetch("/api/tft_flags_set" + query);
    await loadData();
    finishButtonFeedback(button, "TFT-Optionen speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("TFT-Optionen konnten nicht gespeichert werden", "error");
    finishButtonFeedback(button, "TFT-Optionen speichern", "error", "Fehler");
  }
}

async function runTextSave(buttonId, endpoint, value, buttonText, errorText) {
  const button = document.getElementById(buttonId);

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch(endpoint + "?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, buttonText, "success", "gespeichert");
  } catch (error) {
    announceStatus(errorText, "error");
    finishButtonFeedback(button, buttonText, "error", "Fehler");
  }
}

async function refreshUpdateServerAvailability() {
  announceStatus("Server-Dateien werden mit den neuen Update-Angaben neu geprüft...", "warn");

  try {
    await loadData();
    announceStatus("Server-Verfügbarkeit wurde neu geprüft", "ok");
  } catch (error) {
    announceStatus("Server-Verfügbarkeit konnte nicht neu geprüft werden", "error");
  }
}

async function runSimpleAction(buttonId, endpoint, buttonText, errorText, successText) {
  const button = document.getElementById(buttonId);

  beginButtonFeedback(button, "läuft...");

  try {
    await apiFetch(endpoint);
    announceStatus(successText, "ok");
    setTimeout(loadData, 1200);
    finishButtonFeedback(button, buttonText, "success", "fertig");
  } catch (error) {
    announceStatus(errorText, "error");
    finishButtonFeedback(button, buttonText, "error", "Fehler");
  }
}

async function saveAmbilightModeProfile(idx) {
  const button = document.querySelector('[data-alm-save="' + idx + '"]');
  const deceleration = Math.max(0, Math.min(15, Number(document.getElementById("alm-dec-" + idx).value || 0)));

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/ambilight_mode_profile_set?idx=" + idx + "&deceleration=" + encodeURIComponent(deceleration));
    await loadData();
    finishButtonFeedback(button, "Profil speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Ambilight-Profil konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Profil speichern", "error", "Fehler");
  }
}

async function resetAmbilightModeProfile(idx) {
  const button = document.querySelector('[data-alm-default="' + idx + '"]');

  beginButtonFeedback(button, "setzt...");

  try {
    await apiFetch("/api/ambilight_mode_profile_default?idx=" + idx);
    await loadData();
    finishButtonFeedback(button, "Standard", "success", "gesetzt");
  } catch (error) {
    announceStatus("Ambilight-Standardwert konnte nicht gesetzt werden", "error");
    finishButtonFeedback(button, "Standard", "error", "Fehler");
  }
}

async function saveAmbilightBrightness() {
  const slider = document.getElementById("ambilight-brightness-slider");
  const button = document.getElementById("ambilight-brightness-save-button");
  const value = slider.value;

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/ambilight_brightness_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Ambilight-Helligkeit speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Ambilight-Helligkeit konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Ambilight-Helligkeit speichern", "error", "Fehler");
  }
}

async function saveAmbilightMode() {
  const button = document.getElementById("ambilight-mode-save-button");
  const value = document.getElementById("ambilight-mode-select").value;

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/ambilight_mode_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Ambilight-Modus speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Ambilight-Modus konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Ambilight-Modus speichern", "error", "Fehler");
  }
}

async function saveAmbilightLeds() {
  const button = document.getElementById("ambilight-leds-save-button");
  const input = document.getElementById("ambilight-leds-input");
  const value = Math.max(0, Math.min(999, Number(input.value || 0)));
  input.value = String(value);

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/ambilight_leds_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "LED-Anzahl speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("LED-Anzahl konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "LED-Anzahl speichern", "error", "Fehler");
  }
}

async function saveAmbilightOffset() {
  const button = document.getElementById("ambilight-offset-save-button");
  const input = document.getElementById("ambilight-offset-input");
  const value = Math.max(0, Math.min(999, Number(input.value || 0)));
  input.value = String(value);

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/ambilight_offset_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Offset speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Ambilight-Offset konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Offset speichern", "error", "Fehler");
  }
}

async function saveColor(prefix) {
  const button = document.getElementById(prefix + "-color-save-button");
  const rgbHex = document.getElementById(prefix + "-color-rgb").value;
  const white = Number(document.getElementById(prefix + "-color-white").value || 0);
  const rgb = hexToRgb63(rgbHex);
  const endpoint = {
    display: "/api/display_color_set",
    ambilight: "/api/ambilight_color_set",
    marker: "/api/marker_color_set"
  }[prefix];

  beginButtonFeedback(button, "speichert...");

  try {
    const query = "?red=" + rgb.red + "&green=" + rgb.green + "&blue=" + rgb.blue + "&white=" + white;
    await apiFetch(endpoint + query);
    await loadData();
    finishButtonFeedback(button, {
      display: "Display Farbe speichern",
      ambilight: "Ambilight Farbe speichern",
      marker: "Marker Farbe speichern"
    }[prefix], "success", "gespeichert");
  } catch (error) {
    announceStatus("Farbe konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, {
      display: "Display Farbe speichern",
      ambilight: "Ambilight Farbe speichern",
      marker: "Marker Farbe speichern"
    }[prefix], "error", "Fehler");
  }
}

async function saveDfplayerVolume() {
  const button = document.getElementById("dfplayer-volume-save-button");
  const value = document.getElementById("dfplayer-volume-slider").value;

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/dfplayer_volume_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Lautstärke speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("DFPlayer-Lautstärke konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Lautstärke speichern", "error", "Fehler");
  }
}

async function saveDfplayerMode() {
  const button = document.getElementById("dfplayer-mode-save-button");
  const value = document.getElementById("dfplayer-mode-select").value;

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/dfplayer_mode_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Modus speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("DFPlayer-Modus konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Modus speichern", "error", "Fehler");
  }
}

async function saveDfplayerBellFlags() {
  const button = document.getElementById("dfplayer-bell-save-button");
  const m15 = document.getElementById("dfplayer-bell-15").checked ? "on" : "off";
  const m30 = document.getElementById("dfplayer-bell-30").checked ? "on" : "off";
  const m45 = document.getElementById("dfplayer-bell-45").checked ? "on" : "off";

  beginButtonFeedback(button, "speichert...");

  try {
    const query = "?m15=" + m15 + "&m30=" + m30 + "&m45=" + m45;
    await apiFetch("/api/dfplayer_bell_flags_set" + query);
    await loadData();
    finishButtonFeedback(button, "Glockenzeiten speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Glockenzeiten konnten nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Glockenzeiten speichern", "error", "Fehler");
  }
}

async function saveDfplayerSpeakCycle() {
  const button = document.getElementById("dfplayer-speak-save-button");
  const input = document.getElementById("dfplayer-speak-cycle-input");
  const value = Math.max(0, Math.min(255, Number(input.value || 0)));
  input.value = String(value);

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch("/api/dfplayer_speak_cycle_set?value=" + encodeURIComponent(value));
    await loadData();
    finishButtonFeedback(button, "Sprachzyklus speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Sprachzyklus konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Sprachzyklus speichern", "error", "Fehler");
  }
}

async function saveDfplayerSilenceStart() {
  await saveDfplayerSilenceTime("dfplayer-silence-start-input", "dfplayer-silence-start-save-button", "/api/dfplayer_silence_start_set", "Ruhezeit Beginn speichern", "Ruhezeit Beginn konnte nicht gespeichert werden");
}

async function saveDfplayerSilenceStop() {
  await saveDfplayerSilenceTime("dfplayer-silence-stop-input", "dfplayer-silence-stop-save-button", "/api/dfplayer_silence_stop_set", "Ruhezeit Ende speichern", "Ruhezeit Ende konnte nicht gespeichert werden");
}

async function saveDfplayerSilenceTime(inputId, buttonId, endpoint, buttonText, errorText) {
  const button = document.getElementById(buttonId);
  const value = document.getElementById(inputId).value || "00:00";
  const parts = value.split(":");
  const hour = Number(parts[0] || 0);
  const minute = Number(parts[1] || 0);

  beginButtonFeedback(button, "speichert...");

  try {
    await apiFetch(endpoint + "?hour=" + encodeURIComponent(hour) + "&minute=" + encodeURIComponent(minute));
    await loadData();
    finishButtonFeedback(button, buttonText, "success", "gespeichert");
  } catch (error) {
    announceStatus(errorText, "error");
    finishButtonFeedback(button, buttonText, "error", "Fehler");
  }
}

async function playDfplayerTrack() {
  const button = document.getElementById("dfplayer-play-button");
  const folder = Math.max(0, Math.min(255, Number(document.getElementById("dfplayer-folder-input").value || 0)));
  const track = Math.max(0, Math.min(255, Number(document.getElementById("dfplayer-track-input").value || 0)));

  beginButtonFeedback(button, "spielt...");

  try {
    await apiFetch("/api/dfplayer_play?folder=" + encodeURIComponent(folder) + "&track=" + encodeURIComponent(track));
    announceStatus("DFPlayer-Titel gestartet", "ok");
    finishButtonFeedback(button, "Titel abspielen", "success", "gestartet");
  } catch (error) {
    announceStatus("DFPlayer-Titel konnte nicht gestartet werden", "error");
    finishButtonFeedback(button, "Titel abspielen", "error", "Fehler");
  }
}

async function saveDfplayerAlarm(idx) {
  const button = document.querySelector('[data-alarm-save="' + idx + '"]');
  const active = document.getElementById("df-alarm-active-" + idx).checked ? "on" : "off";
  const from = document.getElementById("df-alarm-from-" + idx).value;
  const to = document.getElementById("df-alarm-to-" + idx).value;
  const time = document.getElementById("df-alarm-time-" + idx).value || "00:00";
  const parts = time.split(":");
  const hour = Number(parts[0] || 0);
  const minute = Number(parts[1] || 0);

  beginButtonFeedback(button, "speichert...");

  try {
    const query = "?idx=" + idx +
      "&active=" + active +
      "&from=" + encodeURIComponent(from) +
      "&to=" + encodeURIComponent(to) +
      "&hour=" + encodeURIComponent(hour) +
      "&minute=" + encodeURIComponent(minute);
    await apiFetch("/api/dfplayer_alarm_set" + query);
    await loadData();
    finishButtonFeedback(button, "Titel " + String(idx + 1).padStart(3, "0") + " speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("DFPlayer-Titel konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Titel " + String(idx + 1).padStart(3, "0") + " speichern", "error", "Fehler");
  }
}

async function saveOverlay(idx) {
  const button = document.querySelector('[data-overlay-save="' + idx + '"]');
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

  button.disabled = true;
  button.textContent = "speichert...";

  try {
    const query =
      "?idx=" + idx +
      "&active=" + active +
      "&type=" + encodeURIComponent(type) +
      "&value=" + encodeURIComponent(value) +
      "&interval=" + encodeURIComponent(interval) +
      "&duration=" + encodeURIComponent(duration) +
      "&date_code=" + encodeURIComponent(dateCode) +
      "&month=" + encodeURIComponent(month) +
      "&day=" + encodeURIComponent(day) +
      "&days=" + encodeURIComponent(days);
    await apiFetch("/api/overlay_set" + query);
    await loadData();
    finishButtonFeedback(button, "Overlay speichern", "success", "gespeichert");
  } catch (error) {
    announceStatus("Overlay konnte nicht gespeichert werden", "error");
    finishButtonFeedback(button, "Overlay speichern", "error", "Fehler");
  }
}

async function displayOverlay(idx) {
  try {
    await apiFetch("/api/overlay_display?idx=" + encodeURIComponent(idx));
    announceStatus("Overlay " + idx + " wird angezeigt", "ok");
  } catch (error) {
    announceStatus("Overlay konnte nicht angezeigt werden", "error");
  }
}

async function deleteOverlay(idx) {
  try {
    await apiFetch("/api/overlay_delete?idx=" + encodeURIComponent(idx));
    await loadData();
    announceStatus("Overlay wurde gelöscht", "ok");
  } catch (error) {
    announceStatus("Overlay konnte nicht gelöscht werden", "error");
  }
}

async function saveTimerRow(idx, isAmbilight, options) {
  const opts = options || {};
  const prefix = isAmbilight ? "at" : "t";
  const button = document.querySelector('[data-' + prefix + '-save="' + idx + '"]');
  const active = document.getElementById(prefix + "-active-" + idx).checked ? "on" : "off";
  const switchOn = document.getElementById(prefix + "-action-" + idx).value || "off";
  const from = document.getElementById(prefix + "-from-" + idx).value;
  const to = document.getElementById(prefix + "-to-" + idx).value;
  const time = document.getElementById(prefix + "-time-" + idx).value || "00:00";
  const parts = time.split(":");
  const hour = Number(parts[0] || 0);
  const minute = Number(parts[1] || 0);
  const endpoint = isAmbilight ? "/api/ambilight_timer_set" : "/api/timer_set";

  if (button) {
    beginButtonFeedback(button, "speichert...");
  }

  try {
    const query =
      "?idx=" + idx +
      "&active=" + active +
      "&switch_on=" + switchOn +
      "&from=" + encodeURIComponent(from) +
      "&to=" + encodeURIComponent(to) +
      "&hour=" + encodeURIComponent(hour) +
      "&minute=" + encodeURIComponent(minute);
    await apiFetch(endpoint + query);
    if (opts.reload !== false) {
      await loadData();
    }
    if (button) {
      finishButtonFeedback(button, "Speichern", "success", "gespeichert");
    }
  } catch (error) {
    announceStatus("Timer konnte nicht gespeichert werden", "error");
    if (button) {
      finishButtonFeedback(button, "Speichern", "error", "Fehler");
    }
  }
}

async function clearTimerRow(idx, isAmbilight) {
  const prefix = isAmbilight ? "at" : "t";
  const button = document.querySelector('[data-' + prefix + '-clear="' + idx + '"]');
  const endpoint = isAmbilight ? "/api/ambilight_timer_set" : "/api/timer_set";

  beginButtonFeedback(button, "leert...");

  try {
    const query =
      "?idx=" + idx +
      "&active=off" +
      "&switch_on=off" +
      "&from=0" +
      "&to=0" +
      "&hour=0" +
      "&minute=0";
    await apiFetch(endpoint + query);
    await loadData();
    finishButtonFeedback(button, "Slot leeren", "success", "geleert");
  } catch (error) {
    announceStatus("Timer konnte nicht geleert werden", "error");
    finishButtonFeedback(button, "Slot leeren", "error", "Fehler");
  }
}

async function saveAllTimerRows(isAmbilight) {
  const button = document.getElementById(isAmbilight ? "ambilight-timer-save-all-button" : "timer-save-all-button");
  const root = document.getElementById(isAmbilight ? "ambilight-timer-list" : "timer-list");
  const prefix = isAmbilight ? "at" : "t";
  const buttons = Array.from(root.querySelectorAll('[data-' + prefix + '-save]'));
  const originalText = button.textContent;

  beginButtonFeedback(button, "speichert...");

  try {
    for (const slotButton of buttons) {
      const idx = Number(slotButton.getAttribute('data-' + prefix + '-save'));
      await saveTimerRow(idx, isAmbilight, { reload: false });
    }
    await loadData();
    announceStatus("Alle Timer wurden gespeichert", "ok");
    finishButtonFeedback(button, originalText, "success", "gespeichert");
  } catch (error) {
    announceStatus("Timer konnten nicht vollständig gespeichert werden", "error");
    finishButtonFeedback(button, originalText, "error", "Fehler");
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
  const root = document.getElementById("health-list");
  root.innerHTML = [
    '<div class="info-item"><span class="label">RTC</span><strong>' + escapeHtml(onOff(settings.numvars[NUM.RTC_IS_UP])) + "</strong></div>",
    '<div class="info-item"><span class="label">EEPROM</span><strong>' + escapeHtml(onOff(settings.numvars[NUM.EEPROM_IS_UP])) + "</strong></div>",
    '<div class="info-item"><span class="label">EEPROM Version</span><strong>' + escapeHtml(settings.strvars[STR.EEPROM_VERSION] || "-") + "</strong></div>",
    '<div class="info-item"><span class="label">Ambilight</span><select id="health-ambilight-select" class="inline-select"><option value="on">Online</option><option value="off">Offline</option></select></div>',
    '<div class="info-item"><span class="label">DFPlayer</span><strong>' + escapeHtml(onOff(dfplayerOnline ? 1 : 0)) + "</strong></div>",
    '<div class="info-item"><span class="label">DFPlayer Version</span><strong>' + escapeHtml(toHex4(settings.numvars[NUM.DFPLAYER_VERSION] || 0)) + "</strong></div>"
  ].join("");

  const select = document.getElementById("health-ambilight-select");
  if (select) {
    select.value = ambilightOnline ? "on" : "off";
    select.addEventListener("change", saveAmbilightOnlineState);
  }
}

function renderWordclock(active, settings, layoutPreview) {
  const root = document.getElementById("wordclock-grid");
  root.innerHTML = "";
  const preview = layoutPreview || {};
  const rows = Array.isArray(preview.rows) && preview.rows.length ? preview.rows : fallbackWordclockRows;
  const columnCount = Math.max(...rows.map((row) => row.length), 1);
  const activeSet = buildActiveWordSet(settings, preview, rows);

  root.style.gridTemplateColumns = "repeat(" + columnCount + ", minmax(0, 1fr))";
  root.style.gridTemplateRows = "";
  root.dataset.rows = String(rows.length);
  root.dataset.columns = String(columnCount);
  root.classList.toggle("is-wide-layout", columnCount > 12);
  root.classList.toggle("is-dense-layout", columnCount > 16);

  rows.forEach((row, rowIndex) => {
    row.split("").forEach((char, colIndex) => {
      const cell = document.createElement("span");
      cell.textContent = char === "*" || char === "#" ? " " : char;
      if (active && activeSet.has(rowIndex + "-" + colIndex)) {
        cell.className = "active";
      }
      root.appendChild(cell);
    });
  });

  renderWordclockCorners(settings, preview);
  scheduleWordclockSizing();
}

let wordclockSizingFrame = 0;

function scheduleWordclockSizing() {
  if (wordclockSizingFrame) {
    window.cancelAnimationFrame(wordclockSizingFrame);
  }
  wordclockSizingFrame = window.requestAnimationFrame(() => {
    wordclockSizingFrame = 0;
    updateWordclockSizing();
  });
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
  const currentTable = updateTableInfo && updateTableInfo.current_table ? updateTableInfo.current_table : "";
  const fallbackPreview = getDefaultLayoutPreview(settings);

  if (!currentTable) {
    return fallbackPreview;
  }

  if (layoutPreviewCache[currentTable]) {
    return layoutPreviewCache[currentTable];
  }

  try {
    const response = await fetch("/api/fs_show?filename=" + encodeURIComponent(currentTable), { cache: "no-store" });
    const text = await response.text();
    const table = parseLayoutTable(text);
    const rows = resolveLayoutRows(currentTable, table);
    const preview = { file: currentTable, table, rows };

    layoutPreviewCache[currentTable] = preview;
    return preview;
  } catch (error) {
    return fallbackPreview;
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

function resolveLayoutRows(currentTable, table) {
  const source = LAYOUT_PREVIEW_SOURCES.find((item) => item.file === currentTable)
    || LAYOUT_PREVIEW_SOURCES.find((item) => currentTable.endsWith("-local.txt") && table.normalizedHex.startsWith(item.signature));
  const display = source ? source.display : "";

  if (!display || !table || !table.rows || !table.columns) {
    return [];
  }

  const rows = [];

  for (let row = 0; row < table.rows; row += 1) {
    rows.push(display.slice(row * table.columns, (row + 1) * table.columns));
  }

  return rows;
}

function getDefaultLayoutPreview(settings) {
  const config = settings && settings.numvars ? (settings.numvars[NUM.HARDWARE_CONFIGURATION] || 0) : 0;
  const wcType = config & HW.WC_MASK;
  const fileName = wcType === HW.WC_24H ? "wc24h-tables-de.txt" : "wc12h-tables-de.txt";
  const source = LAYOUT_PREVIEW_SOURCES.find((item) => item.file === fileName);

  if (!source) {
    return {
      file: fileName,
      table: null,
      rows: fallbackWordclockRows
    };
  }

  const columns = fileName.indexOf("wc24h-") === 0 ? 18 : 11;
  const rows = [];

  for (let offset = 0; offset < source.display.length; offset += columns) {
    rows.push(source.display.slice(offset, offset + columns));
  }

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
  const itIsMode = !!(minuteEntry.flags & MDF_IT_IS_1);
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
  const fullOrHalfHour = minute === 0 || minute === 30;

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

    if (!fullOrHalfHour && isItIsWord && ((!showItIs) || (!itIsMode))) {
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

  const config = settings && settings.numvars ? (settings.numvars[NUM.HARDWARE_CONFIGURATION] || 0) : 0;
  const current = settings && settings.tmvars ? settings.tmvars[0] : {};
  const minute = Number(current.minute || 0);
  const is12hLayout = isTwelveHourLayout(layoutPreview, config);
  const activeCount = is12hLayout ? minute % 5 : 0;

  corners.classList.toggle("is-hidden", !is12hLayout);

  corners.querySelectorAll(".wordclock-corner").forEach((corner, index) => {
    corner.classList.toggle("is-active", index < activeCount);
  });
}

function isTwelveHourLayout(layoutPreview, config) {
  if (layoutPreview && typeof layoutPreview.file === "string" && layoutPreview.file.indexOf("wc12h-") === 0) {
    return true;
  }

  return (config & HW.WC_MASK) === HW.WC_12H;
}

function decodeHardware(config) {
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
    await fetch("/api/ambilight_online_set?value=" + desired, { cache: "no-store" });
  } catch (error) {
  }
}

function isDfplayerOnline(settings, debugOverrides) {
  return debugOverrides.dfplayer === "on" ? true : !!settings.numvars[NUM.DFPLAYER_IS_UP];
}

function hasTftDisplay(config, debugOverrides) {
  if (debugOverrides.tft === "on") {
    return true;
  }

  return (config & HW.LED_MASK) === HW.LED_TFT_RGB;
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

  const led = config & HW.LED_MASK;

  switch (led) {
    case HW.LED_SK6812_RGBW:
      return {
        hasColor: true,
        whiteChannel: true,
        mode: "rgbw",
        label: "RGBW",
        note: "RGBW-Hardware erkannt. RGB- und Weißkanal sind verfügbar."
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
          ? "TFT-Hardware erkannt. TFT-Optionen sind verfügbar, der Weißkanal bleibt ausgeblendet."
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

function getFsUploadTargets(config) {
  const wc = config & HW.WC_MASK;
  const led = config & HW.LED_MASK;
  const targets = {
    icon: "",
    weather: "",
    tables: "",
    display: ""
  };

  switch (wc) {
    case 0:
      targets.icon = "wc24h-icon.txt";
      targets.weather = "wc24h-weather.txt";
      targets.tables = "wc24h-tables-local.txt";
      targets.display = led === HW.LED_TFT_RGB ? "wc24h-display-local.txt" : "";
      break;
    case 8:
      targets.icon = "wc12h-icon.txt";
      targets.weather = "wc12h-weather.txt";
      targets.tables = "wc12h-tables-local.txt";
      targets.display = led === HW.LED_TFT_RGB ? "wc12h-display-local.txt" : "";
      break;
    case 16:
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
    0: "Normal",
    1: "Sekunden",
    2: "Datum",
    3: "Temperatur",
    4: "Ticker"
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
    0: "Keiner",
    1: "Glocke",
    2: "Sprache"
  }[mode] || String(mode || 0));
}

function localizeDisplayModeName(name) {
  return ({
    Normal: "Normal",
    Seconds: "Sekunden",
    Date: "Datum",
    Temperature: "Temperatur",
    Ticker: "Ticker"
  }[name] || name);
}

function localizeAmbilightModeName(name) {
  return ({
    Clock: "Uhr",
    Rainbow: "Regenbogen"
  }[name] || name);
}

function localizeAnimationName(name) {
  return ({
    None: "Keiner",
    Normal: "Normal",
    Clock: "Uhr",
    Rainbow: "Regenbogen",
    Temperature: "Temperatur",
    Ticker: "Ticker",
    Date: "Datum",
    Seconds: "Sekunden"
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
  document.getElementById("debug-ambilight-select").value = overrides.ambilight;
  document.getElementById("debug-dfplayer-select").value = overrides.dfplayer;
  document.getElementById("debug-color-select").value = overrides.color;
  document.getElementById("debug-tft-select").value = overrides.tft;
}

function applyDebugOverrides() {
  const overrides = {
    ambilight: document.getElementById("debug-ambilight-select").value,
    dfplayer: document.getElementById("debug-dfplayer-select").value,
    color: document.getElementById("debug-color-select").value,
    tft: document.getElementById("debug-tft-select").value
  };

  saveDebugOverrides(overrides);
  document.getElementById("updated-at").textContent = "Overrides aktiv";
  loadData();
}

function resetDebugOverrides() {
  const overrides = {
    ambilight: "auto",
    dfplayer: "auto",
    color: "auto",
    tft: "auto"
  };

  saveDebugOverrides(overrides);
  loadDebugOverridesIntoUi();
  document.getElementById("updated-at").textContent = "Overrides zurückgesetzt";
  loadData();
}

function onOff(value) {
  return value ? "online" : "offline";
}

function formatHalfDegreeValue(value) {
  if (value === null || value === undefined) {
    return "offline";
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
  const white = useRgbw ? Math.round(((color.white || 0) / 63) * 255) : 0;
  return "linear-gradient(90deg, " + rgb + ", rgb(" + white + ", " + white + ", " + white + "))";
}

function applyWordclockTheme(color, useRgbw) {
  const panel = document.querySelector(".wordclock-panel");

  if (!panel) {
    return;
  }

  const ledColor = mixRgbwToCss(color || { red: 63, green: 45, blue: 18, white: 0 }, useRgbw);
  const glowStrong = colorWithAlpha(ledColor, 0.42);
  const glowSoft = colorWithAlpha(ledColor, 0.2);
  const inactive = "rgba(0, 0, 0, 0.7)";
  const cornerIdle = "rgba(0, 0, 0, 0.78)";
  const cornerBorder = "rgba(255, 255, 255, 0.08)";

  panel.style.setProperty("--wc-active-color", ledColor);
  panel.style.setProperty("--wc-glow-strong", glowStrong);
  panel.style.setProperty("--wc-glow-soft", glowSoft);
  panel.style.setProperty("--wc-inactive-color", inactive);
  panel.style.setProperty("--wc-corner-idle", cornerIdle);
  panel.style.setProperty("--wc-corner-border", cornerBorder);
}

function mixRgbwToCss(color, useRgbw) {
  const base = rgb63ToRgb255(color || {});
  const white = useRgbw ? Math.round(((color && color.white) || 0) / 63 * 255) : 0;

  return "rgb(" +
    String(Math.min(255, base.red + white)) + ", " +
    String(Math.min(255, base.green + white)) + ", " +
    String(Math.min(255, base.blue + white)) +
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
  const days = ["So", "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"];
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
  const icons = overlayIconsCache.length ? overlayIconsCache : [selected || ""];
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
    return "Gerätezeit ist derzeit nicht verfügbar.";
  }

  const weekday = ["Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"][current.wday] || "Unbekannt";
  return weekday + ", " +
    pad2(current.day) + "." + pad2(current.month) + "." + current.year +
    " " + pad2(current.hour || 0) + ":" + pad2(current.minute || 0);
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
