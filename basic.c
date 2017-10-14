#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#define MINIMUM_NUMBER 3
#define SWAP_B(a, b) ((a)^=(b)^=(a)^=(b))
#define SWAP_F(a, b) float c=(a);(a)=(b);(b)=c
#define SWAP(a, b) SWAP_F(a, b)
#define IS_EVEN(a) ((~(a))&1)
#define IS_ODD(a) ((a)&1)

float *num=NULL;
int num_size=0;
int world_size, world_rank;
int valid_size;
int head, tail;
int left_proc, right_proc;
int has_left_proc = 1, has_right_proc=1;



int odd_even_sort(){

    if (!num_size)return 1;   
 
    int swap = 0;
    float buf;
    MPI_Status status;

    //even-phase
    int idx;
    //internal
    for(idx=IS_ODD(head); idx+1<num_size; idx+=2 ){
        if(num[idx]>num[idx+1]){
            SWAP(num[idx], num[idx+1]);
            swap = 1;
        }
    }


    //external
    // [  |  || >]  [< ||  |  ]   [ | ]   [ | ]
    if(IS_ODD(head) && has_left_proc){
        MPI_Sendrecv(&num[0], 1, MPI_FLOAT, left_proc, 0,
                    &buf, 1, MPI_FLOAT, left_proc, 0, MPI_COMM_WORLD, &status);
        if(num[0]<buf){
            num[0]=buf;
            swap = 1;
        }
    }
    if(IS_EVEN(tail) && has_right_proc){
        MPI_Sendrecv(&num[num_size-1], 1, MPI_FLOAT, right_proc, 0,
                    &buf, 1, MPI_FLOAT, right_proc, 0, MPI_COMM_WORLD, &status);
        if(num[num_size-1]>buf){
            num[num_size-1] = buf;
            swap = 1;
        }
    }

    //odd-phase
    // [  ||  |  ]  [  |  || >]  [< || >]  [< || ]
    // internal
    for(idx=IS_EVEN(head); idx+1<num_size; idx+=2){
        if(num[idx] > num[idx+1]){
            SWAP(num[idx], num[idx+1]);
            swap=1;
        }
    }

    //external
    if(IS_EVEN(head) && has_left_proc){
        MPI_Sendrecv(&num[0], 1, MPI_FLOAT, left_proc, 0,
                    &buf, 1, MPI_FLOAT, left_proc, 0, MPI_COMM_WORLD, &status);
        if(num[0]<buf){
            num[0]=buf;
            swap = 1;
        }
    }
    if(IS_ODD(tail) && has_right_proc){
        MPI_Sendrecv(&num[num_size-1], 1, MPI_FLOAT, right_proc, 0,
                    &buf, 1, MPI_FLOAT, right_proc, 0, MPI_COMM_WORLD, &status);
        if(num[num_size-1]>buf){
            num[num_size-1] = buf;
            swap = 1;
        }
    }

    return !swap;
}


//argv[1] = size of the list
//argv[2] = input file name
//argv[3] = output file name
int main(int argc, char **argv){

    // initializing
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int total_num = atoi(argv[1]);
    
    /*  calculate range  */
    valid_size = world_size;

    //make sure the numbers divided to each processor has more than MINIMUM_NUMBER
    if(total_num/world_size < MINIMUM_NUMBER){
        valid_size = total_num/MINIMUM_NUMBER;
    }

#ifdef __DEBUG__
    printf("Rank %d, valid_size %d\n", world_rank, valid_size);
#endif

    
    MPI_File finh, fouth;
    MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL, &finh);
    MPI_File_open(MPI_COMM_WORLD, argv[3], MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fouth);


    //Ex: 14 numbers divided to 5 processors
    // each processor get 2 nubmers and remain 4 numbers
    // so if the processor's rank is less than 4 then will get a bonus
    // something like this: [3, 3, 3, 3, 2]

    int remain = total_num % valid_size;
    int bonus = (world_rank < remain) ? 1:0;
    int divide = total_num/valid_size;
    num_size = bonus + divide;

    // If I got a bonus then the processors which the rank was less than me also got a bonus too
    // so we need to plus the world_rank to offset.
    // If I didnt get a bouns then the offset just need to plus the remains 
    int offset = divide * world_rank + (bonus?world_rank:remain);

    if(world_rank >= valid_size){
        num_size=0;
        offset=0;
    }

    //the index of the first/last item in the whole list
    head = offset;
    tail = offset + num_size-1;

    //check if has left/right neighbors
    left_proc = world_rank-1;
    right_proc = world_rank+1;
    if (left_proc < 0)
        has_left_proc = 0;
    if (right_proc >= valid_size)
        has_right_proc = 0;

#ifdef __DEBUG__
    printf("Rank %d: num_size %d, offset %d\n", world_rank, num_size, offset);
#endif
        /*  read file  */
    num = (float*)malloc(sizeof(float) * num_size);

    MPI_Status status;
    MPI_File_read_at_all(finh, offset*sizeof(float), num, num_size, MPI_FLOAT, &status);


#ifdef __DEBUG__
    int i=0;
    printf("Rank %d get %d numbers: ", world_rank, num_size);
    for(i=0;i < num_size; i++){
        printf("%f, ", num[i]);
    }
    printf("\n");
#endif

    int sorted = 0;

    while(!sorted){
        int st = odd_even_sort();
        MPI_Allreduce(&st, &sorted, 1, MPI_INT, MPI_BAND, MPI_COMM_WORLD);
    }

#ifdef __DEBUG__
    printf("Rank %d:", world_rank);
    for(i=0;i<num_size;i++){
        printf("%f, ", num[i]);
    }
    printf("\n");
#endif

    MPI_File_set_view(fouth, sizeof(float)*offset, MPI_FLOAT, MPI_FLOAT, "native", MPI_INFO_NULL);
    MPI_File_write_all(fouth, num, num_size, MPI_FLOAT, &status);   

    //MPI_Barrier(MPI_COMM_WORLD);

    MPI_File_close(&finh);
    MPI_File_close(&fouth);

    //final
    MPI_Finalize();

    return 0;
}
