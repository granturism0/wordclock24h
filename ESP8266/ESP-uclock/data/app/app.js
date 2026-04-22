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
const APP_VERSION = "1.3.78";
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

const LAYOUT_PREVIEW_SOURCES = [
  {
    file: "wc12h-tables-ch1.txt",
    aliases: ["ch1", "schweizerdeutsch1", "schweizerdeutsch-1", "swiss1", "swiss-1"],
    signature: "ff020a0b17000000000082000384000803010006010803020006020803030002030305040003040304040803050005050504060006060605070005070504080004080704090006090803010000534348",
    display: "ESKISCHAFÜFVIERTUBFZÄÄZWÄNZGSIVORABOHAUBIEPMEISZWÖISDRÜVIERIFÜFIQTSÄCHSISIBNIACHTINÜNIELZÄNIERBEUFIZWÖUFIAMUHR"
  },
  {
    file: "wc12h-tables-ch2.txt",
    aliases: ["ch2", "schweizerdeutsch2", "schweizerdeutsch-2", "swiss2", "swiss-2"],
    signature: "ff020a0b17000000000082000384010004010506000803020006020803030002030305040003040304040803050005050605060006060605070005070604080004080704090006090803010000534348",
    display: "ESKESCHAZÄÄFÖIFCVIERTUZWÄNZGSIVORABOHAUBIEGEEISZWÖISDRÜVIERITFÖIFISÄCHSISEBNIACHTIENÜNILZÄNIERBELFIZWÖLFINAUHR"
  },
  {
    file: "wc12h-tables-ch3.txt",
    aliases: ["ch3", "schweizerdeutsch3", "schweizerdeutsch-3", "swiss3", "swiss-3"],
    signature: "0a0b170000000000820003840008030100070108030200060208030300020303050400020403030408030500050505040600050606050700050705040800050807040900060908030000010000534348",
    display: "ÄSKISCHAFÜFVIERTELFZÄÄZWENZGSIVORABOHALBIEPMISEZWÜISDRÜVIERIFÜFIQTSÄXSICSIBNIACHTINÜNIELZÄÄNIRBELFIZWÖLFIAMUHR"
  },
  {
    file: "wc12h-tables-ch4.txt",
    aliases: ["ch4", "schweizerdeutsch4", "schweizerdeutsch-4", "swiss4", "swiss-4"],
    signature: "0a0b170000000000020003040008030100040104070202070301020307040400050407040500040504030508030600050606050700040707040800030803040807040900050908030000010000534348",
    display: "ESUISCHKZÄÄFÜNFVIERTELWSZWANZIGUHNABHFLNVOORHALBIHZDREIVIERAISPELFSÄGGSTSIIBENÜÜNHUEACHTZÄÄZWAIFÜNFZWÖLFPEXUUR"
  },
  {
    file: "wc12h-tables-ch5.txt",
    aliases: ["ch5", "schweizerdeutsch5", "schweizerdeutsch-5", "swiss5", "swiss-5"],
    signature: "0a0b170000000000820003840100030104070008030200060208030300020303050400030403040408030500050506040600060805060700050706040800040607040900060908030000010000534348",
    display: "ESKISCHAZÄÄFÜFIVIERTELZWANZGSIVORABOHAUBIEGEEISZWÖISDRÜVIERITFÜFIMSECHSIWELFIACHTIENÜNILZÄNIESIEBNIZWÖLFINAUHR"
  },
  {
    file: "wc12h-tables-ch6.txt",
    aliases: ["ch6", "schweizerdeutsch6", "schweizerdeutsch-6", "swiss6", "swiss-6"],
    signature: "0a0b170000000000820003840100040104070202070300050306020308030400050407030500040504070600060606050700040705050800040804040808030900060906050908030000010000534348",
    display: "ÄSUISCHKMEWFÜFIVIERTELWSZWENZIGUHZÄHNILABVORHALBIKFDRIMELFISEGSCHISIEBNIZÄHNIZWEIHACHTIUNINIFÜFIEISZWELFIVIERI"
  },
  {
    file: "wc12h-tables-ch7.txt",
    aliases: ["ch7", "schweizerdeutsch7", "schweizerdeutsch-7", "swiss7", "swiss-7"],
    signature: "0a0b170000000000820003840100040104070008030200060208030300020303050400030403040408030500050507040600060606050700050706040800040807040900060908030000010000534348",
    display: "ESKESCHAZÄÄFÜÜFVIERTELZWÄNZGSIVORABOHALBIEGEEISZWOISDRÜVIERITXFÜFISÄCHSISIBNIACHTIENÜNILZÄNIERBELFIZWÖLFINAUHR"
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
let currentSettingsSnapshot = null;
let currentEepromSettings = null;
let currentUpdateStatus = {};
let currentUpdateTableInfo = {};
let currentNetworkInfo = {};
const layoutPreviewCache = {};
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
let stm32LogTimer = 0;
let stm32LogRefreshInFlight = false;
let settingsImportInProgress = false;
let progressReturnScrollY = null;
let appServiceWorkerRegistration = null;
let appServiceWorkerUpdateApplied = false;
let initialLoadRetryTimer = 0;
let initialLoadAttemptCount = 0;

const INITIAL_LOAD_RETRY_DELAYS_MS = [1800, 3200, 5000];

const DEBUG_STORAGE_KEY = "wordclock-app-debug-overrides";
const MODULE_STORAGE_KEY = "wordclock-app-active-module";
const AMBILIGHT_STORAGE_KEY = "wordclock-app-ambilight-online";
const LAYOUT_PREVIEW_STORAGE_KEY = "wordclock-app-layout-preview";
const LIVE_DISPLAY_COLOR_STORAGE_KEY = "wordclock-app-live-display-color";
const PROGRESS_SCROLL_RESTORE_KEY = "wordclock-progress-scroll-restore";

function bindElementEvent(id, eventName, handler) {
  document.getElementById(id).addEventListener(eventName, handler);
}

function bindDomEvent(target, eventName, handler, options) {
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
  ["update-app-bundle-button", "click", downloadUpdateAppBundle],
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
  ["datetime-save-button", "click", saveDateTime],
  ["learn-ir-button", "click", learnIrRemote],
  ["update-progress-frame", "load", handleProgressFrameLoad],
  ["fs-upload-app-form", "submit", uploadAppBundleFile],
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

document.getElementById("app-version").textContent = "App-Version " + APP_VERSION;
document.getElementById("app-version-card").textContent = APP_VERSION;

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

loadDebugOverridesIntoUi();
clearReloadQueryMarker();
restoreActiveModule();
loadData();
if (!APP_STABILITY_MODE.disableStartupAutoRefresh) {
  startAlignedAutoRefresh();
}
if (!APP_STABILITY_MODE.disableServiceWorkerRegistration) {
  scheduleServiceWorkerRegistration();
}
window.addEventListener("resize", scheduleWordclockSizing);

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
  const phaseOffsetMs = 1000;

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
    void loadData({ auto: true });
    alignedAutoRefreshInterval = window.setInterval(() => {
      void loadData({ auto: true });
    }, intervalMs);
  }, delayToNextTick);
}

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
  if (getCurrentSettingsSnapshot()) {
    if (target === "maintenance") {
      void loadData({ maintenancePriority: true });
    } else if (target === "network" || target === "overlays" || target === "update" || target === "system") {
      void loadData();
    }
  }
  syncLiveDisplayColorPolling(getCurrentSettingsSnapshot());
  syncStm32LogPolling();
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
}

async function loadData(options) {
  const opts = options || {};
  if (opts.auto && hasUnsavedEdits) {
    announceStatus("Automatische Aktualisierung pausiert, bis ungespeicherte Änderungen gespeichert sind", "warn");
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
    announceStatus("Wartet auf Daten...", "warn");
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
    renderOverlayRows(settings);
    renderTimerRows(settings, false);
    renderTimerRows(settings, true);
    hasUnsavedEdits = false;
    announceStatus("Aktualisiert " + new Date().toLocaleTimeString("de-CH"));

    void loadSecondaryData(requestId, settings, coreData, debugOverrides, opts);
  } catch (error) {
    if (!getCurrentSettingsSnapshot()) {
      if (handleInitialLoadPending(error, opts)) {
        return;
      }
      announceStatus("Daten konnten nicht geladen werden", "error");
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

function handleInitialLoadPending(error, options) {
  const opts = options || {};
  const isPendingInitialData = !!(error && error.message === "initial-data-pending");
  const retryIndex = initialLoadAttemptCount;

  if (!isPendingInitialData || retryIndex >= INITIAL_LOAD_RETRY_DELAYS_MS.length) {
    return false;
  }

  initialLoadAttemptCount += 1;
  announceStatus("Wartet auf Daten...", "warn");
  clearInitialLoadRetry();
  initialLoadRetryTimer = window.setTimeout(() => {
    initialLoadRetryTimer = 0;
    void loadData({ ...opts, initialRetry: true });
  }, INITIAL_LOAD_RETRY_DELAYS_MS[retryIndex]);
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
  const activeModule = getActiveModuleName();
  const maintenanceActive = activeModule === "maintenance" || !!opts.maintenancePriority;
  const systemActive = activeModule === "system";
  const networkActive = activeModule === "network";
  const overlaysActive = activeModule === "overlays";
  const updateActive = activeModule === "update";

  // Secondary data is intentionally additive:
  // update/meta information may fail without taking down maintenance/files/network/overlay data.

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

  try {
    const updateTableInfo = await settleFetchJson(getUpdateTableFilesUrl(), getNormalizedUpdateTableInfo(), CONNECTION_STABILITY.updateTableInfoTimeoutMs, CONNECTION_STABILITY.fastReadAttempts);
    if (requestId !== loadRequestSerial) {
      return;
    }
    setCurrentUpdateTableInfo(updateTableInfo);
  } catch (error) {
    console.warn("Update table info load failed", error);
  }

  try {
    refreshUpdateUi(settings, coreData, debugOverrides);
  } catch (error) {
    console.warn("Update UI refresh failed", error);
  }

  if (activeModule === "main") {
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

  if (systemActive || maintenanceActive) {
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

function getActiveModuleName() {
  const activeSection = document.querySelector(".module-section.is-active");
  return activeSection ? (activeSection.dataset.module || "main") : "main";
}

function updateStm32Log(logData) {
  const meta = document.getElementById("stm32-log-meta");
  const output = document.getElementById("stm32-log-output");
  const lines = logData && Array.isArray(logData.lines) ? logData.lines : [];
  const count = typeof (logData && logData.count) === "number" ? logData.count : lines.length;

  if (!lines.length) {
    meta.textContent = "Noch keine STM32-Logs vorhanden.";
    output.textContent = "Noch keine STM32-Logs vorhanden.";
    return;
  }

  meta.textContent = count + " Log-Zeile" + (count === 1 ? "" : "n") + " im Puffer.";
  output.textContent = lines.join("\n");
  output.scrollTop = output.scrollHeight;
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

  beginButtonFeedback(button, "lädt...");

  try {
    await fetchStm32Log(false);
    finishButtonFeedback(button, "Logs neu laden", "success", "geladen");
  } catch (error) {
    announceStatus("STM32-Logs konnten nicht geladen werden", "error");
    finishButtonFeedback(button, "Logs neu laden", "error", "Fehler");
  }
}

async function clearStm32Log() {
  const button = document.getElementById("stm32-log-clear-button");

  if (!window.confirm("STM32-Logbuch wirklich leeren?")) {
    return;
  }

  beginButtonFeedback(button, "leert...");

  try {
    await apiFetch(getStm32LogClearUrl());
    updateStm32Log({ count: 0, lines: [] });
    announceStatus("STM32-Logbuch wurde geleert", "ok");
    finishButtonFeedback(button, "Logs leeren", "success", "geleert");
  } catch (error) {
    announceStatus("STM32-Logbuch konnte nicht geleert werden", "error");
    finishButtonFeedback(button, "Logs leeren", "error", "Fehler");
  }
}

async function toggleDisplayPower() {
  const button = document.getElementById("display-toggle-button");
  await runStateToggleButton(button, getDisplayPowerSetUrl(), {
    currentValue: () => (document.getElementById("display-power").textContent.trim() === "an" ? "on" : "off"),
    idleText: button.dataset.restoreText || "Display umschalten",
    successText: (next) => (next === "on" ? "eingeschaltet" : "ausgeschaltet"),
    errorText: "Display konnte nicht geschaltet werden"
  });
}

async function toggleAmbilightPower() {
  const button = document.getElementById("ambilight-toggle-button");
  await runStateToggleButton(button, getAmbilightPowerSetUrl(), {
    currentValue: () => (document.getElementById("ambilight-power").textContent.trim() === "an" ? "on" : "off"),
    idleText: button.dataset.restoreText || "Ambilight umschalten",
    successText: (next) => (next === "on" ? "eingeschaltet" : "ausgeschaltet"),
    errorText: "Ambilight konnte nicht geschaltet werden"
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
  await runValueSave("brightness-save-button", getDisplayBrightnessSetUrl(), value, "Helligkeit speichern", "Helligkeit konnte nicht gespeichert werden");
}

async function toggleAutoBrightness() {
  const button = document.getElementById("auto-brightness-button");
  await runStateToggleButton(button, getAutoBrightnessSetUrl(), {
    idleText: button.dataset.restoreText || "Automatische Helligkeit",
    errorText: "Automatische Helligkeit konnte nicht geschaltet werden"
  });
}

async function togglePermanentItIs() {
  const button = document.getElementById("display-it-is-button");
  await runStateToggleButton(button, getDisplayItIsSetUrl(), {
    idleText: button.dataset.restoreText || "„ES IST“ dauerhaft anzeigen",
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
  const meta = getDisplayFormUiMeta(settings).display;
  setActionToggleButton("display-it-is-button", "„ES IST“ deaktivieren", "„ES IST“ dauerhaft anzeigen", meta.itIsActive);
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
  setActionToggleButton("network-summertime-button", "Sommerzeit-Berücksichtigung deaktivieren", "Sommerzeit berücksichtigen", meta.summertime);

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
    ? "Ungültiges Dateiformat – keine gültige WordClock-Sicherungsdatei."
    : error && error.message === "unsupported-backup-version"
      ? "Inkompatible Backup-Version – Datei mit einer neueren App erstellt."
      : "Einstellungen konnten nicht importiert werden.";
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
    setSettingsBackupNote("Einstellungen wurden exportiert.", "ok");
    finishButtonFeedback(button, "Einstellungen exportieren", "success", "exportiert");
  } catch (error) {
    setSettingsBackupNote("Einstellungen konnten nicht exportiert werden.", "error");
    finishButtonFeedback(button, "Einstellungen exportieren", "error", "Fehler");
  }
}

async function importSettingsBackup() {
  const input = document.getElementById("settings-import-file-input");
  const button = document.getElementById("settings-import-button");
  const file = input && input.files && input.files[0];

  if (!file) {
    setSettingsBackupNote("Bitte zuerst eine Sicherungsdatei auswählen.", "warn");
    return;
  }

  beginButtonFeedback(button, "importiert...");

  try {
    const importedState = await loadImportedBackupState(file);

    if (!window.confirm("Einstellungen aus „" + file.name + "“ jetzt importieren?")) {
      finishButtonFeedback(button, "Einstellungen importieren");
      return;
    }

    setSettingsBackupNote("Starte Wiederherstellung der Sicherung...");
    hasUnsavedEdits = false;
    await applySettingsBackup(importedState);
    finishButtonFeedback(button, "Einstellungen importieren", "success", "importiert");
  } catch (error) {
    setSettingsBackupNote(getSettingsBackupImportErrorMessage(error), "error");
    finishButtonFeedback(button, "Einstellungen importieren", "error", "Fehler");
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
      note: "Prüfe importierte Einstellungen...",
      reloadOnly: true,
      pauseMs: 600
    },
    {
      note: "Prüfe Display, Klima, Overlays und Timer...",
      run: () => retryImportedSectionsIfNeeded(settings)
    },
    {
      note: "Lade aktualisierte Gerätedaten neu...",
      reloadOnly: true,
      pauseMs: 400
    },
    {
      when: !!network,
      note: "Übernehme Netzwerk-Einstellungen abschließend...",
      run: () => importNetworkSettings(network),
      reload: true,
      pauseMs: 400
    },
    {
      when: !!network,
      note: "Prüfe Netzwerk-Einstellungen erneut...",
      run: () => retryImportedNetworkSettingsIfNeeded(network)
    },
    {
      note: "Warte auf die Übernahme der Netzwerkeinstellungen...",
      pauseMs: 400
    },
    {
      when: !!climate,
      note: "Übernehme Sensor-Korrekturen abschließend...",
      run: () => importSensorCorrectionSettings(climate),
      reload: true,
      pauseMs: 600
    },
    {
      when: !!climate,
      note: "Prüfe Sensor-Korrekturen erneut...",
      run: () => rerunSensorCorrectionImportIfNeeded(climate)
    },
    {
      note: "Schreibe kritische Einstellungen dauerhaft...",
      run: () => finalizeImportedCriticalPersistenceSettings(settings)
    },
    {
      when: !!climate,
      note: "Schreibe Temperatur-Korrekturen endgültig...",
      run: () => finalizeTemperatureCorrectionPersistence(climate)
    },
    {
      note: "Warte, bis Einstellungen dauerhaft gespeichert sind...",
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
    "Übernehme Sensor-Korrekturen erneut...",
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

  setSettingsBackupNote("Import abgeschlossen. STM32 wird jetzt automatisch neu gestartet...");
  announceStatus("Import abgeschlossen. STM32 wird automatisch neu gestartet", "warn");
  await apiFetch(getMaintenanceResetStm32Url());
  await sleep(4500);
  try {
    if (climate) {
      await finalizeTemperatureCorrectionPersistence(climate);
    }
    setSettingsBackupNote("STM32 wurde neu gestartet. Lade Daten neu...");
    await loadData();
    setSettingsBackupNote("Import abgeschlossen. App wird neu geladen...", "success");
    announceStatus("Import abgeschlossen. App wird neu geladen", "ok");
  } catch (error) {
    setSettingsBackupNote("Import abgeschlossen. Uhr startet neu, App wird aktualisiert...", "success");
    announceStatus("Import abgeschlossen. Verbindung wird nach dem Neustart erneut aufgebaut", "ok");
  }
  setTimeout(reloadAppPage, 1200);
}

function buildPrimaryImportStages(executionState) {
  const settings = executionState && executionState.settings ? executionState.settings : {};
  const assets = executionState && executionState.assets ? executionState.assets : {};
  return [
    { note: "Übernehme Wartungs-Einstellungen...", run: () => importMaintenanceSettings(settings.maintenance), pauseMs: 250 },
    { note: "Stelle Dateien und Assets wieder her...", run: () => restoreBackupAssets(assets, settings), pauseMs: 250 },
    { note: "Übernehme Display-Einstellungen...", run: () => importDisplaySettings(settings.display), pauseMs: 250 },
    { note: "Übernehme Klima- und Wetter-Einstellungen...", run: () => importClimateSettings(settings.climate), pauseMs: 250 },
    { note: "Übernehme Animations-Einstellungen...", run: () => importAnimationSettings(settings.animations), pauseMs: 250 },
    { note: "Übernehme TFT-Einstellungen...", run: () => importTftSettings(settings.tft), pauseMs: 250 },
    { note: "Übernehme Ambilight-Einstellungen...", run: () => importAmbilightSettings(settings.ambilight), pauseMs: 250 },
    { note: "Übernehme DFPlayer-Einstellungen...", run: () => importDfplayerSettings(settings.dfplayer), pauseMs: 250 },
    { note: "Übernehme Overlays...", run: () => importOverlaySettings(settings.overlays), pauseMs: 500 },
    { note: "Übernehme Timer...", run: () => importTimerSettings(settings.timers), pauseMs: 800 }
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
      note: "Übernehme Display-Einstellungen erneut...",
      run: () => importDisplaySettings(settings.display)
    },
    {
      shouldRetry: climateImportNeedsRetry(settings.climate, snapshot),
      note: "Übernehme Klima- und Wetter-Einstellungen erneut...",
      run: () => importClimateSettings(settings.climate)
    },
    {
      shouldRetry: overlaysImportNeedsRetry(settings.overlays, snapshot),
      note: "Übernehme Overlays erneut...",
      run: () => importOverlaySettings(settings.overlays)
    },
    {
      shouldRetry: timersImportNeedsRetry(settings.timers, snapshot),
      note: "Übernehme Timer erneut...",
      run: () => importTimerSettings(settings.timers)
    }
  ];
}

function buildClimateFinalizationRetryStages(retryState) {
  const climate = retryState && retryState.settings ? retryState.settings.climate : null;
  const snapshot = retryState && retryState.snapshot ? retryState.snapshot : null;

  return [
    {
      shouldRetry: sensorCorrectionsImportNeedsRetry(climate, snapshot),
      note: "Übernehme Temperatur-Korrekturen erneut...",
      run: () => importSensorCorrectionSettings(climate),
      options: { pauseMs: 1200, reload: true }
    },
    {
      shouldRetry: rtcCorrectionImportNeedsRetry(climate, snapshot),
      note: "Übernehme RTC-Korrektur abschließend...",
      run: () => importRtcCorrectionSetting(climate),
      options: { pauseMs: 1800, reload: true }
    },
    {
      shouldRetry: rtcCorrectionImportNeedsRetry(climate, snapshot),
      note: "Übernehme RTC-Korrektur erneut...",
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
      note: "Übernehme Netzwerk- und Zeiteinstellungen erneut...",
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
      note: "Übernehme Zeitserver- und Uhrzeit-Einstellungen erneut...",
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
      note: "Übernehme Update-Host und Update-Pfad erneut...",
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

  setSettingsBackupNote("Übernehme Zeitserver- und Uhrzeit-Einstellungen abschließend...");
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
    setSettingsBackupNote("Schreibe Temperatur-Korrekturen abschließend...");
    await runTimedImportPhase(() => importSensorCorrectionSettings(climate), 1000);
  }

  if (network) {
    await finalizeImportedNetworkTimeSettings(network);
  }

  if (maintenance) {
    setSettingsBackupNote("Übernehme Update-Host und Update-Pfad abschließend...");
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
  await apiFetchValue(getDisplayModeSetUrl(), Number(display.mode || 0));
  await apiFetchValue(getDisplayUseRgbwSetUrl(), display.use_rgbw ? "on" : "off");
  await apiFetchValue(getAutoBrightnessSetUrl(), display.automatic_brightness ? "on" : "off");
  await sleep(150);
  await apiFetchValue(getDisplayBrightnessSetUrl(), Number(display.brightness || 0));
  await apiFetchValue(getDisplayItIsSetUrl(), display.permanent_it_is ? "on" : "off");
  await apiFetchValue(getTickerSetUrl(), display.ticker_text || "");
  await apiFetchValue(getDateTickerFormatSetUrl(), display.date_ticker_format || "");
  await apiFetchValue(getTickerDecelerationSetUrl(), Number(display.ticker_deceleration || 0));
  await saveImportedColor(getDisplayColorSetUrl(), display.color);
  await saveImportedDimCurve(getDisplayDimLevelSetUrl(), display.dim_curve);
}

async function importMaintenanceSettings(maintenance) {
  if (!maintenance) {
    return;
  }

  setSettingsBackupNote("Importiere Wartungs- und Update-Einstellungen...");

  await apiFetchValue(getUpdateHostSetUrl(), maintenance.update_host || "");
  await sleep(500);
  await apiFetchValue(getUpdatePathSetUrl(), maintenance.update_path || "");
  await sleep(500);
}

async function importClimateSettings(climate) {
  if (!climate) {
    return;
  }

  setSettingsBackupNote("Importiere Klima- und Wetter-Einstellungen...");

  await apiFetchValue(getWeatherAppIdSetUrl(), climate.weather_appid || "");
  await apiFetchValue(getWeatherCitySetUrl(), climate.weather_city || "");
  await apiFetchQuery(getWeatherCoordinatesSetUrl(), {
    lon: climate.weather_lon || "",
    lat: climate.weather_lat || ""
  });
  await apiFetchValue(getLdrMinValueSetUrl(), Number(climate.ldr_min || 0));
  await apiFetchValue(getLdrMaxValueSetUrl(), Number(climate.ldr_max || 0));
}

async function importSensorCorrectionSettings(climate) {
  if (!climate) {
    return;
  }

  setSettingsBackupNote("Importiere Sensor-Korrekturen...");
  await apiFetchValue(getTemperatureRtcCorrectionSetUrl(), Math.max(0, Math.min(10, Number(climate.rtc_temp_correction || 0))));
  await sleep(400);
  await apiFetchValue(getTemperatureDs18xxCorrectionSetUrl(), Math.max(0, Math.min(10, Number(climate.ds18xx_temp_correction || 0))));
  await sleep(400);
}

async function importRtcCorrectionSetting(climate) {
  if (!climate) {
    return;
  }

  await apiFetchValue(getTemperatureRtcCorrectionSetUrl(), Math.max(0, Math.min(10, Number(climate.rtc_temp_correction || 0))));
}

async function finalizeTemperatureCorrectionPersistence(climate) {
  if (!climate) {
    return;
  }

  const rtcCorrection = Math.max(0, Math.min(10, Number(climate.rtc_temp_correction || 0)));
  const ds18xxCorrection = Math.max(0, Math.min(10, Number(climate.ds18xx_temp_correction || 0)));
  setSettingsBackupNote("Übernehme Sensor-Korrekturen als letzten Persistenzschritt...");
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
  for (let idx = 0; idx < 32; idx += 1) {
    try {
      await apiFetchQuery(getOverlayDeleteUrl(), { idx });
    } catch (_) {}
  }

  for (let idx = 0; idx < items.length; idx += 1) {
    const entry = items[idx] || {};
    await apiFetchQuery(getOverlaySetUrl(), {
      idx,
      active: entry.active ? "on" : "off",
      type: Number(entry.type || 0),
      value: entry.value || "",
      interval: Number(entry.interval || 0),
      duration: Number(entry.duration || 0),
      date_code: Number(entry.date_code || 0),
      month: Number(entry.month || 0),
      day: Number(entry.day || 0),
      days: Number(entry.days || 1)
    });
  }
}

async function importTimerSettings(timers) {
  if (!timers) {
    return;
  }

  setSettingsBackupNote("Importiere Timer...");

  const buildTimerParams = (entry) => ({
    idx: Number(entry.idx || 0),
    active: entry.active ? "on" : "off",
    switch_on: entry.switch_on ? "on" : "off",
    from: Number(entry.from || 0),
    to: Number(entry.to || 0),
    hour: Number(entry.hour || 0),
    minute: Number(entry.minute || 0)
  });

  await importIndexedEntries(
    getTimerSetUrl(),
    timers.display || [],
    8,
    (idx) => ({ idx, active: false, switch_on: false, from: 0, to: 0, hour: 0, minute: 0 }),
    buildTimerParams
  );

  await importIndexedEntries(
    getAmbilightTimerSetUrl(),
    timers.ambilight || [],
    8,
    (idx) => ({ idx, active: false, switch_on: false, from: 0, to: 0, hour: 0, minute: 0 }),
    buildTimerParams
  );
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
    ["Farbanimation", previewMeta.animationLabel],
    ["Gespeicherte Display-Farbe", formatRgbwColor(previewMeta.staticColor)],
    ["Live-Farbe vom Gerät", formatRgbwColor(currentLiveDisplayColor)],
    ["Vorschau-Layout", previewMeta.layoutFile]
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
  updateDfplayerControlsFromMeta(getDfplayerControlUiMeta(settings, debugOverrides));
}

function updateDfplayerControlsFromMeta(meta) {
  const isUp = meta.online;
  const note = document.getElementById("dfplayer-note");

  document.getElementById("dfplayer-panel").classList.toggle("is-hidden", !isUp);
  note.textContent = isUp ? "DFPlayer ist online." : "DFPlayer ist offline und wird ausgeblendet.";

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
        '<div class="slider-row"><label class="label" for="can-dec-' + item.idx + '">Verzögerung</label><strong id="can-dec-value-' + item.idx + '" class="value-pill">' + escapeHtml(String(item.deceleration || 0)) + '</strong></div>' +
        '<input id="can-dec-' + item.idx + '" type="range" min="0" max="15" value="' + escapeHtml(String(item.deceleration || 0)) + '">' +
        '<div class="profile-actions">' +
          '<button class="button" type="button" data-can-default="' + item.idx + '">Standard</button>' +
          '<button class="button primary" type="button" data-can-save="' + item.idx + '">Profil speichern</button>' +
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
        '<label class="field"><span class="label">Verzögerung</span><input id="alm-dec-' + item.idx + '" type="range" min="0" max="15" value="' + escapeHtml(String(item.deceleration || 0)) + '"></label>' +
        '<div class="hero-actions">' +
          '<button class="button" type="button" data-alm-default="' + item.idx + '">Standard</button>' +
          '<button class="button primary" type="button" data-alm-save="' + item.idx + '">Profil speichern</button>' +
        "</div>" +
      "</div>" +
    "</section>"
  )).join("") : '<p class="hint">Für die erkannte Hardware gibt es keine konfigurierbaren Ambilight-Modi.</p>';

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
          '<button class="button" type="button" data-fs-show="' + escapeHtml(file.name || "") + '">Anzeigen</button>' +
          '<button class="button" type="button" data-fs-delete="' + escapeHtml(file.name || "") + '">Löschen</button>' +
      "</div>" +
    "</section>"
  )).join("") : '<p class="hint">Noch keine Dateien im LittleFS gefunden.</p>';

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
  setUploadFormSupported("fs-upload-app-form", uploadMeta.appBundleSupported, "PWA-Upload für App-Pakete wird von dieser Firmware noch nicht unterstützt.");
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
      label.textContent = "TFT-Sonderfall: " + fileName;
    } else {
      label.textContent = fileName;
    }
  }
}

function updateUploadFormActions() {
  const actions = [
    ["fs-upload-app-form", getAppBundleUploadUrl()],
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
    ["#fs-upload-app-form input[type=\"file\"]", getAppBundleUploadAccept()],
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
    : '<option value="">keine STM32-Dateien gefunden</option>';

  const tableField = document.getElementById("update-table-field");
  const tableSelect = document.getElementById("update-table-select");
  const tableButton = document.getElementById("update-table-button");
  const assetsButton = document.getElementById("update-assets-button");
  const appBundleButton = document.getElementById("update-app-bundle-button");
  const serverFilesBlock = document.getElementById("update-server-files-block");
  const tableFiles = serverFilesMeta.tableFiles;
  const currentTable = serverFilesMeta.currentTable;

  tableField.classList.toggle("is-hidden", !serverFilesMeta.tableAvailable);
  tableSelect.innerHTML = tableFiles.length
    ? tableFiles.map((file) => '<option value="' + escapeHtml(file) + '"' + (file === currentTable ? " selected" : "") + ">" + escapeHtml(file) + "</option>").join("")
    : '<option value="">keine Layout-Tabellen gefunden</option>';
  tableButton.classList.toggle("is-hidden", !serverFilesMeta.tableActionSupported);
  assetsButton.classList.toggle("is-hidden", !serverFilesMeta.assetsActionSupported);
  appBundleButton.classList.toggle("is-hidden", !serverFilesMeta.appBundleActionSupported);
  serverFilesBlock.classList.toggle("is-hidden", !serverFilesMeta.anyActionSupported);

  document.getElementById("update-release-notes").innerHTML = view.releaseNotes || "<p>Keine Release Notes vom Server gelesen.</p>";
  document.getElementById("update-esp-button").disabled = !view.canUpdate;
  document.getElementById("update-stm32-button").disabled = !view.stm32Files.length;
  tableButton.disabled = !serverFilesMeta.tableAvailable || !serverFilesMeta.tableActionSupported;
  assetsButton.disabled = !serverFilesMeta.assetsAvailable;
  appBundleButton.disabled = !serverFilesMeta.appBundleAvailable;

}

function updateLocalUpdateControls(updateStatus) {
  const meta = getUpdateModuleMeta(updateStatus).localUpdate;
  const note = document.getElementById("local-update-note");
  note.textContent = !meta.supported
    ? (meta.message || "Lokales Update ist bei dieser ESP-Flashgröße nicht verfügbar.")
    : (!meta.localEspSupported || !meta.localStm32Supported)
      ? "Einige lokale PWA-Updatepfade werden von dieser Firmware noch nicht unterstützt."
      : (meta.message || "ESP- oder STM32-Datei auswählen und direkt lokal hochladen.");
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
      ? "Aktuell: " + parts.join(" | ")
      : "Karte und Suche stehen für die Standortwahl bereit."
  };
}

function getMaintenanceUiMeta(settings, eepromSettings, fsInfo, files) {
  const fsInfoItems = [];

  if (fsInfo && fsInfo.total !== undefined) {
    fsInfoItems.push(
      ["Gesamt", formatBytes(fsInfo.total)],
      ["Belegt", formatBytes(fsInfo.used)],
      ["Blockgröße", formatBytes(fsInfo.block_size)],
      ["Seitengröße", formatBytes(fsInfo.page_size)],
      ["Max. offene Dateien", String(fsInfo.max_open_files)],
      ["Max. Pfadlänge", String(fsInfo.max_path_length)]
    );
  }

  return {
    updateHost: settings.strvars[STR.UPDATE_HOST] || "",
    updatePath: settings.strvars[STR.UPDATE_PATH] || "",
    infoItems: [
      ["WLAN-Client", eepromSettings && eepromSettings.ssid ? eepromSettings.ssid : "-"],
      ["Zugangspunkt", eepromSettings && eepromSettings.ap_ssid ? eepromSettings.ap_ssid : "-"],
      ["Bootmodus", eepromSettings && eepromSettings.boot_as_ap ? "Access Point" : "WLAN-Client"]
    ],
    fsInfoItems: fsInfoItems.length ? fsInfoItems : [["LittleFS", "keine Daten"]],
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
      ["Automatische Helligkeit", autoBrightness ? "ein" : "aus"],
      ["Aktueller LDR-Wert", String(settings.numvars[NUM.LDR_RAW_VALUE] || 0)],
      ["Minimum", String(settings.numvars[NUM.LDR_MIN_VALUE] || 0)],
      ["Maximum", String(settings.numvars[NUM.LDR_MAX_VALUE] || 0)]
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
  const buttonText = "Preset anwenden und speichern";

  beginButtonFeedback(button, "speichert...");
  applyDimPreset(prefix);

  try {
    await persistDimCurve(prefix, button, buttonText);
  } catch (error) {
    announceStatus("Preset konnte nicht angewendet werden", "error");
    finishButtonFeedback(button, buttonText, "error", "Fehler");
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

  bindDataAction(root, "data-overlay-save", saveOverlay);
  bindDataAction(root, "data-overlay-display", displayOverlay);
  bindDataAction(root, "data-overlay-delete", deleteOverlay);
  bindIndexedSuffixAction(root, "[id^='ov-type-']", "change", (idx) => {
    void handleOverlayTypeChange(idx);
  });
  bindIndexedSuffixAction(root, "[id^='ov-datecode-']", "change", (idx) => updateOverlayRowVisibility(idx));
  bindIndexedSuffixAction(root, "[id^='ov-month-'], [id^='ov-day-']", "input", (idx) => updateOverlayRowVisibility(idx));
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

  setActionToggleButton("sync-ambilight-button", "Ambilight-Synchronisierung deaktivieren", "Ambilight synchronisieren", meta.syncAmbilight);
  setActionToggleButton("sync-markers-button", "Marker-Synchronisierung deaktivieren", "Marker synchronisieren", meta.syncMarkers);
  setActionToggleButton("fade-clock-seconds-button", "Weiches Ausblenden deaktivieren", "Sekunden weich ausblenden", meta.fadeClockSeconds);
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
  await runValueSave("display-mode-save-button", getDisplayModeSetUrl(), value, "Display-Modus speichern", "Display-Modus konnte nicht gespeichert werden");
}

async function saveTickerText() {
  const value = document.getElementById("ticker-text-input").value;
  await runValueSave("ticker-save-button", getTickerSetUrl(), value, "Ticker speichern", "Ticker konnte nicht gespeichert werden");
}

async function saveDateTickerFormat() {
  const value = document.getElementById("date-format-input").value;
  await runValueSave("date-format-save-button", getDateTickerFormatSetUrl(), value, "Datumsformat speichern", "Datumsformat konnte nicht gespeichert werden");
}

async function saveTickerDeceleration() {
  const input = document.getElementById("ticker-deceleration-input");
  const value = Math.max(0, Math.min(255, Number(input.value || 0)));
  input.value = String(value);
  await runValueSave("ticker-deceleration-save-button", getTickerDecelerationSetUrl(), value, "Ticker-Verzögerung speichern", "Ticker-Verzögerung konnte nicht gespeichert werden");
}

async function testDisplay() {
  const button = document.getElementById("test-display-button");
  const restoreText = "Displaytest starten";
  const maxRunTimeMs = 45000;

  beginButtonFeedback(button, "läuft...");

  try {
    await apiFetch(getDisplayTestUrl());
    announceStatus("Displaytest läuft", "ok");
    window.setTimeout(() => {
      finishButtonFeedback(button, restoreText);
    }, maxRunTimeMs);
  } catch (error) {
    announceStatus("Displaytest konnte nicht gestartet werden", "error");
    finishButtonFeedback(button, restoreText, "error", "Fehler");
  }
}

async function saveWeatherAppId() {
  const value = document.getElementById("weather-appid-input").value || "";
  await runValueSave("weather-appid-save-button", getWeatherAppIdSetUrl(), value, "API-Schlüssel speichern", "API-Schlüssel konnte nicht gespeichert werden");
}

async function saveWeatherCity() {
  const value = document.getElementById("weather-city-input").value || "";
  await runValueSave("weather-city-save-button", getWeatherCitySetUrl(), value, "Ort speichern", "Ort konnte nicht gespeichert werden");
}

async function saveWeatherCoordinates() {
  const lon = document.getElementById("weather-lon-input").value || "";
  const lat = document.getElementById("weather-lat-input").value || "";
  await runQuerySave("weather-coordinates-save-button", getWeatherCoordinatesSetUrl(), { lon, lat }, "Koordinaten speichern", "Koordinaten konnten nicht gespeichert werden");
}

async function getWeatherNow() {
  await runWeatherAction("weather-now-button", getWeatherNowUrl(), "Wetter abrufen", "Wetter konnte nicht angefordert werden");
}

async function getWeatherForecast() {
  await runWeatherAction("weather-forecast-button", getWeatherForecastUrl(), "Wettervorhersage abrufen", "Wettervorhersage konnte nicht angefordert werden");
}

async function runWeatherAction(buttonId, endpoint, buttonText, errorText) {
  await runButtonRequestById(buttonId, {
    busyText: "läuft...",
    idleText: buttonText,
    successText: "angefragt",
    errorText,
    reload: true,
    request: () => apiFetch(endpoint)
  });
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

  beginButtonFeedback(button, "sucht...");
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
      finishButtonFeedback(button, "Suchen", "error", "kein Treffer");
      return;
    }

    const result = results[0];
    const lat = Number(result.lat);
    const lon = Number(result.lon);
    const city = result.display_name || query;

    setWeatherMapSelection(lat, lon, city);
    weatherMap.setView([lat, lon], 11);
    status.textContent = "Ort gefunden und auf der Karte gesetzt.";
    finishButtonFeedback(button, "Suchen", "success", "gefunden");
  } catch (error) {
    status.textContent = "Ortssuche konnte nicht geladen werden.";
    finishButtonFeedback(button, "Suchen", "error", "Fehler");
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

  beginButtonFeedback(button, "liest...");
  status.textContent = "Aktueller Standort wird gelesen...";

  navigator.geolocation.getCurrentPosition(
    (position) => {
      const lat = position.coords.latitude;
      const lon = position.coords.longitude;

      setWeatherMapSelection(lat, lon, document.getElementById("weather-map-city-input").value || "");
      weatherMap.setView([lat, lon], 12);
      status.textContent = "Aktueller Standort gesetzt.";
      reverseLookupWeatherLocation(lat, lon);
      finishButtonFeedback(button, "Aktuellen Standort verwenden", "success", "gesetzt");
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
    finishButtonFeedback(button, "Aktuellen Standort verwenden", "success", "gesetzt");
  } catch (error) {
    status.textContent = "Standort konnte auch näherungsweise nicht ermittelt werden.";
    finishButtonFeedback(button, "Aktuellen Standort verwenden", "error", "Fehler");
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

async function applyWeatherMapSelection() {
  const city = document.getElementById("weather-map-city-input").value || "";
  const lon = document.getElementById("weather-map-lon-input").value || "";
  const lat = document.getElementById("weather-map-lat-input").value || "";

  document.getElementById("weather-city-input").value = city;
  document.getElementById("weather-lon-input").value = lon;
  document.getElementById("weather-lat-input").value = lat;
  document.getElementById("weather-location-preview").textContent = "Aus Karte gewählt: " + (city || "-") + " | " + (lon || "-") + " / " + (lat || "-");

  await runButtonRequestById("weather-map-apply-button", {
    busyText: "übernimmt...",
    idleText: "In Wetter übernehmen",
    successText: "übernommen",
    errorText: "Wetter-Ort und Koordinaten konnten nicht übernommen werden",
    successStatusText: "Wetter-Ort und Koordinaten wurden übernommen",
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

  beginButtonFeedback(button, "lädt...");

  try {
    await loadData();
    announceStatus("WLAN-Liste wurde aktualisiert", "ok");
    finishButtonFeedback(button, "WLANs neu laden", "success", "geladen");
  } catch (error) {
    announceStatus("WLAN-Liste konnte nicht aktualisiert werden", "error");
    finishButtonFeedback(button, "WLANs neu laden", "error", "Fehler");
  }
}

async function saveNetworkClient() {
  const ssid = document.getElementById("network-ssid-select").value || "";
  const key = document.getElementById("network-key-input").value || "";
  await runQueryButtonRequestById("network-client-save-button", {
    endpoint: getNetworkClientSetUrl(),
    query: { ssid, key },
    busyText: "verbindet...",
    idleText: "Als WLAN-Client verbinden",
    successText: "gestartet",
    errorText: "WLAN-Client konnte nicht gesetzt werden",
    successStatusText: "WLAN-Client-Verbindung wurde angestoßen",
    reloadDelayMs: 1500
  });
}

async function saveNetworkAp() {
  const ssid = document.getElementById("network-ap-ssid-input").value || "";
  const key = document.getElementById("network-ap-key-input").value || "";
  await runQueryButtonRequestById("network-ap-save-button", {
    endpoint: getNetworkApSetUrl(),
    query: { ssid, key },
    busyText: "startet...",
    idleText: "Zugangspunkt starten",
    successText: "gestartet",
    errorText: "Zugangspunkt konnte nicht gesetzt werden",
    successStatusText: "Start des Zugangspunkts wurde angestoßen",
    reloadDelayMs: 1500
  });
}

async function saveTimeServer() {
  await runTextSave("network-timeserver-save-button", getNetworkTimeserverSetUrl(), document.getElementById("network-timeserver-input").value || "", "Zeitserver speichern", "Zeitserver konnte nicht gespeichert werden");
}

async function saveTimezone() {
  const input = document.getElementById("network-timezone-input");
  const value = Math.max(-12, Math.min(14, Number(input.value || 0)));
  input.value = String(value);
  await runQuerySave("network-timezone-save-button", getNetworkTimezoneSetUrl(), { value }, "Zeitzone speichern", "Zeitzone konnte nicht gespeichert werden", {
    request: async () => {
      await apiFetchValue(getNetworkTimezoneSetUrl(), value);
      await apiFetch(getNetworkGetTimeUrl());
    }
  });
}

async function toggleSummertime() {
  const button = document.getElementById("network-summertime-button");
  await runStateToggleButton(button, getNetworkSummertimeSetUrl(), {
    idleText: button.dataset.restoreText || "Sommerzeit berücksichtigen",
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

  await runQuerySave("datetime-save-button", getDateTimeSetUrl(), { year, month, day, hour, minute }, "Datum und Uhrzeit speichern", "Datum und Uhrzeit konnten nicht gespeichert werden");
}

async function learnIrRemote() {
  await runSimpleAction("learn-ir-button", getLearnIrUrl(), "IR-Fernbedienung lernen", "IR-Lernmodus konnte nicht gestartet werden", "IR-Lernmodus gestartet");
}

async function getNetTime() {
  await runSimpleAction("network-nettime-button", getNetworkGetTimeUrl(), "Netzzeit abrufen", "Netzzeit konnte nicht angefordert werden", "Netzzeit angefordert");
}

async function runWps() {
  await runSimpleAction("network-wps-button", getNetworkWpsUrl(), "WPS", "WPS konnte nicht gestartet werden", "WPS wurde gestartet");
}

async function saveUpdateHost() {
  await runTextSave("update-host-save-button", getUpdateHostSetUrl(), document.getElementById("update-host-input").value || "", "Update-Host speichern", "Update-Host konnte nicht gespeichert werden");
  await refreshUpdateServerAvailability();
}

async function saveUpdatePath() {
  await runTextSave("update-path-save-button", getUpdatePathSetUrl(), document.getElementById("update-path-input").value || "", "Update-Pfad speichern", "Update-Pfad konnte nicht gespeichert werden");
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

  if (!isBinFileName(file.name)) {
    document.getElementById("local-update-note").textContent = "Falsche ESP-Datei ausgewählt. Erwartet wird eine .bin-Datei.";
    announceStatus("ESP-.bin-Datei erwartet", "error");
    finishButtonFeedback(button, "ESP lokal aktualisieren", "error", "Fehler");
    return;
  }

  button.disabled = true;
  button.textContent = "lädt hoch...";
  stopUpdateProgressPolling();
  announceStatus("ESP-Firmware wird lokal hochgeladen...", "warn");
  document.getElementById("local-update-note").textContent = "ESP-Firmware wird lokal hochgeladen...";
  pendingProgressAction = "esp-local-update";
  pendingProgressButtonId = "local-update-esp-submit-button";
  button.dataset.restoreText = "ESP lokal aktualisieren";
  showRemoteUpdateProgressShell("esp-local-update", "Lokales ESP-Update wird vorbereitet.", pendingProgressButtonId);

  try {
    await uploadRawFile(
      buildUploadUrl(getLocalEspUpdateUrl(), file.name),
      file,
      (loaded, total) => {
        const percent = total ? Math.min(100, Math.round((loaded / total) * 100)) : 0;
        button.textContent = "lädt hoch... " + percent + "%";
        document.getElementById("local-update-note").textContent = "ESP-Firmware wird hochgeladen: " + percent + "%";
        document.getElementById("update-progress-note").textContent = "ESP-Firmware wird hochgeladen: " + percent + "%";
        document.getElementById("updated-at").textContent = document.getElementById("update-progress-note").textContent;
      }
    );
  } catch (error) {
    stopUpdateProgressPolling();
    document.getElementById("local-update-note").textContent = "ESP-Firmware konnte nicht hochgeladen werden: " + (error.message || "unbekannter Fehler");
    button.disabled = false;
    button.textContent = "ESP lokal aktualisieren";
    finishProgressUi(0);
    return;
  }

  button.textContent = "läuft...";
  try {
    await apiFetch(getLocalEspRestartUrl());
  } catch (error) {
  }
  document.getElementById("update-progress-note").textContent = "ESP-Firmware wurde übertragen. Es wird auf den Neustart gewartet.";
  announceStatus("ESP-Firmware wurde übertragen. Es wird auf den Neustart gewartet.", "warn");
  startEspUpdateReconnectWatch(true);
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

  if (!isMatchingLocalStm32File(file.name)) {
    const expected = getExpectedLocalStm32Filename(getCurrentUpdateStatus()) || "passende STM32-.hex-Datei";
    document.getElementById("local-update-note").textContent = "Falsche STM32-Datei ausgewählt. Erwartet wird " + expected + ".";
    announceStatus("Passende STM32-Datei erwartet", "error");
    finishButtonFeedback(button, "STM32 lokal aktualisieren", "error", "Fehler");
    return;
  }

  button.disabled = true;
  button.textContent = "lädt hoch...";
  announceStatus("STM32-Firmware wird lokal hochgeladen...", "warn");
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

function buildUploadUrl(url, fileName) {
  const absoluteUrl = new URL(url, window.location.origin);
  absoluteUrl.searchParams.set("filename", fileName || "");
  return absoluteUrl.pathname + absoluteUrl.search;
}

function setFsActionStatus(message) {
  const node = document.getElementById("fs-action-status");
  if (node) {
    node.textContent = message;
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
  app_bundle_upload_url: "/api/app_bundle_upload",
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
  app_bundle_upload_accept: ".txt,text/plain",
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
const getUpdateDownloadAppBundleUrl = createConfiguredUrlGetter("update_download_app_bundle_url");
const getUpdateDownloadTableBaseUrl = createConfiguredUrlGetter("update_download_table_base_url");
const getAppBundleUploadUrl = createConfiguredUrlGetter("app_bundle_upload_url");
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
const getAppBundleUploadAccept = createConfiguredUrlGetter("app_bundle_upload_accept");
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

function refreshNetworkUi(settings) {
  updateNetworkControls(settings, getCurrentNetworkInfo());
}

function refreshOverlayUi(settings) {
  renderOverlayRows(settings);
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
    ),
    appBundleSupported: !!getAppBundleUploadUrl()
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

function isUpdateAppBundleAvailable(updateStatus) {
  return getUpdateStatusBoolean(updateStatus, "app_bundle_available");
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
      ["ESP-Flash", versionMeta.flashSize ? String(versionMeta.flashSize) + " Bytes" : "-"],
      ["OTA-Update", versionMeta.canUpdate ? "möglich" : "nicht möglich"],
      ["WordClock-Version", versionMeta.wcVersion],
      ["WordClock verfügbar", versionMeta.wcAvailable],
      ["ESP-Version", versionMeta.espVersion],
      ["ESP verfügbar", versionMeta.espAvailable],
      ["App-Version", versionMeta.appVersion],
      ["App verfügbar", versionMeta.appAvailable],
      ["Standard STM32", stm32Default || "-"]
    ]
  };
}

function getUpdateServerFilesMeta(updateStatus, updateTableInfo) {
  const tableFiles = getUpdateTableFilesList(updateTableInfo);
  const currentTable = getUpdateTableCurrentFile(updateTableInfo);
  const tableActionSupported = !!getUpdateDownloadTableBaseUrl();
  const assetsActionSupported = !!getUpdateDownloadAssetsUrl();
  const appBundleActionSupported = !!getUpdateDownloadAppBundleUrl();

  return {
    tableFiles,
    currentTable,
    tableAvailable: tableFiles.length > 0,
    assetsAvailable: areUpdateAssetsAvailable(updateStatus),
    appBundleAvailable: isUpdateAppBundleAvailable(updateStatus),
    tableActionSupported,
    assetsActionSupported,
    appBundleActionSupported,
    anyActionSupported: tableActionSupported || assetsActionSupported || appBundleActionSupported
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
    "Icon-Dateien jetzt wirklich vom Server laden?",
    {
      busyText: "läuft...",
      idleText: "Icon-Dateien laden",
      successText: "fertig",
      errorText: "Icon-Dateien konnten nicht geladen werden",
      successStatusText: "Icon-Dateien geladen",
      reloadDelayMs: 1200,
      request: () => apiFetch(getUpdateDownloadAssetsUrl())
    }
  );
}

async function downloadUpdateAppBundle() {
  if (!window.confirm("App-Paket jetzt wirklich vom Server laden und installieren?")) {
    return;
  }

  const button = document.getElementById("update-app-bundle-button");

  setProgressActionContext("app-bundle-install", "update-app-bundle-button", "App-Paket laden");
  showRemoteUpdateProgressShell("app-bundle-install", "App-Paket wird vom Server geladen...", "update-app-bundle-button");
  beginButtonFeedback(button, "lädt...");
  announceStatus("App-Paket wird vom Server geladen...", "warn");

  try {
    await apiFetch(getUpdateDownloadAppBundleUrl(), {
      timeoutMs: 45000,
      attempts: 1
    });

    button.classList.add("is-busy");
    button.textContent = "installiert...";
    document.getElementById("update-progress-note").textContent = "App-Paket wurde geladen und wird installiert...";
    document.getElementById("updated-at").textContent = "App-Paket wurde geladen und wird installiert...";
    announceStatus("App-Paket wird installiert...", "warn");
    await sleep(250);

    button.classList.add("is-busy");
    button.textContent = "lädt neu...";
    document.getElementById("update-progress-note").textContent = "App-Paket installiert. App wird neu geladen...";
    document.getElementById("updated-at").textContent = "App-Paket installiert. App wird neu geladen...";
    announceStatus("App-Paket installiert. Seite wird neu geladen.", "ok");
    setTimeout(reloadAppPage, 900);
  } catch (error) {
    document.getElementById("update-progress-note").textContent = "App-Paket konnte nicht geladen oder installiert werden.";
    announceStatus("App-Paket konnte nicht geladen werden", "error");
    finishButtonFeedback(button, "App-Paket laden", "error", "Fehler");
    resetProgressButton();
    finishProgressUi(1200);
  }
}

async function uploadAppBundleFile(event) {
  event.preventDefault();

  const form = event.currentTarget;
  const fileInput = form.querySelector('input[type="file"]');
  const button = form.querySelector('button[type="submit"]');
  const file = fileInput && fileInput.files && fileInput.files[0];

  if (!file) {
    document.getElementById("fs-action-status").textContent = "Bitte zuerst eine App-Bundle-Datei auswählen.";
    return;
  }

  if (file.name !== "app-bundle.txt") {
    document.getElementById("fs-action-status").textContent = "Bitte die Datei app-bundle.txt auswählen.";
    announceStatus("Falsche App-Bundle-Datei ausgewählt", "error");
    finishButtonFeedback(button, button.dataset.restoreText || "App installieren", "error", "Fehler");
    return;
  }

  if (!isTxtFileName(file.name)) {
    document.getElementById("fs-action-status").textContent = "App-Paket muss eine .txt-Datei sein.";
    announceStatus("Ungültige Dateiendung", "error");
    finishButtonFeedback(button, button.dataset.restoreText || "App installieren", "error", "Fehler");
    return;
  }

  try {
    await runManagedRawUpload({
      button,
      file,
      uploadUrl: buildUploadUrl(getAppBundleUploadUrl(), file.name),
      startStatusText: "App-Paket wird hochgeladen: " + file.name,
      installStatusText: "App-Paket wurde hochgeladen und wird jetzt installiert...",
      successStatusText: "App-Paket wurde erfolgreich installiert. Seite wird neu geladen.",
      successAnnounceText: "App-Paket wurde erfolgreich installiert.",
      idleText: "App installieren",
      successText: "installiert",
      onProgressText: (percent) => "App-Paket wird hochgeladen: " + percent + "%",
      onInstalled: () => announceStatus("App-Paket wird installiert...", "warn"),
      onSuccess: async () => {
        window.setTimeout(() => window.location.reload(), 1200);
      }
    });
  } catch (error) {
    setFsActionStatus("App-Paket konnte nicht installiert werden: " + (error.message || "unbekannter Fehler"));
    announceStatus("App-Paket konnte nicht installiert werden", "error");
    finishButtonFeedback(button, getUploadActionButtonText(button, "App installieren"), "error", "Fehler");
  }
}

async function uploadFsTargetFile(event, url, successMessage) {
  event.preventDefault();

  const form = event.currentTarget;
  const label = form.querySelector(".label");
  const fileInput = form.querySelector('input[type="file"]');
  const button = form.querySelector('button[type="submit"]');
  const file = fileInput && fileInput.files && fileInput.files[0];
  const targetName = label ? label.textContent : "Datei";

  if (!file) {
    document.getElementById("fs-action-status").textContent = "Bitte zuerst eine Datei für " + targetName + " auswählen.";
    return;
  }

  if (!isMatchingFsUploadFile(url, file.name, targetName)) {
    document.getElementById("fs-action-status").textContent =
      isTablesUploadUrl(url)
        ? "Falsche Datei ausgewählt. Erwartet wird ein passendes Tabellenmuster wie " + targetName.replace("local.txt", "*.txt") + "."
        : "Falsche Datei ausgewählt. Erwartet wird " + targetName + ".";
    announceStatus(targetName + " erwartet", "error");
    finishButtonFeedback(button, button.dataset.restoreText || "Datei hochladen", "error", "Fehler");
    return;
  }

  if (!isTxtFileName(file.name)) {
    document.getElementById("fs-action-status").textContent = targetName + " muss eine .txt-Datei sein.";
    announceStatus("Ungültige Dateiendung", "error");
    finishButtonFeedback(button, button.dataset.restoreText || "Datei hochladen", "error", "Fehler");
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
      idleText: "Datei hochladen",
      successText: "hochgeladen",
      onProgressText: (percent) => targetName + " wird hochgeladen: " + percent + "%",
      onSuccess: async () => {
        await loadData();
      }
    });
  } catch (error) {
    setFsActionStatus(targetName + " konnte nicht hochgeladen werden: " + (error.message || "unbekannter Fehler"));
    announceStatus(targetName + " konnte nicht hochgeladen werden", "error");
    finishButtonFeedback(button, getUploadActionButtonText(button, "Datei hochladen"), "error", "Fehler");
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
  waitForDeviceReady(isLocalUpdate ? 90000 : 120000, isLocalUpdate ? 3000 : 1500, "ESP wieder erreichbar. Seite wird neu geladen.", true, {
    forcedReloadAfterMs: 90000,
    reloadWatchdogDelayMs: 95000,
    requireReconnectCycle: true,
    requiredStableSuccesses: 2,
    probes: buildDeviceReadyProbes(),
    waitingMessage: isLocalUpdate
      ? "Lokales ESP-Update läuft. Warte auf Neustart und Reconnect..."
      : "ESP aktualisiert sich gerade. Warte auf Neustart und Reconnect..."
  });
}

function triggerStm32Update() {
  const remoteUpdate = getRemoteUpdateActionMeta("stm32");
  const fileName = document.getElementById("update-stm32-select").value || "";

  if (!fileName) {
    announceStatus("Bitte zuerst eine STM32-Datei auswählen", "warn");
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
      buttonText: "ESP-Firmware aktualisieren",
      confirmText: "ESP-Firmware jetzt vom Update-Server aktualisieren? Das Gerät startet dabei neu.",
      startMessage: "ESP-Update wird gestartet...",
      useProgressFormSubmit: true,
      keepFrameActiveInBackground: true,
      buildUrl: () => remoteUpdateMeta.esp.url
    };
  }

  return {
    kind: "stm32",
    actionType: "stm32-flash",
      buttonId: "update-stm32-button",
      buttonText: "STM32 flashen",
      confirmText: (fileName) => "STM32 jetzt mit „" + fileName + "“ flashen?",
      startMessage: "STM32-Flash wurde gestartet.",
      buildUrl: (fileName) => remoteUpdateMeta.stm32.url + "?filename=" + encodeURIComponent(fileName) + "&stream=1"
    };
}

function triggerTableUpdate() {
  const fileName = document.getElementById("update-table-select").value || "";

  if (!fileName) {
    announceStatus("Bitte zuerst eine Layout-Tabelle auswählen", "warn");
    return;
  }

  if (!window.confirm("Layout-Tabelle „" + fileName + "“ jetzt laden?")) {
    return;
  }

  const button = document.getElementById("update-table-button");
  beginButtonFeedback(button, "lädt...");
  document.getElementById("updated-at").textContent = "Layout-Tabelle wird geladen...";
  announceStatus("Layout-Tabelle wird geladen...", "warn");

  apiFetch(getUpdateDownloadTableBaseUrl() + encodeURIComponent(fileName))
    .then(async () => {
      announceStatus("Layout-Tabelle wurde geladen.", "ok");
      finishButtonFeedback(button, "Layout-Tabelle laden", "success", "geladen");
      await loadData();
    })
    .catch(() => {
      announceStatus("Layout-Tabelle konnte nicht geladen werden", "error");
      finishButtonFeedback(button, "Layout-Tabelle laden", "error", "Fehler");
    });
}

async function resetStm32() {
  await runConfirmedButtonAction(
    "maintenance-reset-stm32-button",
    "STM32 jetzt wirklich resetten?",
    {
      busyText: "läuft...",
      idleText: "STM32 zurücksetzen",
      successText: "fertig",
      errorText: "STM32 konnte nicht zurückgesetzt werden",
      successStatusText: "STM32-Reset wurde ausgelöst. Warte auf Abschluss...",
      request: async () => {
        await apiFetch(getMaintenanceResetStm32Url());
        await sleep(4000);
        announceStatus("STM32 wurde zurückgesetzt", "ok");
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
            announceStatus("STM32 wieder bereit. App wird neu geladen...", "ok");
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

  announceStatus("STM32-Reconnect nicht sicher erkannt. App wird vorsorglich neu geladen...", "warn");
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

  beginButtonFeedback(maintenanceButton, "setzt zurück...");
  announceStatus("EEPROM-Reset wird ausgelöst...", "warn");

  try {
    await apiFetch(getMaintenanceResetEepromUrl());
    announceStatus("EEPROM-Reset ausgelöst. STM32 wird neu gestartet...", "warn");
    maintenanceButton.textContent = "wartet...";
    await sleep(250);
    await apiFetch(getMaintenanceResetStm32Url());
    announceStatus("Warte auf STM32-Neustart. Danach wird die App neu geladen...", "warn");
    await waitForStm32ResetAndReload(30000, 1500);
  } catch (error) {
    announceStatus("EEPROM konnte nicht zurückgesetzt werden", "error");
    finishButtonFeedback(maintenanceButton, "EEPROM zurücksetzen", "error", "Fehler");
  }
}

async function formatLittleFs() {
  await runConfirmedButtonAction(
    "maintenance-format-fs-button",
    "LittleFS wirklich formatieren?",
    {
      busyText: "läuft...",
      idleText: "LittleFS formatieren",
      successText: "fertig",
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
    "LittleFS wirklich formatieren?",
    {
      busyText: "läuft...",
      idleText: "LittleFS formatieren",
      successText: "fertig",
      errorText: "LittleFS konnte nicht formatiert werden",
      successStatusText: "LittleFS wurde formatiert",
      reloadDelayMs: 1200,
      request: () => apiFetch(getMaintenanceFormatFsUrl())
    }
  );
  if (didRun) {
    setFsActionStatus("LittleFS wurde formatiert.");
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
    setFsActionStatus("Datei „" + fileName + "“ wird angezeigt.");
    announceStatus(fileName + " geladen", "ok");
  } catch (error) {
    announceStatus("Datei konnte nicht geladen werden", "error");
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
    await apiFetch(getFsRemoveBaseUrl() + encodeURIComponent(fileName));
    document.getElementById("fs-preview-content").textContent = "Mit „Anzeigen“ aus der Dateiliste wird hier der Inhalt der gewählten Datei eingeblendet.";
    setFsActionStatus("Datei „" + fileName + "“ wurde gelöscht.");
    await loadData();
  } catch (error) {
    announceStatus("Datei konnte nicht gelöscht werden", "error");
  }
}

function handleProgressFrameLoad() {
  const note = document.getElementById("update-progress-note");

  try {
    if (pendingProgressAction === "esp-update") {
      note.textContent = "ESP aktualisiert. Es wird gewartet, bis das Gerät wieder bereit ist.";
      return;
    }

    if (pendingProgressAction === "stm32-flash" && !stm32AutoResetStarted) {
      const text = readUpdateProgressFrameText();
      const normalized = (text || "").replace(/\s+/g, " ").trim();

      if (normalized.indexOf("Flash failed") >= 0 || normalized.indexOf("Check failed") >= 0 || normalized.indexOf("verify failed") >= 0) {
        failStm32Update("STM32-Flash ist fehlgeschlagen.");
        return;
      }

      beginStm32AutoReset("STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.");
    }
  } catch (error) {
    note.textContent = "Update-Antwort empfangen.";
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
    keepFrameActiveInBackground: actionType === "stm32-flash" || isEspLikeAction || actionType === "app-bundle-install",
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
  setBusyButton(progressMeta.buttonId, "läuft...");
  if (progressMeta.showVisualProgress) {
    beginStm32Progress();
  } else {
    stopStm32Progress();
  }
  rememberProgressReturnScrollPosition();
  progressShell.scrollIntoView({ behavior: "smooth", block: "start" });
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
    failStm32Update(event.message || "STM32-Flash ist fehlgeschlagen.");
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
    document.getElementById("update-progress-note").textContent = "Lokaler STM32-Flash wird gestartet.";
    document.getElementById("updated-at").textContent = "Lokaler STM32-Flash wird gestartet.";
    setBusyButton(buttonId, "läuft...");

    if (button) {
      button.dataset.restoreText = buttonText || button.textContent;
    }

    beginStm32Progress();
    rememberProgressReturnScrollPosition();
    progressShell.scrollIntoView({ behavior: "smooth", block: "start" });

    uploadRawFile(
      buildUploadUrl(getLocalStm32UploadUrl(), file.name),
      file,
      (loaded, total) => {
        const percent = total ? Math.min(100, Math.round((loaded / total) * 100)) : 0;
        document.getElementById("local-update-note").textContent = "STM32-Firmware wird hochgeladen: " + percent + "%";
        document.getElementById("update-progress-note").textContent = "STM32-Firmware wird hochgeladen: " + percent + "%";
      },
      () => {
        document.getElementById("local-update-note").textContent = "STM32-Firmware wurde hochgeladen. Flash startet...";
        document.getElementById("update-progress-note").textContent = "STM32-Firmware wurde hochgeladen. Flash startet...";
      }
    ).then(() => {
      const xhr = new XMLHttpRequest();
      xhr.open("GET", getLocalStm32FlashUrl(), true);
      startUpdateProgressPolling();

      xhr.onprogress = () => {
        const text = xhr.responseText || "";
        syncStm32ProgressFromText(text);

        if (!stm32AutoResetStarted && hasStm32FlashFinished(text)) {
          beginStm32AutoReset("STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.");
        }
      };

      xhr.onload = () => {
        const text = xhr.responseText || "";
        syncStm32ProgressFromText(text);

        if (xhr.status < 200 || xhr.status >= 300) {
          document.getElementById("local-update-note").textContent = "Lokaler STM32-Flash konnte nicht gestartet werden.";
          failStm32Update("Lokaler STM32-Flash konnte nicht gestartet werden.");
          reject(new Error("stm32 local failed"));
          return;
        }

        if (!stm32AutoResetStarted && hasStm32FlashFinished(text)) {
          beginStm32AutoReset("STM32-Flash abgeschlossen. STM32 wird jetzt automatisch zurückgesetzt.");
        }

        resolve();
      };

      xhr.onerror = () => {
        document.getElementById("local-update-note").textContent = "Lokaler STM32-Flash konnte nicht gestartet werden.";
        failStm32Update("Lokaler STM32-Flash konnte nicht gestartet werden.");
        reject(new Error("stm32 local failed"));
      };

      xhr.send();
    }).catch(() => {
      document.getElementById("local-update-note").textContent = "Lokaler STM32-Upload ist fehlgeschlagen.";
      failStm32Update("Lokaler STM32-Upload ist fehlgeschlagen.");
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
    document.getElementById("updated-at").textContent = "STM32 wurde nach dem Flash automatisch zurückgesetzt. Warte auf Abschluss...";
    document.getElementById("update-progress-note").textContent = "STM32 wird automatisch zurückgesetzt. Daten werden danach neu geladen.";
    await sleep(4000);
    setStm32ProgressStage(8);
    document.getElementById("updated-at").textContent = "STM32 wurde nach dem Flash automatisch zurückgesetzt";
    document.getElementById("update-progress-note").textContent = "STM32-Update erfolgreich abgeschlossen.";
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
    document.getElementById("update-progress-note").textContent = "STM32-Flash fertig, automatischer Reset ist fehlgeschlagen.";
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
  const waitingMessage = config.waitingMessage || "Warte auf Neustart des ESP...";
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
  url.searchParams.set("_reload", String(Date.now()));
  window.location.replace(url.toString());
}

function clearReloadQueryMarker() {
  try {
    const url = new URL(window.location.href);

    if (!url.searchParams.has("_reload")) {
      return;
    }

    url.searchParams.delete("_reload");
    const nextPath = url.pathname + (url.searchParams.toString() ? "?" + url.searchParams.toString() : "") + url.hash;
    window.history.replaceState({}, document.title, nextPath);
  } catch (_) {
  }
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
    note.textContent = progress.message || "STM32-Flash ist fehlgeschlagen.";
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
    shell.scrollIntoView({ behavior: "smooth", block: "start" });
  }
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
  await saveTemperatureCorrection("temperature-ds18xx-correction-input", "temperature-ds18xx-correction-save-button", getTemperatureDs18xxCorrectionSetUrl(), "DS18xx-Korrektur speichern", "DS18xx-Korrektur konnte nicht gespeichert werden");
}

async function saveTemperatureCorrection(inputId, buttonId, endpoint, buttonText, errorText) {
  const input = document.getElementById(inputId);
  const value = Math.max(0, Math.min(10, Number(input.value || 0)));
  input.value = String(value);
  await runValueSave(buttonId, endpoint, value, buttonText, errorText);
}

async function displayTemperatureNow() {
  await runSimpleAction("temperature-display-button", getTemperatureDisplayUrl(), "Temperatur anzeigen", "Temperatur konnte nicht angezeigt werden", "Temperaturanzeige ausgelöst");
}

async function setLdrMinValue() {
  await runSimpleAction("ldr-min-button", getLdrMinSetUrl(), "Aktuellen Wert als Minimum setzen", "LDR-Minimum konnte nicht gesetzt werden", "LDR-Minimum gespeichert");
}

async function setLdrMaxValue() {
  await runSimpleAction("ldr-max-button", getLdrMaxSetUrl(), "Aktuellen Wert als Maximum setzen", "LDR-Maximum konnte nicht gesetzt werden", "LDR-Maximum gespeichert");
}

async function saveAnimationMode() {
  await runSelectSave("animation-mode-select", "animation-mode-save-button", getAnimationModeSetUrl(), "Anzeigeanimation speichern", "Anzeigeanimation konnte nicht gespeichert werden");
}

async function saveColorAnimationMode() {
  await runSelectSave("color-animation-mode-select", "color-animation-mode-save-button", getColorAnimationModeSetUrl(), "Farbanimation speichern", "Farbanimation konnte nicht gespeichert werden");
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
    busyText: "speichert...",
    idleText: "Profil speichern",
    successText: "gespeichert",
    errorText: "Animationsprofil konnte nicht gespeichert werden",
    reload: true
  });
}

async function resetAnimationProfileDefault(idx) {
  await runIndexedQueryButtonRequest('[data-an-default="%idx%"]', idx, {
    endpoint: getAnimationProfileDefaultUrl(),
    query: { idx },
    busyText: "setzt...",
    idleText: "Standard",
    successText: "gesetzt",
    errorText: "Standardwert konnte nicht gesetzt werden",
    reload: true
  });
}

async function saveColorAnimationProfile(idx) {
  const deceleration = document.getElementById("can-dec-" + idx).value;
  await runIndexedQueryButtonRequest('[data-can-save="%idx%"]', idx, {
    endpoint: getColorAnimationProfileSetUrl(),
    query: { idx, deceleration },
    busyText: "speichert...",
    idleText: "Profil speichern",
    successText: "gespeichert",
    errorText: "Farbanimationsprofil konnte nicht gespeichert werden",
    reload: true
  });
}

async function resetColorAnimationProfileDefault(idx) {
  await runIndexedQueryButtonRequest('[data-can-default="%idx%"]', idx, {
    endpoint: getColorAnimationProfileDefaultUrl(),
    query: { idx },
    busyText: "setzt...",
    idleText: "Standard",
    successText: "gesetzt",
    errorText: "Standardwert konnte nicht gesetzt werden",
    reload: true
  });
}

async function saveDimCurve(prefix) {
  const button = document.getElementById(prefix === "ambi" ? "ambilight-dim-save-button" : "display-dim-save-button");
  const buttonText = prefix === "ambi" ? "Ambilight-Dimmkurve speichern" : "Display-Dimmkurve speichern";
  await persistDimCurve(prefix, button, buttonText);
}

async function persistDimCurve(prefix, button, buttonText) {
  const endpoint = prefix === "ambi" ? getAmbilightDimLevelSetUrl() : getDisplayDimLevelSetUrl();

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
  const rgb = document.getElementById("tft-rgb-checkbox").checked ? "on" : "off";
  const hflip = document.getElementById("tft-hflip-checkbox").checked ? "on" : "off";
  const vflip = document.getElementById("tft-vflip-checkbox").checked ? "on" : "off";
  await runQueryButtonRequest(document.getElementById("tft-save-button"), {
    endpoint: getTftFlagsSetUrl(),
    query: { rgb, hflip, vflip },
    busyText: "speichert...",
    idleText: "TFT-Optionen speichern",
    successText: "gespeichert",
    errorText: "TFT-Optionen konnten nicht gespeichert werden",
    reload: true
  });
}

async function runTextSave(buttonId, endpoint, value, buttonText, errorText) {
  await runValueSave(buttonId, endpoint, value, buttonText, errorText);
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
  await runTriggerAction(buttonId, endpoint, buttonText, errorText, successText, {
    busyText: "läuft...",
    successText: "fertig"
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
    busyText: "speichert...",
    idleText: buttonText,
    successText: "gespeichert",
    errorText,
    reload: true,
    ...(options || {})
  });
}

async function runQuerySave(buttonId, endpoint, query, buttonText, errorText, options) {
  return runQueryButtonRequestById(buttonId, {
    endpoint,
    query,
    busyText: "speichert...",
    idleText: buttonText,
    successText: "gespeichert",
    errorText,
    reload: true,
    ...(options || {})
  });
}

async function runTriggerAction(buttonId, endpoint, buttonText, errorText, successStatusText, options) {
  return runButtonRequestById(buttonId, {
    busyText: (options && options.busyText) || "startet...",
    idleText: buttonText,
    successText: (options && options.successText) || "gestartet",
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
  const successText = options && options.successText ? options.successText(next) : (next === "on" ? "aktiviert" : "deaktiviert");

  await runQueryButtonRequest(button, {
    endpoint,
    query: { value: next },
    busyText: "schaltet...",
    idleText,
    successText,
    errorText: options && options.errorText ? options.errorText : "Schalter konnte nicht gesetzt werden",
    reload: true,
    preserveCurrentText: options && options.preserveCurrentText !== undefined ? options.preserveCurrentText : true
  });
}

async function runButtonRequest(button, options) {
  const {
    busyText = "läuft...",
    idleText = button && (button.dataset.restoreText || button.textContent) ? (button.dataset.restoreText || button.textContent) : "",
    successText = "fertig",
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
    finishButtonFeedback(button, idleText, "error", "Fehler", preserveCurrentText);
  }
}

async function saveAmbilightModeProfile(idx) {
  const deceleration = Math.max(0, Math.min(15, Number(document.getElementById("alm-dec-" + idx).value || 0)));
  await runIndexedQueryButtonRequest('[data-alm-save="%idx%"]', idx, {
    endpoint: getAmbilightModeProfileSetUrl(),
    query: { idx, deceleration },
    busyText: "speichert...",
    idleText: "Profil speichern",
    successText: "gespeichert",
    errorText: "Ambilight-Profil konnte nicht gespeichert werden",
    reload: true
  });
}

async function resetAmbilightModeProfile(idx) {
  await runIndexedQueryButtonRequest('[data-alm-default="%idx%"]', idx, {
    endpoint: getAmbilightModeProfileDefaultUrl(),
    query: { idx },
    busyText: "setzt...",
    idleText: "Standard",
    successText: "gesetzt",
    errorText: "Ambilight-Standardwert konnte nicht gesetzt werden",
    reload: true
  });
}

async function saveAmbilightBrightness() {
  const value = document.getElementById("ambilight-brightness-slider").value;
  await runButtonRequestById("ambilight-brightness-save-button", {
    busyText: "speichert...",
    idleText: "Ambilight-Helligkeit speichern",
    successText: "gespeichert",
    errorText: "Ambilight-Helligkeit konnte nicht gespeichert werden",
    reload: true,
    request: () => apiFetch(getAmbilightBrightnessSetUrl() + "?value=" + encodeURIComponent(value))
  });
}

async function saveAmbilightMode() {
  const value = document.getElementById("ambilight-mode-select").value;
  await runButtonRequestById("ambilight-mode-save-button", {
    busyText: "speichert...",
    idleText: "Ambilight-Modus speichern",
    successText: "gespeichert",
    errorText: "Ambilight-Modus konnte nicht gespeichert werden",
    reload: true,
    request: () => apiFetch(getAmbilightModeSetUrl() + "?value=" + encodeURIComponent(value))
  });
}

async function saveAmbilightLeds() {
  const input = document.getElementById("ambilight-leds-input");
  const value = Math.max(0, Math.min(999, Number(input.value || 0)));
  input.value = String(value);
  await runButtonRequestById("ambilight-leds-save-button", {
    busyText: "speichert...",
    idleText: "LED-Anzahl speichern",
    successText: "gespeichert",
    errorText: "LED-Anzahl konnte nicht gespeichert werden",
    reload: true,
    request: () => apiFetch(getAmbilightLedsSetUrl() + "?value=" + encodeURIComponent(value))
  });
}

async function saveAmbilightOffset() {
  const input = document.getElementById("ambilight-offset-input");
  const value = Math.max(0, Math.min(999, Number(input.value || 0)));
  input.value = String(value);
  await runButtonRequestById("ambilight-offset-save-button", {
    busyText: "speichert...",
    idleText: "Offset speichern",
    successText: "gespeichert",
    errorText: "Ambilight-Offset konnte nicht gespeichert werden",
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
    display: "Display Farbe speichern",
    ambilight: "Ambilight Farbe speichern",
    marker: "Marker Farbe speichern"
  }[prefix];
  await runQueryButtonRequest(document.getElementById(prefix + "-color-save-button"), {
    endpoint,
    query: { red: rgb.red, green: rgb.green, blue: rgb.blue, white },
    busyText: "speichert...",
    idleText,
    successText: "gespeichert",
    errorText: "Farbe konnte nicht gespeichert werden",
    reload: true
  });
}

async function saveDfplayerVolume() {
  const value = document.getElementById("dfplayer-volume-slider").value;
  await runQueryButtonRequest(document.getElementById("dfplayer-volume-save-button"), {
    endpoint: getDfplayerVolumeSetUrl(),
    query: { value },
    busyText: "speichert...",
    idleText: "Lautstärke speichern",
    successText: "gespeichert",
    errorText: "DFPlayer-Lautstärke konnte nicht gespeichert werden",
    reload: true
  });
}

async function saveDfplayerMode() {
  const value = document.getElementById("dfplayer-mode-select").value;
  await runQueryButtonRequest(document.getElementById("dfplayer-mode-save-button"), {
    endpoint: getDfplayerModeSetUrl(),
    query: { value },
    busyText: "speichert...",
    idleText: "Modus speichern",
    successText: "gespeichert",
    errorText: "DFPlayer-Modus konnte nicht gespeichert werden",
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
    busyText: "speichert...",
    idleText: "Glockenzeiten speichern",
    successText: "gespeichert",
    errorText: "Glockenzeiten konnten nicht gespeichert werden",
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
    busyText: "speichert...",
    idleText: "Sprachzyklus speichern",
    successText: "gespeichert",
    errorText: "Sprachzyklus konnte nicht gespeichert werden",
    reload: true
  });
}

async function saveDfplayerSilenceStart() {
  await saveDfplayerSilenceTime("dfplayer-silence-start-input", "dfplayer-silence-start-save-button", getDfplayerSilenceStartSetUrl(), "Ruhezeit Beginn speichern", "Ruhezeit Beginn konnte nicht gespeichert werden");
}

async function saveDfplayerSilenceStop() {
  await saveDfplayerSilenceTime("dfplayer-silence-stop-input", "dfplayer-silence-stop-save-button", getDfplayerSilenceStopSetUrl(), "Ruhezeit Ende speichern", "Ruhezeit Ende konnte nicht gespeichert werden");
}

async function saveDfplayerSilenceTime(inputId, buttonId, endpoint, buttonText, errorText) {
  const { hour, minute } = parseTimeInput(document.getElementById(inputId).value || "00:00");
  await runQueryButtonRequest(document.getElementById(buttonId), {
    endpoint,
    query: { hour, minute },
    busyText: "speichert...",
    idleText: buttonText,
    successText: "gespeichert",
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
    busyText: "spielt...",
    idleText: "Titel abspielen",
    successText: "gestartet",
    errorText: "DFPlayer-Titel konnte nicht gestartet werden",
    successStatusText: "DFPlayer-Titel gestartet"
  });
}

async function saveDfplayerAlarm(idx) {
  const active = document.getElementById("df-alarm-active-" + idx).checked ? "on" : "off";
  const from = document.getElementById("df-alarm-from-" + idx).value;
  const to = document.getElementById("df-alarm-to-" + idx).value;
  const { hour, minute } = parseTimeInput(document.getElementById("df-alarm-time-" + idx).value || "00:00");
  const idleText = "Titel " + String(idx + 1).padStart(3, "0") + " speichern";
  await runIndexedQueryButtonRequest('[data-alarm-save="%idx%"]', idx, {
    endpoint: getDfplayerAlarmSetUrl(),
    query: { idx, active, from, to, hour, minute },
    busyText: "speichert...",
    idleText,
    successText: "gespeichert",
    errorText: "DFPlayer-Titel konnte nicht gespeichert werden",
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
    busyText: "speichert...",
    idleText: "Overlay speichern",
    successText: "gespeichert",
    errorText: "Overlay konnte nicht gespeichert werden",
    reload: true
  });
}

async function displayOverlay(idx) {
  await runIndexedQueryButtonRequest('[data-overlay-display="%idx%"]', idx, {
    endpoint: getOverlayDisplayUrl(),
    query: { idx },
    busyText: "zeigt...",
    idleText: "Anzeigen",
    successText: "gestartet",
    errorText: "Overlay konnte nicht angezeigt werden",
    successStatusText: "Overlay " + idx + " wird angezeigt"
  });
}

async function deleteOverlay(idx) {
  await runIndexedQueryButtonRequest('[data-overlay-delete="%idx%"]', idx, {
    endpoint: getOverlayDeleteUrl(),
    query: { idx },
    busyText: "löscht...",
    idleText: "Löschen",
    successText: "gelöscht",
    errorText: "Overlay konnte nicht gelöscht werden",
    successStatusText: "Overlay wurde gelöscht",
    reload: true
  });
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
    busyText: "speichert...",
    idleText: "Speichern",
    successText: "gespeichert",
    errorText: "Timer konnte nicht gespeichert werden",
    reload: opts.reload !== false
  });
}

async function clearTimerRow(idx, isAmbilight) {
  const prefix = isAmbilight ? "at" : "t";
  const endpoint = isAmbilight ? getAmbilightTimerSetUrl() : getTimerSetUrl();
  await runIndexedQueryButtonRequest('[data-' + prefix + '-clear="%idx%"]', idx, {
    endpoint,
    query: { idx, active: "off", switch_on: "off", from: 0, to: 0, hour: 0, minute: 0 },
    busyText: "leert...",
    idleText: "Slot leeren",
    successText: "geleert",
    errorText: "Timer konnte nicht geleert werden",
    reload: true
  });
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
  const currentTable = getUpdateTableCurrentFile(updateTableInfo);
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
    const rows = resolveLayoutRows(currentTable, table);
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

function resolveLayoutRows(currentTable, table) {
  const source = findLayoutPreviewSource(currentTable, table);
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

function normalizeLayoutFileName(fileName) {
  return String(fileName || "")
    .split("/")
    .pop()
    .trim()
    .toLowerCase();
}

function findLayoutPreviewSource(fileName, table) {
  const normalizedFileName = normalizeLayoutFileName(fileName);
  const normalizedHex = table && table.normalizedHex ? table.normalizedHex : "";

  return LAYOUT_PREVIEW_SOURCES.find((item) => normalizeLayoutFileName(item.file) === normalizedFileName)
    || LAYOUT_PREVIEW_SOURCES.find((item) => Array.isArray(item.aliases) && item.aliases.includes(normalizedFileName.replace(/\.txt$/, "")))
    || LAYOUT_PREVIEW_SOURCES.find((item) => normalizedHex && item.signature && normalizedHex.startsWith(item.signature))
    || null;
}

function getDefaultLayoutPreview(settings) {
  const resolvedAssetMeta = getResolvedAssetMeta(settings, null, null);
  const resolvedLayoutMeta = getResolvedLayoutMeta(settings, null, null);
  const assetPrefix = resolvedAssetMeta.assetPrefix;
  const layoutMeta = resolvedLayoutMeta;
  const fileName = layoutMeta.previewFile
    || (assetPrefix === "wc24h" ? "wc24h-tables-de.txt" : (assetPrefix === "uc" ? "" : "wc12h-tables-de.txt"))
    || "wc12h-tables-de.txt";
  const source = LAYOUT_PREVIEW_SOURCES.find((item) => item.file === fileName);

  if (!source) {
    return {
      file: fileName,
      table: null,
      rows: fallbackWordclockRows
    };
  }

  const columns = getResolvedLayoutColumns(fileName);
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
    displayPowerLabel: displayPower === "on" ? "an" : "aus",
    ambilightPowerLabel: featureMeta.moduleState.ambilightOnline ? (ambilightPower === "on" ? "an" : "aus") : "offline",
    firmwareVersion: displayMeta.firmwareVersion || "-",
    espVersion: getUpdateAvailableVersion(updateStatus, "esp_version") || displayMeta.espVersion || "-",
    lastStartLabel: formatLastStartFromSettings(settings),
    configItems
  };
}

function buildOverviewConfigItems(settings, featureMeta, displayMeta, networkMeta, climateMeta, ambilightMeta, dfplayerMeta) {
  const configItems = [
    ["Display-Modus", getDisplayModeName(displayMeta.mode)],
    ["Helligkeit", String(displayMeta.brightness)],
    ["Automatische Helligkeit", displayMeta.automaticBrightness ? "an" : "aus"],
    ["LED-Fähigkeiten", featureMeta.ledCapabilities.label],
    ["Zeitserver", networkMeta.timeserver || "-"],
    ["Ticker-Verzögerung", String(displayMeta.tickerDeceleration || 0)]
  ];

  if (settings.strvars[STR.RESET_CAUSE]) {
    configItems.unshift(["Letzter STM32-Neustart", settings.strvars[STR.RESET_CAUSE]]);
  }

  const lastStartLabel = formatLastStartFromSettings(settings);
  configItems.unshift(["Letzter Start", lastStartLabel]);

  const weatherLocation = climateMeta.weatherCity
    ? climateMeta.weatherCity
    : ((climateMeta.weatherLon || climateMeta.weatherLat)
      ? (climateMeta.weatherLon || "-") + " / " + (climateMeta.weatherLat || "-")
      : "");

  if (weatherLocation) {
    configItems.splice(configItems.length - 1, 0, ["Wetter-Ort", weatherLocation]);
  }

  if (displayMeta.tickerText) {
    configItems.splice(configItems.length - 1, 0, ["Ticker", displayMeta.tickerText]);
  }

  if (displayMeta.dateTickerFormat) {
    configItems.splice(configItems.length - 1, 0, ["Datumsformat", displayMeta.dateTickerFormat]);
  }

  if (featureMeta.moduleState.ambilightOnline) {
    configItems.splice(4, 0,
      ["Ambilight-Modus", getAmbilightModeName(settings)],
      ["Ambilight-Helligkeit", String(ambilightMeta.brightness || 0)],
      ["Ambilight LEDs", String(ambilightMeta.leds || 0)],
      ["Ambilight Offset", String(ambilightMeta.offset || 0)]
    );
  }

  if (featureMeta.moduleState.dfplayerOnline) {
    configItems.push(
      ["DFPlayer-Modus", getDfplayerModeName(dfplayerMeta.mode || 0)],
      ["DFPlayer-Lautstärke", String(dfplayerMeta.volume || 0)],
      ["Sprechintervall", String(dfplayerMeta.speakCycle || 0)]
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
        note: "RGBW-Hardware erkannt. RGB- und Weißkanal sind verfügbar."
      };
    }

    if (mode === "rgb" || mode === "tft") {
      return {
        hasColor: true,
        whiteChannel,
        mode: whiteChannel ? "rgbw" : "rgb",
        label: displayInfo.label || (hasTft ? "TFT RGB" : "RGB"),
        note: hasTft
          ? "TFT-Hardware erkannt. TFT-Optionen sind verfügbar, der Weißkanal bleibt ausgeblendet."
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
    0: "Keine",
    1: "Glocke",
    2: "Sprache"
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
    None: "Keine",
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

async function applyDebugOverrides() {
  const button = document.getElementById("debug-apply-button");
  const overrides = {
    ambilight: document.getElementById("debug-ambilight-select").value,
    dfplayer: document.getElementById("debug-dfplayer-select").value,
    color: document.getElementById("debug-color-select").value,
    tft: document.getElementById("debug-tft-select").value
  };

  beginButtonFeedback(button, "übernimmt...");
  try {
    saveDebugOverrides(overrides);
    document.getElementById("updated-at").textContent = "Overrides aktiv";
    await loadData();
    finishButtonFeedback(button, "Overrides anwenden", "success", "aktiv");
  } catch (error) {
    announceStatus("Overrides konnten nicht angewendet werden", "error");
    finishButtonFeedback(button, "Overrides anwenden", "error", "Fehler");
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

  beginButtonFeedback(button, "setzt zurück...");
  try {
    saveDebugOverrides(overrides);
    loadDebugOverridesIntoUi();
    document.getElementById("updated-at").textContent = "Overrides zurückgesetzt";
    await loadData();
    finishButtonFeedback(button, "Overrides zurücksetzen", "success", "zurückgesetzt");
  } catch (error) {
    announceStatus("Overrides konnten nicht zurückgesetzt werden", "error");
    finishButtonFeedback(button, "Overrides zurücksetzen", "error", "Fehler");
  }
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
  if (settingsImportInProgress) {
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
    }, 1200);
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
    return "Gerätezeit ist derzeit nicht verfügbar.";
  }

  const weekday = ["Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"][current.wday] || "Unbekannt";
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
    return "nicht verfügbar";
  }

  if (uptimeSeconds <= 0) {
    return "nicht verfügbar";
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
    return "nicht verfügbar";
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
