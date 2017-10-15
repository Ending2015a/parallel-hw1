#include <iostream>
#include <fstream>


int main(int argc, char **argv){
    
    std::ifstream fin(argv[1], std::ifstream::binary);

    bool fst=true;
    bool sorted=true;
    float n, m;
    int count=0;
    while(fin.read((char*)&n, sizeof(float))){
        if(count <= 10000)
            printf("%f, ", n);
        count++;
        if(count <= 10000 && count%5==0){
            printf("\n");
        }
        
        if(!fst){
            if(m>n)sorted=false;
        }else{
            fst=false;
        }
        m = n;
    }
    
    std::cout << std::endl;
    std::cout << "Total: " << count << " numbers" << std::endl;
    if(sorted)
        std::cout << "The list is sorted" << std::endl;
    else
        std::cout << "The list is not sorted" << std::endl;

    fin.close();

    return 0;
}
