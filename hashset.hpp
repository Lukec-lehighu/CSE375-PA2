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
    std::vector<std::vector<T>> table0;
    std::vector<std::vector<T>> table1;
    std::vector<std::mutex> lock0;
    std::vector<std::mutex> lock1;

    // helper method
    bool removeFrom(std::vector<T> &set, T item) {
        for(size_t i=0; i<set.size(); i++) {
            // if the set contains the item...
            if(set[i]==item) {
                // remove it
                set.erase(set.begin()+i);
                return true;
            }
        }
        return false;
    }

    void acquire(T &x) {
        lock0[this->hash0(x, lock0.size()) % lock0.size()].lock();
        lock1[this->hash1(x, lock1.size()) % lock1.size()].lock();
    }
    void release(T &x) {
        lock0[this->hash0(x, lock0.size()) % lock0.size()].unlock();
        lock1[this->hash1(x, lock1.size()) % lock1.size()].unlock();
    }

    void resize() {
        size_t oldCapacity = table0.size();
        for(auto &lock : lock0)
            lock.lock();

        
    }

    bool relocate(int i, size_t hi) {
        // copied directly from the book, with C++ translation
        int hj = 0;
        int j = 1-i;

        for(int round = 0; round < LIMIT; round++) {
            std::vector<T>* iSet = &(i==0?table0:table1)[hi];
            T y = iSet->at(0);
            switch(i) {
                case 0: hj = this->hash1(y, table1.size()); break;
                case 1: hj = this->hash0(y, table0.size()); break;
            }
            acquire(y);
            std::vector<T>* jSet = &(j==0?table0:table1)[hj];
            if(removeFrom(*jSet, y)) {
                if(jSet->size() < THRESHOLD) {
                    jSet->push_back(y);
                    release(y);
                    return true;
                } else if(jSet->size() < PROBE_SIZE) {
                    jSet->push_back(y);
                    i = 1-i;
                    hi = hj;
                    j = 1-j;
                } else {
                    iSet->push_back(y);
                    release(y);
                    return false;
                }
            } else if(iSet->size() >= THRESHOLD) {
                release(y);
                continue;
            } else {
                release(y);
                return true;
            }

            release(y);
        }

        return false;
    }
public:
    PhasedCuckooHashSet() : lock0(std::vector<std::mutex>(K)), lock1(std::vector<std::mutex>(K)) {
        for(size_t i=0; i<K; i++) {
            table0.push_back(std::vector<T>());
            table1.push_back(std::vector<T>());
        }
    }

    bool add(T item) {
        acquire(item);
        if(contains(item)) return false;

        size_t h0 = this->hash0(item, table0.size());
        size_t h1 = this->hash1(item, table1.size());
        size_t h = 0;
        int i = 0;

        bool mustResize = false;
        if(table0[h0].size() < THRESHOLD) {
            table0[h0].push_back(item);
            release(item);
            return true;
        } else if(table1[h1].size() < THRESHOLD) {
            table1[h1].push_back(item);
            release(item);
            return true;
        } else if (table0[h0].size() < PROBE_SIZE) {
            table0[h0].push_back(item);
            i = 0;
            h = h0;
        } else if (table1[h1].size() < PROBE_SIZE) {
            table1[h1].push_back(item);
            i = 1;
            h = h1;
        } else {
            mustResize = true;
        }

        release(item);

        if(mustResize) {
            resize(); add(item);
        } else if(!relocate(i, h)) {
            resize();
        }

        return true;
    }

    bool remove(T item) {
        acquire(item);

        size_t h0 = this->hash0(item, table0.size());
        if(removeFrom(table0[h0], item)) {
            release(item);
            return true;
        }

        size_t h1 = this->hash1(item, table0.size());
        if(removeFrom(table1[h1], item)) {
            release(item);
            return true;
        }

        release(item);
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