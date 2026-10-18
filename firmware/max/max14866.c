// SPDX-License-Identifier: GPL-3.0-or-later
//!
//! \file       max14866.c
//! \author     Abdelrahman Ali
//! \date       2025-01-06
//!
//! \brief      max14866 HV mux on the shared SPI1 bus.
//!
//! SCK/DIN (GPIO26/27) are shared with the MCP4812 gain DAC. The MAX14866 has
//! no chip select, so every DAC write also clocks 16 bits into its shift
//! register; that is harmless because nLE stays high and the switches only
//! change on the nLE pulse that follows a mux write.
//!

//---------------------------------------------------------------------------
// INCLUDES
//---------------------------------------------------------------------------

#include "max14866.h"

#include "hardware/spi.h"

#include "spi_bus.h"
#include "u4rk.h"

//---------------------------------------------------------------------------
// MAX INIT FUNCTION
//---------------------------------------------------------------------------
void max14866_init()
{
    gpio_init(U4RK_MUX_LE_PIN);
    gpio_init(U4RK_MUX_SET_PIN);
    gpio_init(U4RK_MUX_CLR_PIN);
    gpio_put(U4RK_MUX_SET_PIN, 0);
    gpio_put(U4RK_MUX_CLR_PIN, 0);
    gpio_put(U4RK_MUX_LE_PIN, 1);
    gpio_set_dir(U4RK_MUX_LE_PIN, GPIO_OUT);
    gpio_set_dir(U4RK_MUX_SET_PIN, GPIO_OUT);
    gpio_set_dir(U4RK_MUX_CLR_PIN, GPIO_OUT);
    u4rk_spi_bus_init();
}

//---------------------------------------------------------------------------
// MAX WRITE FUNCTION
//---------------------------------------------------------------------------
void max14866_write(uint16_t data)
{
    // Shift all 16 bits first, then latch them with one nLE pulse. Pulsing nLE
    // before the shift would latch whatever the last DAC write left behind.
    uint8_t bytes[2] = {(uint8_t)(data >> 8), (uint8_t)data};
    spi_write_blocking(spi1, bytes, 2);
    gpio_put(U4RK_MUX_LE_PIN, 0);
    sleep_us(10);
    gpio_put(U4RK_MUX_LE_PIN, 1);
}

//---------------------------------------------------------------------------
// MAX SET FUNCTION
//---------------------------------------------------------------------------
void max14866_set(const char *input) {
    gpio_put(U4RK_MUX_SET_PIN, 1);
    sleep_us(5);
    gpio_put(U4RK_MUX_SET_PIN, 0);
}

//---------------------------------------------------------------------------
// MAX CLEAR FUNCTION
//---------------------------------------------------------------------------
void max14866_clear(const char *input) {
    gpio_put(U4RK_MUX_CLR_PIN, 1);
    sleep_us(5);
    gpio_put(U4RK_MUX_CLR_PIN, 0);
}

//---------------------------------------------------------------------------
// MAX MAIN FUNCTION
//---------------------------------------------------------------------------
void max14866(const char *input)
{
    uint16_t data = (uint16_t)strtol(input, NULL, 16);
    printf("MAX14866 writing started\n");
    max14866_write(data);
    printf("MAX14866 writing ended\n");
}

//---------------------------------------------------------------------------
// END OF FILE
//---------------------------------------------------------------------------
