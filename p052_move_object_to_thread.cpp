#include <iostream>
#include <thread>
#include <memory>
#include <chrono>

using namespace std;

void thread_func(unique_ptr<string> pstr) {
    cout << *pstr << endl;
}

int main() {
    {
        auto pstr = make_unique<string>("Let's output this unique string");
        /* thread t(thread_func, move(pstr)); */  // will not compile
        thread t(thread_func, move(pstr));      // OK
        t.detach();
    }
    this_thread::sleep_for(100ms);
}
