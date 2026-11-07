#include <iostream>
#include <functional>
#include <chrono>
#include <thread>
#include <random>

#include "hashset.hpp"

#define NUM_OPS 1000000

#define CONTAINS_PERCENT 80
#define INSERT_PERCENT 10
#define REMOVE_PERCENT 10

using namespace std;

int64_t timeExecution(function<void()> func) {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    func();
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    return chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}

// a thread safe rng
int safeRand() {
    static thread_local mt19937 generator;
    std::uniform_int_distribution<int> distribution(0,100000);
    return distribution(generator);
}

void TestMultiThreaded(HashSet<int>* set, int numThreads) {
    // for thread safe random generatino
    vector<thread> jobs;
    for(int t=0; t<numThreads; t++) {
        jobs.push_back(
            move(thread([&] {
                for(size_t i=0; i<NUM_OPS/numThreads; i++) {
                    int op = safeRand() % 100;
                    if(op < CONTAINS_PERCENT) {
                        auto tmp = safeRand();
                        // cout << "contains" << endl;
                        set->contains(tmp);
                    }
                    else if(op < CONTAINS_PERCENT + INSERT_PERCENT) {
                        // cout << "insert" << endl;
                        set->add(safeRand());
                    }
                    else if(op < CONTAINS_PERCENT + INSERT_PERCENT + REMOVE_PERCENT) {
                        // cout << "remove" << endl;
                        set->remove(safeRand());
                    }
                }
            }))
        );
    }

    for(int i=0; i<numThreads; i++) {
        jobs[i].join();
    }
}

template<typename T>
void TestSequential(SequentialHashSet<T> set) {
    for(size_t i=0; i<NUM_OPS; i++) {
        int op = rand() % 100;
        if(op < CONTAINS_PERCENT) {
            auto tmp = rand();
            set.contains(tmp);
        }
        else if(op < CONTAINS_PERCENT + INSERT_PERCENT)
            set.add(rand());
        else if(op < CONTAINS_PERCENT + INSERT_PERCENT + REMOVE_PERCENT)
            set.remove(rand());
    }
}

int main() {
    srand(time(0));
    SequentialHashSet<int>          seqHashSet;
    PhasedCuckooHashSet<int>        phasedHashSet;
    TransactionalMemoryHashSet<int> transactionalHashSet;

    HashSet<int>* castedPhase = static_cast<HashSet<int>*>(&phasedHashSet);
    HashSet<int>* castedTransactional = static_cast<HashSet<int>*>(&transactionalHashSet);

    cout << "Sequential time: " << timeExecution([&]{ TestSequential(seqHashSet); }) << " μs" << endl;

    for(int i=1; i<=16; i++) {
        cout << "Phased Cuckoo time (" << i << " threads): " << timeExecution([&]{ TestMultiThreaded(castedPhase, i); }) << " μs" << endl;
        cout << "Transactional time: (" << i << " threads): " << timeExecution([&]{ TestMultiThreaded(castedTransactional, i); }) << " μs" << endl;
    }

    return 0;
}