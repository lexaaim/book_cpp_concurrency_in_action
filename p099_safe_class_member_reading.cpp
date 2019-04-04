#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>

using namespace std;

class SomeClass {
private:
    int _detail;
    mutable mutex _mutex;

public:
    SomeClass(int value) : _detail(value) { }

    int detail() const {
        lock_guard<mutex> lock(_mutex);
        return _detail;
    }

};

bool operator==(const SomeClass & lhs, const SomeClass & rhs) {
    if (&lhs == &rhs) { return true; }

    // compares details, but not at the same moment
    return lhs.detail() == rhs.detail();
}

int main() {

}
