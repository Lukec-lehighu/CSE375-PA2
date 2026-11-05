#include <iostream>

#include "hashset.hpp"

using namespace std;

int main() {
    SequentialHashSet<int> test;

    test.add(4);
    test.add(5);
    test.add(6);

    cout << test.size() << endl;
    return 0;
}