#include <iostream>

#include "hashset.hpp"

using namespace std;

int main() {
    SequentialHashSet<int> test;

    test.populate();

    cout << test.size() << endl;
    return 0;
}