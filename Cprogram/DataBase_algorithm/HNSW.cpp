#include <iosteam>
#include <limits>
#include <queue>


class HNSWNode{
    public:
    int id;
    // 节点的最高等级
    int maxLevel;
    // 每层的邻居列表
    std::vector<std::vector<int>> neighbors;

    HNSWNode(int nodeid , int level) : id(nodeid),maxLevel(level){
        neighbors.resize(level + 1);
    }
}

// 定义HNSW的图类
class HNSW{
    private:
    // 存储所有的节点
    std::vector<HNSWNode*> nodes;
    // 每层最多的连接数
    int maxConnections;
    // 当前图的最高层数
    int maxlayer;
    // 用于生成层级的常数
    float someconstant;
    public:
    HNSW(int maxConn, float constant ) : maxConnections(maxConn), maxlayer(0),someconstant(constant){}

    int chooseRandomLevel(){
        double r = static_cast<double>(rand()) / RAND_MAX;
        int level = static_cast<int>(-log(r) * someconstant);
        return level;
    }

    void insertNode(int nodeid, const std::vector<float>& data){
        // 随机生成层数
        int level = chooseRandomLevel();
        HNSWNode* newnode = new HNSWNode(nodeid,level);

        nodes.push(newnode);

        if(level > maxlayer){
            maxlayer = level;
        }

        // 从最高层往下遍历，寻找位置
        for(int l = maxlayer; l >= 0; l --){
            if(l > level) continue;
            // 如果在最高层，使用一个随机节点开始搜索
            if(l == maxlayer && l > 0){
                HNSWNode* entryPoint = nodes[0];
                searchlayer(entryPoint,newnode,data, l);
            }else{
                // 从前一层找到的最近邻节点开始查找
                HNSWNode* entrypoint = newnode;
                searchlayer(entryPoint,newnode,data, l);
            }
            updateConnections(newNode, l);
        }
    }

    void searchlayer(HNSWNode* entrypoint , HNSWNode* newnode ,const std::vector<float>& data, int layer ){
        std::unordered_set<in> visited;
        std::priority_queue<std::pair<float,int>> candidates;
        std::priority_queue<std::pair<float,int >> closestNeighbors;

        float dist = distacne(entrypoint, -1, query);
        candidates.push({dist, entrypoint->id});
        closestNeighbors.push({dist , entrypoint->id});
        visited.insert(entrypoint->id);

        while(!candidates.empty()){
            auto topCandidate = candidates.top();
            float closestDist = -topCandidate.first;
            int currentNodeid = topCandidate.second;
            candidates.pop();
            
            // 如果候选节点的距离大于找到的最大距离
            float farthestDist = closestNeighbors.top().first;
            if(closestDist > farthestDist){
                break;
            }

            HNSWNode* currentnode = nodes[currentNodeid];
            for(int neighborid : currentnode->neighbors[layer]){
                if(visited.find(neighborid) != visited.end()){
                    continue;
                }
                visited.insert(neighborid);
                float d = distance(neighborid, -1, query);
                if(closestNeighbors.size() < ef || d < farthestDist){
                    candidates.push({-d, neighborid});
                    closestNeighbors({d,neighborid});
                    if(closestNeighbors.size() > ef){
                        closestNeighbors.pop();
                    }
                }
            }
        }
        std::vector<int> result;
        while( !closestNeighbors.empty()){
            result.push_back(closestNeighbors.top().second);
            closestNeighbors.pop();
        }
        return result;
    }

    void updateConnections(HNSWNode* newnode, int layer){
        // 确保邻接表的大小不超过最大值
        while(newnode->neighbors[layer].size() > maxConnections){
            newnode->neighbors[layer].pop();
        }
    }

    float distance(int id1, int id2 , const std::vector<float>& data){
        float sum =  0.0f;
        for(size_t i = 0 ; i < data.size() ; ++i){
            float diff = data[id1 * data.size() + i] - data[id2 * data.size() + i];
            sum += diff * diff;
        }
        return std::sqrt(sum);
    }

    std::vector<int> seatchKnn(const std::vector<float>& query , int k ){
        std::priority_queue<std::pair<float, int >> nearestNeighbors;
        for(HNSWNode* node: nodes){
            float dist = distance(node->id , -1, query);
            if(nearestNeighbors.size() < k){
                nearestNeighbors.push({dist,node->id});
            }else if(dist < nearestNeighbors.top().first()){
                nearestNeighbors.pop();
                nearestNeighbors.push({dist, node->id});
            }
        }

        std::vector<int> result;
        while(!nearestNeighbors.empty()){
            result.push_back(nearestNeighbors.top().second);
            nearestNeighbors.pop();
        }
        return result;
    }
}