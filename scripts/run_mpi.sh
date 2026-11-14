#!/usr/bin/env bash
# Examples: bash scripts/run_mpi.sh data_exchange
#           bash scripts/run_mpi.sh parallel_sum --display-map
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"

if [[ $# -lt 1 ]]; then
    echo "Usage: bash scripts/run_mpi.sh {data_exchange|parallel_sum|collective_comm|mpi_performance} [--display-map]" >&2
    exit 2
fi
program="$1"; shift
case "$program" in
    data_exchange|parallel_sum|collective_comm|mpi_performance) ;;
    *) echo "Unknown MPI program: $program" >&2; exit 2 ;;
esac
if [[ ! -x "${ROOT}/bin/${program}" ]]; then
    echo "Missing ${ROOT}/bin/${program}; run make first." >&2
    exit 1
fi
for flag in "$@"; do
    if [[ "$flag" != "--display-map" ]]; then
        echo "Unsupported option: $flag" >&2
        exit 2
    fi
done

# Open MPI 4.1.x uses SSH to launch remote processes. Executables are present
# on BOTH nodes in the same absolute directory (see deploy.sh).
cd "$ROOT"
mpirun -np 2 \
    --host "${NODE1_IP}:1,${NODE2_IP}:1" \
    --mca btl_tcp_if_include "$MPI_SUBNET" \
    --tag-output "$@" \
    "${ROOT}/bin/${program}"
