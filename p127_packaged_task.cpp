#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <utility>
#include <chrono>
#include <iostream>

using namespace std;

mutex                        tasks_mutex;
queue<packaged_task<void()>> tasks;
/* condition_variable           tasks_cond; */

template <typename F>
future<void> add_task(F f) {
    packaged_task<void()> task(f);
    future<void> result = task.get_future();

    lock_guard lock(tasks_mutex);
    tasks.push(move(task));             // !move!  packaged_task doesn't have copy constructor
    return result;
}

void f1() {
    this_thread::sleep_for(500ms);
    cout << "F1" << endl;
}

void f2() {
    this_thread::sleep_for(500ms);
    cout << "F2" << endl;
}

void f3() {
    this_thread::sleep_for(500ms);
    cout << "F3" << endl;
}

void thread_routine(int i) {
    while (true) {
        if      (i == 1) { add_task<void()>(f1); }
        else if (i == 2) { add_task<void()>(f2); }
        else if (i == 3) { add_task<void()>(f3); }
        this_thread::sleep_for(500ms);
    }
}

int main() {
    thread t1{ thread_routine, 1 };
    thread t2{ thread_routine, 2 };
    thread t3{ thread_routine, 3 };

    t1.detach();
    t2.detach();
    t3.detach();

    while(true) {
        packaged_task<void()> task;
        {
            lock_guard lock(tasks_mutex);

            if (tasks.empty()) { continue; }
            task = move(tasks.front());             // move!
            tasks.pop();
        }
        task();
    }
}
