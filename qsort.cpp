#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *finp, *foutp;
int num_size;
float *num;

int compare(const void* a, const void* b){
    float f1 = *(float*)a;
    float f2 = *(float*)b;
    return f1>f2?1:f1==f2?0:-1;
}

int main(int argc, char **argv){
    finp = fopen(argv[2], "rb");
    foutp = fopen(argv[3], "wb");

    num_size = atoi(argv[1]);

    num = (float*)malloc(sizeof(float) * num_size);

    fread(num, sizeof(float), num_size, finp);

    qsort(num, num_size, sizeof(float), compare);


    fwrite(num, sizeof(float), num_size, foutp);


    fclose(finp);
    fclose(foutp);

    free(num);

    return 0;
}
