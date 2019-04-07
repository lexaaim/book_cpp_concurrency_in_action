#include <atomic>
#include <thread>
#include <cassert>
#include <iostream>

using namespace std;

atomic<bool> x, y;
atomic<int>  z;

void write_x_then_y() {
    x.store(true, memory_order_relaxed);
    y.store(true, memory_order_relaxed);
}

void read_y_then_x() {
    while (!y.load(memory_order_relaxed));
    if (x.load(memory_order_relaxed)) { ++z; };
}

int main() {
    for (int i = 0; i < 20; i++) {
        x = false;
        y = false;
        z = 0;

        thread a{ write_x_then_y };
        thread b{ read_y_then_x };
        a.join();
        b.join();

        if (z == 0) { cout << "z = " << z << endl; }
        else {        cout << "z = " << z << endl; }
    }
    // memory_order_relaxed требует минимальных дополнительных затрат
    // на поддержание синхронизации данных между потоками и гаранитирует
    // только то, что внутри ОДНОГО потока атомарная переменная будет читаться
    // упорядоченно (т.е. следующее чтение не может прочитать состояние,
    // которое было до предыдущего чтения)
}
