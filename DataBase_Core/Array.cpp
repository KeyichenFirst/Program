#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <iostream>     // 新增
#include <type_traits>  // 新增
#include <vector>

// const 调用的对象不可以更改
// std::size_t ： 非类型模板
template <typename T, std::size_t N>
class Array{
    private:
        T data[N];
    public:
        Array() = default;
        ~Array() = default;
        // 元素访问
        // 1.下标运算符（const)
        T& operator[](std::size_t index) {
            return data[index];
        }
        // 2.下标运算符
        const T& operator[](std::size_t index) const{
            return data[index];
        }
        // 3.at：带范围检查
        T& at(std::size_t index){
            if(index >= N){
                throw std::out_of_range("超出数组界限");
            }
            return data[index];
        }
        // 4.
        const T& at(std::size_t index) const {
            if(index >= N){
                throw std::out_of_range("超出数组界限");
            }
            return data[index];
        }
        // 获取数组大小
        constexpr std::size_t size() const noexcept{
            return N;
        }
        // 前后元素的访问
        T& front(){
            return data[0];
        }
        const T& front() const {
            return data[0];
        }
        T& back(){
            return data[N - 1];
        }
        const T& back() const {
            return data[N - 1];
        }
        // 返回数组首元素指针
        T* _data() noexcept{
            return data;
        }
        const T* _data() const noexcept{
            return data;    
        }

        // 迭代器生成
        using iterator = T*;
        using const_iterator = const T*;
        
        // begin() 和 end()
        iterator begin() noexcept{
            return data;
        }
        const_iterator begin() const noexcept{
            return data;
        }
        const_iterator cbegin() const noexcept{
            return data;
        }
        iterator end() noexcept{
            return data + N;
        }
        const_iterator end() const noexcept{
            return data + N;
        }
        const_iterator cend() const noexcept{
            return data + N;
        }
        // 填充和交换
        void fill(const T& value){
            std::fill(begin(),end(),value);
        }
        // 交换两个Array的内容
        void swap(Array& other) noexcept(std::is_nothrow_swappable<T>::value){
            for(std::size_t i = 0; i < N ; ++i){
                std::swap(data[i],other.data[i]);
            }
        }
        // 比较运算符
        bool operator==(const Array& other) const{
            return std::equal(begin(),end(),other.begin());
        }
        bool operator!=(const Array& other) const{
            return !(*this == other);
        }
        bool operator<(const Array& other) const{
            return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
        }

        bool operator>(const Array& other) const {
            return other < *this;
        }
        bool operator<=(const Array& other) const{
            return !(*this < other);
        }
        bool operator>=(const Array& other) const {
            return !(*this < other);
        }
};


int main(){
    Array<int,5> arr;
    arr.fill(10);
    arr[0] = 1;
    arr.at(1) = 2;
    for(auto x : arr){
        std::cout<< x << " ";
    }
    std::cout << std::endl;
    std::cout <<"using iterator" << std::endl;
    for(auto it = arr.begin(); it != arr.end(); ++ it){
        std::cout << *it << std::endl;
    }
    // 访问前后元素
    std::cout << "Front element: " << arr.front() << std::endl;
    std::cout << "Back element: " << arr.back() << std::endl;

    // 比较数组
    Array<int, 5> arr2;
    arr2.fill(10);
    arr2[0] = 1;
    arr2[1] = 2;

    if (arr == arr2) {
        std::cout << "arr and arr2 are equal." << std::endl;
    } else {
        std::cout << "arr and arr2 are not equal." << std::endl;
    }

    return 0;
}