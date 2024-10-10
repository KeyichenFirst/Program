// DataReader.h
#ifndef DATA_READER_H
#define DATA_READER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

inline std::vector<std::vector<float>> ReadData(const std::string& file_path, int64_t& num_points, int64_t& dim, int max_lines) {
    std::ifstream file(file_path);
    std::vector<std::vector<float>> data;
    std::string line;
    int line_count = 0;

    std::cout << "begin -------" << std::endl;

    while (std::getline(file, line) && line_count < max_lines) {
        std::istringstream ss(line);
        std::string word;  // For storing the first word in the line
        ss >> word;        // Skip the first word of the line
        
        std::vector<float> vec;
        float value;

        // Read the remaining float values
        while (ss >> value) {
            vec.push_back(value); // Add the float to the vector
        }

        if (dim == 0) {
            dim = vec.size(); // Set the dimension if not already set
        }
        data.push_back(vec);
        
        line_count++;
    }
    num_points = data.size();
    return data;
}

#endif // DATA_READER_H
