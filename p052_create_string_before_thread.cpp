#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void thread_func(const string & str) {
    this_thread::sleep_for(20ms);
    cout << str << endl;
}

int main() {
    {
        char buffer[] = "hello from thread!";
        thread t1(thread_func, buffer);         // constructs string within thread
        thread t2(thread_func, string(buffer)); // constructs string within main thread
        t1.detach();
        t2.detach();
    }
    char buffer[]     = "overwrite stack!!";    // glitch!
    this_thread::sleep_for(100ms);
}
