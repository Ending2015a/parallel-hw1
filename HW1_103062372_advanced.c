#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

//#define __MEASURE_TIME__

#define MINIMUM_NUMBER 4
#define SWAP_B(a, b) ((a)^=(b)^=(a)^=(b))
#define SWAP_F(a, b) float c=(a);(a)=(b);(b)=c
#define SWAP(a, b) SWAP_F(a, b)
#define IS_EVEN(a) ((~(a))&1)
#define IS_ODD(a) ((a)&1)

#ifdef __MEASURE_TIME__
    double __temp_time=0;
    #define TIC     __temp_time = MPI_Wtime()
    #define TOC(X)  X += (MPI_Wtime() - __temp_time)
    #define TIME(X) X = MPI_Wtime()
#else
    #define TIC
    #define TOC(X)
    #define TIME(X)
#endif


float *num=NULL;
float *buffer=NULL;
float *result=NULL;
int num_size=0;
int world_size, world_rank;
int valid_size;
int head, tail;
int left_proc, right_proc;
int has_left_proc = 1, has_right_proc=1;
int left_size, right_size;

double total_comm_time = 0;

double start_time;
double end_time;
double start_read_file;
double end_read_file;
double start_write_file;
double end_write_file;


#define FROM_HEAD 0
#define FROM_TAIL 1

int compare(const void *a, const void *b){
    float f1 = *(float*)a;
    float f2 = *(float*)b;
    return f1 > f2 ? 1 : f1==f2 ? 0:-1;
}

int merge(float *left, int left_size,
            float *right, int right_size, int direction){

    
    int count=(num_size-1)*direction;
    
    int l = (left_size-1) * direction;
    int r = (right_size-1) * direction;
    
    //from head
    if(!direction){
        while(count < num_size){
            if(l==left_size) result[count++] = right[r++];
            else if(r==right_size) result[count++] = left[l++];
            else result[count++] = (right[r]<left[l]) ? right[r++]:left[l++];
        }
    }else{
        while(count>=0){
            if(l<0)result[count--] = right[r--];
            else if(r<0)result[count--] = left[l--];
            else result[count--] = (right[r]>=left[l]) ? right[r--]:left[l--];
        }
    }

    if(result[0] == 0){
        printf("Rank %d, result is zero\n", world_rank);
    }
    //if same as before then sorted
    if(memcmp((void*)num, (void*)result, num_size*sizeof(float))){
        memcpy((void*)num, (void*)result, num_size*sizeof(float));
        return 1; //has swapped
    }else
        return 0;  //no swap
}


int odd_even_sort(){

    if (!num_size)return 1;


 
    int swap = 0;
    MPI_Status status;

    //even-phase

    //external
    if(IS_EVEN(world_rank)){

        // N <- N+1
        if(has_right_proc){
        
            TIC;

            MPI_Sendrecv(num, num_size, MPI_FLOAT, right_proc, 0,
                        buffer, right_size, MPI_FLOAT, right_proc, 0, MPI_COMM_WORLD, &status);

            TOC(total_comm_time);

            swap = merge(num, num_size, buffer, right_size, FROM_HEAD);
        }
    }else{
        
        // N-1 -> N
        if(has_left_proc){

            TIC;

            MPI_Sendrecv(num, num_size, MPI_FLOAT, left_proc, 0,
                        buffer, left_size, MPI_FLOAT, left_proc, 0, MPI_COMM_WORLD, &status);

            TOC(total_comm_time);

            swap = merge(buffer, left_size, num, num_size, FROM_TAIL);
        }
    }

    

    //odd-phase

    //external
    if(IS_EVEN(world_rank)){
    
        //N-1 -> N
        if(has_left_proc){

            TIC;

            MPI_Sendrecv(num, num_size, MPI_FLOAT, left_proc, 0,
                        buffer, left_size, MPI_FLOAT, left_proc, 0, MPI_COMM_WORLD, &status);

            TOC(total_comm_time);

            swap = merge(buffer, left_size, num, num_size, FROM_TAIL);
        }

    }else{
        // N <- N+1
        if(has_right_proc){
    
            TIC;

            MPI_Sendrecv(num, num_size, MPI_FLOAT, right_proc, 0,
                        buffer, right_size, MPI_FLOAT, right_proc, 0, MPI_COMM_WORLD, &status);

            TOC(total_comm_time);

            swap = merge(num, num_size, buffer, right_size, FROM_HEAD);
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

    TIME(start_time);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int total_num = atoi(argv[1]);
    
    /*  calculate range  */
    valid_size = world_size;

    //make sure the numbers divided to each processor has more than MINIMUM_NUMBER
    if(total_num < MINIMUM_NUMBER * world_size){
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

    int divide = total_num/valid_size;
    int remain = total_num % valid_size;
    int bonus = (world_rank < remain) ? 1:0;
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

    left_size = divide + ((world_rank-1) < remain ? 1:0);
    right_size = divide + ((world_rank+1) < remain ? 1:0);


#ifdef __DEBUG__
    printf("Rank %d: num_size %d, offset %d, left %d, right %d\n", world_rank, num_size, offset, left_size, right_size);
#endif


    TIME(start_read_file);

        /*  read file  */
    num = (float*)malloc(sizeof(float) * num_size);
    result = (float*)malloc(sizeof(float) * num_size);
    buffer = (float*)malloc(sizeof(float) * (num_size+1));

    MPI_Status status;
    MPI_File_read_at_all(finh, offset*sizeof(float), num, num_size, MPI_FLOAT, &status);


    TIME(end_read_file);


#ifdef __DEBUG__
    int i=0;
    printf("Rank %d get %d numbers\n", world_rank, num_size);
    for(i=0;i<5;i++){
        printf("%f, ", num[i]);
    }
    printf("....\n");
#endif

    int sorted = 0;
    int round=0;
    int st;

    qsort(num, num_size, sizeof(float), compare);

    while(!sorted){
#ifdef __MEASURE_TIME__
        round++;
#endif
        st = odd_even_sort();
        MPI_Allreduce(&st, &sorted, 1, MPI_INT, MPI_BAND, MPI_COMM_WORLD);
    }

    TIME(start_write_file);

    MPI_File_set_view(fouth, sizeof(float)*offset, MPI_FLOAT, MPI_FLOAT, "native", MPI_INFO_NULL);
    MPI_File_write_all(fouth, num, num_size, MPI_FLOAT, &status);   


    TIME(end_write_file);

    MPI_File_close(&finh);
    MPI_File_close(&fouth);

    free(num);
    free(buffer);
    free(result);


    TIME(end_time);

#ifdef __MEASURE_TIME__
    printf("%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %d\n", world_rank,
        start_time, start_read_file, end_read_file, start_write_file, end_write_file, end_time, total_comm_time, round);
#endif    

    //final
    MPI_Finalize();

    return 0;
}
