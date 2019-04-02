#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <memory>
#include <functional>

using namespace std;

void thread_func(unique_ptr<string> pstr) {
    this_thread::sleep_for(100ms);
    cout << "thread ("
        /* << this_thread::get_id()  */
        << "): " << *pstr << endl;
}

int main() {
    vector<thread> threads;
    for (auto i = 0; i < 10; i++) {
        auto pstr = make_unique<string>("string " + to_string(i));
        threads.push_back(thread(thread_func, move(pstr)));
    }

    for_each (begin(threads), end(threads), mem_fn(&thread::join));
}
