#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;
    int numbers[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int local_data[4];
    int local_sum = 0;
    int total_sum = 0;

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

    MPI_Scatter(numbers, 4, MPI_INT,
                local_data, 4, MPI_INT,
                0, MPI_COMM_WORLD);

    for (int i = 0; i < 4; i++) {
        local_sum += local_data[i];
    }

    printf("Rank %d on %s: Local sum = %d\n",
           rank, hostname, local_sum);
    fflush(stdout);

    MPI_Reduce(&local_sum, &total_sum, 1,
               MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Final parallel sum = %d\n", total_sum);
    }

    MPI_Finalize();
    return 0;
}
