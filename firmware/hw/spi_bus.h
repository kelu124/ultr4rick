// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef U4RK_SPI_BUS_H
#define U4RK_SPI_BUS_H

/* SPI1 on GPIO26 (SCK) / GPIO27 (TX). The MCP4812 (CS_DAC) and the MAX14866
 * (nLE) share it; each driver owns its own select line. Safe to call twice. */
void u4rk_spi_bus_init(void);

#endif
