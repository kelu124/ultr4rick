/*
 * Ultr4rick RP2350B board definition.
 *
 * This intentionally does not inherit boards/pico2.h: that header selects the
 * 30-GPIO RP2350A package and assigns GPIO23/24/29 to Pico 2 board functions.
 */
#ifndef _BOARDS_ULTR4RICK_H
#define _BOARDS_ULTR4RICK_H

pico_board_cmake_set(PICO_PLATFORM, rp2350)

#define ULTR4RICK
#define PICO_RP2350A 0

/* The schematic uses the standard 12 MHz crystal. */
#ifndef PICO_XOSC_FREQ_HZ
#define PICO_XOSC_FREQ_HZ 12000000
#endif

/* Board I2C bus (preserved; not initialized by the signal-processing build). */
#ifndef PICO_DEFAULT_I2C
#define PICO_DEFAULT_I2C 0
#endif
#ifndef PICO_DEFAULT_I2C_SDA_PIN
#define PICO_DEFAULT_I2C_SDA_PIN 24
#endif
#ifndef PICO_DEFAULT_I2C_SCL_PIN
#define PICO_DEFAULT_I2C_SCL_PIN 25
#endif

/* MCP4812 jumper bus: GPIO22 -> SCLK1 and GPIO23 -> MOSI1. */
#ifndef PICO_DEFAULT_SPI
#define PICO_DEFAULT_SPI 0
#endif
#ifndef PICO_DEFAULT_SPI_SCK_PIN
#define PICO_DEFAULT_SPI_SCK_PIN 22
#endif
#ifndef PICO_DEFAULT_SPI_TX_PIN
#define PICO_DEFAULT_SPI_TX_PIN 23
#endif
#ifndef PICO_DEFAULT_SPI_CSN_PIN
#define PICO_DEFAULT_SPI_CSN_PIN 11
#endif

/* W25Q128JV: 16 MiB external QSPI flash. */
#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1
#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif
pico_board_cmake_set_default(PICO_FLASH_SIZE_BYTES, (16 * 1024 * 1024))
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)
#endif

pico_board_cmake_set_default(PICO_RP2350_A2_SUPPORTED, 1)
#ifndef PICO_RP2350_A2_SUPPORTED
#define PICO_RP2350_A2_SUPPORTED 1
#endif

#endif
