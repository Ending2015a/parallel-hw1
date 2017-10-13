#include <mpi.h>
#include <stilib.h>
#include <math.h>


//argv[1] = size of the list
//argv[2] = input file name
//argv[3] = output file name
int main(int argc, char **argv){


    // initializing
    MPI_Init(&argc, &argv);
    
    int world_size;
    int world_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int num;
    num = atoi(argv[1]);

    

    //read file



    // start sorting
    int sorted=0;

    while(!sorted){

        sorted = 1;

    }
    

    //final
    MPI_Finalize();
}
