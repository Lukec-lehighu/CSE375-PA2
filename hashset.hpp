#pragma once

#include <cstdlib>
#include <vector>
#include <memory>
#include <iostream>

#define K 5000      // size of the tables
#define N 100       // random number of elements to initially populate the array with
#define LIMIT 15

template<typename T>
class HashSet {
public:
    HashSet() {}

    virtual bool add(T item) = 0;
    virtual bool remove(T item) = 0;
    virtual bool contains(T &item) = 0;

    void populate();
protected:    
    size_t hash0(T i) {
        return std::hash<T>{}(i) % (K+5) % K;
    }

    size_t hash1(T i) {
        return std::hash<T>{}(i) % (K+11) % K;
    }
};

template<typename T>
class SequentialHashSet : public HashSet<T> {
private:
    std::vector<std::shared_ptr<T>> table0;
    std::vector<std::shared_ptr<T>> table1;
public:
    SequentialHashSet() : table0(std::vector<std::shared_ptr<T>>(K, nullptr)), 
                            table1(std::vector<std::shared_ptr<T>>(K, nullptr))
    {}

    bool add(T item) {
        if(contains(item))
            return false;

        auto tmp = std::make_shared<T>(item);
        for(int i=0; i<LIMIT; i++) {
            std::swap(table0[this->hash0(item)], tmp);
            if(tmp == nullptr)
                return true;

            std::swap(table1[this->hash1(item)], tmp);
            if(tmp == nullptr)
                return true;
        }

        return false;
    }

    bool remove(T item) {
        size_t hashVal = this->hash0(item);
        if(table0[hashVal] != nullptr && *table0[hashVal] == item) {
            table0[hashVal] = nullptr;
            return true;
        }

        hashVal = this->hash1(item);
        if(table1[hashVal] != nullptr && *table1[hashVal] == item) {
            table1[hashVal] = nullptr;
            return true;
        }

        return false;
    }

    bool contains(T &item) {
        return (table0[this->hash0(item)] != nullptr && *table0[this->hash0(item)] == item)
            || (table1[this->hash1(item)] != nullptr && *table1[this->hash1(item)] == item);
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

// template<typename T>
// class FixedLocksHashSet : public HashSet<T> {
// public:
//     bool add(T item);
//     bool remove(T item);
//     bool contains(T &item);
// };

// template<typename T>
// class TransactionalMemoryHashSet : public HashSet<T> {
// public:
//     bool add(T item);
//     bool remove(T item);
//     bool contains(T &item);
// };

template<>
void HashSet<int>::populate() {
    srand(time(0));
    for(int i=0; i<N; i++) {
        add(rand());
    }
}