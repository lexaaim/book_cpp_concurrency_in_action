#include <atomic>
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>

using namespace std;

class SpinlockMutex {
    atomic_flag _flag;
public:
    SpinlockMutex() : _flag(ATOMIC_FLAG_INIT) { }

    void lock() {
        while (_flag.test_and_set(memory_order_acquire)); // !! active waiting
    }

    void unlock() {
        _flag.clear(memory_order_release);
    }
};

SpinlockMutex coutmutex;

void thread_routine() {
    for (int i = 0; i < 10; i++) {
        {
            lock_guard<SpinlockMutex> lock(coutmutex);
            cout << "h" << "e" << "l" << "l" << "o" << " " << "w" << "d" << endl;
        }

        this_thread::sleep_for(10ms);
    }
}

int main () {
    thread t1{thread_routine};
    thread t2{thread_routine};
    thread t3{thread_routine};

    t1.join();
    t2.join();
    t3.join();
}
