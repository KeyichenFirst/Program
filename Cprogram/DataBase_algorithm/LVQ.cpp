#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <utility>
#include <tuple>

// 1.实现ReadData函数
std::vector<std::vector<float>> ReadData(const std::string& file_path, int64_t& num_points,int64_t& dim, int max_lines){
    std::ifstream file(file_path);
    std::vector<std::vector<float>> data;
    std::string line;
    int line_count = 0;

    std::cout << "begin -------" << std::endl;

    while(std::getline(file , line) && line_count < max_lines) {
        std::istringstream ss(line);
        std::string word;  // 用于存储单词部分
        ss >> word;        // 跳过该行的第一个单词
        
        std::vector<float> vec;
        float value;

        // 读取剩下的浮点数
        while (ss >> value) {
            vec.push_back(value); // 将浮点数加入向量
        }

        if(dim == 0){
            dim  = vec.size();
        }
        data.push_back(vec);
        line_count++;
    }
    num_points = data.size();
    return data;
}

// 1.进行粗量化 (First-level Quantization)
std::tuple<std::vector<std::vector<int>>, std::vector<float>, std::vector<float>> FirstLevelQuantization(const std::vector<std::vector<float>>& data, int B) {
    int num_points = data.size();
    int dim = data[0].size();

    // 确定数据的上界和下界
    std::vector<float> min_vals(dim, std::numeric_limits<float>::max());
    std::vector<float> max_vals(dim, std::numeric_limits<float>::lowest());

    for (const auto& vec : data) {
        for (int i = 0; i < dim; i++) {
            if (vec[i] > max_vals[i]) max_vals[i] = vec[i];
            if (vec[i] < min_vals[i]) min_vals[i] = vec[i];
        }
    }

    // 计算每个维度的步长 Δ
    std::vector<float> deltas(dim);
    for (int i = 0; i < dim; i++) {
        deltas[i] = (max_vals[i] - min_vals[i]) / (pow(2, B) - 1);
    }

    // 执行量化
    std::vector<std::vector<int>> quantized_data(num_points, std::vector<int>(dim));
    for (int i = 0; i < num_points; i++) {
        for (int j = 0; j < dim; j++) {
            // 计算量化值 Q(x) = floor((x - l) / Δ + 0.5)
            quantized_data[i][j] = static_cast<int>(std::floor((data[i][j] - min_vals[j]) / deltas[j] + 0.5));
        }
    }

    return {quantized_data, min_vals, deltas};
}

// 计算残差 (Residuals)
std::vector<std::vector<float>> ComputeResiduals(const std::vector<std::vector<float>>& original_data, 
                                                 const std::vector<std::vector<int>>& quantized_data, 
                                                 const std::vector<float>& min_vals, 
                                                 const std::vector<float>& deltas) {
    int num_point = original_data.size();
    int dim = original_data[0].size();

    std::vector<std::vector<float>> residuals(num_point, std::vector<float>(dim));

    for (int i = 0; i < num_point; i++) {
        for (int j = 0; j < dim; j++) {
            float quantized_value = min_vals[j] + quantized_data[i][j] * deltas[j];
            // 计算残差 r = x - Q(x)
            residuals[i][j] = original_data[i][j] - quantized_value;
        }
    }
    return residuals;
}

// Step 2：量化残差 (Second-level Quantization)
std::tuple<std::vector<std::vector<int>>, std::vector<float>, std::vector<float>> SecondLevelQuantization(const std::vector<std::vector<float>>& residuals, int B_res) {
    int num_points = residuals.size();
    int dim = residuals[0].size();

    // 获取残差的上界和下界
    std::vector<float> min_vals_res(dim, std::numeric_limits<float>::max());
    std::vector<float> max_vals_res(dim, std::numeric_limits<float>::lowest());

    for (const auto& vec : residuals) {
        for (int i = 0; i < dim; i++) {
            if (vec[i] > max_vals_res[i]) max_vals_res[i] = vec[i];
            if (vec[i] < min_vals_res[i]) min_vals_res[i] = vec[i];
        }
    }

    // 计算残差的步长 Δ_res
    std::vector<float> deltas_res(dim);
    for (int i = 0; i < dim; i++) {
        deltas_res[i] = (max_vals_res[i] - min_vals_res[i]) / (pow(2, B_res) - 1);
    }

    // 执行量化
    std::vector<std::vector<int>> quantized_residuals(num_points, std::vector<int>(dim));
    for (int i = 0; i < num_points; i++) {
        for (int j = 0; j < dim; j++) {
            quantized_residuals[i][j] = static_cast<int>(std::floor((residuals[i][j] - min_vals_res[j]) / deltas_res[j] + 0.5));
        }
    }

    return {quantized_residuals, min_vals_res, deltas_res};
}

// 还原初步量化的数据
std::vector<std::vector<float>> ReconstructFirstLevel(const std::vector<std::vector<int>>& quantized_data, 
                                                     const std::vector<float>& min_vals, 
                                                     const std::vector<float>& deltas) {
    int num_points = quantized_data.size();
    int dim = quantized_data[0].size();

    std::vector<std::vector<float>> reconstructed_data(num_points, std::vector<float>(dim));

    for (int i = 0; i < num_points; i++) {
        for (int j = 0; j < dim; j++) {
            reconstructed_data[i][j] = quantized_data[i][j] * deltas[j] + min_vals[j];
        }
    }

    return reconstructed_data;
}

// 还原残差
std::vector<std::vector<float>> ReconstructResiduals(const std::vector<std::vector<int>>& quantized_residuals, 
                                                    const std::vector<float>& min_vals_res, 
                                                    const std::vector<float>& deltas_res) {
    int num_points = quantized_residuals.size();
    int dim = quantized_residuals[0].size();

    std::vector<std::vector<float>> reconstructed_residuals(num_points, std::vector<float>(dim));

    for (int i = 0; i < num_points; i++) {
        for (int j = 0; j < dim; j++) {
            reconstructed_residuals[i][j] = quantized_residuals[i][j] * deltas_res[j] + min_vals_res[j];
        }
    }

    return reconstructed_residuals;
}

// 组合初步量化和残差的结果
std::vector<std::vector<float>> ReconstructFinalData(const std::vector<std::vector<float>>& first_level_data, 
                                                     const std::vector<std::vector<float>>& residuals) {
    int num_points = first_level_data.size();
    int dim = first_level_data[0].size();

    std::vector<std::vector<float>> reconstructed_data(num_points, std::vector<float>(dim));

    for (int i = 0; i < num_points; i++) {
        for (int j = 0; j < dim; j++) {
            reconstructed_data[i][j] = first_level_data[i][j] + residuals[i][j];
        }
    }

    return reconstructed_data;
}

int main(){
    int64_t num_points = 0;
    int64_t dim =  0;
    int64_t max_lines =  100;
    int B = 8; // 第一层量化比特数
    int B_res = 4; // 第二层量化比特数
    std::string path = "../glove.6B/glove.6B.50d.txt";

    // 1.读取数据
    std::vector<std::vector<float>> data = ReadData(path ,num_points, dim , max_lines);
    std::cout << "Loaded " << num_points << " lines of dimension " << dim << std::endl;

    // 2.执行初步量化
    auto [quantized_data, min_vals, deltas] = FirstLevelQuantization(data, B);
    std::cout << "First-level quantization completed." << std::endl;

    // 3.计算残差
    std::vector<std::vector<float>> residuals = ComputeResiduals(data, quantized_data, min_vals, deltas);
    std::cout << "Residuals computed." << std::endl;

    // 4.对残差进行第二层量化
    auto [quantized_residuals, min_vals_res, deltas_res] = SecondLevelQuantization(residuals, B_res);
    std::cout << "Second-level quantization completed." << std::endl;

    // Step 5: 还原数据
    // 还原初步量化的数据
    std::vector<std::vector<float>> first_level_reconstructed = ReconstructFirstLevel(quantized_data, min_vals, deltas);

    // 还原残差
    std::vector<std::vector<float>> residuals_reconstructed = ReconstructResiduals(quantized_residuals, min_vals_res, deltas_res);

    // 组合初步量化和残差，得到最终的还原数据
    std::vector<std::vector<float>> final_reconstructed_data = ReconstructFinalData(first_level_reconstructed, residuals_reconstructed);

    // 输出部分结果
    for (int i = 0; i < 5; ++i) { 
        std::cout << "Original data vector " << i + 1 << ": ";
        for (const auto& val : data[i]) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
        
        std::cout << "Reconstructed data vector " << i + 1 << ": ";
        for (const auto& val : final_reconstructed_data[i]) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
