#include <list>
#include <mutex>
#include <algorithm>
#include <vector>
#include <functional>
#include <thread>
#include <iostream>
#include <iomanip>

using namespace std;

list<int> shared_list;
mutex list_mutex;

void add_to_list(int new_value) {
    lock_guard<mutex> guard(list_mutex);
    shared_list.push_back(new_value);

    cout << "Was inserted  (" << new_value << ")\n";
}

bool list_contains(int value_to_find) {
    lock_guard<mutex> guard(list_mutex);
    const bool result = find(begin(shared_list), end(shared_list), value_to_find)
        != end(shared_list);
    cout << "Search result (" << value_to_find << "): " << boolalpha << result << '\n';
    return result;
}

int main() {
    vector<thread> threads;
    for (auto i = 0; i < 10; i++) {
        threads.push_back(thread{add_to_list, i});
        threads.push_back(thread{list_contains, i});
    }

    for_each (threads.rbegin(), threads.rend(), mem_fn(&thread::join));
}
