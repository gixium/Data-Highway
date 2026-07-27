#!/usr/bin/env bash
# test/run_tests.sh — automated regression runner for Data Highway
#
# Usage:
#   chmod +x run_tests.sh
#   ./run_tests.sh            # run all 111 test cases
#   ./run_tests.sh 1 5 10     # run only tests 1, 5, and 10
#
# Exit code: 0 if all tests pass, 1 if any fail.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$SCRIPT_DIR/../src"
INPUT_DIR="$SCRIPT_DIR/input"
OUTPUT_DIR="$SCRIPT_DIR/output"
BINARY="$SRC_DIR/highway"

# ── Build ─────────────────────────────────────────────────────────────────
echo "Building..."
make -C "$SRC_DIR" --silent
echo "Build OK."
echo ""

# ── Determine which tests to run ──────────────────────────────────────────
pass=0
fail=0
total=0
failed_cases=""

run_case() {
    local n="$1"
    local input="$INPUT_DIR/open_${n}.input.txt"
    local expected="$OUTPUT_DIR/open_${n}.output.txt"

    if [[ ! -f "$input" || ! -f "$expected" ]]; then
        printf "SKIP  test %-4s (files not found)\n" "$n"
        return
    fi

    total=$((total + 1))

    if diff -q <("$BINARY" < "$input") "$expected" > /dev/null 2>&1; then
        printf "PASS  test %s\n" "$n"
        pass=$((pass + 1))
    else
        printf "FAIL  test %s\n" "$n"
        fail=$((fail + 1))
        failed_cases="$failed_cases $n"
    fi
}

if [[ $# -gt 0 ]]; then
    for n in "$@"; do run_case "$n"; done
else
    for input_file in "$INPUT_DIR"/open_*.input.txt; do
        n="${input_file##*/open_}"
        n="${n%.input.txt}"
        run_case "$n"
    done
fi

# ── Summary ───────────────────────────────────────────────────────────────
echo ""
echo "Results: $pass/$total passed, $fail failed"
[[ -n "$failed_cases" ]] && echo "Failed: $failed_cases"
[[ $fail -gt 0 ]] && exit 1
exit 0

# ── Debugging a failing test with GDB ─────────────────────────────────────
#
# 1. Build with debug symbols and sanitisers:
#      make -C ../src debug
#
# 2. Start GDB:
#      gdb ../src/highway
#
# 3. Inside GDB:
#      (gdb) break find_path_backward     # set a breakpoint
#      (gdb) run < input/open_1.input.txt # run with redirected input
#      (gdb) print *finish                # inspect the current station
#      (gdb) backtrace                    # show the call stack
#      (gdb) continue                     # resume execution
#      (gdb) quit
#
# 4. For memory errors, use Valgrind instead:
#      valgrind --leak-check=full ../src/highway < input/open_1.input.txt
#
# 5. For cache-miss profiling:
#      valgrind --tool=cachegrind ../src/highway < input/open_1.input.txt
#      cg_annotate cachegrind.out.<pid>
