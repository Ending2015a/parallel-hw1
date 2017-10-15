#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <string>

int main(int argc, char **argv){

    clock_t start = clock();

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dis(-987654321.123, 987654321.123);
    std::uniform_int_distribution<int> idis(0, 6);

    std::ofstream fout(argv[2], std::ofstream::binary);
    std::ofstream fans(std::string(argv[2])+".ans", std::ofstream::binary);

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

    std::cout << "gen: take " << ((double) (end-start)) / CLOCKS_PER_SEC << " secs" << std::endl;

    std::cout << "sorting..." << std::endl;
    start = clock();
    std::sort(que.begin(), que.end());

    for(int i=0;i<num;i++){
        fans.write((char*)&que[i], sizeof(float));
    }

    fans.close();

    std::cout << "sort: take " << ((double)(clock()-start)) / CLOCKS_PER_SEC << " secs" << std::endl;
    return 0;

}
