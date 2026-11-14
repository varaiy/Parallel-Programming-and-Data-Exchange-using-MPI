#!/usr/bin/env bash
# Checks output content rather than output line order (MPI stdout may interleave).
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "${ROOT}/results/local"

check_contains() {
    local file="$1" text="$2"
    if ! grep -Fq -- "$text" "$file"; then
        echo "FAIL: expected '$text' in ${file}" >&2
        cat "$file" >&2
        exit 1
    fi
}

run_check() {
    local program="$1"; shift
    local log="${ROOT}/results/local/${program}_latest.txt"
    echo "Testing ${program} ..."
    bash "${ROOT}/scripts/run_mpi.sh" "$program" > "$log" 2>&1
    for expected in "$@"; do check_contains "$log" "$expected"; done
    echo "PASS: ${program}"
}

run_check data_exchange \
    'System 1: Sending 42 to System 2' \
    'System 2: Received 42 from System 1' \
    'System 1: Received 43 from System 2'
run_check parallel_sum \
    'Rank 0 on' 'Local sum = 10' 'Rank 1 on' 'Local sum = 26' \
    'Final parallel sum = 36'
run_check collective_comm \
    'Broadcasting multiplier = 5' '10 x 5 = 50' \
    '20 x 5 = 100' 'Final gathered results: [50, 100]'
run_check mpi_performance \
    'MPI PERFORMANCE ANALYSIS' '1048576'

# Single-process execution is intentionally rejected by the source program.
negative_log="${ROOT}/results/local/negative_test_latest.txt"
if mpirun -np 1 "${ROOT}/bin/data_exchange" > "$negative_log" 2>&1; then
    echo 'FAIL: invalid one-process run unexpectedly succeeded' >&2
    exit 1
fi
check_contains "$negative_log" 'This program requires exactly 2 MPI processes.'
echo 'PASS: invalid process-count handling'
echo 'ALL CHECKS PASSED (distributed runs require both VMs)'
