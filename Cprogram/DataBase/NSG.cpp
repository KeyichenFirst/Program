#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <utility>
#include <mutex>
#include <queue> 

struct Neighbor {
    int id;
    float distance;
    bool flag;

    Neighbor(int id ,float distance , bool flag )
        :id(id),distance(distance),flag(flag){}
    
    bool operator<(const Neighbor& other) const {
        return distance > other.distance;
    }

};

struct NNDescent {
    struct Nhood {
        std::vector<Neighbor> pool;
        int M;

        Nhood(int s, int64_t N) {
            M = s;
            pool.reserve(N);  // 为 pool 预留内存
        }
    };

    std::vector<Nhood> graph;
    int64_t d;
    int K;

    // 传入维度，和距离计算
    NNDescent(int64_t dim, const std::string& metric) : d(dim) {
        std::cout << "NNDescent initialized with dimension: " << dim 
                  << " and metric: " << metric << std::endl;
    }

    void Descent() {
         std::cout << "Descent process started..." << std::endl;

        // 这里添加图的稀疏化逻辑
        // 例如更新每个节点的邻居列表，合并候选邻居，移除不必要的邻居等
        for (size_t i = 0; i < 5; ++i) {
            // 简单示例：输出每个节点的邻居数量
            std::cout << "Node " << i << " has : ";
            for(size_t j = 0 ; j < graph[i].pool.size();j ++){
                std::cout << "(" << graph[i].pool[j].id << ", " 
                      << graph[i].pool[j].distance << ") ";
            }
            std::cout << std::endl;
        }

        std::cout << "Descent process finished." << std::endl;
    }
};


// 距离公式计算L2
float L2Sqr(const float* A, const float* B, int dim) {
    float dist_sqr = 0.0;  // 用于存储最终的距离平方值

    // 遍历每个维度，计算 (A[i] - B[i])^2 并累加
    for (int i = 0; i < dim; ++i) {
        float diff = A[i] - B[i];  // 计算 A[i] 和 B[i] 的差
        dist_sqr += diff * diff;   // 将差的平方累加到 dist_sqr
    }

    return dist_sqr;  // 返回最终的距离平方
}

// 计算离质点最近的点
int CalculateCentroid(const std::vector<std::vector<float>>& points) {
    int num_points = points.size();
    if (num_points == 0) {
        std::cerr << "No points available." << std::endl;
        return -1;
    }

    int dim = points[0].size();
    std::vector<float> centroid(dim, 0.0f);

    // 计算质点坐标
    for (const auto& point : points) {
        for (int i = 0; i < dim; ++i) {
            centroid[i] += point[i];
        }
    }

    for (int i = 0; i < dim; ++i) {
        centroid[i] /= num_points;
    }

    // 初始化最近点的索引和最小距离
    int closest_index = -1;
    float min_distance = std::numeric_limits<float>::max();

    // 找到距离质点最近的点
    for (size_t i = 0; i < num_points; ++i) {
        float dist = L2Sqr(points[i].data(), centroid.data(), dim);
        if (dist < min_distance) {
            min_distance = dist;
            closest_index = i;
        }
    }

    return closest_index;  // 返回最近点的索引
}

void UpdateGraph(NNDescent& nndescent, int node_id, const std::vector<Neighbor>& neighbors) {
    // 更新图的结构，确保邻居集合 R 更新到 graph 中
    nndescent.graph[node_id].pool.clear();  // 清除之前的邻居
    for (const auto& neighbor : neighbors) {
        nndescent.graph[node_id].pool.push_back(neighbor);  // 添加新邻居
    }
}


// 1.实现ReadData函数
std::vector<std::vector<float>> ReadData(const std::string& file_path, int64_t& num_points,int64_t& dim, int max_lines){
    std::ifstream file(file_path);
    std::vector<std::vector<float>> data;
    std::string line;
    int line_count = 0;

    std::cout  << "beging -------" << std::endl;

    while(std::getline(file , line) && line_count < max_lines) {
        std::istringstream ss(line);
        std::string word;  // 用于存储单词部分
        ss >> word;        // 跳过该行的第一个单词
        
        std::vector<float> vec;
        float value;

        // 读取剩下的浮点数
        while (ss >> value) {
            // std::cout << "begin ss >> value " << std::endl; 
            vec.push_back(value); // 将浮点数加入向量
        }

        // 输出每行的数据
        // std::cout << "Read vector: ";
        // for (float val : vec) {
        //     std::cout << val << " ";
        // }
        // std::cout << std::endl;
        // std::cout << "dim == " << dim << std::endl;
        // std::cout << "vec.size() == " << vec.size() << std::endl;
        if(dim == 0){
            dim  = vec.size();
        }
        data.push_back(vec);
        
        line_count ++;
    }
    num_points = data.size();
    return data;
}

// 2.实现KNN初始化
std::vector<std::vector<int>> KNNInit(
    const std::vector<std::vector<float>>& data ,int K){
    int num_points = data.size();
    int dim = data[0].size();
    // 候选池
    std::vector<std::vector<int>> knn_graph(num_points,std::vector<int>(K));

    for(int i = 0; i < num_points ; i ++){
        std::vector<std::pair<int,float>> distance;
        for(int j = 0; j < num_points ; ++j){
            if(i != j){
                float dis = L2Sqr(data[i].data(), data[j].data(), dim);
                distance.push_back(std::make_pair(j,dis));
            }
        }
        std::sort(distance.begin(),distance.end(),[](const auto& a,const auto& b  ){
            return a.second < b.second;
        });

        for(int k = 0; k < K; k ++){
            knn_graph[i][k] = distance[k].first;
        }
    }

    return knn_graph;
}

// 3.使用NSG索引方法
void  BuildNSGIndex(
    const std::vector<std::vector<int>>& knn_graph,
    const std::vector<std::vector<float>>& data,
    int K,int L, int m)
    {
    int num_points = data.size();
    int dim = data[0].size();
    // 导航点设置
    int entry_point = CalculateCentroid(data);
    std::cout << "entry_point---------------------------------------------- :" << entry_point << std::endl;
    std::vector<int> visited(num_points, 0);
    std::priority_queue<Neighbor> candidates;

    NNDescent nndescent(dim ,"L2");

    nndescent.graph.reserve(num_points);

    for(int i = 0; i < num_points ; i ++){
        NNDescent::Nhood nhood(K, num_points);
        for(int j = 0; j < K; j ++){
            int neighbor_id = knn_graph[i][j];
            float dist = L2Sqr(data[i].data(),data[neighbor_id].data(),dim);
            nhood.pool.push_back(Neighbor(neighbor_id,dist , true));
        }
        std::make_heap(nhood.pool.begin(),nhood.pool.end());
        nhood.pool.reserve(L);
        nndescent.graph.emplace_back(nhood);

    }

    // NSG构建过程
    candidates.push(Neighbor(entry_point, 0.0f, true));

    while (!candidates.empty())
    {
        Neighbor cur_code = candidates.top();
        candidates.pop();

        if(visited[cur_code.id]){
            continue;
        }
        visited[cur_code.id] = 1;


        // 1.获取当前候选集
        std::vector<Neighbor> E;
        for(int i = 0; i < K ; i ++){
            int neighbor_id = knn_graph[cur_code.id][i];
            if(!visited[neighbor_id]){
                float dist = L2Sqr(data[cur_code.id].data(),data[neighbor_id].data(),dim);
                E.push_back(Neighbor(neighbor_id,dist,true));
            }

        }

        // 2.对E 进行排序
        std::sort(E.begin(),E.end(),[](const Neighbor& a,const Neighbor& b){
            return a.distance < b.distance;
        });

        // 3.选择邻居，避免冲突
        std::vector<Neighbor> R;
        if(!E.empty()){
            R.push_back(E[0]);
        }

        for(size_t i = 0; i < E.size() && R.size() < static_cast<size_t>(m); i ++){
            bool conflict = false;
            for(const auto& r : R){
                float dist_pr = L2Sqr(data[E[i].id].data(),data[r.id].data(),dim); 
                // 这边的判定规则比较单一
                if(dist_pr < E[i].distance){
                    conflict = true;
                    break;
                }
            }
            if(!conflict){
                R.push_back(E[i]);
            }
        }

         // 将节点 cur_code 的最终邻居集合 R 更新到 graph 中
        UpdateGraph(nndescent, cur_code.id, R);
        
        for(const auto& neighbor : R){
            if(!visited[neighbor.id]){
                candidates.push(neighbor);
            }
        }

    }
    
    // Step 5: 确保所有点连接到了图中
    while (true) {
        // 检查是否所有节点都链接到了主图
        bool all_connected = true;
        for (int i = 0; i < num_points; ++i) {
            if (!visited[i]) {
                all_connected = false;
                break;
            }
        }
        if (all_connected) {
            break;
        }

        // 如果有未连接的节点，则将其与主图中的某个节点连接
        for (int i = 0; i < num_points; ++i) {
            if (!visited[i]) {
                // 找到最近的在主图中的邻居
                float min_dist = std::numeric_limits<float>::max();
                int nearest_neighbor = -1;
                for (int j = 0; j < num_points; ++j) {
                    if (visited[j]) {
                        float dist = L2Sqr(data[i].data(), data[j].data(), dim);
                        if (dist < min_dist) {
                            min_dist = dist;
                            nearest_neighbor = j;
                        }
                    }
                }

                // 连接未连接的节点和主图中的邻居
                if (nearest_neighbor != -1) {
                    visited[i] = 1;
                    candidates.push(Neighbor(i, min_dist, true));
                }
            }
        }
    }

    nndescent.Descent();
}






int main(){
    int64_t num_points = 0;
    int64_t dim =  0;
    int64_t max_lines =  10000;
    int K = 30;
    int L = 10;
    int m = 30;
    std::string path = "../glove.6B/glove.6B.50d.txt";

    // 1.读取 glove.6B.50d.txt 数据
    std::vector<std::vector<float>> data = ReadData(path ,num_points, dim , max_lines);
    std::cout << "Loaded " << num_points << " lines of dimension " << dim << std::endl;


    // 步骤2：KNN初始化
    std::vector<std::vector<int>> knn_graph = KNNInit(data, K);
    std::cout << "KNN graph initialized." << std::endl;

    for (int i = 0; i < 5; ++i) {
        std::cout << "Lines " << i + 1 << ": ";
        for (int j = 0; j < K; j++) {
            std::cout << knn_graph[i][j] << " ";
        }
        std::cout << std::endl;
    }
    BuildNSGIndex(knn_graph, data, K, L, m);
    std::cout << "NSG index built." << std::endl;

    return 0;
}