#ifndef DATA_READER_H
#define DATA_READER_H

#include <vector>
#include <string>

std::vector<std::vector<float>> ReadData(const std::string& file_path, int64_t& num_points, int64_t& dim, int max_lines);

#endif // DATA_READER_H
