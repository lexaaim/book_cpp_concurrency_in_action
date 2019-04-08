#include <memory>
#include <cassert>
#include <iostream>
#include <mutex>
#include <thread>
#include <random>

using namespace std;

template <typename T>
class Queue {
private:
    struct Node {
        shared_ptr<T>    _data;
        unique_ptr<Node> _next;

        Node() = default;
        explicit Node (T data) : _data(move(data)) { }
    };

    unique_ptr<Node> _head;
    Node *           _tail;
    mutex            _head_mutex;
    mutex            _tail_mutex;

    Node * get_tail() {
        lock_guard<mutex> tail_lock(_tail_mutex);
        return _tail;
    }

    unique_ptr<Node> pop_head() {
        lock_guard<mutex> head_lock(_head_mutex);
        if (_head.get() == get_tail()) { return nullptr; }

        unique_ptr<Node> old_head = move(_head);
        _head = move(old_head->_next);
        return old_head;
    }

public:
    Queue() : _head{ new Node }, _tail{ _head.get() } { }

    Queue(const Queue &) = delete;
    Queue & operator=(const Queue &) = delete;

    shared_ptr<T> try_pop() {
        unique_ptr<Node> old_head = pop_head();
        return old_head ? old_head->_data : make_shared<T>();
        /* return old_head ? old_head->_data : nullptr; */
    }

    void push(T new_value) {
        shared_ptr<T> new_data{ make_shared<T>(move(new_value)) };
        unique_ptr<Node> p{ new Node };
        Node * const new_tail = p.get();

        lock_guard<mutex> tail_lock(_tail_mutex);
        _tail->_data = new_data;
        _tail->_next = move(p);
        _tail = new_tail;
    }
};

Queue<int> tsq;
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
        if (auto pval = tsq.try_pop();
            pval.get()) {
            unique_lock lock(cout_mutex);
            cout << "READ: " << *pval << endl;
        }
        this_thread::sleep_for(50ms);
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

