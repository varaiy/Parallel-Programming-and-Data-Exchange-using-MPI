#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;
    int data;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0)
            printf("This program requires exactly 2 MPI processes.\n");

        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        data = 42;

        printf("System 1: Sending %d to System 2\n", data);

        MPI_Send(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

        MPI_Recv(&data, 1, MPI_INT, 1, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("System 1: Received %d from System 2\n", data);
    }
    else if (rank == 1) {
        MPI_Recv(&data, 1, MPI_INT, 0, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("System 2: Received %d from System 1\n", data);

        data = data + 1;

        MPI_Send(&data, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
