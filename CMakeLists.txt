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

function(embitz_collect_sources EBP_FILE OUT_VAR)
  get_filename_component(EBP_DIR "${EBP_FILE}" DIRECTORY)
  file(STRINGS "${EBP_FILE}" UNIT_LINES REGEX "<Unit filename=")
  set(SOURCES)
  foreach(LINE IN LISTS UNIT_LINES)
    string(REGEX MATCH "<Unit filename=\"([^\"]+)\"" _ "${LINE}")
    set(UNIT_FILE "${CMAKE_MATCH_1}")
    if(UNIT_FILE MATCHES "\\.(c|S)$")
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
  set(oneValueArgs EBP_FILE FAMILY CPU FLOAT_ABI FPU LINKER_SCRIPT HSE BOARD_DEFINE MODE_DEFINE MCU_DEFINE OUTPUT_NAME)
  set(multiValueArgs COMPILE_DEFINITIONS EXTRA_COMPILE_OPTIONS EXTRA_LINK_OPTIONS EXTRA_INCLUDE_DIRS EXCLUDE_SOURCE_PATTERNS)
  cmake_parse_arguments(TGT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  embitz_collect_sources("${TGT_EBP_FILE}" TARGET_SOURCES)
  file(GLOB STUB_SOURCES "${CMAKE_SOURCE_DIR}/src/stubs/*.c")
  list(APPEND TARGET_SOURCES ${STUB_SOURCES})

  foreach(PATTERN IN LISTS TGT_EXCLUDE_SOURCE_PATTERNS)
    list(FILTER TARGET_SOURCES EXCLUDE REGEX "${PATTERN}")
  endforeach()

  wordclock_collect_repo_include_dirs(REPO_INCLUDE_DIRS)

  add_executable(${TARGET_NAME} ${TARGET_SOURCES})

  target_include_directories(${TARGET_NAME} PRIVATE
    ${REPO_INCLUDE_DIRS}
    ${TGT_EXTRA_INCLUDE_DIRS}
  )

  target_compile_definitions(${TARGET_NAME} PRIVATE
    ${TGT_COMPILE_DEFINITIONS}
    DSP_USE_LED=SK6812_RGBW_LED
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
