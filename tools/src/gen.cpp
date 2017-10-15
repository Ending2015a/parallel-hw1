#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>

int main(int argc, char **argv){

    clock_t start = clock();

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dis(-987654321.123, 987654321.123);
    std::uniform_int_distribution<int> idis(0, 6);

    std::ofstream fout(argv[2], std::ofstream::binary);

    int num = atoi(argv[1]);

    std::vector<float> que;

    for(int i=0;i<num; i++){
        que.push_back(dis(gen)/pow(10, idis(gen)));
    }

    std::random_shuffle(que.begin(), que.end());

    for(int i=0;i< num;i++){
        float n = que[i];
        fout.write((char*)&n, sizeof(n));
    }

    fout.close();


    clock_t end = clock();

    std::cout << "take " << ((double) (end-start)) / CLOCKS_PER_SEC << " secs" << std::endl;
    return 0;

}
