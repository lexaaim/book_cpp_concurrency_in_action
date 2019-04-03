#include <thread>
#include <iostream>
#include <vector>
#include <functional>

using namespace std;

const thread::id master_id = this_thread::get_id();

void thread_func() {
    if (this_thread::get_id() > master_id) {
        cout << this_thread::get_id() << " is more than" << endl;
    } else {
        cout << this_thread::get_id() << " is less than" << endl;
    }
}

int main() {
    vector<thread> threads;

    for (auto i = 0; i < thread::hardware_concurrency(); i++) {
        threads.push_back(thread{thread_func});
    }

    cout << master_id << " MASTER" << endl;
    for_each (begin(threads), end(threads), mem_fn(&thread::join));
}
