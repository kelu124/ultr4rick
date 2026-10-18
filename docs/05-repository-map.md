# 5. Repository map

```
ultr4rick/
├── README.md            short overview and quick start
├── LICENSE.md           which license covers which part
├── LICENSES/            full license texts: TAPR OHL 1.0, GPL-3.0-or-later, CC BY-SA 4.0
├── TODO.md              hardware changes planned for the next board revision
├── requirements.txt     Python dependencies
├── docs/                this documentation (versioned, see version.yaml)
├── firmware/            RP2350B firmware (C, Pico SDK)
├── python/              host library, examples and notebooks
├── hardware/            KiCad project of the v2 board
└── .claude/             notes and procedures used by Claude Code
```

## Licenses

| Part | License |
|---|---|
| `hardware/` | [TAPR Open Hardware License v1.0](../LICENSES/TAPR-OHL-1.0.txt) |
| `firmware/`, `python/` | [GPL-3.0-or-later](../LICENSES/GPL-3.0-or-later.txt) |
| `docs/` and the Markdown files | [CC BY-SA 4.0](../LICENSES/CC-BY-SA-4.0.txt) |

[`LICENSE.md`](../LICENSE.md) also lists the third-party code (Pico SDK, TinyUSB, CMSIS-DSP).

## `docs/`

| Path | Contents |
|---|---|
| [`README.md`](README.md) | index of the pages |
| `01-…` to `05-…` | the pages |
| [`version.yaml`](version.yaml) | docs version, and the firmware / Python versions the pages were checked against |
| [`CHANGELOG.md`](CHANGELOG.md) | docs history |
| [`images/`](images/README.md) | photos, one folder per hardware version, each described in `images/README.md` |

## `firmware/`

| Path | Contents |
|---|---|
| `main.c` | stdio build: text console |
| `main_dsp.c` | DSP build: command parser, capture and stream scheduling, frame output (core 0) |
| `hw/` | hardware layer: `acquisition.{c,pio}` (ADC, PIO0 + DMA), `pulser.pio` (PIO1), `dac.c` (MCP4812), `spi_bus.c` (shared SPI1), `led.c` (LP5562), `u4rk.h` (pins and constants) |
| `max/` | MAX14866 mux driver |
| `dsp/` | `dsp.c` (Hilbert envelope, A-law), `pipeline.c` (core 1), `protocol.c` (frame header, CRC32), `usb_transport.c`, `usb_descriptors.c`, `tusb_config.h` |
| `boards/ultr4rick.h` | Pico SDK board definition |
| `CMakeLists.txt`, `build.sh`, `cmake/`, `pico_sdk_import.cmake` | build; `build.sh` makes both UF2s in `dist/` |
| `version.yaml`, `version.h.in`, `version_git.h.in`, `CHANGELOG.md` | firmware version and history |
| [`PINMAP.md`](../firmware/PINMAP.md) | every GPIO, the pulser truth table, the shared SPI bus |

Build outputs (`build/`, `build-dsp/`, `dist/`, `.deps/`) are not tracked.

## `python/`

| Path | Contents |
|---|---|
| `ultr4rick/device.py` | `Ultr4rick` serial driver |
| `ultr4rick/dsp.py` | `U4RK` frame reader, status parser, A-law decoder, self-test check |
| `ultr4rick/ndt_acquisition.py` | NDT capture, analysis and HDF5 storage |
| `ultr4rick/version.yaml`, `ultr4rick/__init__.py` | library version |
| `example_dsp.ipynb`, `example_dsp.py`, `example_simple.ipynb`, `example_acquisition.ipynb`, `example_process.ipynb` | examples (see [Python library](04-python-library.md#examples)) |
| `data/` | notebook outputs: captures, figures, HDF5 files |
| `Readme.md` | package notes |

## `hardware/`

KiCad must be closed before you move or rename anything here.

| Path | Contents |
|---|---|
| `Ultr4rick.kicad_pro`, `Ultr4rick.kicad_sch`, `Ultr4rick.kicad_pcb` | project, top-level schematic, PCB (120 × 80 mm) |
| `RP2350B.kicad_sch` | microcontroller, flash, crystal |
| `DAC+AFE.kicad_sch` | gain DAC and receive front end, ADC |
| `PULSER.kicad_sch` | bipolar HV pulser |
| `SEPIC_HV.kicad_sch` | ±HV supply |
| `MUX.kicad_sch` | MAX14866 mux and the 8 element connectors |
| `USB3.kicad_sch`, `uSD.kicad_sch`, `PMODS.kicad_sch` | USB, uSD slot, expansion headers |
| `Ultr4rick_F.png`, `Ultr4rick_B.png` | board renders |
| `production/`, `gerber_to_order/` | BOM, placement, netlist and gerbers for manufacturing |
| `MAX14866_PMOD.pdf` | MAX14866 PMOD reference |

## `.claude/`

| Path | Contents |
|---|---|
| `skills/` | procedures: firmware, build and flash, board test, change logging, versioning, docs, docs images |
| `CHANGELOG.md`, `commits/` | log of the changes made with Claude Code |
| `TODO.md`, `DONE.md` | Claude's open and finished tasks |

The old Ultr4rick 1.5 firmware and the v1 hardware are in git history at commit `4da14a5`.
