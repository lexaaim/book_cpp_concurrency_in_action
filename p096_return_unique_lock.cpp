#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>

using namespace std;

unique_lock<mutex> prepare_and_lock() {
    static mutex private_mutex; // we can access to it only within 'prepare_and_lock'
    unique_lock<mutex> lock(private_mutex);

    cout << "perform some preparations" << endl;
    this_thread::sleep_for(100ms);

    return lock; // returns an unique_lock, not a mutex!
}

int main() {
    auto lock = prepare_and_lock();
    cout << "do something safely" << endl;
    this_thread::sleep_for(100ms);

    lock.unlock();
    cout << "do something without blocking" << endl;
    this_thread::sleep_for(200ms);

    lock.lock();
    cout << "do something else safely" << endl;
    this_thread::sleep_for(100ms);

    // unlock the mutex in the destructor
}
