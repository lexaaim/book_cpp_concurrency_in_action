#include <iostream>
#include <thread>
#include <chrono>
#include <string>

using namespace std;

class scoped_thread {
    thread _thread;

public:
    explicit scoped_thread(thread t) : _thread{ move(t) } {
        if (!_thread.joinable()) {
            throw logic_error("NO THREAD");
        }
    }

    ~scoped_thread() {
        _thread.join();
    }

    scoped_thread(const scoped_thread &) = delete;
    scoped_thread & operator=(const scoped_thread &) = delete;
};

void func_captured_int(const string & value) {
    this_thread::sleep_for(10ms);
    cout << value << endl;
}

int main() {
    {
        string str1 = "forty-two is the answer for life";
        scoped_thread st(thread(func_captured_int, ref(str1))); // OK, calls join before destruction
        string str2 = "forty-two is the answer for the universe";
        thread t(func_captured_int, ref(str2));
        t.detach();                       // ERROR! uses the destroyed captured local variable
    }
    this_thread::sleep_for(100ms);
}
