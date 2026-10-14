#!/usr/bin/env bash
# Build the Ultr4rick firmware and flash it to an RP2350 over USB without sudo.
# Usage: flash.sh [--no-build] [uf2]
# Works if the board is already in BOOTSEL, or is running stock SDK stdio_usb
# firmware (1200-baud touch -> BOOTSEL). Ultr4rick 1.5 needs the BOOTSEL button (mass-storage drive labelled RP2350).
set -euo pipefail

FW="$(cd "$(dirname "$0")/../../../firmware" && pwd)"
BUILD=1
if [[ "${1:-}" == "--no-build" ]]; then BUILD=0; shift; fi
UF2="${1:-$FW/build/adc-pulse.uf2}"

if (( BUILD )); then
    export PICO_SDK_PATH="${PICO_SDK_PATH:-$HOME/.pico-sdk/sdk/2.3.0}"
    [[ -f "$FW/build/build.ninja" ]] || \
        cmake -S "$FW" -B "$FW/build" -G Ninja -DPICO_TOOLCHAIN_PATH=/usr
    ninja -C "$FW/build"
fi

# If not in BOOTSEL, ask the running firmware to reboot via 1200-baud touch.
if ! lsusb | grep -q '2e8a:000f'; then
    for tty in /dev/ttyACM*; do
        [[ -e "$tty" ]] || continue
        python3 -c "import serial,time; s=serial.Serial('$tty',1200); time.sleep(0.2); s.close()" || true
    done
    for _ in $(seq 1 30); do lsusb | grep -q '2e8a:000f' && break; sleep 0.5; done
fi
lsusb | grep -q '2e8a:000f' || {
    echo "Board not in BOOTSEL (Ultr4rick 1.5 ignores the 1200-baud touch). Hold BOOTSEL and replug, then rerun with --no-build." >&2
    exit 1
}

DEV=""
for _ in $(seq 1 20); do
    DEV="$(lsblk -rno NAME,LABEL | awk '$2=="RP2350"{print "/dev/"$1}' | head -1)"
    [[ -n "$DEV" ]] && break; sleep 0.5
done
[[ -n "$DEV" ]] || { echo "RP2350 drive did not appear" >&2; exit 1; }

MNT="$(lsblk -no MOUNTPOINT "$DEV")"
if [[ -z "$MNT" ]]; then
    udisksctl mount -b "$DEV" --no-user-interaction >/dev/null
    MNT="$(lsblk -no MOUNTPOINT "$DEV")"
fi
cp "$UF2" "$MNT/" && sync
echo "Flashed $UF2 via $MNT"

for _ in $(seq 1 20); do lsusb | grep -q 'cafe:4011' && break; sleep 0.5; done
lsusb | grep 'cafe:4011' || echo "warning: Ultr4rick CDC device (cafe:4011) not seen yet" >&2
