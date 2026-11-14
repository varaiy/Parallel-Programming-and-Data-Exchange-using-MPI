#!/usr/bin/env bash
# Run on MPI-Node1 after building. Both machines must use the same absolute path.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"

for name in data_exchange parallel_sum collective_comm mpi_performance; do
    if [[ ! -x "${ROOT}/bin/${name}" ]]; then
        echo "Missing executable: bin/${name}. Run 'make' first." >&2
        exit 1
    fi
done

printf 'Copying executables to %s:%s/bin/\n' "$REMOTE_TARGET" "$ROOT"
ssh -o BatchMode=yes -o ConnectTimeout=8 "$REMOTE_TARGET" "mkdir -p '$ROOT/bin'"
scp -q "${ROOT}/bin/"{data_exchange,parallel_sum,collective_comm,mpi_performance} "${REMOTE_TARGET}:${ROOT}/bin/"
ssh -o BatchMode=yes "$REMOTE_TARGET" "ls -l '$ROOT/bin/'"
echo 'Deploy successful. Source files remain in the GitHub repository; the remote node needs executables at run time.'
