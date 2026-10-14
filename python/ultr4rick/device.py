# Main libs
import os
import re
import sys
import glob
import time
import struct
# Third party
import serial
import numpy as np

# USB IDs, preferred in this order: DSP build (raw TinyUSB, Ultr4rick strings),
# then the stdio REPL build (SDK stdio_usb default IDs).
USB_IDS = ((0xCAFE, 0x4011), (0x2E8A, 0x0009))


def _find_port():
    try:
        from serial.tools import list_ports
        ports = list(list_ports.comports())
        for vid, pid in USB_IDS:
            for p in ports:
                if p.vid == vid and p.pid == pid:
                    return p.device
    except ImportError:
        pass
    if sys.platform.startswith("win"):
        ports = glob.glob("COM[0-9]*")
    elif sys.platform.startswith("linux"):
        ports = glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*")
    elif sys.platform.startswith("darwin"):  # macOS
        ports = glob.glob("/dev/tty.usbmodem*") + glob.glob("/dev/tty.usbserial*")
    else:
        raise OSError(f"Unsupported platform: {sys.platform}")

    if not ports:
        raise OSError("No serial device found")
    return ports[0]


def pprint(ans):
    return "".join(b.decode("utf-8") for b in ans)


class Ultr4rick:
    """Serial driver for the Ultr4rick RP2350B firmware (``firmware/``).

    Speaks both firmware builds:
      * stdio REPL build (``main.c``): ``dac``/``pulse_adc_trigger``/``read``;
      * DSP build (``-DDSP``, ``main_dsp.c``): ``status``/``capture``/
        ``read_raw``/``read_fft``/streaming/self-test (binary ``U4RK`` frames).
    Mux, ``version`` and ``reboot_dfu`` work on both. The MAX14866 mux is
    on the board, so mux commands are always available.
    """

    def sread(self):
        done = False
        ans = []
        while not done:
            res = self.ser.readline()
            ans.append(res)
            if res == b"":
                done = True
        if self.verbose:
            pprint(ans)
        if self.log:
            with open(self.log_file, "a") as f:
                for line in ans:
                    if line and line != b"":
                        f.write(line.decode("utf-8", errors="replace"))
        return ans

    def __init__(self, port=None, verbose=True, logging=False, log_file=".log"):
        self.verbose = verbose
        self.log = logging
        self.log_file = log_file

        # Detect the port
        port_device = port if port is not None else _find_port()
        print("Device on", port_device)

        self.ser = serial.Serial(port_device, 115200, timeout=0.2)
        self.ser.baudrate = 115200
        time.sleep(1)  # wait for the serial connection to initialize
        self.sread()

        self.Fech = 60e6  # ADC sampling frequency (Hz)


    def dac(self, N):
        """Write a value to the 10-bit DAC MCP4812 (write dac).

        Args:
            N: DAC value (10-bit).
        """
        self.ser.write(bytearray("write dac " + str(N) + "\n", "ascii"))
        ans = self.sread()
        return ans
    
    def read(self):
        self.ser.write(bytearray("read\n", "ascii"))
        ans = self.sread()
        return ans

    def reboot_dfu(self):
        """Reboot the RP2350B into the USB bootloader (``reboot-dfu``).

        The board re-enumerates as the ``RP2350`` mass-storage drive, ready
        for a new UF2; the serial port is closed here because it disappears.
        Works on both firmware builds.
        """
        self.ser.write(b"reboot-dfu\n")
        self.ser.flush()
        time.sleep(0.3)
        try:
            ans = self.ser.read(self.ser.in_waiting or 1)
        except (serial.SerialException, OSError):
            ans = b""
        self.close()
        return ans.decode("utf-8", "replace")

    def close(self):
        """Close the serial port."""
        try:
            self.ser.close()
        except (serial.SerialException, OSError):
            pass

    def version(self):
        """Query the firmware `version` command and parse its report.

        Returns a dict with whatever the firmware reported. Keys (all optional,
        present only if the firmware emits the corresponding line):
            version : firmware version string, e.g. "0.1.1"
            changes : one-line change summary
            board   : PICO_BOARD, e.g. "ultr4rick"
            chip    : e.g. "RP2350B"
            mux     : raw mux line, e.g. "enabled (MAX14866)"
            mux_enabled : bool derived from `mux`
            build   : git build hash, e.g. "b0d9435" or "b0d9435-dirty"
            release : GitHub release URL for this version
            raw     : the full decoded response text
        """
        self.ser.write(bytearray("version\n", "ascii"))
        text = pprint(self.sread())
        info = {"raw": text}

        m = re.search(r"firmware\s+v(\d+\.\d+\.\d+)", text)
        if m:
            info["version"] = m.group(1)

        for line in text.splitlines():
            line = line.strip()
            if line.startswith("changes:"):
                info["changes"] = line.split(":", 1)[1].strip()
            elif line.startswith("board:"):
                rest = line.split(":", 1)[1].strip()
                bm = re.match(r"(\S+)(?:\s*\(([^)]*)\))?", rest)
                if bm:
                    info["board"] = bm.group(1)
                    if bm.group(2):
                        info["chip"] = bm.group(2)
            elif line.startswith("mux:"):
                mux = line.split(":", 1)[1].strip()
                info["mux"] = mux
                info["mux_enabled"] = mux.lower().startswith("enabled")
            elif line.startswith("build:"):
                info["build"] = line.split(":", 1)[1].strip()
            elif line.startswith("release:"):
                info["release"] = line.split(":", 1)[1].strip()

        return info
    
    def pulse_adc_trigger(self, pon: int=200,poff:int=200,damp:int=2000):
        self.ser.write(bytearray("start acq "+str(pon)+" "+str(poff)+" "+str(damp)+"\n",'ascii'))
        ans = self.sread()
        return ans

    def write_mux(self, value):
        """Load the MAX14866 mux shift register (write mux).

        Firmware parses the argument as hexadecimal (strtol base 16), so an int
        is sent as an uppercase hex string. A string is passed through verbatim,
        allowing an explicit "0x" prefix if desired.

        Args:
            value: mux pattern as an int, or a hex string (e.g. 0xFFFF or "FFFF").
        """
        arg = format(value, "X") if isinstance(value, int) else str(value)
        self.ser.write(bytearray("write mux " + arg + "\n", "ascii"))
        ans = self.sread()
        return ans

    def set_mux(self):
        """Pulse the MAX14866 SET line, enabling all switches (set mux)."""
        self.ser.write(bytearray("set mux\n", "ascii"))
        ans = self.sread()
        return ans

    def clear_mux(self):
        """Pulse the MAX14866 CLR line, disabling all switches (clear mux)."""
        self.ser.write(bytearray("clear mux\n", "ascii"))
        ans = self.sread()
        return ans

    # ------------------------------------------------------------------
    # DSP-build (-DDSP) helpers. These speak the binary framed protocol;
    # use them only against a DSP firmware build (see status()).
    # ------------------------------------------------------------------

    def command(self, line, timeout=1.0):
        """Send one DSP-build text command; return its ``OK``/``ERR`` line.

        Raises RuntimeError on an ``ERR`` reply, TimeoutError on no reply.
        """
        self.ser.reset_input_buffer()
        self.ser.write((line.strip() + "\n").encode("ascii"))
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            reply = self.ser.readline().decode("utf-8", "replace").strip()
            if reply.startswith("OK"):
                return reply
            if reply.startswith("ERR"):
                raise RuntimeError(f"{line!r}: {reply}")
        raise TimeoutError(f"no reply to {line!r}")

    def help(self):
        """Return the DSP build's command list (``help``)."""
        return self.command("help")

    def status(self):
        """Query the DSP-build `status` line and return it parsed (dict).

        Raises ValueError against a non-DSP firmware (no status line).
        """
        from ultr4rick import dsp
        self.ser.reset_input_buffer()
        self.ser.write(b"status\n")
        line = self.ser.readline().decode("utf-8", "replace")
        info = dsp.parse_status(line)
        info["raw"] = line.strip()  # the Pico's exact status text
        return info

    def capture(self, payload="envelope"):
        """Trigger a one-shot DSP capture and return the parsed `dsp.Frame`.

        Args:
            payload: 'raw' (read_raw, 8000 samples), 'envelope' (read_fft, 4096)
                     or 'alaw' (4096). DSP firmware only.
        """
        from ultr4rick import dsp
        command = {"raw": "read_raw", "envelope": "read_fft",
                   "alaw": "acq alaw"}.get(payload)
        if command is None:
            raise ValueError("payload must be raw, envelope or alaw")
        self.ser.reset_input_buffer()
        self.ser.write((command + "\n").encode("ascii"))
        # The firmware sends an "OK capture started ..." text line before the
        # binary frame; capture it (self.last_reply) so callers can display it.
        self.last_reply = self.ser.readline().decode("utf-8", "replace").strip()
        return dsp.FrameReader(self.ser).read_frame()

    def read_fft(self):
        """One-shot 4096-sample Hilbert-envelope capture (DSP firmware)."""
        return self.capture("envelope")

    def read_raw(self):
        """One-shot 8000-sample raw ADC capture as a frame (DSP firmware)."""
        return self.capture("raw")

    def set_gain(self, value):
        """Set the TGC gain DAC, 0..1023 (DSP build: `dac write <n>`).

        (The stdio build uses `write dac`, i.e. `Ultr4rick.dac()`.)
        """
        self.ser.write(("dac write %d\n" % int(value)).encode("ascii"))
        return self.sread()

    def configure_pulse(self, negative_ns, damp_ns, positive_ns,
                        order="pos-first"):
        """Configure the pulser (DSP build: `pulse config ...`)."""
        self.ser.write(
            ("pulse config %d %d %d %s\n"
             % (int(negative_ns), int(damp_ns), int(positive_ns), order))
            .encode("ascii"))
        return self.sread()

    def arm_pulser(self):
        """Arm the pulser so the next capture transmits (DSP build)."""
        self.ser.write(b"pulser arm\n")
        return self.sread()

    def disarm_pulser(self):
        """Disarm the pulser (DSP build)."""
        self.ser.write(b"pulser disarm\n")
        return self.sread()

    def set_alaw_reference(self, reference):
        """Set the A-law full-scale reference in ADC counts (``dsp scale``)."""
        return self.command("dsp scale %g" % float(reference))

    def selftest(self):
        """Run the on-board DSP self-test (``dsp selftest``).

        Returns the list of `dsp.Frame` (raw, envelope and A-law for every
        synthetic case). Compare with `dsp.check_selftest()`.
        """
        from ultr4rick import dsp
        reply = self.command("dsp selftest")
        count = int(dsp.parse_fields(reply)["frames"])
        reader = dsp.FrameReader(self.ser)
        return [reader.read_frame() for _ in range(count)]

    def stream_start(self, payload="envelope", rate_hz=10):
        """Start periodic captures (``stream start``).

        Max rates: raw 100 Hz, envelope 50 Hz, alaw 70 Hz. Read frames with
        `stream_frames()` and always finish with `stream_stop()`.
        """
        if payload not in ("raw", "envelope", "alaw"):
            raise ValueError("payload must be raw, envelope or alaw")
        from ultr4rick import dsp
        reply = self.command("stream start %s %d" % (payload, int(rate_hz)))
        self._stream_reader = dsp.FrameReader(self.ser)
        return reply

    def stream_frames(self, count):
        """Yield `count` frames from a running stream."""
        for _ in range(int(count)):
            yield self._stream_reader.read_frame()

    def stream_stop(self, timeout=2.0):
        """Stop a running stream and return the ``OK stream stopped`` line.

        Frames already in flight are discarded.
        """
        self.ser.write(b"stream stop\n")
        buf = bytearray()
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            buf.extend(self.ser.read(self.ser.in_waiting or 1))
            i = buf.find(b"OK stream ")
            if i >= 0 and buf.find(b"\n", i) >= 0:
                self._stream_reader = None
                return buf[i:buf.find(b"\n", i)].decode("ascii", "replace").strip()
        raise TimeoutError("stream did not stop")


# Backwards-compatible alias for code written against the pic0rick library.
Pic0rick = Ultr4rick
