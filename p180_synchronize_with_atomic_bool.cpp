#include <vector>
#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

vector<int>  data_container;
atomic<bool> data_ready{ false };

void reader_thread() {
    while (!data_ready.load()) {
        this_thread::sleep_for(1ms);
    }

    cout << "Response: " << data_container[0] << '\n';
}

void writer_thread() {
    this_thread::sleep_for(30ms);

    data_container.push_back(42);
    data_ready = true;
}

int main() {
    thread tw{ writer_thread };
    thread tr{ reader_thread };

    tw.join();
    tr.join();
}
