# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Make the project-specific RP2350B definition visible before the Pico SDK
# selects a platform/package.
list(APPEND PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards)

# This package is hardware-specific. Force the custom board even when the VS
# Code extension remembers a different board from another Pico project.
set(PICO_BOARD ultr4rick CACHE STRING "Board type" FORCE)

include(pico_sdk_import.cmake)

project(adc-pulse C CXX ASM)
pico_sdk_init()

add_executable(adc-pulse)
pico_set_program_name(adc-pulse "adc-pulse")

if(PICO_BOARD STREQUAL "ultr4rick")
    pico_set_program_version(adc-pulse "1.5")
    pico_enable_stdio_uart(adc-pulse 0)
    pico_enable_stdio_usb(adc-pulse 0)

    include(FetchContent)
    FetchContent_Declare(
        CMSISDSP
        GIT_REPOSITORY https://github.com/ARM-software/CMSIS-DSP.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
    )

    # CMSIS-DSP uses the Pico SDK's CMSIS Core headers. Float16 and MVE are
    # deliberately disabled: RP2350 has a Cortex-M33 scalar FPU, not Helium.
    set(CMSISCORE
        ${PICO_SDK_PATH}/src/rp2_common/cmsis/stub/CMSIS/Core
        CACHE PATH "CMSIS Core headers" FORCE
    )
    set(DISABLEFLOAT16 ON CACHE BOOL "Disable CMSIS-DSP float16" FORCE)
    FetchContent_MakeAvailable(CMSISDSP)

    # Build only the float32 transform sources this firmware uses. Building the
    # complete CMSIS-DSP archive needlessly compiles every Q7/Q15/Q31/float64
    # module and has exposed GCC 15 failures in code that is never linked.
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

    pico_generate_pio_header(adc-pulse
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick/acquisition.pio)
    pico_generate_pio_header(adc-pulse
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick/pulser.pio)

    target_sources(adc-pulse PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick/main.c
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick/acquisition.c
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick/dac.c
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick/dsp.c
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick/pipeline.c
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick/protocol.c
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick/usb_transport.c
    )
    target_include_directories(adc-pulse PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/ultr4rick
        ${cmsisdsp_SOURCE_DIR}/Include
        ${CMSISCORE}/Include
    )
    target_compile_definitions(adc-pulse PRIVATE
        ARM_MATH_CM33
        DISABLEFLOAT16
        U4RK_CMSIS_DSP_VERSION="1.17.0"
    )
    target_compile_options(adc-pulse PRIVATE
        -O3
        -ffast-math
        -ffunction-sections
        -fdata-sections
    )
    target_link_options(adc-pulse PRIVATE -Wl,--gc-sections)
    target_link_libraries(adc-pulse PRIVATE
        pico_stdlib
        pico_multicore
        pico_util
        hardware_pio
        hardware_dma
        hardware_clocks
        hardware_gpio
        hardware_spi
        tinyusb_device
        cmsisdsp_u4rk
        m
    )
else()
    # The original RP2040 firmware remains intact, including MAX14866 and its
    # legacy PIO-driven DAC.
    pico_set_program_version(adc-pulse "0.1")
    pico_enable_stdio_uart(adc-pulse 0)
    pico_enable_stdio_usb(adc-pulse 1)

    pico_generate_pio_header(adc-pulse
        ${CMAKE_CURRENT_LIST_DIR}/adc/adc.pio)
    pico_generate_pio_header(adc-pulse
        ${CMAKE_CURRENT_LIST_DIR}/max/max14866.pio)

    target_sources(adc-pulse PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/adc/adc.c
        ${CMAKE_CURRENT_LIST_DIR}/max/max14866.c
    )
    target_include_directories(adc-pulse PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(adc-pulse PRIVATE
        pico_stdlib
        hardware_pio
        hardware_dma
        hardware_clocks
        hardware_gpio
        hardware_irq
    )
endif()

pico_add_extra_outputs(adc-pulse)
