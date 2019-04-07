#include <atomic>
#include <thread>
#include <cassert>

using namespace std;

atomic<bool> x, y;
atomic<int>  z;

void write_x() {
    x.store(true, memory_order_seq_cst);    // default memory order
}

void write_y() {
    y.store(true, memory_order_seq_cst);    // default memory order
}

void read_x_then_y() {
    while(!x.load(memory_order_seq_cst));
    if (y.load(memory_order_seq_cst)) { ++z; }
}

void read_y_then_x() {
    while(!y.load(memory_order_seq_cst));
    if (x.load(memory_order_seq_cst)) { ++z; }
}

int main() {
    for (int i = 0; i < 10; i++) {
        x = false;
        y = false;
        z = 0;

        thread a{ write_x };
        thread b{ write_y };
        thread c{ read_x_then_y };
        thread d{ read_y_then_x };

        a.join();
        b.join();
        c.join();
        d.join();

        // разные потоки могут видеть одну последовательность событий в разном порядке,
        // т.е. один поток может увидеть установку флагов в порядке x, y, а другой в
        // порядке y, x. Поэтому совсем не обязательно, что сработает хотя бы раз ++z.
        // std::memory_order_seq_cst гарантирует что разные потоки видят установку флагов
        // в одном и том же порядке! Но требуются максимальные издержки на поддержание
        // глобальной синхронизации между всеми потоками.

        assert( z.load() != 0);
    }
}
