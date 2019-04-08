#include <atomic>
#include <thread>
#include <cassert>

using namespace std;

atomic<int>  sdata[5];
atomic<int> sync{ 0 };

void thread_1() {
    sdata[0].store(42, memory_order_relaxed);
    sdata[1].store(97, memory_order_relaxed);
    sdata[2].store(17, memory_order_relaxed);
    sdata[3].store(11, memory_order_relaxed);
    sdata[4].store(20, memory_order_relaxed);

    sync.store(1, memory_order_release);
}

void thread_2() {
    int expected = 1;
    while (!sync.compare_exchange_strong(expected, 2, memory_order_acq_rel)) {
        expected = 1;
    }
}

void thread_3() {
    while (sync.load(memory_order_acquire) < 2);

    assert(sdata[0].load(memory_order_relaxed) == 42);
    assert(sdata[1].load(memory_order_relaxed) == 97);
    assert(sdata[2].load(memory_order_relaxed) == 17);
    assert(sdata[3].load(memory_order_relaxed) == 11);
    assert(sdata[4].load(memory_order_relaxed) == 20);
}

int main() {
    thread t1{ thread_1 };
    thread t2{ thread_2 };
    thread t3{ thread_3 };

    t1.join();
    t2.join();
    t3.join();

    // через флаги синхронизации полностью синхронизировано
    // чтение данных между 1 и 3 потоками
}
