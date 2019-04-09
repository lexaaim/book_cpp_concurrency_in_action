#include <memory>
#include <mutex>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;

template <typename T>
class List {
    struct Node {
        mutex            _mutex;
        shared_ptr<T>    _data;
        unique_ptr<Node> _next;

        Node() : _next{ nullptr } { }

        Node(const T & value) : _data{ make_shared<T>(value) } { }
    };

    Node _head;

public:
    List() { }

    ~List() {
        remove_if([](const Node &) { return true; });
    }

    List(const List &) = delete;
    List & operator=(const List &) = delete;

    void push_front(const T & value) {
        unique_ptr<Node> new_node{ new Node(value) };

        lock_guard<mutex> lock(_head._mutex);
        new_node->_next = move(_head._next);
        _head._next = move(new_node);
    }

    template <typename Function>
    void for_each(Function f) {
        Node * current = &_head;

        unique_lock<mutex> cur_lock(_head._mutex);
        while (Node * const next = current->_next.get()) {
            unique_lock<mutex> next_lock(next->_mutex);
            cur_lock.unlock();

            f(*next->_data);

            current = next;
            cur_lock = move(next_lock);
        }
    }

    template <typename Predicate>
    shared_ptr<T> find_first_if(Predicate p) {
        Node * current = &_head;
        unique_lock<mutex> cur_lock(_head._mutex);

        while (Node * const next = current->_next.get()) {
            unique_lock<mutex> next_lock(next->_mutex);
            cur_lock.unlock();

            if (p(*next->_data)) { return next->_data; }

            current = next;
            cur_lock = move(next_lock);
        }
        return shared_ptr<T>();
    }

    template <typename Predicate>
    void remove_if(Predicate p) {
        Node * current = &_head;
        unique_lock<mutex> cur_lock(_head._mutex);

        while (Node * const next = current->_next.get()) {
            unique_lock<mutex> next_lock(next->_mutex);

            if (p(*next->_data)) {
                unique_ptr<Node> old_next = move(current->_next);
                current->_next = move(next->_next);
                next_lock.unlock();
            } else {
                cur_lock.unlock();
                current  = next;
                cur_lock = move(next_lock);
            }
        }
    }
};

List<int> list;
mutex   coutmutex;

void thread_writer() {
    for (int i = 0; i < 30; i++) {
        list.push_front(i);
        {
            lock_guard lock(coutmutex);
            cout << "INSERT: " << i << endl;
        }
        this_thread::sleep_for(20ms);
    }
}

void thread_finder() {
    for (int i = 0; i < 30; i++) {
        list.find_first_if([&i](const int & item){ return item == i; });
        {
            lock_guard lock(coutmutex);
            cout << "SEARCH: " << i << endl;
        }
        this_thread::sleep_for(20ms);
    }
}

void thread_multiplier() {
    for (int i = 0; i < 3; i++) {
        this_thread::sleep_for(200ms);

        list.for_each([](int & item){
                /* { */
                /*     lock_guard lock(coutmutex); */
                /*     cout << "MULTIPLY: " << item << endl; */
                /* } */
                item *= 2;
            });
    }
}

int main() {
    thread t1{ thread_writer };
    thread t2{ thread_finder };
    thread t3{ thread_multiplier };

    t1.join();
    t2.join();
    t3.join();

    cout << "Result: ";
    list.for_each([](int & item){
        lock_guard lock(coutmutex);
        cout << item << ' ';
    });
    cout << endl;
}
