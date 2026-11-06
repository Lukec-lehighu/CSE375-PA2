#pragma once

#include <cstdlib>
#include <vector>
#include <memory>
#include <mutex>

#include <iostream>

#define K 5000      // size of the tables
#define N 100       // random number of elements to initially populate the array with
#define LIMIT 15

// for concurrent hash sets specifically
#define PROBE_SIZE 4
#define THRESHOLD 2

template<typename T>
class HashSet {
public:
    HashSet() {}

    virtual bool add(T item) = 0;
    virtual bool remove(T item) = 0;
    virtual bool contains(T &item) = 0;

    virtual size_t size() = 0;
    void populate();
protected:
    size_t hash0(T i, size_t size) {
        return std::hash<T>{}(i) % (size+5) % size;
    }

    size_t hash1(T i, size_t size) {
        return std::hash<T>{}(i) % (size+11) % size;
    }
private:
    virtual void resize() = 0;
};

template<typename T>
class SequentialHashSet : public HashSet<T> {
private:
    std::vector<std::shared_ptr<T>> table0;
    std::vector<std::shared_ptr<T>> table1;

    void resize() {
        auto oldTable0 = table0;
        auto oldTable1 = table1;

        table0 = std::vector<std::shared_ptr<T>>(table0.size() * 2, nullptr);
        table1 = std::vector<std::shared_ptr<T>>(table1.size() * 2, nullptr);

        for(auto &i : oldTable0) {
            if (i != nullptr)
                add(*i);
        }
        for(auto &i : oldTable1) {
            if (i != nullptr)
                add(*i);
        }
    }
public:
    SequentialHashSet() : table0(std::vector<std::shared_ptr<T>>(K, nullptr)), 
                        table1(std::vector<std::shared_ptr<T>>(K, nullptr))
    {}

    bool add(T item) {
        if(contains(item))
            return false;

        auto tmp = std::make_shared<T>(item);
        for(int i=0; i<LIMIT; i++) {
            std::swap(table0[this->hash0(item, table0.size())], tmp);
            if(tmp == nullptr)
                return true;

            std::swap(table1[this->hash1(item, table1.size())], tmp);
            if(tmp == nullptr)
                return true;
        }

        std::cout << "REsizeing" << std::endl;
        resize();
        return add(item);
    }

    bool remove(T item) {
        size_t hashVal = this->hash0(item, table0.size());
        if(table0[hashVal] != nullptr && *table0[hashVal] == item) {
            table0[hashVal] = nullptr;
            return true;
        }

        hashVal = this->hash1(item, table1.size());
        if(table1[hashVal] != nullptr && *table1[hashVal] == item) {
            table1[hashVal] = nullptr;
            return true;
        }

        return false;
    }

    bool contains(T &item) {
        // TODO: optimize this by only running hash once per table
        return (table0[this->hash0(item, table0.size())] != nullptr && *table0[this->hash0(item, table0.size())] == item)
            || (table1[this->hash1(item, table1.size())] != nullptr && *table1[this->hash1(item, table1.size())] == item);
    }

    // non-thread safe
    size_t size() {
        size_t count = 0;
        for(auto &i : table0) {
            if(i != nullptr)
                count++;
        }
        for(auto &i : table1) {
            if(i != nullptr)
                count++;
        }
        return count;
    }
};

template<typename T>
class PhasedCuckooHashSet : public HashSet<T> {
private:
    std::vector<std::vector<std::shared_ptr<T>>> table0;
    std::vector<std::vector<std::shared_ptr<T>>> table1;
    std::vector<std::mutex> lock0;
    std::vector<std::mutex> lock1;

    void acquire(T &x) {
        lock0[this->hash0(x, lock0.size()) % lock0.size()].lock();
        lock1[this->hash1(x, lock1.size()) % lock1.size()].lock();
    }
    void release(T &x) {
        lock0[this->hash0(x) % lock0.size()].unlock();
        lock1[this->hash1(x) % lock1.size()].unlock();
    }

    void resize() {

    }
public:
    PhasedCuckooHashSet() : lock0(std::vector<std::mutex>(K)), lock1(std::vector<std::mutex>(K)) {
        for(size_t i=0; i<K; i++) {
            table0.push_back(std::vector<std::shared_ptr<T>>(PROBE_SIZE, nullptr));
            table1.push_back(std::vector<std::shared_ptr<T>>(PROBE_SIZE, nullptr));
        }
    }

    bool add(T item) {
        acquire(item);

        return false;
    }

    bool remove(T item) {
        return false;
    }

    bool contains(T &item) {
        return false;
    }

    size_t size() {
        return 0;
    }
};

// template<typename T>
// class TransactionalMemoryHashSet : public HashSet<T> {
// public:
//     bool add(T item);
//     bool remove(T item);
//     bool contains(T &item);
// };

// Only have an integer implementation of this function
template<>
void HashSet<int>::populate() {
    srand(time(0));
    for(int i=0; i<N; i++) {
        add(rand());
    }
}