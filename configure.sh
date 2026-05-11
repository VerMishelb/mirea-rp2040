#! /usr/bin/env bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR"

cmake -S . -B build -D PICO_BOARD=waveshare_rp2040_zero -D CMAKE_BUILD_TYPE=Release
