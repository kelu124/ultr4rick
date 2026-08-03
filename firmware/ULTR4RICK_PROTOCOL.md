# Ultr4rick RP2350B firmware test guide

This guide covers testing the installed Ultr4rick signal-processing firmware.
It assumes that the included `ultr4rick-envelope.uf2` (or the equivalent
`build/adc-pulse.uf2` produced by VS Code) has already been loaded.

## What this firmware implements

- 4,096-sample, 10-bit ADC captures at 60 MS/s
- Hilbert-envelope calculation using CMSIS-DSP
- Positive-envelope 8-bit A-law compression with `A = 87.6`
- Raw, envelope, and A-law one-shot captures
- Continuous raw, envelope, and A-law streaming
- MCP4812 DAC control
- Safe, explicitly armed bipolar pulser control
- USB CDC commands and CRC-protected binary frames

MAX14866 control is intentionally not present in this RP2350B firmware.

## Connections and initial safety

The MCP4812 requires these jumpers:

- GPIO22 (`SPI0 SCK`) to `SCLK1`
- GPIO23 (`SPI0 TX`) to `MOSI1`

`CS_DAC` is already routed to GPIO11. GPIO22 and GPIO23 are unavailable for
other PMOD functions while the jumpers are fitted.

Keep high voltage disabled during initial validation.

The pulser controls are:

- GPIO27: input A / `P+`
- GPIO28: input B / `PDAMP`

Their schematic truth table, written as `GPIO27 GPIO28`, is:

| GPIO27 | GPIO28 | State |
| ---: | ---: | --- |
| 0 | 0 | Idle/open |
| 1 | 0 | Negative pulse |
| 1 | 1 | Damping |
| 0 | 1 | Positive pulse |

At boot both pins must remain at `00`. The pulser starts disarmed and is also
disarmed by USB disconnect, acquisition/DMA fault, reset, and stream stop.

Default settings after every reboot are:

- Pulse order: negative first
- Negative duration: 96 ns
- Damping duration: 6000 ns
- Positive duration: 96 ns
- A-law reference: 512 ADC counts

## PC requirements

The board appears as a USB CDC COM port after the firmware starts. Replace
`COM7` in the examples with the port assigned by the operating system.

Install the host-tool requirements from the firmware directory:

```powershell
python -m pip install -r tools/requirements.txt
```

`tools/ultr4rick_capture.py` is the board-test program. It opens the COM port,
sends the required firmware command, receives binary frames, checks their
length and CRC, detects sequence gaps, and saves the results.

## 1. Automated DSP self-test

Run:

```powershell
python tools/ultr4rick_capture.py --port COM7 --selftest --output captures/selftest
```

The utility sends `dsp selftest` automatically. No command needs to be entered
manually. The firmware returns raw, float-envelope, and A-law frames for each
of these seven deterministic signals. Before starting, the utility checks that
the board reports firmware `1.5`; an older UF2 is rejected.

1. Zero
2. DC
3. Centred sinusoid
4. Amplitude-modulated tone
5. Two separated bursts
6. Impulse
7. Clipping/extreme square wave

The utility compares the received firmware results with
`scipy.signal.hilbert`. The test passes only when:

- All 21 frames are received with valid CRCs
- No sequence gap is detected
- Normalized envelope RMS error is no greater than `1e-4`
- Envelope peak index differs by no more than one sample
- A-law output differs by no more than one byte level

Successful output prints one result line for every signal without a final
`ERROR` message.

The output directory contains:

- `raw.npy`: received raw self-test signals
- `envelope.npy`: received float envelopes
- `alaw.npy`: received A-law bytes
- `alaw_decoded.npy`: A-law data expanded into approximate ADC-count amplitude
- `headers.json`: header and measurement information for every frame

This is the primary functional test of the DSP implementation running on the
RP2350B.

## 2. One-shot acquisition tests

Capture one raw ADC frame:

```powershell
python tools/ultr4rick_capture.py --port COM7 --mode raw --output captures/raw
```

Capture one float-envelope frame:

```powershell
python tools/ultr4rick_capture.py --port COM7 --mode envelope --output captures/envelope
```

Capture one A-law frame:

```powershell
python tools/ultr4rick_capture.py --port COM7 --mode alaw --output captures/alaw
```

The program automatically sends `acq raw`, `acq envelope`, or `acq alaw`.
Each returned frame must contain exactly 4,096 samples and pass its CRC check.

For an external-signal test, keep the pulser disarmed and feed a known,
continuous tone or burst waveform into the ADC. Check:

- `raw.npy` contains 10-bit values in the range `0..1023`
- `headers.json` reports `sample_count: 4096`
- `headers.json` reports `sample_rate_hz: 60000000`
- `adc_dc_mean` is consistent with the applied ADC bias
- The envelope follows the applied signal amplitude
- A-law values remain in `0..255`

## 3. A-law scaling and saturation

The default A-law reference is `512.0` ADC counts. It can be changed while
streaming is stopped:

```text
dsp scale <reference>
```

For example:

```text
dsp scale 300
```

The reference must be greater than zero and no greater than `65535`. It is
runtime-only and returns to `512.0` after reboot.

Envelope values above the reference are clipped before encoding. Header flag
bit 0 is set when clipping occurs. The reference used for the frame is also
stored in the binary header and in `headers.json`.

## 4. Continuous-stream test

Supported rate limits are:

| Payload | Allowed rate |
| --- | ---: |
| Raw `uint16` | 1–100 Hz |
| Float envelope | 1–50 Hz |
| A-law `uint8` | 1–70 Hz |

Rates above these limits are rejected.

The original 200 Hz A-law requirement is **not met by the exact 4,096-sample
Hilbert implementation**. The same Cortex-M33/FPU pipeline measured about
12.98 ms worst case on a Pico 2 W (RP2350A), so the firmware uses a conservative
70 Hz limit rather than accepting an unsustainable rate and silently dropping
frames. Reaching 200 Hz requires a different streaming envelope detector or a
reduced transform/window; that would change the signal-processing design.

Run a 60-second A-law stability test at the current exact-Hilbert limit:

```powershell
python tools/ultr4rick_capture.py --port COM7 --mode alaw --rate 70 --frames 4200 --timeout 30 --output captures/alaw-60s
```

Reboot the board immediately before this acceptance test so the cumulative
drop counter and worst-case timing start from a known state.

The utility sends `stream start alaw 70`, receives 4,200 frames, sends
`stream stop`, and requests final status. During streaming, the firmware emits
only binary frames; normal command text is intentionally suppressed until the
stream has stopped.

Acceptance requires:

- 4,200 frames received
- No CRC error
- No sequence-gap error
- `drops=0` on every displayed frame
- `dropped_frames: 0` in every `headers.json` entry
- Final `status.txt` reports `drops=0`

`status.txt` also reports the DSP backend, stage timings, and `worst_us`.
`performance=ok` is reserved for the original 200 Hz timing target:

```text
worst_us <= 4500
performance=ok
```

With the current exact-Hilbert backend, `performance=over-budget` is expected
unless an on-board benchmark proves otherwise. The 70 Hz stability test still
requires zero drops, sequence gaps, and CRC failures.

The capture directory contains `alaw.npy`, `alaw_decoded.npy`,
`headers.json`, and `status.txt`.

## 5. ADC clock and exact capture length

With high voltage disabled, trigger a one-shot raw acquisition while observing
GPIO0:

```powershell
python tools/ultr4rick_capture.py --port COM7 --mode raw --output captures/adc-clock
```

Verify:

- GPIO0 produces a 60 MHz ADC clock during capture
- The returned payload contains exactly 4,096 samples
- No acquisition timeout or `ERR DMA` response occurs

The ADC data bus is GPIO1 through GPIO10, corresponding to D0 through D9.

## 6. Pulser tests

Use a USB serial terminal for pulser tests. USB CDC does not depend on the
selected terminal baud rate; `115200` is suitable.

The pulser must be explicitly armed:

```text
pulser arm
```

Configure and test the default negative-first sequence:

```text
pulse config 96 6000 96 neg-first
pulser arm
start acq
pulser disarm
```

`start acq` is useful for oscilloscope testing because it performs the capture
without sending a binary frame. While armed, that acquisition also produces
one pulse sequence.

Verify on GPIO27/28:

```text
00 idle -> 10 negative -> 11 damping -> 01 positive -> 00 idle
```

Then test positive-first:

```text
pulse config 96 6000 96 pos-first
pulser arm
start acq
pulser disarm
```

Verify:

```text
00 idle -> 01 positive -> 11 damping -> 10 negative -> 00 idle
```

All requested durations are rounded to the nearest 8 ns PIO tick. Verify each
state duration within one 8 ns tick. Also confirm that both pins immediately
return to `00` after `pulser disarm`.

Do not leave the pulser armed after a test.

## 7. MCP4812 DAC test

With GPIO22 and GPIO23 connected to `SCLK1` and `MOSI1`, send these commands
from a USB serial terminal while streaming is stopped:

```text
dac write 0
dac write 512
dac write 1023
```

Verify:

- GPIO22 carries a 2 MHz, SPI mode-0 clock during each write
- GPIO23 carries the corresponding SPI data
- GPIO11 (`CS_DAC`) goes low for the 16-bit transfer
- The DAC output corresponds to zero, midpoint, and full-scale codes

The firmware controls MCP4812 channel A with gain 1. Values outside `0..1023`
are rejected.

## Command reference

Commands are ASCII lines sent over the USB CDC COM port:

| Command | Purpose |
| --- | --- |
| `status` | Show configuration, drop counters, and DSP timings |
| `help` | Show the firmware command summary |
| `pulser arm` | Allow the next acquisitions to produce a pulse |
| `pulser disarm` | Disable the pulser and force GPIO27/28 to `00` |
| `pulse config <negative_ns> <damp_ns> <positive_ns> <neg-first\|pos-first>` | Configure pulse timing and order |
| `dac write <0..1023>` | Write MCP4812 channel A |
| `dsp scale <reference>` | Set the runtime A-law reference |
| `dsp selftest` | Emit the 21 deterministic test frames |
| `acq <raw\|envelope\|alaw>` | Request one binary capture |
| `stream start <raw\|envelope\|alaw> <rate_hz>` | Start binary streaming |
| `stream stop` | Stop streaming and disarm the pulser |
| `start acq` | Legacy capture without a binary response frame |
| `read` | Print the latest 4,096 raw samples in hexadecimal |

Normal command responses begin with `OK` or `ERR <code>`. DAC, pulse, and
A-law scale changes are not allowed while an acquisition or stream is active.

## Binary frame format

Each binary result has a fixed 64-byte little-endian header followed by its
payload:

| Offset | Type | Value |
| ---: | --- | --- |
| 0 | 4 bytes | Magic `U4RK` |
| 4 | u8 | Protocol version (`1`) |
| 5 | u8 | Payload: `1` raw, `2` envelope, `3` A-law |
| 6 | u16 | Flags |
| 8 | u32 | Sequence number |
| 12 | u32 | Sample count (`4096`) |
| 16 | u32 | Sample rate (`60000000`) |
| 20 | u32 | Payload size in bytes |
| 24 | u64 | Capture timestamp in microseconds since boot |
| 32 | f32 | ADC frame mean |
| 36 | f32 | Envelope peak |
| 40 | f32 | A-law reference |
| 44 | u32 | Negative pulse duration in ns |
| 48 | u32 | Damping duration in ns |
| 52 | u32 | Positive pulse duration in ns |
| 56 | u32 | Cumulative dropped-frame count |
| 60 | u32 | IEEE CRC32 of the payload |

Flags:

| Bit | Meaning |
| ---: | --- |
| 0 | A-law saturation |
| 1 | Processing drop has occurred |
| 2 | USB drop has occurred |
| 3 | Deterministic self-test frame |
| 4 | Pulser was armed for the capture |
| 8–15 | Self-test case number |

Payload formats:

- Raw: 4,096 little-endian `uint16` ADC codes, 8,192 bytes
- Envelope: 4,096 little-endian IEEE-754 `float32` values, 16,384 bytes
- A-law: 4,096 `uint8` values, 4,096 bytes

## Final acceptance checklist

- Automated DSP self-test passes all seven signals
- Boot and disarmed pulser state is `00`
- ADC clock is 60 MHz and every capture contains 4,096 samples
- Negative-first and positive-first pulser sequences match the truth table
- Pulse durations are within one 8 ns PIO tick
- DAC codes 0, 512, and 1023 are correct
- Known external ADC signal produces the expected raw and envelope data
- A-law 70 Hz stream runs for 60 seconds with zero drops, gaps, and CRC errors
- Measured `worst_us` is recorded; 4.5 ms/200 Hz remains an unmet optimization target
