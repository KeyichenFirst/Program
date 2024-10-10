#ifndef NNDESCENT_H
#define NNDESCENT_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <queue>
#include <utility>
#include <limits>

// Neighbor结构体表示邻居信息，包括邻居ID、距离以及标志位
struct Neighbor {
    int id;
    float distance;
    bool flag;

    Neighbor(int id, float distance, bool flag);

    // 定义比较操作符，以便在优先队列中按距离排序
    bool operator<(const Neighbor& other) const;
};

// NNDescent算法核心类，包含Nhood结构体用于存储邻居池以及核心的近邻下降算法
class NNDescent {
public:
    struct Nhood {
        std::vector<Neighbor> pool;
        int M;

        Nhood(int s, int64_t N);  // 构造函数
    };

    std::vector<Nhood> graph; // 邻接图
    int64_t d;  // 向量维度
    int K;      // 每个点的邻居数量

    NNDescent(int64_t dim, const std::string& metric);  // 构造函数
    void Descent();  // 近邻下降算法

private:
    // 你可以在此添加其他私有成员或方法
};

// L2范数距离计算函数
float L2Sqr(const float* A, const float* B, int dim);

// 计算离质点最近的点
int CalculateCentroid(const std::vector<std::vector<float>>& points);

// 更新图的结构
void UpdateGraph(NNDescent& nndescent, int node_id, const std::vector<Neighbor>& neighbors);

// 读取数据的函数声明
std::vector<std::vector<float>> ReadData(const std::string& file_path, int64_t& num_points, int64_t& dim, int max_lines);

// KNN初始化函数声明
std::vector<std::vector<int>> KNNInit(const std::vector<std::vector<float>>& data, int K);

// NSG索引构建方法
void BuildNSGIndex(const std::vector<std::vector<int>>& knn_graph, const std::vector<std::vector<float>>& data, int K, int L, int m);

#endif // NNDESCENT_H
