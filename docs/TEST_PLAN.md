# Verification plan and archived test evidence

## Tested environment

Two Ubuntu Server 24.04.5 LTS VirtualBox VMs on one x86_64 iMac, with Open MPI 4.1.6 and GCC 13.3.0. Host-only IPv4 addresses: Node 1 `192.168.56.201`; Node 2 `192.168.56.202`. Rank mapping: rank 0 on MPI-Node1, rank 1 on mpi-node2. All distributed tests were launched from Node 1.

## Automated reproduction

On Node 1 after `make && make deploy`:

```bash
make test
make mapping
make performance
make repeatability
make summary
```

`make test` checks the critical text of each program and invalid process counts. It compares **content**, not interleaving order across processes. `make mapping` checks MPI placement via `--display-map`. `make repeatability` takes three fresh performance trials; original results are in `results/reference/`.

## Test cases and expected outcomes

| ID | Test | Command or method | Expected output / behavior | Evidence |
|---|---|---|---|---|
| T01 | Node 1 can reach Node 2 | On Node 1: `ping -c 4 192.168.56.202` | Four replies, 0% packet loss | `docs/screenshots/network-node1-to-node2.png` |
| T02 | Node 2 can reach Node 1 | On Node 2: `ping -c 4 192.168.56.201` | Four replies, 0% packet loss | `docs/screenshots/network-node2-to-node1.png` |
| T03 | MPI installation | `mpicc --version && mpirun --version` on both | GCC 13.3.0, Open MPI 4.1.6 | `docs/screenshots/openmpi-version-node2.png` |
| T04 | Point-to-point exchange | `bash scripts/run_mpi.sh data_exchange` | Send 42; receive 42 on Node 2; return/receive 43 | `docs/screenshots/experiment1-data-exchange.png` |
| T05 | Parallel sum | `bash scripts/run_mpi.sh parallel_sum` | Rank 0 local 10, rank 1 local 26, final 36 | `docs/screenshots/experiment2-parallel-sum.png` |
| T06 | Broadcast and gather | `bash scripts/run_mpi.sh collective_comm` | Multiplier 5, values 50 and 100, gathered `[50, 100]` | `docs/screenshots/experiment3-collective.png` |
| T07 | Performance program | `bash scripts/run_mpi.sh mpi_performance` | 6 rows, 1 B–1 MiB with positive RTT | `docs/screenshots/experiment4-performance.png` |
| T08 | Negative validation | `mpirun -np 1 ./bin/data_exchange` | Process-count rejection, exit code 1 | `docs/screenshots/negative-test.png` |
| T09 | Placement verification | `make mapping` | One rank on each node; overall 36 | `results/reference/process_mapping.txt`; screenshot |
| T10 | Repeatability | `make repeatability` | 3 full benchmark tables, nonidentical but valid | `results/reference/performance_repeatability.txt`; screenshot |

All ten checks were completed during the original project development. Different future VMs/hosts may produce different latency/throughput numbers; process placement should still be checked after each topology change.

## Measured reference performance

| Bytes | RTT Run 1 (ms) | RTT Run 2 (ms) | RTT Run 3 (ms) | Mean RTT (ms) |
|---:|---:|---:|---:|---:|
| 1 | 0.4517 | 0.4961 | 0.4871 | 0.4783 |
| 1024 | 0.5244 | 0.4696 | 0.4120 | 0.4687 |
| 16384 | 1.0991 | 1.2005 | 0.9622 | 1.0873 |
| 65536 | 2.9708 | 2.9289 | 2.4992 | 2.7996 |
| 262144 | 5.1474 | 5.3970 | 5.4290 | 5.3245 |
| 1048576 | 15.3449 | 14.5835 | 14.5629 | 14.8304 |

The one-way delay approximation is half the mean round-trip time; there is no independent one-way timing here. The round-trip throughput counts bytes sent **and** returned. The original 1-byte throughput output rounds to 0.00 MiB/s; the CSV contains a more precise reconstructed value from RTT.

## Notes on reading results

- Standard output from two MPI processes can be displayed out of execution order because each process buffers and forwards messages independently.
- `MPI_Reduce()` combines both local sums before rank 0 prints `36`, regardless of where the rank 1 `printf()` appears in the merged terminal stream.
- A deliberately nonzero exit code from a one-process invocation is a **passing negative test**, not a successful application run.
- CI is a local **two-process** smoke check on one runner. Only the archived VirtualBox process map and terminal screenshots document **two-VM** operation.
