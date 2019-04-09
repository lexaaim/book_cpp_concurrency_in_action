#include <memory>
#include <atomic>

using namespace std;

template <typename T>
class Stack {
private:
    struct Node {
        // shared_ptr для обращения к данным, которые уже могли быть удалены другим потоком
        shared_ptr<T> _data;
        Node *        _next;

        Node(const T & data) : _data{ make_shared<T>(data) } { }
    };

    atomic<Node *>    _head;

public:
    void push(const T & data) {
        Node * const new_node = new Node(data);
        new_node->_next = _head.load();

        // записать в _head указатель на новый узел:
        // compare_exchange_weak проверяет, что new_node->_next == _head.load()
        // и если это так - присваивает _head новое значение.
        // Если же это не так, она записываеь в new_node->next новое значение _head,
        // а цикл проверки-присваивания повторяется
        while (!_head.compare_exchange_weak(new_node->_next, new_node));
    }

    shared_ptr<T> pop() {
        Node * old_head = _head.load();

        // безопасно прочитать данные можно только тогда, когда они уже
        // удалены из стека и ни один другой поток не может их прочитать/изменить,
        // поэтому сначала удаляем из стека.
        // Также old_head может оказаться равным нулю сразу или после обновления,
        // это необходимо проверять.
        while (old_head && !_head.compare_exchange_weak(old_head, old_head->_next));

        // Внимание, мы извлекли узел но не удалили его, а просто возвращаем данные из него.
        // Мы не можем удалить его сразу, т.к. другие потоки могут хранить данные,
        // ссылающиеся на него
        return old_head ? old_head->_data : shared_ptr<T>();
        // Утечка памяти
    }
};

Stack<int> st;

int main() {
}
