#include "NSG.h"
#include <iostream>
#include <vector>


int main() {
    int64_t num_points = 0;
    int64_t dim = 0;
    int max_lines = 10000;
    std::string path = "../glove.6B/glove.6B.50d.txt";

    // 使用 ReadData 函数读取数据
    std::vector<std::vector<float>> data = ReadData(path, num_points, dim, max_lines);
    std::cout << "Loaded " << num_points << " lines of dimension " << dim << std::endl;


    return 0;
}
