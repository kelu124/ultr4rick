# SPDX-License-Identifier: GPL-3.0-or-later
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# Ultr4rick targets the RP2350B (48-GPIO, no radio) only: boards/ultr4rick.h.
list(APPEND PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards)
set(PICO_BOARD ultr4rick CACHE STRING "Board type" FORCE)
set(PICO_PLATFORM rp2350-arm-s CACHE STRING "Platform" FORCE)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(adc-pulse C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# ---- Firmware version (single source of truth: version.yaml) ----
file(READ ${CMAKE_CURRENT_LIST_DIR}/version.yaml _version_yaml)
string(REGEX MATCH "version:[ \t]*\"?([0-9]+\\.[0-9]+\\.[0-9]+)\"?" _ "${_version_yaml}")
set(FW_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "changes:[ \t]*\"([^\"]*)\"" _ "${_version_yaml}")
set(FW_CHANGES "${CMAKE_MATCH_1}")
if(NOT FW_VERSION)
    message(FATAL_ERROR "Could not parse 'version:' from version.yaml")
endif()
message(STATUS "ultr4rick firmware version: ${FW_VERSION}")

set(FW_CHIP "RP2350B")

# ---- Release URL, derived from the git remote + fw-v<version> tag scheme ----
execute_process(
    COMMAND git config --get remote.origin.url
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE _git_remote
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
string(REGEX REPLACE "^git@github.com:" "" _slug "${_git_remote}")
string(REGEX REPLACE "^https://github.com/" "" _slug "${_slug}")
string(REGEX REPLACE "\\.git$" "" _slug "${_slug}")
if(_slug MATCHES "^[^/]+/[^/]+$")
    set(FW_RELEASE_URL "https://github.com/${_slug}/releases/tag/fw-v${FW_VERSION}")
else()
    set(FW_RELEASE_URL "n/a")
endif()

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/version.h
    @ONLY
)

# ---- Git build hash: version_git.h, regenerated on every build ----
set(_git_hdr_out ${CMAKE_CURRENT_BINARY_DIR}/generated/version_git.h)
# Generate once now so the header exists at configure time.
execute_process(COMMAND ${CMAKE_COMMAND}
    -DGIT_SRC_DIR=${CMAKE_CURRENT_LIST_DIR}
    -DGIT_HDR_IN=${CMAKE_CURRENT_LIST_DIR}/version_git.h.in
    -DGIT_HDR_OUT=${_git_hdr_out}
    -P ${CMAKE_CURRENT_LIST_DIR}/cmake/git_version.cmake
)
# Refresh it on every build.
add_custom_target(gen_git_version
    COMMAND ${CMAKE_COMMAND}
        -DGIT_SRC_DIR=${CMAKE_CURRENT_LIST_DIR}
        -DGIT_HDR_IN=${CMAKE_CURRENT_LIST_DIR}/version_git.h.in
        -DGIT_HDR_OUT=${_git_hdr_out}
        -P ${CMAKE_CURRENT_LIST_DIR}/cmake/git_version.cmake
    BYPRODUCTS ${_git_hdr_out}
    COMMENT "Capturing git build hash"
    VERBATIM
)

# ---- Onboard-DSP feature toggle ----
# -DDSP=ON builds the raw-TinyUSB Hilbert-envelope / A-law firmware
# (main_dsp.c, CMSIS-DSP); the default is the stdio REPL (main.c).
# The MAX14866 mux is on the board, so MUX support is always built.
option(DSP "Build with the onboard-DSP feature set" OFF)

if(DSP)
    message(STATUS "Onboard-DSP feature: ENABLED (fetching CMSIS-DSP v1.17.0)")
    include(FetchContent)
    FetchContent_Declare(
        CMSISDSP
        GIT_REPOSITORY https://github.com/ARM-software/CMSIS-DSP.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
    )
    set(CMSISCORE
        ${PICO_SDK_PATH}/src/rp2_common/cmsis/stub/CMSIS/Core
        CACHE PATH "CMSIS Core headers" FORCE
    )
    set(DISABLEFLOAT16 ON CACHE BOOL "Disable CMSIS-DSP float16" FORCE)
    FetchContent_MakeAvailable(CMSISDSP)

    # Build only the float32 transform sources the Hilbert backend needs, not the
    # whole CMSIS-DSP archive.
    set_property(TARGET CMSISDSP PROPERTY EXCLUDE_FROM_ALL TRUE)
    add_library(cmsisdsp_u4rk STATIC
        ${cmsisdsp_SOURCE_DIR}/Source/TransformFunctions/arm_bitreversal2.c
        ${cmsisdsp_SOURCE_DIR}/Source/TransformFunctions/arm_cfft_f32.c
        ${cmsisdsp_SOURCE_DIR}/Source/TransformFunctions/arm_cfft_init_f32.c
        ${cmsisdsp_SOURCE_DIR}/Source/TransformFunctions/arm_cfft_radix8_f32.c
        ${cmsisdsp_SOURCE_DIR}/Source/TransformFunctions/arm_rfft_fast_f32.c
        ${cmsisdsp_SOURCE_DIR}/Source/TransformFunctions/arm_rfft_fast_init_f32.c
        ${cmsisdsp_SOURCE_DIR}/Source/CommonTables/arm_common_tables.c
        ${cmsisdsp_SOURCE_DIR}/Source/CommonTables/arm_const_structs.c
    )
    target_include_directories(cmsisdsp_u4rk PUBLIC
        ${cmsisdsp_SOURCE_DIR}/Include
        ${CMSISCORE}/Include
    )
    target_include_directories(cmsisdsp_u4rk PRIVATE
        ${cmsisdsp_SOURCE_DIR}/PrivateInclude
    )
    target_compile_definitions(cmsisdsp_u4rk PUBLIC
        ARM_MATH_CM33
        DISABLEFLOAT16
    )
    target_compile_options(cmsisdsp_u4rk PRIVATE
        -O3
        -ffast-math
        -ffunction-sections
        -fdata-sections
    )

    # ---- u4rk_dsp: the onboard-DSP modules (firmware/dsp/) as a static lib ----
    # P2 lands the USB-free compute/hardware modules. The USB transport
    # (usb_transport.c / usb_descriptors.c) is wired in P3 together with the
    # raw-TinyUSB command loop, since it conflicts with the stdio_usb REPL.
    add_library(u4rk_dsp STATIC
        ${CMAKE_CURRENT_LIST_DIR}/dsp/dsp.c
        ${CMAKE_CURRENT_LIST_DIR}/dsp/pipeline.c
        ${CMAKE_CURRENT_LIST_DIR}/dsp/protocol.c
        ${CMAKE_CURRENT_LIST_DIR}/hw/acquisition.c
        ${CMAKE_CURRENT_LIST_DIR}/hw/dac.c
        ${CMAKE_CURRENT_LIST_DIR}/hw/spi_bus.c
        ${CMAKE_CURRENT_LIST_DIR}/hw/led.c
    )
    pico_generate_pio_header(u4rk_dsp ${CMAKE_CURRENT_LIST_DIR}/hw/acquisition.pio)
    pico_generate_pio_header(u4rk_dsp ${CMAKE_CURRENT_LIST_DIR}/hw/pulser.pio)
    target_include_directories(u4rk_dsp PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/dsp
        ${CMAKE_CURRENT_LIST_DIR}/hw
    )
    target_compile_definitions(u4rk_dsp PUBLIC
        ARM_MATH_CM33
        DISABLEFLOAT16
    )
    target_link_libraries(u4rk_dsp PUBLIC
        pico_stdlib
        pico_multicore
        pico_util
        hardware_pio
        hardware_dma
        hardware_clocks
        hardware_gpio
        hardware_spi
        hardware_i2c
        cmsisdsp_u4rk
    )
else()
    message(STATUS "Onboard-DSP feature: disabled")
endif()

# Add executable. Default name is the project name.

add_executable(adc-pulse)

pico_set_program_name(adc-pulse "adc-pulse")
pico_set_program_version(adc-pulse "${FW_VERSION}")

# Ensure the git hash header is refreshed before adc-pulse compiles.
add_dependencies(adc-pulse gen_git_version)

pico_enable_stdio_uart(adc-pulse 0)

# Common include dirs (firmware root for max/*, and the generated version headers)
target_include_directories(adc-pulse PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

# MAX14866 HV mux (on SPI1, shared with the DAC); always on this board.
target_sources(adc-pulse PRIVATE ${CMAKE_CURRENT_LIST_DIR}/max/max14866.c)
target_compile_definitions(adc-pulse PRIVATE MUX=1)

if(DSP)
    # ---- RP2350 DSP build: raw-TinyUSB command loop (main_dsp.c) ----
    # SDK stdio is OFF; usb_transport/usb_descriptors drive TinyUSB directly so
    # text + binary frames share one CDC.
    pico_enable_stdio_usb(adc-pulse 0)
    target_sources(adc-pulse PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main_dsp.c
        ${CMAKE_CURRENT_LIST_DIR}/dsp/usb_transport.c
        ${CMAKE_CURRENT_LIST_DIR}/dsp/usb_descriptors.c
    )
    target_include_directories(adc-pulse PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/dsp
        ${CMAKE_CURRENT_LIST_DIR}/hw
    )
    target_compile_definitions(adc-pulse PRIVATE
        DSP=1
        U4RK_CMSIS_DSP_VERSION="1.17.0"
    )
    target_link_libraries(adc-pulse PRIVATE
        pico_stdlib
        pico_bootrom
        pico_multicore
        pico_util
        hardware_pio
        hardware_dma
        hardware_clocks
        hardware_gpio
        hardware_spi
        hardware_i2c
        tinyusb_device
        u4rk_dsp
    )
else()
    # ---- stdio REPL build ----
    # Uses the SAME shared acquisition + pulser + spi1 DAC as the DSP build
    # (firmware/hw), in raw-8000 mode. No FFT/CMSIS.
    pico_enable_stdio_usb(adc-pulse 1)
    pico_generate_pio_header(adc-pulse ${CMAKE_CURRENT_LIST_DIR}/hw/acquisition.pio)
    pico_generate_pio_header(adc-pulse ${CMAKE_CURRENT_LIST_DIR}/hw/pulser.pio)
    target_sources(adc-pulse PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/hw/acquisition.c
        ${CMAKE_CURRENT_LIST_DIR}/hw/dac.c
        ${CMAKE_CURRENT_LIST_DIR}/hw/spi_bus.c
        ${CMAKE_CURRENT_LIST_DIR}/hw/led.c
    )
    target_include_directories(adc-pulse PRIVATE ${CMAKE_CURRENT_LIST_DIR}/hw)
    target_link_libraries(adc-pulse PRIVATE
        pico_stdlib
        pico_bootrom
        hardware_pio
        hardware_dma
        hardware_clocks
        hardware_gpio
        hardware_spi
        hardware_i2c
    )
endif()

# Run from SRAM: the whole image is copied out of flash at boot. With XIP the
# core-1 FFT time depended on where code/tables happened to land in flash
# (13.4 to 20.3 ms for identical DSP code across three builds); from RAM it
# no longer depends on the layout or on what core 0 runs. Uses ~110 KB of
# the 520 KB SRAM on top of the ~170 KB of buffers.
pico_set_binary_type(adc-pulse copy_to_ram)

pico_add_extra_outputs(adc-pulse)