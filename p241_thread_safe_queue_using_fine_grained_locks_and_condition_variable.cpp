#include <memory>
#include <mutex>
#include <condition_variable>
#include <random>
#include <iostream>
#include <thread>

using namespace std;

template <typename T>
class Queue {
private:
    struct Node {
        shared_ptr<T>    _data;
        unique_ptr<Node> _next;
    };

    mutex              _head_mutex;
    mutex              _tail_mutex;
    condition_variable _data_cond;

    unique_ptr<Node>   _head;
    Node *             _tail;

    Node * get_tail() {
        lock_guard<mutex> tail_lock(_tail_mutex);
        return _tail;
    }

    // этот метод всегда должен выполняться с залоченным _head_mutex
    unique_ptr<Node> pop_head() {
        unique_ptr<Node> old_head = move(_head);
        _head = move(old_head->_next);

        return old_head;
    }

    unique_lock<mutex> wait_for_data() {
        unique_lock<mutex> head_lock(_head_mutex);
        _data_cond.wait(head_lock, [&]{ return _head.get() != get_tail(); });

        // вернуть объект блокировки c захваченным мьютексом
        return move(head_lock);
    }

    unique_ptr<Node> wait_pop_head() {
        unique_lock<mutex> head_lock(wait_for_data());

        return pop_head();
    }

    unique_ptr<Node> wait_pop_head(T & value) {
        unique_lock<mutex> head_lock(wait_for_data());

        // при копировании (перемещении) может возникнуть исключение.
        // если сначала вытолкнуть head, данные будут невосстановимы
        value = move(*_head->_data);

        return pop_head();
    }

    unique_ptr<Node> try_pop_head() {
        lock_guard<mutex> head_lock(_head_mutex);
        if (_head.get() == get_tail()) {
            return unique_ptr<Node>();
        }

        return pop_head();
    }

    unique_ptr<Node> try_pop_head(T & value) {
        lock_guard<mutex> head_lock(_head_mutex);

        if (_head.get() == get_tail()) {
            return unique_ptr<Node>();
        }
        value = move(*_head->_data);

        return pop_head();
    }

public:
    Queue() : _head{ new Node }, _tail{ _head.get() } { }

    Queue(const Queue &) = delete;
    Queue & operator=(const Queue &) = delete;

    shared_ptr<T> try_pop() {
        unique_ptr<Node> old_head = try_pop_head();
        return old_head ? old_head->_data : shared_ptr<T>();
    }

    bool try_pop(T & value) {
        const unique_ptr<Node> old_head = try_pop_head(value);
        return old_head;
    }

    shared_ptr<T> wait_and_pop() {
        const unique_ptr<Node> old_head = wait_pop_head();
        return old_head->_data;
    }

    void wait_and_pop(T & value) {
        const unique_ptr<Node> old_head = wait_pop_head(value);
    }

    void push(T new_value) {
        shared_ptr<T> new_data{ make_shared<T>(move(new_value)) };
        unique_ptr<Node> p{ new Node };

        {
            lock_guard<mutex> tail_lock(_tail_mutex);
            _tail->_data = new_data;

            Node * const new_tail = p.get();
            _tail->_next = move(p);
            _tail = new_tail;
        }
        // освобождем _tail_mutex чтобы сэкономить время потока,
        // который проснется, на его (мьютекса) захват
        _data_cond.notify_one();
    }

    void empty() {
        lock_guard<mutex> head_lock(_head_mutex);
        return (_head.get() == get_tail());
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
