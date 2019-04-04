#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

class X {
private:
    string _data;
    mutex  _mutex;

public:
    X(const string & str) : _data{str} { }

    const string & data() const { return _data; }

    friend void swap(X & lhs, X & rhs) {
        // if lhs == rhs, we lock the same mutex twice and get UB
        if (&lhs == &rhs) return;

        // create unique lock guards (for the case of an exception in swap method) parametrized with
        // defer_lock (do not acquire ownership of the mutex)
        unique_lock<mutex> lock_lhs(lhs._mutex, defer_lock);
        unique_lock<mutex> lock_rhs(rhs._mutex, defer_lock);

        //lock both mutexes in a certain order:
        lock(lock_lhs, lock_rhs);

        // perform an unsafe operations
        lhs._data.swap(rhs._data);
    }
};

mutex coutmutex;
X xcommon{ "common X" };

void thread_routine1() {
    X x1{ "X from thread 1" };

    for (auto i = 0; i < 10; i++) {
        swap(x1, xcommon);
        {
            lock_guard<mutex> lock(coutmutex);
            cout << "thread_id: " << this_thread::get_id() << ", X: " << x1.data() << endl;
        }
        this_thread::sleep_for(100ms);
    }
}

void thread_routine2() {
    X x2{ "X from thread 2" };

    for (auto i = 0; i < 10; i++) {
        swap(x2, xcommon);
        {
            lock_guard<mutex> lock(coutmutex);
            cout << "thread_id: " << this_thread::get_id() << ", X: " << x2.data() << endl;
        }
        this_thread::sleep_for(100ms);
    }
}

int main() {
    thread t1{thread_routine1};
    thread t2{thread_routine2};

    t1.join();
    t2.join();

    cout << "OK!" << endl;
}
