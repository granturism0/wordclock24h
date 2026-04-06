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
