#include <memory>
#include <atomic>
#include <random>
#include <thread>
#include <mutex>
#include <iostream>

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

    atomic<Node *>   _head;
    atomic<unsigned> _threads_in_pop;
    atomic<Node *>   _to_be_deleted;

    static void delete_nodes(Node * nodes) {
        while (nodes) {
            Node * next = nodes->_next;
            delete nodes;
            nodes = next;
        }
    }

    void try_reclaim(Node * old_head) {
        // проверяем, что это единственный поток, который в pop()
        if (_threads_in_pop == 1) {
            // берем монопольные права на список узлов для удаления
            Node * nodes_to_delete = _to_be_deleted.exchange(nullptr);

            // если после того как мы взяли права никого нет в pop() - можно
            // спокойно удалять, иначе - обратно ложим
            if (!--_threads_in_pop) {
                delete_nodes(nodes_to_delete);
            } else if (nodes_to_delete) {
                chain_pending_nodes(nodes_to_delete);
            }

            delete old_head;
        } else {
            chain_pending_node(old_head);
            --_threads_in_pop;
        }
    }

    void chain_pending_nodes(Node * nodes) {
        Node * last = nodes;

        // проходим до конца цепочки
        while (Node * const next = last->_next) {
            last = next;
        }

        // добавляем ее к _to_be_deleted
        chain_pending_nodes(nodes, last);
    }

    void chain_pending_nodes(Node * first, Node * last) {
        last->_next = _to_be_deleted;

        while (!_to_be_deleted.compare_exchange_weak(last->_next, first));
    }

    void chain_pending_node(Node * n) {
        chain_pending_nodes(n, n);
    }

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
        // подсчитываем количество потоков внутри метода pop()
        ++_threads_in_pop;

        Node * old_head = _head.load();

        // безопасно прочитать данные можно только тогда, когда они уже
        // удалены из стека и ни один другой поток не может их прочитать/изменить,
        // поэтому сначала удаляем из стека.
        // Также old_head может оказаться равным нулю сразу или после обновления,
        // это необходимо проверять.
        while (old_head && !_head.compare_exchange_weak(old_head, old_head->_next));

        shared_ptr<T> result;
        if (old_head) {
            result.swap(old_head->_data);
        }

        // пытаемся удалить узлы, которые извлекли
        try_reclaim(old_head);
        return result;
    }
};

Stack<int> st;
mutex cout_mutex;

void writer_routine() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, 100);

    while (true) {
        this_thread::sleep_for(200ms);
        {
            int value = distr(gen);
            st.push(value);
            {
                unique_lock lock(cout_mutex);
                cout << "WRITE: " << value << endl;
            }
        }
    }
}

void reader_routine() {
    while (true) {
        if (auto pval = st.pop();
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

