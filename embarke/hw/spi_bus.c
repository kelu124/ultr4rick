#include "spi_bus.h"

#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "u4rk.h"

/* Both parts sample on the rising edge (mode 0); MAX14866 and MCP4812 are
 * fine well above this rate. */
#define U4RK_SPI_BAUD 2000000u

static bool initialized;

void u4rk_spi_bus_init(void) {
    if (initialized) {
        return;
    }
    spi_init(spi1, U4RK_SPI_BAUD);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(U4RK_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(U4RK_SPI_TX_PIN, GPIO_FUNC_SPI);
    initialized = true;
}
