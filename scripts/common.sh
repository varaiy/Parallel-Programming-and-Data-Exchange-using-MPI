#!/usr/bin/env bash
# Shared configuration for the tested two-VM topology.
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
NODE1_IP="${NODE1_IP:-192.168.56.201}"
NODE2_IP="${NODE2_IP:-192.168.56.202}"
MPI_SUBNET="${MPI_SUBNET:-192.168.56.0/24}"
MPI_USER="${MPI_USER:-vboxuser}"
REMOTE_TARGET="${MPI_USER}@${NODE2_IP}"

if [[ ! -d "${ROOT}/src" ]]; then
    echo "Cannot locate project source files." >&2
    exit 1
fi
