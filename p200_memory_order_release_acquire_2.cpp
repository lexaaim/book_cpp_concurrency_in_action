#include <atomic>
#include <thread>
#include <cassert>

using namespace std;

atomic<bool> x, y;
atomic<int>  z;

void write_x_then_y() {
    x.store(true, memory_order_relaxed);
    y.store(true, memory_order_release);
}

void read_y_then_x() {
    while (!y.load(memory_order_acquire));
    if (x.load(memory_order_relaxed)) { ++z; }
}

int main() {
    for (int i = 0; i < 10; i++) {
        y = false;
        x = false;
        z = 0;

        thread a{ write_x_then_y };
        thread b{ read_y_then_x };

        a.join();
        b.join();

        assert( z.load() != 0 );  // a синхронизируется с b по y; x синхронизируется с y т.к.
        // обе меняются внутри потока a
        // поэтому assert никогда не сработает
    }
}
