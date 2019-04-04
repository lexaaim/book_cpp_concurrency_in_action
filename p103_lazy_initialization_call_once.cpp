#include <mutex>
#include <thread>
#include <iostream>
#include <string>
#include <chrono>

using namespace std;

class LazyInitialization {
private:
    string *  _string1 = nullptr;
    string *  _string2 = nullptr;
    once_flag _init_flag;

    void initialize_strings() {
        _string1 = new string("the first initialized string");
        _string2 = new string("the second initialized string");
    }

public:
    LazyInitialization() = default;

    const string & first_method_requiring_the_strings() {
        call_once(_init_flag, &LazyInitialization::initialize_strings, this);

        return *_string1;
    }

    const string & second_method_requiring_the_strings() {
        call_once(_init_flag, &LazyInitialization::initialize_strings, this);

        return *_string2;
    }

    const string & return_static_string() {
        static string thread_safe_string = "static is thread safe";
        return thread_safe_string;
    }
};

LazyInitialization li;

void thread_routine1() {
    this_thread::sleep_for(100ms);
    cout << li.first_method_requiring_the_strings() << endl;
    cout << li.return_static_string() << endl;
}

void thread_routine2() {
    this_thread::sleep_for(100ms);
    cout << li.second_method_requiring_the_strings() << endl;
    cout << li.return_static_string() << endl;
}

int main() {
    thread t1{thread_routine1};
    thread t2{thread_routine2};

    t1.join();
    t2.join();
}
