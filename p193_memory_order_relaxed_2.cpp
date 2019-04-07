#include <thread>
#include <atomic>
#include <iostream>

using namespace std;

atomic<int> x{ 0 }, y{ 0 }, z{ 0 };
atomic<bool> go{ false };

unsigned const loop_count = 9;

struct ReadValues {
    int x, y, z;
};

ReadValues values1[loop_count];
ReadValues values2[loop_count];
ReadValues values3[loop_count];
ReadValues values4[loop_count];
ReadValues values5[loop_count];

void increment(atomic<int> * var_to_inc, ReadValues * values) {
    while (!go) {
        this_thread::yield();
    }

    for (unsigned i = 0; i < loop_count; ++i) {
        values[i].x = x.load(memory_order_relaxed);
        values[i].y = y.load(memory_order_relaxed);
        values[i].z = z.load(memory_order_relaxed);

        var_to_inc->store(i + 1, memory_order_relaxed);
        this_thread::yield();
    }
}

void read_vals(ReadValues * values) {
    while (!go) {
        this_thread::yield();
    }

    for (unsigned i = 0; i < loop_count; ++i) {
        values[i].x = x.load(memory_order_relaxed);
        values[i].y = y.load(memory_order_relaxed);
        values[i].z = z.load(memory_order_relaxed);

        this_thread::yield();
    }
}

void print(ReadValues * v) {
    for (unsigned i = 0; i < loop_count; ++i) {
        if (i) { cout << ","; }
        cout << "(" << v[i].x << "," << v[i].y << "," << v[i].z << ")";
    }
    cout << endl;
}

int main() {
    thread t1{ increment, &x, values1 };
    thread t2{ increment, &y, values2 };
    thread t3{ increment, &z, values3 };
    thread t4{ read_vals, values4 };
    thread t5{ read_vals, values5 };

    go = true;

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    print(values1);
    print(values2);
    print(values3);
    print(values4);
    print(values4);
}
