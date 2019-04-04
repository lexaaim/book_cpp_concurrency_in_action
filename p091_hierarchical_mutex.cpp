#include <mutex>
#include <exception>
#include <limits>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;

class HierarchicalMutex {
private:
    mutex  _internal_mutex;
    size_t _hier_value;
    size_t _prev_hier_value;
    static thread_local size_t _this_thread_hier_value;

    void check_for_hier_violation() {
        if (_this_thread_hier_value <= _hier_value) {
            throw logic_error("mutex hierarchy violated");
        }
    }

    void update_hier_value() {
        _prev_hier_value = _this_thread_hier_value;
        _this_thread_hier_value = _hier_value;
    }

public:
    explicit HierarchicalMutex(size_t value)
        : _hier_value{ value },
          _prev_hier_value{ 0 }
    { }

    void lock() {
        check_for_hier_violation();
        _internal_mutex.lock();
        update_hier_value();
    }

    void unlock() {
        _this_thread_hier_value = _prev_hier_value;
        _internal_mutex.unlock();
    }

    bool try_lock() {
        check_for_hier_violation();
        if (!_internal_mutex.try_lock()) { return false; }
        update_hier_value();
        return true;
    }
};

thread_local size_t HierarchicalMutex::_this_thread_hier_value{ numeric_limits<size_t>::max() };

HierarchicalMutex mutex20(20);
HierarchicalMutex mutex10(10);

void thread_routine1() {
    for (auto i = 0; i < 10; i++) {
        try {
            lock_guard<HierarchicalMutex> lock1(mutex20);
            lock_guard<HierarchicalMutex> lock2(mutex10);

            cout << "Thread 1 is doing something" << endl;
        }
        catch (const logic_error & er) {
            cout << "ERROR: " << er.what() << endl;
        }

        this_thread::sleep_for(100ms);
    }
}

void thread_routine2() {
    for (auto i = 0; i < 10; i++) {
        try {
            lock_guard<HierarchicalMutex> lock1(mutex10);
            lock_guard<HierarchicalMutex> lock2(mutex20);

            cout << "Thread 2 is doing something" << endl;
        }
        catch (const logic_error & er) {
            cout << "ERROR: " << er.what() << endl;
        }

        this_thread::sleep_for(100ms);
    }
}

int main() {
    thread t1{thread_routine1};
    thread t2{thread_routine2};

    t1.join();
    t2.join();
}
