#include <iostream>
#include <thread>
#include <chrono>

// see also p057_scoped_guard.cpp

using namespace std;

class thread_guard {
    thread & _thread;
public:
    explicit thread_guard(thread & t) : _thread(t) { }

    ~thread_guard() {
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    thread_guard(const thread_guard &) = delete;
    thread_guard & operator=(const thread_guard &) = delete;
};

struct func_object {
    int & _i;
    func_object(int & i) : _i{ i } { }

    void operator()() {
        thread::id this_id = this_thread::get_id();
        for (unsigned i = 0; i < 5; i++) {
            cout << "from thread(" << this_id << ") " << _i++ << endl;
            this_thread::sleep_for(1ms);
        }
        cout << endl;
    }
};

int main() {
    int var1 = 10;
    thread th1{func_object(var1)};
    thread_guard tg1(th1);

    int var2 = 20;
    thread th2{func_object(var2)};
    thread_guard tg2(th2);

    cout << "End of main function" << endl;
}
