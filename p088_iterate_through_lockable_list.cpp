#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <functional>

using namespace std;

template <typename T>
class LockableForwardList;

template <typename T>
class LockableNode {
private:
    LockableNode<T> * _next = nullptr;
    T     _data;
    mutex _mutex;

public:
    LockableNode(const T & data = T()) : _data{data}, _mutex{} { };

    void lock()     { _mutex.lock(); }
    void unlock()   { _mutex.unlock(); }
    T data() const { return _data; }
    const LockableNode<T> * const next() const { return _next; }
    LockableNode<T> * & next() { return _next; }
};

template <typename T>
class LockableForwardList {
private:
    LockableNode<T> * _front = nullptr;

public:
    LockableForwardList() = default;

    void push_back(const T & new_value) {
        auto newp = new LockableNode<T>(new_value);
        if (_front == nullptr) {
            _front = newp;
            return;
        }

        auto it = _front;
        while (it->next() != nullptr) {
            it = it->next();
        }
        it->next() = newp;
    }

    template <typename Function>
    void for_each(Function f) {
        if (_front == nullptr) { return; }

        _front->lock();
        auto it = _front;
        while (it != nullptr) {
            f(it->data());

            // lock the next node
            if (it->next() != nullptr) {
                it->next()->lock();
            }

            // we can read the next pointer only when the current node is locked
            // but in the end we must unlock current node
            auto itcopy = it;
            it = it->next();
            itcopy->unlock();
        }
    }
};

LockableForwardList<int> lfl;
mutex coutmutex;

void print(int value) {
    {
        lock_guard<mutex> lock(coutmutex);
        cout << "thread: " << this_thread::get_id() << ", read " << value << endl;
    }
    this_thread::sleep_for(10ms);
}

void thread_routine() {
    for (auto i = 0; i < 1; i++) {
        lfl.for_each(print);
    }
}

int main() {

    for (auto i = 0; i < 10; i++) {
        lfl.push_back(i);
    }

    vector<thread> threads;
    for (auto i = 0; i < 7; i++) {
        threads.push_back(thread{thread_routine});
    }

    for_each (begin(threads), end(threads), mem_fn(&thread::join));
}
