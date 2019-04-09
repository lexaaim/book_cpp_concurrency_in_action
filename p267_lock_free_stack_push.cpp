#include <atomic>

using namespace std;

template <typename T>
class Stack {
private:
    struct Node {
        T      _data;
        Node * _next;
        Node(const T & data) : _data(data) { }
    };

    atomic<Node * > _head;

public:
    void push(const T & data) {
        Node * const new_node = new Node(data);

        // записываем в next текущее значение _head
        new_node->_next = _head.load();

        // записать в _head указатель на новый узел:
        // compare_exchange_weak проверяет, что new_node->_next == _head.load()
        // и если это так - присваивает _head новое значение.
        // Если же это не так, она записываеь в new_node->next новое значение _head,
        // а цикл проверки-присваивания повторяется
        while(!_head.compare_exchange_weak(new_node->_next, new_node));
    }
};

Stack<int> st;

int main() {
}
