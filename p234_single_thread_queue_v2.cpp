#include <memory>
#include <cassert>
#include <iostream>

using namespace std;

template <typename T>
class Queue {
private:
    struct Node {
        shared_ptr<T>    _data;  // keeps shared_ptr to T
        unique_ptr<Node> _next;

        Node() = default;
        explicit Node (T data) : _data(move(data)) { }
    };

    unique_ptr<Node> _head;
    Node *           _tail;

public:
    Queue() : _head{ new Node }, _tail{ _head.get() } { }  // creates a dummy node

    Queue(const Queue &) = delete;
    Queue & operator=(const Queue &) = delete;

    shared_ptr<T> try_pop() {
        if (_head.get() == _tail) { return shared_ptr<T>(); }

        const shared_ptr<T> result{ _head->_data };
        const unique_ptr<Node> old_head = move(_head);
        _head = move(old_head->_next);
        return result;
    }

    void push(T new_value) {
        shared_ptr<T> new_data{ make_shared<T>(move(new_value)) };
        _tail->_data = new_data;

        unique_ptr<Node> p{ new Node };
        Node * const new_tail = p.get();

        _tail->_next = move(p);
        _tail = new_tail;
    }
};


int main() {
    Queue<int> q;

    q.push(1);
    q.push(2);

    auto v = q.try_pop();
    assert(*v == 1);
    v = q.try_pop();
    assert(*v == 2);
    v = q.try_pop();
    assert(v == nullptr);
}
