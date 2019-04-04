#include <thread>
#include <mutex>
#include <iostream>
#include <string>

using namespace std;

mutex resource_mutex;
string * resource_ptr;

// naive version
void initialize_resources_naive_locking() {
    // to lock every time we need to do something is too long
    unique_lock<mutex> lock(resource_mutex);
    if (resource_ptr == nullptr) {
        resource_ptr = new string();
    }
    lock.unlock();

    // do something with resource_ptr...
    /* resource_ptr */
}

// undefined behaviour!!
void initialize_resources_double_checked_locking() {
    // here, it may read while the resource is still under
    // construction, but resource_ptr already set
    // DATA RACE
    if (resource_ptr == nullptr) {
        lock_guard<mutex> lock(resource_mutex);
        if (resource_ptr == nullptr) {
            resource_ptr = new string();
        }
    }

    // do something with resource_ptr...
    /* resource_ptr */

}

int main() {
}
