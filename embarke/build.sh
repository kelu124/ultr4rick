#!/bin/bash
# Build both embarke (RP2350B) firmware variants into dist/:
#   embarke      stdio REPL (main.c)
#   embarke-dsp  raw-TinyUSB DSP build (main_dsp.c, needs CMSIS-DSP)
set -e
cd "$(dirname "$0")"

VERSION=$(grep -E '^version:' version.yaml | sed -E 's/version:[[:space:]]*"?([0-9.]+)"?.*/\1/')
echo "Building ultr4rick (embarke) firmware v${VERSION}"
mkdir -p dist

# Shared FetchContent cache so CMSIS-DSP is cloned only once.
DEPS_DIR="${PWD}/.deps"
GEN=()
command -v ninja >/dev/null && GEN=(-G Ninja)

# args: build-dir  DSP(ON|OFF)  variant-label
build_one() {
    local dir=$1 dsp=$2 label=$3
    echo "==> ${label} (DSP=${dsp})"
    cmake -S . -B "$dir" "${GEN[@]}" -DDSP="$dsp" \
        -DFETCHCONTENT_BASE_DIR="${DEPS_DIR}" "$@" >/dev/null
    cmake --build "$dir" -j"$(nproc)"
    cp "$dir/adc-pulse.uf2" "dist/embarke-v${VERSION}-${label}.uf2"
}

build_one build     OFF embarke
build_one build-dsp ON  embarke-dsp

echo "Done. Artifacts in dist/:"
ls -1 dist/
