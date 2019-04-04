#include <stack>
#include <iostream>
#include <exception>
#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <functional>
#include <algorithm>

using namespace std;

struct EmptyStackException : std::exception {
    const char * what() const throw() {
        return "stack is empty";
    }
};

template <typename T>
class ThreadSafeStack {
private:
    stack<T> _stack;
    mutable mutex _mutex;

public:
    ThreadSafeStack() = default;
    ThreadSafeStack(const ThreadSafeStack & other) {
        lock_guard<mutex> lock(other._mutex);
        _stack = other._stack;
    }

    ThreadSafeStack & operator=(const ThreadSafeStack &) = delete;

    void push(T new_value) {
        lock_guard<mutex> lock(_mutex);
        _stack.push(new_value);

        cout << "PUSH " << new_value << endl;
    }

    shared_ptr<T> pop() {
        lock_guard<mutex> lock(_mutex);
        if (_stack.empty()) { throw EmptyStackException(); }

        const auto result = make_shared<T>(_stack.top());
        _stack.pop();
        cout << "POP  " << *result << endl;

        return result;
    }

    void pop(T & value) {
        lock_guard<mutex> lock(_mutex);
        if (_stack.empty()) { throw EmptyStackException(); }

        value = _stack.top();
        _stack.pop();
        cout << "POP  " << value << endl;
    }

    bool empty() {
        lock_guard<mutex> lock(_mutex);
        return _stack.empty();
    }
};

ThreadSafeStack<int> tss;

void thread_routine() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(1, 9);

    auto max = distr(gen);
    for (auto i = 0; i < max; i++) {
        int value = distr(gen);
        tss.push(value);

        this_thread::sleep_for(chrono::milliseconds(value));
    }

    max = distr(gen);
    for (auto i = 0; i < max; i++) {
        try {
        //    tss.pop(value);
            auto result = tss.pop();
            /* int value = *result; */
        }
        catch (const EmptyStackException &) {
            cout << "Exception was caught" << endl;
        }
        this_thread::sleep_for(10ms);
    }
}

int main() {
    vector<thread> threads;

    auto tcount = max(thread::hardware_concurrency() - 1, 2u);
    for (size_t i = 0; i < tcount; ++i) {
        threads.push_back(thread{thread_routine});
    }

    for_each (begin(threads), end(threads), mem_fn(&thread::detach));
    thread_routine();
    this_thread::sleep_for(100ms);
}
