#include "dac.h"

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "spi_bus.h"
#include "u4rk.h"

static uint16_t last_value;

void u4rk_dac_init(void) {
    gpio_init(U4RK_DAC_CS_PIN);
    gpio_set_dir(U4RK_DAC_CS_PIN, GPIO_OUT);
    gpio_put(U4RK_DAC_CS_PIN, true);

    u4rk_spi_bus_init();
    last_value = 0;
}

bool u4rk_dac_write(uint16_t value) {
    if (value > 1023u) {
        return false;
    }

    /* MCP4812: channel A, unbuffered, gain=1, active; 10 data bits at 11:2. */
    uint16_t command = (uint16_t)(0x3000u | (value << 2));
    uint8_t bytes[2] = {
        (uint8_t)(command >> 8),
        (uint8_t)command,
    };
    gpio_put(U4RK_DAC_CS_PIN, false);
    spi_write_blocking(spi1, bytes, 2);
    gpio_put(U4RK_DAC_CS_PIN, true);
    last_value = value;
    return true;
}

uint16_t u4rk_dac_last_value(void) {
    return last_value;
}
