#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void thread_func1() {
    this_thread::sleep_for(10ms);
    cout << "From 1st function" << endl;
}

void thread_func2() {
    this_thread::sleep_for(10ms);
    cout << "From 2nd function" << endl;
}

int main() {
    thread t1(thread_func1);
    thread t2 = move(t1);           // t1 turns into a default constructed state
    t1 = thread(thread_func2);
    thread t3;
    t3 = move(t2);
    /* t1 = move(t3); */            // ERROR! t1 is already assigned and has an
                                    // associated running thread (test t1.joinable() == true)
                                    // In this case it calls std::terminate()

    if (t1.joinable()) { t1.detach(); }
    if (t2.joinable()) { t2.detach(); } // fails test
    if (t3.joinable()) { t3.detach(); }

    this_thread::sleep_for(20ms);
}
