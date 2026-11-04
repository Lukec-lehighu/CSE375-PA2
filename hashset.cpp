#include "hashset.hpp"

using namespace std;

template<typename T>
HashSet<T>::HashSet() : 
    table0(vector<shared_ptr<HashNode<T>>>(k, nullptr)), 
    table1(vector<shared_ptr<HashNode<T>>>(k, nullptr))
{}

template<typename T>
int HashSet<T>::hash0(int i) {
    return i % k;
}

template<typename T>
int HashSet<T>::hash1(int i) {
    return i % k;
}

template<typename T>
bool SequentialHashSet<T>::add(HashNode<T> node) {

}