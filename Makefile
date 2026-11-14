SHELL := /bin/bash
CC := mpicc
CFLAGS := -Wall -Wextra -O2
PROGRAMS := data_exchange parallel_sum collective_comm mpi_performance
BINARIES := $(addprefix bin/,$(PROGRAMS))

.PHONY: all clean deploy test performance repeatability mapping summary help

all: $(BINARIES)

bin:
	mkdir -p bin

bin/%: src/%.c | bin
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BINARIES)

deploy: all
	bash scripts/deploy.sh

test: all
	bash tests/smoke_test.sh

performance: all
	mkdir -p results/local
	bash scripts/run_mpi.sh mpi_performance | tee results/local/performance_latest.txt

repeatability: all
	bash scripts/repeatability.sh

mapping: all
	mkdir -p results/local
	bash scripts/run_mpi.sh parallel_sum --display-map 2>&1 | tee results/local/process_mapping_latest.txt

summary:
	python3 analysis/analyze_performance.py results/reference/performance_repeatability.txt --csv results/local/performance_summary.csv

help:
	@printf '%s\n' 'make             Compile all four MPI C programs' \
		'make deploy      Copy compiled executables to Node 2 (SSH keys required)' \
		'make test        Test four experiments and invalid process count' \
		'make mapping     Verify rank placement on the two nodes' \
		'make performance Run current ping-pong latency/throughput experiment' \
		'make repeatability  Repeat the performance experiment three times' \
		'make summary     Summarize the three captured reference runs' \
		'make clean       Delete generated binaries'
