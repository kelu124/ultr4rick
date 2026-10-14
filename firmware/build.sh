#!/bin/bash
# Build both Ultr4rick (RP2350B) firmware variants into dist/:
#   ultr4rick      stdio REPL (main.c)                 -> build/
#   ultr4rick-dsp  raw-TinyUSB DSP build (main_dsp.c)  -> build-dsp/ (needs CMSIS-DSP)
# Extra arguments are passed to CMake, e.g. ./build.sh -DPICO_TOOLCHAIN_PATH=/usr
set -e
cd "$(dirname "$0")"
CMAKE_ARGS=("$@")

VERSION=$(grep -E '^version:' version.yaml | sed -E 's/version:[[:space:]]*"?([0-9.]+)"?.*/\1/')
echo "Building ultr4rick firmware v${VERSION}"
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
        -DFETCHCONTENT_BASE_DIR="${DEPS_DIR}" "${CMAKE_ARGS[@]}" >/dev/null
    cmake --build "$dir" -j"$(nproc)"
    cp "$dir/adc-pulse.uf2" "dist/ultr4rick-v${VERSION}-${label}.uf2"
}

build_one build     OFF ultr4rick
build_one build-dsp ON  ultr4rick-dsp

echo "Done. Artifacts in dist/:"
ls -1 dist/
