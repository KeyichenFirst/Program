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
        // 贪心搜索
        float currentDist = distance(entrypoint->id , newnode->id , data);
        std::priority_queue<std::pair<float ,HNSWNode*>> candidates;
        candidates.push({-currentDist, entrypoint});

        while(!candidates.empty()){
            auto topCandidate = candidates.top();
            float dist = -topCandidate,first;
            HNSWNode* currentNode = topCandidate.second;

            candidates.pop();
            for(int neighborid : currentNode->neighbors[layer]){
                HNSWNode* neighborid = nodes[neighborid];
                float d = distance(neighborid->id, newnode->id, data);

                if(d < currentDist){
                    candidates.push({-d,neighbor});
                    currentDist = d;
                }
            }
        }
    }
}