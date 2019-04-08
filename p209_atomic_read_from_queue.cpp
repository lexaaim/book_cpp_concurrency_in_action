#include <atomic>
#include <thread>
#include <vector>
#include <iostream>

using namespace std;

vector<int> queue_data;
atomic<int> count;

void populate_queue() {
    const unsigned number_of_items = 20;
    queue_data.clear();
    for (unsigned i = 0; i < number_of_items; i++) {
        queue_data.push_back(i);
        this_thread::sleep_for(25ms);
    }
    count.store(number_of_items, memory_order_release);
}

void consume_queue_items() {
    while (true) {
        int item_index;
        if ((item_index = count.fetch_sub(1, memory_order_acquire)) <= 0) {
            /* wait_for_more_items(); */
            this_thread::sleep_for(10ms);
            continue;
        }
        cout << queue_data[item_index - 1] << endl;
        /* process(queue_data[item_index - 1]); */
    }
}

int main() {
    thread a{ populate_queue };
    thread b{ consume_queue_items };
    thread c{ consume_queue_items };

    a.join();
    b.join();
    c.join();
}
