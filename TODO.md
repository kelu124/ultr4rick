# Changes from V2.0

- Remove VGA, add 2.54mm headers 
- Flash button closer to USB
- Swap orientation of SD card
- Move RGB closer to rp2350
- Fix I2C on U2 (MCP4725): in the v2 schematic the `I2C0_SDA` net (GPIO24) goes to its SCL pin (4→5)
  and `I2C0_SCL` (GPIO25) to its SDA pin. The miswired part can disturb the bus and also break the
  LP5562 (U10) on the same lines. Swap the two nets at U2 (or check the symbol against the SOT-23-6
  pinout: 4 = SDA, 5 = SCL).
