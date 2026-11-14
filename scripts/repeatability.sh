#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
mkdir -p "${ROOT}/results/local"

# tee writes each run's full output. pipefail preserves MPI failures.
for i in 1 2 3; do
    echo "========== PERFORMANCE RUN $i =========="
    bash "${ROOT}/scripts/run_mpi.sh" mpi_performance
    echo
done | tee "${ROOT}/results/local/performance_repeatability_latest.txt"

echo "Saved results/local/performance_repeatability_latest.txt"
