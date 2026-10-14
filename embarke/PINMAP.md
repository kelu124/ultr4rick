# embarke pin map (RP2350B, Ultr4rick v2 hardware)

Checked against the `hardware_v2/` netlist (2026-10-14). The firmware constants are in
`hw/u4rk.h`, and the SDK board defaults are in `boards/embarke.h`.

The board has an RP2350B (QFN-80, 48 GPIO), a 12 MHz crystal (U5) and a W25Q128JV (16 MiB QSPI) flash.
There's no radio.

| GPIO | Net | Connected to | Firmware |
|---|---|---|---|
| 0 | ADC CLK | AD9215 clock | PIO0 side-set, 60 MS/s |
| 1–10 | ADC_0..ADC_9 | AD9215 D0..D9 (D9 = MSB on GPIO10) | PIO0 `in` |
| 11 | CS_DAC | MCP4812 ~CS (IC7) | GPIO, active low |
| 21 | nLE | MAX14866 ~LE (U17) | GPIO, idle high |
| 22 | INPUT_A | pulser logic (H5 AND, U7 inverter), 10k R52 | PIO1 pulser, bit 0 |
| 23 | INPUT_B | pulser logic, 10k R53 | PIO1 pulser, bit 1 |
| 24 | I2C0_SDA | LP5562 (U10), MCP4725 (U2) | defined only |
| 25 | I2C0_SCL | LP5562 (U10), MCP4725 (U2) | defined only |
| 26 | CLK | MCP4812 SCK **and** MAX14866 CLK | SPI1 SCK |
| 27 | DIN | MCP4812 SDI **and** MAX14866 DIN | SPI1 TX |
| 28 | SET | MAX14866 SET, 10k R73 | GPIO, idle low |
| 29 | CLR | MAX14866 CLR, 10k R74 | GPIO, idle low |
| 30 | SD_SCK | uSD CLK | defined only |
| 31 | SD_CMD | uSD CMD, 10k pull | defined only |
| 40–43 | SD_D0..D3 | uSD DAT0..DAT3, 10k pulls | defined only |

GPIO21 and GPIO26–29 also go to header H11 (pins 8, 10, 6, 12 and 14).

## Pulser truth table (GPIO22 = A, GPIO23 = B)

| B | A | PIO state | Output |
|---|---|---|---|
| 0 | 0 | 0 | all low (idle) |
| 0 | 1 | 1 | negative pulse |
| 1 | 0 | 2 | positive pulse |
| 1 | 1 | 3 | damping |

One PIO1 state machine plays three phases: first polarity, second polarity, then damping. After
that it drives `00`. The tick is 8 ns. Every phase must last at least 5 ticks (40 ns) because of
the PIO loop overhead.

## Shared SPI1 bus

The MCP4812 and the MAX14866 share SCK and DIN, and each has its own select line:
- **DAC**: CS_DAC goes low, 16 bits are clocked in (channel A, gain 1x), then CS_DAC goes high.
- **MUX**: 16 bits are clocked in, MSB first, then nLE is pulsed low for 10 µs to latch them.
  The MAX14866 has no chip select, so a DAC write also shifts bits into its shift register.
  The switches only change on an nLE pulse, so this does no harm. **Never pulse nLE before shifting
  in a new mux word**, because that latches the DAC's leftover bits.
- Both are SPI mode 0 at 2 MHz (`hw/spi_bus.c`).

## Hardware notes
- The schematic part is the **MAX14866** (U17, 16-channel), so the driver keeps that name.
- On U2 (MCP4725), the `I2C0_SDA` net lands on the pin the symbol calls **SCL**, and `I2C0_SCL` on
  **SDA**. Check this against the MCP4725 footprint before relying on that part.
- GPIO40–43 only exist on the RP2350B. Accessing them from PIO needs `pio_set_gpio_base(pio, 16)`,
  so the SD PIO must run on a PIO block other than the ADC's (PIO0, base 0).
