#include<iostream>
#include<vector>

template <class T>
// Unique_Ptr ：确保只有一个对象可以独占资源
class Unique_Ptr{
    private:
        T* ptr; //原始指针
    public:
        // 1.默认构造函数
        // 初始化函数内部指针 
        Unique_Ptr():ptr(nullptr){}
        // 2.带参数的构造函数
        // 接受一个原始的指针
        // explicit : 避免隐式转化
        explicit Unique_Ptr(T* p): ptr(p){}
        // 3.析构函数
        // 释放函数的内存
        ~Unique_Ptr(){
            delete ptr;
        }
        // 4.拷贝函数
        // const 函数内部无法修改other 的值
        Unique_Ptr(const Unique_Ptr& other) = delete;
        // 5.拷贝赋值函数
        Unique_Ptr &operator=(const Unique_Ptr& other) = delete;
        // 6.移动构造函数
        // &&：右值，左值可以通过std::move()转化为右值
        Unique_Ptr(Unique_Ptr&& other) noexcept : ptr(other.ptr){
            other.ptr = nullptr;
        }
        // 7.移动赋值函数
        Unique_Ptr& operator=(Unique_Ptr&& other) noexcept{
            if(this!= &other){
                delete ptr;
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }
        // 重载操作符
        // 1.解引用操作符
        T& operator*() const {
            return *ptr;
        }
        // 2.成员访问操作符
        T* operator->() const{
            return ptr;
        }
        // 3.布尔类型转化
        explicit operator bool() const {
            return ptr != nullptr;
        }
        // 辅助函数
        // 1.获取原始指针
        T* get() const{
            return ptr;
        }
        // 2.释放所有权
        T* release(){
            T* temp = ptr;
            ptr = nullptr;
            return temp;
        }
        // 重置指针
        void reset(T* p = nullptr){
            if(ptr != p){
                delete ptr;
                ptr = p;
            }
        }

};

struct Test{
    void greet(){
        std::cout << "hello,1111" << std::endl;
    }
};

int main(){
    Unique_Ptr<Test> x1(new Test());
    
    if(x1){
        x1->greet();
    }

    Unique_Ptr<Test> x2 = std::move(x1);

    if(!x1){
        std::cout << "x1 is deleted" << std::endl;
    }
    
    if(x2){
        x2->greet();
    }

    x2.reset(new Test());
    x2->greet();

    x2.reset();
    if(!x2){
        std::cout << "x2 is deleted" << std::endl;
    }
    return 0;
}