#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <algorithm>


int main(int argc, char **argv){


    std::srand(unsigned(std::time(0)));
    std::ofstream fout(argv[2], std::ofstream::binary);

    int num = atoi(argv[1]);

    std::vector<float> que;

    for(int i=0;i<num; i++){
        que.push_back(i+10);
    }

    std::random_shuffle(que.begin(), que.end());

    for(int i=0;i< num;i++){
        float n = que[i];
        fout.write((char*)&n, sizeof(n));
    }

    fout.close();
    return 0;

}
