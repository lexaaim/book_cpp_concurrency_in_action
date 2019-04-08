#include <memory>
#include <cassert>
#include <iostream>

using namespace std;

template <typename T>
class Queue {
private:
    struct Node {
        T                _data;
        unique_ptr<Node> _next;

        Node (T data) : _data(move(data)) { }
    };

    unique_ptr<Node> _head;
    Node *           _tail;

public:
    Queue() : _head{ nullptr }, _tail{ nullptr } { }

    Queue(const Queue &) = delete;
    Queue & operator=(const Queue &) = delete;

    shared_ptr<T> try_pop() {
        if (!_head) { return shared_ptr<T>(); }

        const shared_ptr<T> result{ make_shared<T>(move(_head->_data)) };
        const unique_ptr<Node> old_head = move(_head);
        _head = move(old_head->_next);
        return result;
    }

    void push(T new_value) {
        unique_ptr<Node> p{ new Node(move(new_value)) };
        Node * const new_tail = p.get();

        if (_tail) { _tail->_next = move(p); }
        else       { _head = move(p); }

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
