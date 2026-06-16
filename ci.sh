#!/usr/bin/env bash
# ci.sh — local CI pipeline for Tiny Blaster Runner
# Stages: format check → firmware build → host unit tests → coverage → Doxygen docs
# All stages run regardless of individual failures. Exits non-zero if any mandatory
# stage failed. Prints a stage-by-stage summary table at the end.
set -uo pipefail

IDF_PATH="${IDF_PATH:-$HOME/.local/opt/esp-idf-v5.5.4}"
PIPELINE_LOG_DIR="${TMPDIR:-/tmp}/tbr-ci-$$"
trap 'rm -rf "${PIPELINE_LOG_DIR}"' EXIT

declare -a SUMMARY_LINES=()
FORMAT_OK=0; BUILD_OK=0; TEST_OK=0; COV_OK=0; DOCS_OK=0
FORMAT_DETAILS=""; BUILD_DETAILS=""; TEST_DETAILS=""; COV_DETAILS=""; DOCS_DETAILS=""

# ---------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------

mark_result() {
    local label="$1" status="$2" details="$3"
    SUMMARY_LINES+=("$(printf '%-14s : %-4s  %s' "$label" "$status" "$details")")
}

print_summary() {
    printf '\n========== CI Pipeline Summary ==========\n'
    local line
    for line in "${SUMMARY_LINES[@]}"; do
        printf '%s\n' "$line"
    done
    printf '==========================================\n'
}

run_with_log() {
    local log_path="$1"; shift
    mkdir -p "${PIPELINE_LOG_DIR}"
    "$@" 2>&1 | tee "${log_path}"
    return "${PIPESTATUS[0]}"
}

# ---------------------------------------------------------------
# IDF environment (PATH must be set before running build or tests)
# ---------------------------------------------------------------
if ! command -v idf.py &>/dev/null; then
    # shellcheck source=/dev/null
    source "$IDF_PATH/export.sh"
fi

# ---------------------------------------------------------------
# Stage 1 — clang-format
# ---------------------------------------------------------------
printf '\n==> [1/5] clang-format check\n'
FORMAT_LOG="${PIPELINE_LOG_DIR}/format.log"
mkdir -p "${PIPELINE_LOG_DIR}"
VIOLATIONS=$(find main test -name '*.c' -o -name '*.h' | \
             xargs clang-format --dry-run --Werror 2>&1 || true)
if [[ -z "$VIOLATIONS" ]]; then
    FORMAT_OK=1
    FORMAT_DETAILS="0 violations"
    printf '    OK\n'
    mark_result "Format" "PASS" "$FORMAT_DETAILS"
else
    printf '%s\n' "$VIOLATIONS"
    V_COUNT=$(printf '%s\n' "$VIOLATIONS" | grep -c 'error:' || true)
    F_COUNT=$(printf '%s\n' "$VIOLATIONS" | grep 'error:' | awk -F: '{print $1}' | sort -u | wc -l || true)
    FORMAT_DETAILS="${V_COUNT} violation(s) across ${F_COUNT} file(s)"
    mark_result "Format" "FAIL" "$FORMAT_DETAILS"
fi

# ---------------------------------------------------------------
# Stage 2 — firmware build
# ---------------------------------------------------------------
printf '\n==> [2/5] Build firmware\n'
BUILD_LOG="${PIPELINE_LOG_DIR}/build.log"
if run_with_log "$BUILD_LOG" bash build_flash.sh build; then
    BUILD_OK=1
    # Prefer the build_flash.sh summary line; fall back to idf.py size line
    SIZE_LINE=$(grep -oE '[0-9]+ KB / [0-9]+ KB \([0-9]+% free\) in [0-9]+s' "$BUILD_LOG" | tail -1 || true)
    if [[ -z "$SIZE_LINE" ]]; then
        SIZE_LINE=$(grep -oE '[0-9]+\.[0-9]+% free' "$BUILD_LOG" | tail -1 || true)
    fi
    BUILD_DETAILS="${SIZE_LINE:-build succeeded}"
    mark_result "Build" "PASS" "$BUILD_DETAILS"
else
    BUILD_DETAILS="see ${BUILD_LOG}"
    mark_result "Build" "FAIL" "$BUILD_DETAILS"
fi

# ---------------------------------------------------------------
# Stage 3 — host unit tests
# ---------------------------------------------------------------
printf '\n==> [3/5] Host unit tests\n'
TEST_LOG="${PIPELINE_LOG_DIR}/tests.log"
if run_with_log "$TEST_LOG" env IDF_PATH="$IDF_PATH" make -C test run; then
    TEST_OK=1
else
    : # handled below — parse results regardless of exit code
fi
RESULT_LINE=$(grep -E '^[0-9]+ Tests [0-9]+ Failures' "$TEST_LOG" | tail -1 || true)
if [[ -n "$RESULT_LINE" ]]; then
    TOTAL=$(echo "$RESULT_LINE" | awk '{print $1}')
    FAIL=$(echo  "$RESULT_LINE" | awk '{print $3}')
    IGN=$(echo   "$RESULT_LINE" | awk '{print $5}')
    PASS=$(( TOTAL - FAIL ))
    TEST_DETAILS="${PASS}/${TOTAL} pass, ${FAIL} failure(s), ${IGN} ignored"
else
    TEST_DETAILS="see ${TEST_LOG}"
fi
if [[ "$TEST_OK" -eq 1 ]]; then
    mark_result "Tests" "PASS" "$TEST_DETAILS"
else
    mark_result "Tests" "FAIL" "$TEST_DETAILS"
fi

# ---------------------------------------------------------------
# Stage 4 — coverage report
# ---------------------------------------------------------------
printf '\n==> [4/5] Coverage report\n'
if [[ "$TEST_OK" -eq 1 ]]; then
    COV_LOG="${PIPELINE_LOG_DIR}/coverage.log"
    if run_with_log "$COV_LOG" env IDF_PATH="$IDF_PATH" make -C test coverage; then
        COV_OK=1
        # "Coverage: 100.0% lines → ..." is the summary line emitted by the Makefile
        COV_LINE=$(grep -E '^Coverage:' "$COV_LOG" | tail -1 || true)
        COV_DETAILS="${COV_LINE#Coverage: }"
        # Also surface the lcov function-coverage line for completeness
        FUNC_LINE=$(grep -E 'functions\.\.' "$COV_LOG" | tail -1 | grep -oE '[0-9.]+% \([0-9]+ of [0-9]+' || true)
        [[ -n "$FUNC_LINE" ]] && COV_DETAILS="${COV_DETAILS}; functions: ${FUNC_LINE})"
        [[ -z "$COV_DETAILS" ]] && COV_DETAILS="generated test/coverage_html/index.html"
        mark_result "Coverage" "PASS" "$COV_DETAILS"
    else
        COV_DETAILS="generation failed"
        mark_result "Coverage" "FAIL" "$COV_DETAILS"
    fi
else
    mark_result "Coverage" "SKIP" "skipped — tests did not pass"
fi

# ---------------------------------------------------------------
# Stage 5 — Doxygen docs
# ---------------------------------------------------------------
printf '\n==> [5/5] Doxygen docs\n'
DOCS_LOG="${PIPELINE_LOG_DIR}/docs.log"
# Run with QUIET=NO so we can count processed files; Doxyfile keeps QUIET=YES for
# the build_flash.sh docs target (which doesn't need this detail).
DOXY_TMP=$(mktemp /tmp/tbr-doxy.XXXXX)
{ cat Doxyfile; printf '\nQUIET = NO\n'; } > "$DOXY_TMP"
if run_with_log "$DOCS_LOG" doxygen "$DOXY_TMP"; then
    DOCS_OK=1
    SRC_COUNT=$(grep -c 'Generating docs for file' "$DOCS_LOG" || true)
    HTML_COUNT=$(find docs/html -name '*.html' 2>/dev/null | wc -l || true)
    WARN_COUNT=$(grep -c ': warning:' "$DOCS_LOG" || true)
    DOCS_DETAILS="${SRC_COUNT} source file(s) → ${HTML_COUNT} HTML page(s), ${WARN_COUNT} warning(s)"
    mark_result "Docs" "PASS" "$DOCS_DETAILS"
else
    DOCS_DETAILS="doxygen exited non-zero"
    mark_result "Docs" "FAIL" "$DOCS_DETAILS"
fi
rm -f "$DOXY_TMP"

# ---------------------------------------------------------------
# Result
# ---------------------------------------------------------------
print_summary

if [[ "$FORMAT_OK" -eq 1 && "$BUILD_OK" -eq 1 && "$TEST_OK" -eq 1 && \
      "$COV_OK"    -eq 1 && "$DOCS_OK"  -eq 1 ]]; then
    printf '\nAll checks passed. To flash: bash build_flash.sh deploy\n'
    exit 0
else
    printf '\nOne or more stages failed.\n'
    exit 1
fi
