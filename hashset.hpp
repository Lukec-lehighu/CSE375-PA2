#pragma once

#include <cstdlib>
#include <vector>
#include <memory>

#define k 5000  // size of the tables
#define n 100   // random number of elements to initially populate the array with

template<typename T>
struct HashNode {
    int key;
    T val;
};

template<typename T>
class HashSet {
public:
    HashSet();

    virtual bool add(HashNode<T> node) = 0;
    virtual bool remove(int key) = 0;
    virtual bool contains(HashNode<T>& node) = 0;

    // non-thread safe
    int size();
    void populate();
private:
    std::vector<std::shared_ptr<HashNode<T>>> table0;
    std::vector<std::shared_ptr<HashNode<T>>> table1;
    
    int hash0(int i);
    int hash1(int i);
};

template<typename T>
class SequentialHashSet : public HashSet<T> {
public:
    bool add(HashNode<T> node);
    bool remove(int key);
    bool contains(HashNode<T>& node);
};

template<typename T>
class FixedLocksHashSet : public HashSet<T> {
public:
    bool add(HashNode<T> node);
    bool remove(int key);
    bool contains(HashNode<T>& node);
};

template<typename T>
class TransactionalMemoryHashSet : public HashSet<T> {
public:
    bool add(HashNode<T> node);
    bool remove(int key);
    bool contains(HashNode<T>& node);
};