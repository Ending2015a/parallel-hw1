#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int num_proc;
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    printf("argv: %d", atoi(argv[1]));

    char proc_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(proc_name, &name_len);

    printf("Hello world from processor %s, rank %d out of %d processors\n", proc_name, my_rank, num_proc);

    MPI_Finalize();

}
