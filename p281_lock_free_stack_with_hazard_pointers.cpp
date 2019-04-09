#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <iostream>
#include <random>

using namespace std;

const unsigned max_hazard_pointers = 100; // соответствует максимальному количеству потоков

struct HazardPointer {
    atomic<thread::id> _id;               // id потока-владельца
    atomic<void *>     _pointer;          // указатель
};

// таблица с опасными указателями, которую читают все потоки
HazardPointer hazard_pointers[max_hazard_pointers];

class HPOwner {
    HazardPointer * _hp;

public:
    HPOwner(const HPOwner &) = delete;
    HPOwner & operator=(const HPOwner &) = delete;

    HPOwner() : _hp(nullptr) {
        for (unsigned i = 0; i < max_hazard_pointers; ++i) {
            thread::id old_id;

            // ищем первую не занятую запись (у которой нет владельца)
            if (hazard_pointers[i]._id.compare_exchange_strong(old_id, this_thread::get_id())) {
                // запоминаем адрес свободной записи
                _hp = &hazard_pointers[i];
                break;
            }
        }

        // проверяем что мы нашли свободную запись
        if (!_hp) {
            throw runtime_error("No hazard pointers available");
        }
    }

    atomic<void *> & get_pointer() {
        return _hp->_pointer;
    }

    ~HPOwner() {
        // освобождаем запись в таблице
        _hp->_pointer.store(nullptr);
        _hp->_id.store(thread::id());
    }
};

atomic<void *> & get_hazard_pointer_for_current_thread() {
    // при первом вызове для каждого потока создается экземпляр
    thread_local static HPOwner hazard;

    return hazard.get_pointer();
}

// проверить есть ли еще опасные указатели на этот
bool outstanding_hazard_pointers_for(void * p) {
    for (unsigned i = 0; i < max_hazard_pointers; ++i) {
        if (hazard_pointers[i]._pointer.load() == p) { return true; }
    }

    return false;
}

// DataToReclaim сохраняет указатель и подходящую ему функцию удаления
// также содержит следующий указатель, т.е. позволяет создавать
// цепочку указателей на разные типы объектов
template <typename T>
void do_delete(void * p) {
    delete static_cast<T *>(p);
}

struct DataToReclaim {
    void *                 _data;
    function<void(void *)> _deleter;
    DataToReclaim *        _next;

    template <typename T>
    DataToReclaim(T * p) : _data(p), _deleter(&do_delete<T>), _next(0) { }

    ~DataToReclaim() {
        _deleter(_data);
    }
};

// указатель на первую ноду
atomic<DataToReclaim *> nodes_to_reclaim;

// добавляем ноду в список на удаление
void add_to_reclaim_list(DataToReclaim * node) {
    node->_next = nodes_to_reclaim.load();
    while (!nodes_to_reclaim.compare_exchange_weak(node->_next, node));
}

template <typename T>
void reclaim_later(T * data) {
    add_to_reclaim_list(new DataToReclaim(data));
}

void delete_nodes_with_no_hazards() {
    // забирает во владение весь список подлежащих удалению узлов
    DataToReclaim * current = nodes_to_reclaim.exchange(nullptr);

    while (current) {
        DataToReclaim * const next = current->_next;

        // если нет указателей опасности на этот узел удаляем его
        if (!outstanding_hazard_pointers_for(current->_data)) {
            delete current;
        } else {
            // иначе добавляем обратно
            add_to_reclaim_list(current);
        }

        current = next;
    }
}

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
        atomic<void *> & hp = get_hazard_pointer_for_current_thread();
        Node * old_head = _head.load();

        do {
            Node * temp;
            do {
                temp = old_head;
                hp.store(old_head);
                old_head = _head.load();
            } while (old_head != temp);
        } while (old_head && !_head.compare_exchange_strong(old_head, old_head->_next));

        hp.store(nullptr);
        shared_ptr<T> result;
        if (old_head) {
            result.swap(old_head->_data);
            if (outstanding_hazard_pointers_for(old_head)) {
                reclaim_later(old_head);
            } else {
                delete old_head;
            }
            delete_nodes_with_no_hazards();
        }

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

