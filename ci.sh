#!/usr/bin/env bash
# One-command CI: format check → build firmware → host unit tests
# Exits non-zero on any failure; safe to run as a pre-commit / pre-push gate.
set -euo pipefail

IDF_PATH="${IDF_PATH:-$HOME/.local/opt/esp-idf-v5.5.4}"
if ! command -v idf.py &>/dev/null; then
    # shellcheck source=/dev/null
    source "$IDF_PATH/export.sh"
fi

echo "==> [1/4] clang-format check"
CHANGED=$(find main test -name '*.c' -o -name '*.h' | \
          xargs clang-format --dry-run --Werror 2>&1 || true)
if [[ -n "$CHANGED" ]]; then
    echo "$CHANGED"
    echo "ERROR: Code is not clang-formatted. Run: find main test -name '*.c' -o -name '*.h' | xargs clang-format -i"
    exit 1
fi
echo "    OK"
echo ""

echo "==> [2/4] Build firmware"
bash build_flash.sh build
echo ""

echo "==> [3/4] Host unit tests"
IDF_PATH="$IDF_PATH" make -C test run
echo ""

echo "==> [4/4] All checks passed"
echo "    To flash: bash build_flash.sh deploy"
