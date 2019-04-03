#include <iostream>
#include <string>
#include <thread>
#include <mutex>

using namespace std;

class X {
private:
    string _data;
    mutex  _mutex;

public:
    X(const string & str) : _data{str} { }

    friend void swap(X & lhs, X & rhs) {
        // if lhs == rhs, we lock the same mutex twice and get UB
        if (&lhs == &rhs) return;

        //lock both mutexes in a certain order:
        lock(lhs._mutex, rhs._mutex);

        // create lock_guards (for the case of an exception in swap method) parametrized with
        // adopt_lock (assume the calling thread already has ownership of the mutex)
        lock_guard<mutex> lock_lhs(lhs._mutex, adopt_lock);
        lock_guard<mutex> lock_rhs(rhs._mutex, adopt_lock);

        // perform an unsafe operations
        lhs._data.swap(rhs._data);
    }
};

int main() {
}
