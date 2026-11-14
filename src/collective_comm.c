#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;
    int multiplier = 0;
    int local_value, local_result;
    int gathered_results[2] = {0, 0};

    char hostname[MPI_MAX_PROCESSOR_NAME];
    int name_length;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0)
            printf("This program requires exactly 2 processes.\n");

        MPI_Finalize();
        return 1;
    }

    MPI_Get_processor_name(hostname, &name_length);

    // Rank 0 initializes the multiplier
    if (rank == 0) {
        multiplier = 5;
        printf("Rank 0: Broadcasting multiplier = %d\n",
               multiplier);
    }

    // Broadcast multiplier to all processes
    MPI_Bcast(&multiplier, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process performs its own calculation
    local_value = (rank + 1) * 10;
    local_result = local_value * multiplier;

    printf("Rank %d on %s: %d x %d = %d\n",
           rank, hostname, local_value,
           multiplier, local_result);
    fflush(stdout);

    // Gather the results at Rank 0
    MPI_Gather(&local_result, 1, MPI_INT,
               gathered_results, 1, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Final gathered results: [%d, %d]\n",
               gathered_results[0], gathered_results[1]);
    }

    MPI_Finalize();
    return 0;
}
