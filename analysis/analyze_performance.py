#!/usr/bin/env python3
"""Summarize real captured MPI ping-pong trials; Python standard library only.

Usage:
  python3 analysis/analyze_performance.py results/reference/performance_repeatability.txt
  python3 analysis/analyze_performance.py results/local/performance_repeatability_latest.txt \
      --csv results/local/performance_summary_latest.csv

The benchmark's 1-byte throughput is rounded to 0.00 in the original log.
We recompute a precise effective rate from measured RTT for CSV/plotting.
"""
import argparse
import csv
from collections import defaultdict
from pathlib import Path
import re
import statistics

LINE = re.compile(r"^\s*(\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s*$")
EXPECTED = (1, 1024, 16384, 65536, 262144, 1048576)


def read_trials(path: Path):
    records = defaultdict(list)
    for line in path.read_text(encoding="utf-8").splitlines():
        m = LINE.match(line)
        if not m:
            continue
        size = int(m.group(1))
        if size not in EXPECTED:
            continue
        rtt_ms = float(m.group(2))
        one_way_ms = float(m.group(3))
        logged_mibs = float(m.group(4))
        if rtt_ms <= 0:
            raise ValueError("Nonpositive RTT in captured results")
        precise_mibs = (2 * size * 1000.0) / (rtt_ms * 1024.0 * 1024.0)
        records[size].append((rtt_ms, one_way_ms, logged_mibs, precise_mibs))
    if set(records) != set(EXPECTED):
        raise ValueError(f"Expected measurements for {EXPECTED}, found {sorted(records)}")
    trial_counts = {len(x) for x in records.values()}
    if len(trial_counts) != 1:
        raise ValueError("Each message size must have the same number of trials")
    return records


def summarize(records):
    summary = []
    for size in EXPECTED:
        rows = records[size]
        rtts = [r[0] for r in rows]
        oneways = [r[1] for r in rows]
        throughput = [r[3] for r in rows]
        summary.append({
            "message_bytes": size,
            "trials": len(rows),
            "mean_rtt_ms": round(statistics.mean(rtts), 6),
            "stdev_rtt_ms": round(statistics.stdev(rtts), 6) if len(rows) > 1 else 0,
            "mean_approx_oneway_ms": round(statistics.mean(oneways), 6),
            "mean_effective_mib_s": round(statistics.mean(throughput), 6),
        })
    return summary


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("input", type=Path, help="Measured repeatability text log")
    p.add_argument("--csv", type=Path, default=None, help="Optional output CSV path")
    args = p.parse_args()
    summary = summarize(read_trials(args.input))
    print(f"Summary for {args.input} (real measured trials):")
    print(f"{'Bytes':>10} {'Runs':>5} {'Mean RTT ms':>13} {'Std dev ms':>13} {'Mean MiB/s':>13}")
    for row in summary:
        print(f"{row['message_bytes']:>10} {row['trials']:>5} "
              f"{row['mean_rtt_ms']:>13.4f} {row['stdev_rtt_ms']:>13.4f} "
              f"{row['mean_effective_mib_s']:>13.4f}")
    if args.csv:
        args.csv.parent.mkdir(parents=True, exist_ok=True)
        with args.csv.open("w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=list(summary[0]))
            writer.writeheader()
            writer.writerows(summary)
        print(f"Wrote {args.csv}")


if __name__ == "__main__":
    main()
