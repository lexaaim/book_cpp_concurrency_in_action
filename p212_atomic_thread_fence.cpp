#include <atomic>
#include <thread>
#include <cassert>

using namespace std;

/* atomic<bool> x; */
bool x = false;
atomic<bool> y;
atomic<int>  z;

void write_x_then_y() {
    /* x.store(true, memory_order_relaxed); */
    x = true;
    atomic_thread_fence(memory_order_release);
    y.store(true, memory_order_relaxed);
}

void read_y_then_x() {
    while (!y.load(memory_order_relaxed));
    atomic_thread_fence(memory_order_acquire);
    /* if (x.load(memory_order_relaxed)) {  */
    if (x) {
        ++z;
    }
}

int main() {
    x = false;
    y = false;
    z = 0;

    thread a{ write_x_then_y };
    thread b{  read_y_then_x };
    a.join();
    b.join();

    assert(z.load() != 0); // не сработает никогда
    // гонки за x нет, т.к. происходит только одна запись x и одно чтение x
    // а барьер разграничивает их по времени.
    // гонка за y может быть, т.к. мы читаем y до барьера постоянно
}
