cmake_minimum_required(VERSION 3.20)

project(wordclock24h-stm C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Match the legacy EmBitz release profile as closely as possible.
# We provide the optimization/debug flags ourselves per target and do not want
# CMake's default Release flags like -O3 -DNDEBUG on top.
set(CMAKE_C_FLAGS_RELEASE "" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE STRING "" FORCE)

# Diagnosebau: schaltet die 172 debug_log_*-Aufrufe scharf (src/log/log.h:23-31).
#
# Bis zum 03.10.2026 liess sich DEBUG ueberhaupt nicht setzen. Naheliegend waere
# -DTGT_COMPILE_DEFINITIONS=DEBUG gewesen, aber das wirkt nicht: Die Variable ist
# keine freie, sondern das von cmake_parse_arguments(TGT ...) geparste Argument
# COMPILE_DEFINITIONS des jeweiligen wordclock_add_target()-Aufrufs und wird beim
# Parsen ueberschrieben. Am Fabrikat gemessen: byteidentisch, -DDEBUG kam nie beim
# Compiler an. Einen Diagnosebau bekam man nur, indem man die Definitionsliste von
# Hand editierte.
#
#   cmake -S . -B build/stm-debug -DWORDCLOCK_DEBUG=ON ...
#
# ZWEI WARNUNGEN, und beide sind der Grund fuer die Vorgabe OFF:
#
#   1. Die Zeilen gehen blockierend ueber dieselbe UART, die zum STM fuehrt
#      (log.c:27-34). Ein Diagnosebau ist langsamer als ein Normalbau, und er
#      fuellt den 64-Zeilen-Ring um ein Vielfaches schneller -- unter LED-Last
#      reicht der schon im Normalbau nur rund 1,2 s (BEFUNDE.md, L75).
#
#   2. Ein Diagnosebau schreibt Geheimnisse im Klartext in den Ring, etwa
#      "cmd: set weather appid = '...'" (main.c:2199). /api/stm32_log ist ohne
#      Anmeldung aus dem ganzen LAN lesbar. Ein solches Fabrikat gehoert auf den
#      Basteltisch, nicht in den Dauerbetrieb.
option(WORDCLOCK_DEBUG "Diagnosebau: debug_log_* scharf schalten (siehe Warnungen oben)" OFF)

# Link Time Optimization, fuer BEIDE Ziele. Gemessen am 04.10.2026, Release-Bau,
# arm-none-eabi-gcc, gegen denselben Stand ohne LTO:
#
#   F103   -4'352 Byte Flash  = 6,63 %   |  RAM -316 Byte
#   F411   -4'352 Byte Flash  = 6,73 %   |  RAM -316 Byte
#   neue Codewarnungen: keine
#
# Der F103 ist der Grund. Sein stm32f103c8 hat 65'536 Byte Flash, und der Bau war
# ohne LTO am Link gescheitert -- nicht knapp, sondern gar nicht. Die 4'352 Byte
# sind hier kein Komfortgewinn, sondern die Differenz zwischen "linkt" und
# "linkt nicht". Entsprechend ist das keine Option, die man bei Bedarf abschaltet:
# Wer sie entfernt, bekommt den F103 nicht mehr gebaut.
#
# WARUM "=4" UND NICHT BLANK -- das ist kein Geschmack, sondern eine Messung:
#
# Blankes -flto laesst gcc die Zahl der Link-Jobs selbst bestimmen. Findet der
# Treiber keine brauchbare Vorgabe, faellt er zurueck und schreibt dabei
#
#     lto-wrapper: warning: using serial compilation of N LTRANS jobs
#
# auf stderr. Das Fabrikat ist byteidentisch -- am Hex nachgemessen, die Meldung
# kostet kein Byte. Sie kostet etwas anderes: Guardrail-Stufe S4 zaehlt stumpf
# 'grep -c "warning:"' ueber das Bauprotokoll und fuehrt den Bestand als feste
# Zahl. Jede Treibermeldung, die keine Codewarnung ist, verschiebt diesen Bestand
# und macht die Stufe genau so lange unbrauchbar, bis jemand die Zahl von Hand
# nachzieht -- also bis zum naechsten Mal. Eine Schwelle, die aus einem anderen
# Grund wandert als dem, den sie ueberwachen soll, wird irgendwann hochgesetzt
# statt gelesen.
#
# "=4" gibt die Zahl vor, der Rueckfall entfaellt, die Meldung entfaellt, der
# Bestand bleibt bei 18. Die 4 passt zu den "-j4" der Build-Ziele im Makefile.
set(WORDCLOCK_LTO_FLAG -flto=4)

# embitz_collect_sources(<ebp> <out-var> [<ebp-ziel>])
#
# Ohne drittes Argument: alle .c/.S-Units der ebp, wie bisher -- die beiden
# bestehenden Ziele f103/f411 rufen es so auf und waehlen per Ausschlussmuster.
#
# Mit drittem Argument (Zielname aus der ebp, etwa "WC12h-Black401-25-Release")
# wird <Option target="..."> je Unit beachtet, so wie EmBitz es tut: Eine Unit
# OHNE Zielbindung gehoert zu allen Zielen, eine Unit MIT Zielbindung nur zu den
# genannten. In wclock24h.ebp betrifft das die vier Startdateien. Die Matrix
# waehlt damit genau die Quellen des jeweiligen Originalziels aus, statt sie
# ueber Ausschlussmuster nachzubauen.
function(embitz_collect_sources EBP_FILE OUT_VAR)
  # Bewusst ARGN und nicht ARGV2: Ohne drittes Argument ist ARGV2 in einer
  # Funktion nicht leer, sondern erbt den Wert aus dem aufrufenden Scope --
  # hier das ARGV2 von wordclock_add_target, also einen Pfad.
  set(EBP_TARGET "")
  if(ARGN)
    list(GET ARGN 0 EBP_TARGET)
  endif()
  get_filename_component(EBP_DIR "${EBP_FILE}" DIRECTORY)
  if(EBP_TARGET)
    file(STRINGS "${EBP_FILE}" TITLE_LINES REGEX "<Target title=\"${EBP_TARGET}\">")
    if(NOT TITLE_LINES)
      message(FATAL_ERROR "embitz_collect_sources: Ziel '${EBP_TARGET}' steht nicht in ${EBP_FILE}")
    endif()
  endif()
  file(STRINGS "${EBP_FILE}" UNIT_LINES REGEX "<Unit filename=|<Option target=|</Unit>")
  set(SOURCES)
  set(UNIT_OPEN FALSE)
  set(UNIT_FILE "")
  set(UNIT_TARGETS "")
  foreach(LINE IN LISTS UNIT_LINES)
    set(TAKE_UNIT FALSE)
    if(LINE MATCHES "<Unit filename=\"([^\"]+)\"")
      set(UNIT_FILE "${CMAKE_MATCH_1}")
      set(UNIT_TARGETS "")
      if(LINE MATCHES "/>[ \t]*$")
        # selbstschliessend: keine Zielbindung moeglich
        set(TAKE_UNIT TRUE)
        set(UNIT_OPEN FALSE)
      else()
        set(UNIT_OPEN TRUE)
      endif()
    elseif(UNIT_OPEN AND LINE MATCHES "<Option target=\"([^\"]+)\"")
      list(APPEND UNIT_TARGETS "${CMAKE_MATCH_1}")
    elseif(UNIT_OPEN AND LINE MATCHES "</Unit>")
      set(UNIT_OPEN FALSE)
      if(NOT EBP_TARGET OR NOT UNIT_TARGETS)
        set(TAKE_UNIT TRUE)
      else()
        list(FIND UNIT_TARGETS "${EBP_TARGET}" TARGET_INDEX)
        if(NOT TARGET_INDEX EQUAL -1)
          set(TAKE_UNIT TRUE)
        endif()
      endif()
    endif()
    if(TAKE_UNIT AND UNIT_FILE MATCHES "\\.(c|S)$")
      file(TO_CMAKE_PATH "${UNIT_FILE}" UNIT_FILE)
      get_filename_component(UNIT_ABS "${UNIT_FILE}" REALPATH BASE_DIR "${EBP_DIR}")
      list(APPEND SOURCES "${UNIT_ABS}")
    endif()
  endforeach()
  list(REMOVE_DUPLICATES SOURCES)
  set(${OUT_VAR} "${SOURCES}" PARENT_SCOPE)
endfunction()

function(wordclock_collect_repo_include_dirs OUT_VAR)
  set(DIRS "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
  file(GLOB CHILDREN LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/src/*")
  foreach(CHILD IN LISTS CHILDREN)
    if(IS_DIRECTORY "${CHILD}")
      list(APPEND DIRS "${CHILD}")
    endif()
  endforeach()
  list(REMOVE_DUPLICATES DIRS)
  set(${OUT_VAR} "${DIRS}" PARENT_SCOPE)
endfunction()

function(wordclock_add_target TARGET_NAME)
  set(options)
  # EBP_TARGET, LED, STARTUP und OUTPUT_DIR sind optional und werden nur von der
  # Matrix (WORDCLOCK_MATRIX) gesetzt. Ohne sie verhaelt sich die Funktion exakt
  # wie zuvor -- die beiden bestehenden Ziele bleiben byteidentisch.
  set(oneValueArgs EBP_FILE EBP_TARGET FAMILY CPU FLOAT_ABI FPU LINKER_SCRIPT HSE BOARD_DEFINE MODE_DEFINE MCU_DEFINE OUTPUT_NAME OUTPUT_DIR LED STARTUP)
  set(multiValueArgs COMPILE_DEFINITIONS EXTRA_COMPILE_OPTIONS EXTRA_LINK_OPTIONS EXTRA_INCLUDE_DIRS EXCLUDE_SOURCE_PATTERNS)
  cmake_parse_arguments(TGT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT TGT_LED)
    set(TGT_LED SK6812_RGBW_LED)
  endif()

  embitz_collect_sources("${TGT_EBP_FILE}" TARGET_SOURCES ${TGT_EBP_TARGET})
  file(GLOB STUB_SOURCES "${CMAKE_SOURCE_DIR}/src/stubs/*.c")
  list(APPEND TARGET_SOURCES ${STUB_SOURCES})

  foreach(PATTERN IN LISTS TGT_EXCLUDE_SOURCE_PATTERNS)
    list(FILTER TARGET_SOURCES EXCLUDE REGEX "${PATTERN}")
  endforeach()

  # STARTUP ersetzt die aus der ebp gewaehlte Startdatei AN IHRER STELLE, damit
  # die Linkreihenfolge gleich bleibt. Gebraucht fuer Black411: Das Original
  # bindet dort startup_stm32f401xc.S, wir bewusst startup_stm32f411xe.S wie das
  # bestehende Ziel f411.
  if(TGT_STARTUP)
    set(REPLACED_SOURCES)
    set(STARTUP_DONE FALSE)
    foreach(SRC IN LISTS TARGET_SOURCES)
      if(SRC MATCHES "/startup_[^/]*\\.S$")
        if(NOT STARTUP_DONE)
          list(APPEND REPLACED_SOURCES "${TGT_STARTUP}")
          set(STARTUP_DONE TRUE)
        endif()
      else()
        list(APPEND REPLACED_SOURCES "${SRC}")
      endif()
    endforeach()
    if(NOT STARTUP_DONE)
      list(APPEND REPLACED_SOURCES "${TGT_STARTUP}")
    endif()
    set(TARGET_SOURCES ${REPLACED_SOURCES})
  endif()

  # Genau eine Startdatei. Zwei linken nicht (doppelte Vektortabelle), keine
  # linkt zwar mit Warnung, startet aber nicht.
  set(STARTUP_SOURCES ${TARGET_SOURCES})
  list(FILTER STARTUP_SOURCES INCLUDE REGEX "/startup_[^/]*\\.S$")
  list(LENGTH STARTUP_SOURCES STARTUP_COUNT)
  if(NOT STARTUP_COUNT EQUAL 1)
    message(FATAL_ERROR "${TARGET_NAME}: ${STARTUP_COUNT} Startdateien statt genau einer: ${STARTUP_SOURCES}")
  endif()

  wordclock_collect_repo_include_dirs(REPO_INCLUDE_DIRS)

  add_executable(${TARGET_NAME} ${TARGET_SOURCES})

  target_include_directories(${TARGET_NAME} PRIVATE
    ${REPO_INCLUDE_DIRS}
    ${TGT_EXTRA_INCLUDE_DIRS}
  )

  target_compile_definitions(${TARGET_NAME} PRIVATE
    ${TGT_COMPILE_DEFINITIONS}
    DSP_USE_LED=${TGT_LED}
    $<$<BOOL:${WORDCLOCK_DEBUG}>:DEBUG>
  )

  target_compile_options(${TARGET_NAME} PRIVATE
    -mthumb
    -mcpu=${TGT_CPU}
    -fdata-sections
    -ffunction-sections
    -fno-strict-aliasing
    -Wall
    -Wextra
    -Wuninitialized
    -Wunused
    -Wunused-function
    -Wunused-parameter
    -Wunused-variable
    -Wunused-value
    $<$<COMPILE_LANGUAGE:ASM>:-Wa,--no-warn>
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
    # LTO nur fuer C. Die Startup-Dateien sind Assembler; -flto hat dort nichts
    # zu optimieren und der Assembler-Treiber reicht es nur durch.
    $<$<COMPILE_LANGUAGE:C>:${WORDCLOCK_LTO_FLAG}>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Release>:-g2>
    ${TGT_EXTRA_COMPILE_OPTIONS}
  )

  if(TGT_FLOAT_ABI)
    target_compile_options(${TARGET_NAME} PRIVATE -mfloat-abi=${TGT_FLOAT_ABI})
    target_link_options(${TARGET_NAME} PRIVATE -mfloat-abi=${TGT_FLOAT_ABI})
  endif()

  if(TGT_FPU)
    target_compile_options(${TARGET_NAME} PRIVATE -mfpu=${TGT_FPU})
    target_link_options(${TARGET_NAME} PRIVATE -mfpu=${TGT_FPU})
  endif()

  target_link_options(${TARGET_NAME} PRIVATE
    -mthumb
    -mcpu=${TGT_CPU}
    # LTO muss beim Link noch einmal stehen: Die eigentliche Optimierung ueber
    # Uebersetzungseinheiten hinweg macht der lto-wrapper erst hier. Fehlt das
    # Flag, liegen die LTO-Objekte zwar vor, werden aber nur noch zusammengefasst
    # -- und der F103 linkt wieder nicht.
    ${WORDCLOCK_LTO_FLAG}
    -T${TGT_LINKER_SCRIPT}
    --specs=nano.specs
    -Wl,--gc-sections
    -Wl,--defsym=__STACK_SIZE=0x0100
    -Wl,--defsym=__HEAP_SIZE=0x0000
    -Wl,-Map,$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.map
    ${TGT_EXTRA_LINK_OPTIONS}
  )

  set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME "${TGT_OUTPUT_NAME}")
  if(TGT_OUTPUT_DIR)
    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${TGT_OUTPUT_DIR}")
  endif()

  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> $<TARGET_FILE_DIR:${TARGET_NAME}>/${TGT_OUTPUT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> $<TARGET_FILE_DIR:${TARGET_NAME}>/${TGT_OUTPUT_NAME}.bin
    COMMENT "Generating HEX/BIN for ${TARGET_NAME}"
  )

  if(CMAKE_SIZE)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET_NAME}>
      COMMENT "Size for ${TARGET_NAME}"
    )
  endif()
endfunction()

set(WCLOCK_MODE_DEFINE WCLOCK24H=0)

wordclock_add_target(wordclock_f103_rgbw
  EBP_FILE ${CMAKE_SOURCE_DIR}/wclock24h-F103/wclock24h-F103.ebp
  CPU cortex-m3
  FLOAT_ABI soft
  LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/wclock24h-F103/stm32f103c8_flash.ld
  OUTPUT_NAME wc12h-stm32f103-sk6812-rgbw
  COMPILE_DEFINITIONS
    STM32F10X
    STM32F10X_MD
    STM32F103
    STM32F103C8
    BLUEPILL_BOARD
    USE_STDPERIPH_DRIVER
    HSE_VALUE=8000000
    ${WCLOCK_MODE_DEFINE}
  EXTRA_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/wclock24h-F103/inc
    ${CMAKE_SOURCE_DIR}/wclock24h-F103/SPL/inc
    ${CMAKE_SOURCE_DIR}/wclock24h-F103/cmsis
  EXCLUDE_SOURCE_PATTERNS
    ".*/src/startup_stm32f4.*\\.S$"
    ".*/src/system_stm32f4xx\\.c$"
)

wordclock_add_target(wordclock_f411_rgbw
  EBP_FILE ${CMAKE_SOURCE_DIR}/wclock24h.ebp
  CPU cortex-m4
  FLOAT_ABI hard
  FPU fpv4-sp-d16
  LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/stm32f411ce_flash.ld
  OUTPUT_NAME wc12h-stm32f411ce-25-sk6812-rgbw
  COMPILE_DEFINITIONS
    ARM_MATH_CM4
    STM32F411CE
    STM32F411xE
    STM32F411xx
    STM32F411
    STM32F4XX
    BLACKPILL_BOARD
    USE_STDPERIPH_DRIVER
    HSE_VALUE=25000000
    ${WCLOCK_MODE_DEFINE}
  EXTRA_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/SPL/inc
    ${CMAKE_SOURCE_DIR}/cmsis
  EXCLUDE_SOURCE_PATTERNS
    ".*/src/startup_stm32f401xc\\.S$"
    ".*/src/startup_stm32f401xe\\.S$"
    ".*/src/startup_stm32f407xx\\.S$"
)

# ---------------------------------------------------------------------------
# Komplettbuild aller STM-Varianten des Ursprungsprojekts ("make matrix").
#
# Vorgabe AUS. Nur das Ziel "matrix" im Makefile konfiguriert ein eigenes
# Verzeichnis (build/matrix) mit -DWORDCLOCK_MATRIX=ON. build/stm-rgbw-12h,
# f103, f411 und release-zip sehen davon nichts: Dort ist die Option aus, und es
# entsteht kein einziges zusaetzliches Ziel.
#
# 72 Varianten wie wordclock-hex.zip des Originals: 12h/24h x sieben Platinen x
# fuenf LED-Typen, dazu je eine F407 mit TFT. Die Parameter stammen aus den
# Release-Zielen der beiden ebp-Dateien; die Quellen waehlt embitz_collect_sources
# ueber den ebp-Zielnamen aus.
#
# Bewusste Abweichungen vom Original, alle drei so gewollt:
#   - F103 setzt zusaetzlich STM32F10X_MD, wie unser Ziel f103. Die Matrix baut
#     damit denselben Code wie der Teilbuild.
#   - Black411 (-25/-8) bindet startup_stm32f411xe.S statt startup_stm32f401xc.S,
#     wie unser Ziel f411 (STARTUP-Ersatz, Entscheidung des Nutzers).
#   - LTO (-flto=4) wie bei den bestehenden Zielen; EmBitz baute ohne.
#
# Die Ausgabe liegt in <build>/hex/. Nebenher entsteht matrix-manifest.txt
# (Variante, Linkerskript); cmake/matrix-report.sh liest daraus den Flash-Umfang
# fuer das Gate.
option(WORDCLOCK_MATRIX "Alle 72 STM-Varianten des Originals bauen (nur fuer make matrix)" OFF)

if(WORDCLOCK_MATRIX)
  set(MATRIX_OUT_DIR "${CMAKE_BINARY_DIR}/hex")
  set(MATRIX_MANIFEST "${CMAKE_BINARY_DIR}/matrix-manifest.txt")
  file(WRITE "${MATRIX_MANIFEST}" "")
  set_property(GLOBAL PROPERTY WORDCLOCK_MATRIX_TARGETS "")

  set(MATRIX_LEDS_WS
    apa102:APA102_RGB_LED
    sk6812-rgb:SK6812_RGB_LED
    sk6812-rgbw:SK6812_RGBW_LED
    ws2812-grb:WS2812_GRB_LED
    ws2812-rgb:WS2812_RGB_LED
  )

  # wordclock_matrix_board(<name-im-zip> EBP_TARGET <WC@MODE@-...> LEDS <key:DEF>...
  #                        <weitere Argumente wie wordclock_add_target>)
  # @MODE@ im ebp-Zielnamen wird zu 12h bzw. 24h.
  function(wordclock_matrix_board BOARD_KEY)
    cmake_parse_arguments(MB "" "EBP_FILE;EBP_TARGET;CPU;FLOAT_ABI;FPU;LINKER_SCRIPT;STARTUP"
      "LEDS;COMPILE_DEFINITIONS;EXTRA_INCLUDE_DIRS;EXCLUDE_SOURCE_PATTERNS" ${ARGN})
    foreach(MODE 12h 24h)
      if(MODE STREQUAL "24h")
        set(MODE_DEFINE WCLOCK24H=1)
      else()
        set(MODE_DEFINE WCLOCK24H=0)
      endif()
      string(REPLACE "@MODE@" "${MODE}" EBP_TARGET_NAME "${MB_EBP_TARGET}")
      foreach(LED_PAIR IN LISTS MB_LEDS)
        string(REPLACE ":" ";" LED_PARTS "${LED_PAIR}")
        list(GET LED_PARTS 0 LED_KEY)
        list(GET LED_PARTS 1 LED_DEFINE)
        set(OUT_NAME "wc${MODE}-${BOARD_KEY}-${LED_KEY}")
        string(REPLACE "-" "_" TGT_NAME "matrix_${OUT_NAME}")
        set(OPTIONAL_ARGS)
        if(MB_FPU)
          list(APPEND OPTIONAL_ARGS FPU ${MB_FPU})
        endif()
        if(MB_STARTUP)
          list(APPEND OPTIONAL_ARGS STARTUP ${MB_STARTUP})
        endif()
        if(MB_EXCLUDE_SOURCE_PATTERNS)
          list(APPEND OPTIONAL_ARGS EXCLUDE_SOURCE_PATTERNS ${MB_EXCLUDE_SOURCE_PATTERNS})
        endif()
        wordclock_add_target(${TGT_NAME}
          EBP_FILE ${MB_EBP_FILE}
          EBP_TARGET ${EBP_TARGET_NAME}
          CPU ${MB_CPU}
          FLOAT_ABI ${MB_FLOAT_ABI}
          LINKER_SCRIPT ${MB_LINKER_SCRIPT}
          OUTPUT_NAME ${OUT_NAME}
          OUTPUT_DIR ${MATRIX_OUT_DIR}
          LED ${LED_DEFINE}
          COMPILE_DEFINITIONS ${MB_COMPILE_DEFINITIONS} ${MODE_DEFINE}
          EXTRA_INCLUDE_DIRS ${MB_EXTRA_INCLUDE_DIRS}
          ${OPTIONAL_ARGS}
        )
        set_property(GLOBAL APPEND PROPERTY WORDCLOCK_MATRIX_TARGETS ${TGT_NAME})
        file(APPEND "${MATRIX_MANIFEST}" "${OUT_NAME} ${MB_LINKER_SCRIPT}\n")
      endforeach()
    endforeach()
  endfunction()

  set(MATRIX_F4_INCLUDES
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/SPL/inc
    ${CMAKE_SOURCE_DIR}/cmsis
  )

  wordclock_matrix_board(stm32f103
    EBP_FILE ${CMAKE_SOURCE_DIR}/wclock24h-F103/wclock24h-F103.ebp
    EBP_TARGET WC@MODE@-STM32F103-Release
    CPU cortex-m3
    FLOAT_ABI soft
    LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/wclock24h-F103/stm32f103c8_flash.ld
    LEDS ${MATRIX_LEDS_WS}
    COMPILE_DEFINITIONS STM32F10X STM32F10X_MD STM32F103 STM32F103C8 BLUEPILL_BOARD USE_STDPERIPH_DRIVER HSE_VALUE=8000000
    EXTRA_INCLUDE_DIRS
      ${CMAKE_SOURCE_DIR}/wclock24h-F103/inc
      ${CMAKE_SOURCE_DIR}/wclock24h-F103/SPL/inc
      ${CMAKE_SOURCE_DIR}/wclock24h-F103/cmsis
    EXCLUDE_SOURCE_PATTERNS
      ".*/src/startup_stm32f4.*\\.S$"
      ".*/src/system_stm32f4xx\\.c$"
  )

  foreach(HSE 25 8)
    wordclock_matrix_board(stm32f401cc-${HSE}
      EBP_FILE ${CMAKE_SOURCE_DIR}/wclock24h.ebp
      EBP_TARGET WC@MODE@-Black401-${HSE}-Release
      CPU cortex-m4 FLOAT_ABI hard FPU fpv4-sp-d16
      LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/stm32f401cc_flash.ld
      LEDS ${MATRIX_LEDS_WS}
      COMPILE_DEFINITIONS ARM_MATH_CM4 STM32F401CC STM32F401xx STM32F401 STM32F4XX BLACKPILL_BOARD USE_STDPERIPH_DRIVER HSE_VALUE=${HSE}000000
      EXTRA_INCLUDE_DIRS ${MATRIX_F4_INCLUDES}
    )
    wordclock_matrix_board(stm32f411ce-${HSE}
      EBP_FILE ${CMAKE_SOURCE_DIR}/wclock24h.ebp
      EBP_TARGET WC@MODE@-Black411-${HSE}-Release
      CPU cortex-m4 FLOAT_ABI hard FPU fpv4-sp-d16
      LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/stm32f411ce_flash.ld
      STARTUP ${CMAKE_SOURCE_DIR}/src/startup_stm32f411xe.S
      LEDS ${MATRIX_LEDS_WS}
      COMPILE_DEFINITIONS ARM_MATH_CM4 STM32F411CE STM32F411xE STM32F411xx STM32F411 STM32F4XX BLACKPILL_BOARD USE_STDPERIPH_DRIVER HSE_VALUE=${HSE}000000
      EXTRA_INCLUDE_DIRS ${MATRIX_F4_INCLUDES}
    )
  endforeach()

  wordclock_matrix_board(stm32f401
    EBP_FILE ${CMAKE_SOURCE_DIR}/wclock24h.ebp
    EBP_TARGET WC@MODE@-Nucleo401-Release
    CPU cortex-m4 FLOAT_ABI hard FPU fpv4-sp-d16
    LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/stm32f401re_flash.ld
    LEDS ${MATRIX_LEDS_WS}
    COMPILE_DEFINITIONS ARM_MATH_CM4 STM32F401RE STM32F401xx STM32F401 STM32F4XX NUCLEO_BOARD USE_STDPERIPH_DRIVER HSE_VALUE=8000000
    EXTRA_INCLUDE_DIRS ${MATRIX_F4_INCLUDES}
  )

  wordclock_matrix_board(stm32f411
    EBP_FILE ${CMAKE_SOURCE_DIR}/wclock24h.ebp
    EBP_TARGET WC@MODE@-Nucleo411-Release
    CPU cortex-m4 FLOAT_ABI hard FPU fpv4-sp-d16
    LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/stm32f411re_flash.ld
    LEDS ${MATRIX_LEDS_WS}
    COMPILE_DEFINITIONS ARM_MATH_CM4 STM32F411RE STM32F411xE STM32F411 STM32F4XX NUCLEO_BOARD USE_STDPERIPH_DRIVER HSE_VALUE=8000000
    EXTRA_INCLUDE_DIRS ${MATRIX_F4_INCLUDES}
  )

  wordclock_matrix_board(stm32f407
    EBP_FILE ${CMAKE_SOURCE_DIR}/wclock24h.ebp
    EBP_TARGET WC@MODE@-Black407-Release
    CPU cortex-m4 FLOAT_ABI hard FPU fpv4-sp-d16
    LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/stm32f407ve_flash.ld
    LEDS tft-rgb:TFTLED_RGB_LED
    COMPILE_DEFINITIONS ARM_MATH_CM4 STM32F407VE STM32F407 STM32F40_41xxx STM32F4XX BLACK_BOARD USE_STDPERIPH_DRIVER HSE_VALUE=8000000
    EXTRA_INCLUDE_DIRS ${MATRIX_F4_INCLUDES}
  )

  get_property(MATRIX_TARGETS GLOBAL PROPERTY WORDCLOCK_MATRIX_TARGETS)
  list(LENGTH MATRIX_TARGETS MATRIX_COUNT)
  if(NOT MATRIX_COUNT EQUAL 72)
    message(FATAL_ERROR "Matrix: ${MATRIX_COUNT} Varianten definiert, erwartet 72")
  endif()
  add_custom_target(wordclock_matrix DEPENDS ${MATRIX_TARGETS})
  message(STATUS "Matrix: ${MATRIX_COUNT} Varianten, Ausgabe ${MATRIX_OUT_DIR}")
endif()
