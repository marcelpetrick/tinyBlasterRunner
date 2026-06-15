#!/usr/bin/env bash
# One-command CI: build firmware → host unit tests → optional flash
# Exits non-zero on any failure; safe to run as a pre-push gate.
set -euo pipefail

IDF_PATH="${IDF_PATH:-$HOME/.local/opt/esp-idf-v5.5.4}"
if ! command -v idf.py &>/dev/null; then
    # shellcheck source=/dev/null
    source "$IDF_PATH/export.sh"
fi

echo "==> [1/3] Build firmware"
bash build_flash.sh build
echo ""

echo "==> [2/3] Host unit tests"
make -C test run
echo ""

echo "==> [3/3] All checks passed"
echo "    To flash: bash build_flash.sh deploy"
