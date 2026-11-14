#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0)
            printf("Exactly 2 MPI processes required.\n");
        MPI_Finalize();
        return 1;
    }

    int message_sizes[] = {
        1, 1024, 16384, 65536, 262144, 1048576
    };

    int tests = sizeof(message_sizes) / sizeof(message_sizes[0]);
    int max_size = 1048576;

    char *buffer = malloc(max_size);

    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    memset(buffer, 'A', max_size);

    if (rank == 0) {
        printf("\nMPI PERFORMANCE ANALYSIS\n");
        printf("Node1 <--> Node2\n\n");

        printf("%12s %14s %16s %18s\n",
               "Bytes", "RTT (ms)",
               "One-way (ms)", "Throughput (MiB/s)");
    }

    for (int t = 0; t < tests; t++) {
        int n = message_sizes[t];
        int iterations = (n <= 1024) ? 1000 : 100;
        int warmup = 10;

        // Warm-up exchanges
        for (int i = 0; i < warmup; i++) {
            if (rank == 0) {
                MPI_Send(buffer, n, MPI_BYTE, 1, 0,
                         MPI_COMM_WORLD);
                MPI_Recv(buffer, n, MPI_BYTE, 1, 1,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                MPI_Recv(buffer, n, MPI_BYTE, 0, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(buffer, n, MPI_BYTE, 0, 1,
                         MPI_COMM_WORLD);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);

        double start = 0.0;

        if (rank == 0)
            start = MPI_Wtime();

        // Timed ping-pong exchanges
        for (int i = 0; i < iterations; i++) {
            if (rank == 0) {
                MPI_Send(buffer, n, MPI_BYTE, 1, 0,
                         MPI_COMM_WORLD);
                MPI_Recv(buffer, n, MPI_BYTE, 1, 1,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                MPI_Recv(buffer, n, MPI_BYTE, 0, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(buffer, n, MPI_BYTE, 0, 1,
                         MPI_COMM_WORLD);
            }
        }

        if (rank == 0) {
            double elapsed = MPI_Wtime() - start;

            double rtt_ms = elapsed * 1000.0 / iterations;
            double one_way_ms = rtt_ms / 2.0;

            double throughput =
                (2.0 * n * iterations) /
                (elapsed * 1024.0 * 1024.0);

            printf("%12d %14.4f %16.4f %18.2f\n",
                   n, rtt_ms, one_way_ms, throughput);
            fflush(stdout);
        }
    }

    free(buffer);

    MPI_Finalize();
    return 0;
}
