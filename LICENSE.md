# Licenses

Ultr4rick is open hardware, open software and open documentation. Each part of the repository has its own license.
The full texts are in [`LICENSES/`](LICENSES/).

| Part | Paths | License | Text |
|---|---|---|---|
| Hardware design | `hardware/` | TAPR Open Hardware License v1.0 | [`LICENSES/TAPR-OHL-1.0.txt`](LICENSES/TAPR-OHL-1.0.txt) |
| Firmware | `firmware/` | GNU General Public License v3.0 or later (`GPL-3.0-or-later`) | [`LICENSES/GPL-3.0-or-later.txt`](LICENSES/GPL-3.0-or-later.txt) |
| Python library, scripts and notebooks | `python/` | GNU General Public License v3.0 or later (`GPL-3.0-or-later`) | [`LICENSES/GPL-3.0-or-later.txt`](LICENSES/GPL-3.0-or-later.txt) |
| Documentation | `docs/`, the Markdown files at the root and in the other folders | Creative Commons Attribution-ShareAlike 4.0 International (`CC-BY-SA-4.0`) | [`LICENSES/CC-BY-SA-4.0.txt`](LICENSES/CC-BY-SA-4.0.txt) |

Copyright © the Ultr4rick contributors.

The firmware and Python code are free software: you can redistribute them and/or modify them under the terms of the
GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version. They are distributed in the hope that they will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

## Third-party code

These files are not covered by the table above and keep their own licenses:

- `firmware/pico_sdk_import.cmake` is a copy from the Raspberry Pi Pico SDK (BSD-3-Clause, Raspberry Pi Ltd).
- The Pico SDK, TinyUSB and CMSIS-DSP are not stored in this repository. They are fetched or referenced at build time
  and keep their own licenses (BSD-3-Clause, MIT and Apache-2.0 respectively). Firmware images built from this
  repository include them.
