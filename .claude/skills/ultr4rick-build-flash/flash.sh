#!/usr/bin/env bash
# Build the Ultr4rick firmware and flash it to the RP2350B over USB without sudo.
# Usage: flash.sh [--stdio] [--no-build] [uf2]
#   default: build both variants, flash the DSP variant (dist/ultr4rick-v<ver>-ultr4rick-dsp.uf2)
# The running firmware is put in BOOTSEL with the `reboot-dfu` command (both variants),
# falling back to a 1200-baud touch (stdio variant). Otherwise hold BOOTSEL and replug.
set -euo pipefail

FW="$(cd "$(dirname "$0")/../../../firmware" && pwd)"
BUILD=1
LABEL=ultr4rick-dsp
while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-build) BUILD=0; shift ;;
        --stdio) LABEL=ultr4rick; shift ;;
        *) break ;;
    esac
done

if (( BUILD )); then
    export PICO_SDK_PATH="${PICO_SDK_PATH:-$HOME/.pico-sdk/sdk/2.3.0}"
    "$FW/build.sh" -DPICO_TOOLCHAIN_PATH=/usr
fi
VERSION=$(grep -E '^version:' "$FW/version.yaml" | sed -E 's/version:[[:space:]]*"?([0-9.]+)"?.*/\1/')
UF2="${1:-$FW/dist/ultr4rick-v${VERSION}-${LABEL}.uf2}"
[[ -f "$UF2" ]] || { echo "missing $UF2" >&2; exit 1; }

if ! lsusb | grep -q '2e8a:000f'; then
    for tty in /dev/ttyACM*; do
        [[ -e "$tty" ]] || continue
        python3 - "$tty" <<'PY' || true
import serial, sys, time
port = sys.argv[1]
try:
    s = serial.Serial(port, 115200, timeout=0.5); s.write(b"reboot-dfu\n"); time.sleep(0.3); s.close()
except Exception:
    pass
time.sleep(1.0)
try:
    s = serial.Serial(port, 1200); time.sleep(0.2); s.close()
except Exception:
    pass
PY
    done
    for _ in $(seq 1 30); do lsusb | grep -q '2e8a:000f' && break; sleep 0.5; done
fi
lsusb | grep -q '2e8a:000f' || {
    echo "Board not in BOOTSEL. Check fuser -v /dev/ttyACM*, or hold BOOTSEL and replug, then rerun with --no-build." >&2
    exit 1
}

DEV=""
for _ in $(seq 1 20); do
    DEV="$(lsblk -rno NAME,LABEL | awk '$2=="RP2350"{print "/dev/"$1}' | head -1)"
    [[ -n "$DEV" ]] && break; sleep 0.5
done
[[ -n "$DEV" ]] || { echo "RP2350 drive did not appear" >&2; exit 1; }

# Give the desktop automounter a moment; mount ourselves only if it doesn't.
MNT=""
for _ in $(seq 1 6); do
    MNT="$(lsblk -no MOUNTPOINT "$DEV")"
    [[ -n "$MNT" ]] && break; sleep 0.5
done
if [[ -z "$MNT" ]]; then
    udisksctl mount -b "$DEV" --no-user-interaction >/dev/null 2>&1 || true   # may race the automounter
    MNT="$(lsblk -no MOUNTPOINT "$DEV")"
fi
[[ -n "$MNT" ]] || { echo "could not mount $DEV" >&2; exit 1; }
cp "$UF2" "$MNT/" && sync
echo "Flashed $UF2 via $MNT"

for _ in $(seq 1 20); do lsusb | grep -qE 'cafe:4011|2e8a:0009' && break; sleep 0.5; done
lsusb | grep -E 'cafe:4011|2e8a:0009' || echo "warning: Ultr4rick CDC device not seen yet" >&2
