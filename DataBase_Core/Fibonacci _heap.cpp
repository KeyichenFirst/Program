#include <cmath>
#include <vector>
#include <limits>
#include <stdexcept>

template <typename T>
class Node{
    public:
        T key;
        int degree;
        bool mark;
        Node<T>* parent;
        Node<T>* child;
        Node<T>* left;
        Node<T>* right;
        
        Node(T value):key (value),degree(0),mark(false),parent(nullptr),child(nullptr),left(this),right(this){
        };
};

template <typename T>
class FibonacciHeap {
    private:
        Node<T>* minNode;
        int nNodes;
    public:
        FibonacciHeap() : minNode(nullptr) ,nNodes(0){
        }

        ~FibonacciHeap(){
        }

        void insert(T key);
        Node<T>* findMin();
        void merge(FibonacciHeap& other);
        T extractMin();
        void decreaseKey(Node<T>* node,T newKey);
        void deleteNode(Node<T>* node);
};

// 1.插入元素
template <typename T>
void FibonacciHeap<T>::insert(T key){
    Node<T>* node = new Node<T>(key);
    if(minNode == nullptr){
        minNode = node;
    }else{
        minNode->left->right = node;
        node->left = minNode->left;
        node->right = minNode;
        minNode->left = node;
        if( key < minNode->key){
            minNode = key;
        }
    }
    nNodes ++;
}

// 2.查找最小值
// template <typename T>
// Node<T>* FibonacciHeap<T>::findMin() {
//     return minNode;
// }

// 3.合并堆
template <typename T>
void FibonacciHeap<T>::merge(FibonacciHeap& other){
    if(other.minNode == nullptr){
        return;
    }
    if(minNode == nullptr){
        minNode = other.minNode;
        nNodes = other.nNodes;
    }else{
        Node<T>* temp = minNode->right;
        minNode->right = other.minNode->right;
        other.minNode->right->left = minNode;
        other.minNode->rigth = temp;
        if(other.minNode->key < minNode->key){
            minNode = other.minNode;
        }
        nNodes += other.nNode;
    }
    other.minNode = nullptr;
    other.nNodes = 0;
}

// 4.删除最小元素
template <typename T>
T FibonacciHeap<T>::extractMin(){
    Node<T>* z = minNode;
    if(z != nullptr){
        if(z->child != nullptr){
            Node<T>* child = z->child;
            do{
                Node<T>* next = child->right;
                minNode->left->right =  child;
                child->left = minNode->left;
                child->right = minNode;
                minNode->left = child;

                child->parent = nullptr;
                child = next;
            }while(child != z->child);
        }
        // 从列表中删除z
        z->left->right = z->right;
        z->right->left = z->left;
        if(z == z->right){
            minNode = nullptr;
        }else {
            minNode = z->right;
            consolidate();
        }
        nNodes --;
    }
    T minKey = z->key;
    delete z;
    return minKey;
}

// 合并操作
template <typename T>
void FibonacciHeap<T>::consolidate() {
    int maxDegree = static _cast<int>(std::log2(nNodes)) + 1;
    std::vector<Node<T>*> A(maxDegree, nullptr);

    std::vector<Node<T>*> rootlist;
    Node<T>* curr = minNode;
    if(curr != nullptr){
        do{
            rootlist.push_back(curr);
            curr = curr->right;
        }while(curr != minNode);
    }

    for(Node<T>* w : rootlist){
        Node<T>* x = w;
        ind d = x->degree;
        while(A[d] != nullptr) {
            Node<T>* y = A[d];
            if( x->key > y->key){
                std::swap(x,y);
            }
            link(y,x);
            A[d] = nullptr;
            d ++;
        }
        A[d] = x;
    }

    minNode = nullptr;
    for(Node<T>* node : A){
        if(node != nullptr){
            if(minNode == nullptr){
                node->left = node->right = node;
                minNode = node;
            }else{
                minNode->left->right = node;
                node->left = minNode->left;
                node->right = minNode;
                minNode->left = node;

                if(node->key < minNode->key){
                    minNode = node;
                }
            }
        }
    }
}

// 链接两颗树
template <typename T>
void FibonacciHeap<T>::link(Node<T>* y, Node<T>* x){
    // 移除 y
    y->left->right = y->right;
    y->right->left = y->left;
    // 将 y 作为x 的子节点
    y->parent = x;
    if(x->child == nullptr){
        x->child = y;
        y->left = y->right = y;
    }else{
        y->left = x->child;
        y->right = x->child->right;
        x->child->right->left = y;
        x->child->right = y;
    }
    x->degree ++;
    y->mark = false;
}

// 减少节点的值

// 删除节点
template <typename T>
void FibonacciHeap<T>::deleteNode(Node<T>* x){
    decreaseKey(x,std::numeric_limits<T>::min());
    extractMin();
}