#include <condition_variable>
#include <mutex>
#include <iostream>
#include <thread>
#include <queue>
#include <random>
#include <chrono>

using namespace std;

mutex              cout_mutex;
mutex              queue_mutex;
queue<string>      queue_data;
condition_variable queue_cond;

void writer_routine() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, 10);

    while (true) {
        string str = to_string(distr(gen));
        this_thread::sleep_for(200ms);
        {
            unique_lock lock(queue_mutex);
            queue_data.push(str);
            queue_cond.notify_one();
            {
                unique_lock lock(cout_mutex);
                cout << "WRITE: " << str << endl;
            }
        }
    }
}

void reader_routine() {
    while (true) {
        unique_lock lock(queue_mutex);
        queue_cond.wait(lock, []{ return queue_data.size() >= 5; });

        {
            unique_lock coutlock(cout_mutex);
            cout << "READ: ";
            while (!queue_data.empty()) {
                cout << queue_data.front() << ' ';
                queue_data.pop();
            }
            cout << endl;
        }
    }
}

int main() {
    thread writer1{ writer_routine };
    thread writer2{ writer_routine };
    thread writer3{ writer_routine };
    thread reader1{ reader_routine };

    writer1.join();
    writer2.join();
    writer3.join();
    reader1.join();
}
