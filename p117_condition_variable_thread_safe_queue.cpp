#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <iostream>
#include <random>
#include <memory>

using namespace std;

template <typename T>
class ThreadSafeQueue {
private:
    queue<T>                   _data_queue;
    mutable mutex              _data_mutex;
    mutable condition_variable _data_cond;

public:
    void push(T new_value) {
        lock_guard lock(_data_mutex);
        _data_queue.push(new_value);
        _data_cond.notify_one();
    }

    void wait_and_pop(T & value) {
        unique_lock ulock(_data_mutex);
        _data_cond.wait(ulock, [this]{ return !_data_queue.empty(); });
        value = _data_queue.front();
        _data_queue.pop();
    }

    shared_ptr<T> wait_and_pop() {
        unique_lock ulock(_data_mutex);
        _data_cond.wait(ulock, [this]{ return !_data_queue.empty(); });
        auto result = make_shared<T>(_data_queue.front());
        _data_queue.pop();
        return result;
    }

    bool try_pop(T & value) {
        unique_lock ulock(_data_mutex);
        if (_data_queue.empty()) { return false; }

        value = _data_queue.front();
        _data_queue.pop();
        return true;
    }

    bool empty() const {
        unique_lock lock(_data_mutex);
        return _data_queue.empty();
    }
};

ThreadSafeQueue<int> tsq;
mutex cout_mutex;

void writer_routine() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, 100);

    while (true) {
        this_thread::sleep_for(200ms);
        {
            int value = distr(gen);
            tsq.push(value);
            {
                unique_lock lock(cout_mutex);
                cout << "WRITE: " << value << endl;
            }
        }
    }
}

void reader_routine() {
    while (true) {
        int value;
        tsq.wait_and_pop(value);
        {
            unique_lock lock(cout_mutex);
            cout << "READ: " << value << endl;
        }
        this_thread::sleep_for(100ms);
    }
}

int main() {
    thread writer1{ writer_routine };
    this_thread::sleep_for(100ms);
    thread writer2{ writer_routine };
    this_thread::sleep_for(100ms);
    thread writer3{ writer_routine };

    thread reader1{ reader_routine };

    writer1.join();
    writer2.join();
    writer3.join();
    reader1.join();
}
