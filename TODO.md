# Changes from V2.0

- Remove VGA, add 2.54mm headers 
- Flash button closer to USB
- Swap orientation of SD card
- Move RGB closer to rp2350
- Fix I2C on U2 (MCP4725): in the v2 schematic the `I2C0_SDA` net (GPIO24) goes to its SCL pin (4→5)
  and `I2C0_SCL` (GPIO25) to its SDA pin. The miswired part can disturb the bus and also break the
  LP5562 (U10) on the same lines. Swap the two nets at U2 (or check the symbol against the SOT-23-6
  pinout: 4 = SDA, 5 = SCL).
- Add I2C pull-ups on I2C0 (GPIO24/25). The v2 schematic has none on SDA/SCL. The firmware runs the LP5562 at 100 kHz
  on the RP2350's internal pull-ups, which works but is marginal.
- There are two TR4 on the silkscreen in the back

