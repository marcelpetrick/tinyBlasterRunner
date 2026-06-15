#!/usr/bin/env bash
# Build, flash, and monitor helper for the ESP32 project.
# Usage: ./build_flash.sh [build|flash|monitor|deploy|all]
#   build   – compile only (default)
#   flash   – flash the last build to the device
#   monitor – open serial monitor
#   deploy  – build then flash
#   all     – build, flash, then monitor (Ctrl-] to exit monitor)
set -euo pipefail

IDF_PATH="${IDF_PATH:-$HOME/.local/opt/esp-idf-v5.5.4}"
PORT="${PORT:-/dev/ttyACM0}"
FLASH_BAUD="${FLASH_BAUD:-460800}"
MONITOR_BAUD="${MONITOR_BAUD:-115200}"
PART_SIZE_KB=2048  # factory app partition (2 MB)

check_device() {
    if [[ ! -c "$PORT" ]]; then
        echo "ERROR: Serial device '$PORT' not found." >&2
        echo "       Connect the ESP32 or set PORT=/dev/ttyXXX" >&2
        exit 1
    fi
    if [[ ! -r "$PORT" || ! -w "$PORT" ]]; then
        echo "ERROR: No read/write access to '$PORT'." >&2
        echo "       Run: sudo setfacl -m u:$USER:rw $PORT" >&2
        exit 1
    fi
}

if ! command -v idf.py &>/dev/null; then
    # shellcheck source=/dev/null
    source "$IDF_PATH/export.sh"
fi

print_build_summary() {
    local elapsed=$1
    local bin="build/tdisplay_games.bin"
    if [[ -f "$bin" ]]; then
        local bytes kb free_kb free_pct
        bytes=$(stat -c%s "$bin")
        kb=$(( bytes / 1024 ))
        free_kb=$(( PART_SIZE_KB - kb ))
        free_pct=$(( (free_kb * 100) / PART_SIZE_KB ))
        echo "✓ build complete — ${kb} KB / ${PART_SIZE_KB} KB (${free_pct}% free) in ${elapsed}s"
    fi
}

CMD="${1:-build}"

case "$CMD" in
    build)
        t0=$SECONDS
        echo "==> Building..."
        idf.py build
        print_build_summary $(( SECONDS - t0 ))
        ;;
    flash)
        check_device
        echo "==> Flashing to $PORT at ${FLASH_BAUD} baud..."
        idf.py -p "$PORT" -b "$FLASH_BAUD" flash
        ;;
    monitor)
        check_device
        echo "==> Monitoring $PORT at ${MONITOR_BAUD} baud (Ctrl-] to exit)..."
        idf.py -p "$PORT" -b "$MONITOR_BAUD" monitor
        ;;
    deploy)
        t0=$SECONDS
        echo "==> Building..."
        idf.py build
        print_build_summary $(( SECONDS - t0 ))
        check_device
        echo "==> Flashing to $PORT at ${FLASH_BAUD} baud..."
        idf.py -p "$PORT" -b "$FLASH_BAUD" flash
        echo "==> Done. Run './build_flash.sh monitor' to watch output."
        ;;
    all)
        t0=$SECONDS
        echo "==> Building..."
        idf.py build
        print_build_summary $(( SECONDS - t0 ))
        check_device
        echo "==> Flashing to $PORT at ${FLASH_BAUD} baud..."
        idf.py -p "$PORT" -b "$FLASH_BAUD" flash
        echo "==> Monitoring $PORT at ${MONITOR_BAUD} baud (Ctrl-] to exit)..."
        idf.py -p "$PORT" -b "$MONITOR_BAUD" monitor
        ;;
    *)
        echo "Usage: $0 [build|flash|monitor|deploy|all]"
        exit 1
        ;;
esac
