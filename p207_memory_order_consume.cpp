#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <cassert>

using namespace std;

struct X {
    int i;
    string s;
};

atomic<X *> p;
atomic<int> a;

void create_x() {
    X * x = new X;
    x->i = 42;
    x->s = "hello";

    a.store(99, memory_order_relaxed);
    p.store(x,  memory_order_release);
}

void use_x() {
    X * x;
    while (!(x = p.load(memory_order_consume))) {
        this_thread::sleep_for(1us);
    }

    assert(x->i == 42);         // x->i зависит от x, memory_order_consume гарантирует,
    // что x синхронизировано и все что зависит от x синхронизировано, но не больше!,
    // поэтому assert не сработает никогда
    assert(x->s == "hello");    // не сработает
    assert(a.load(memory_order_relaxed) == 99);  // может сработать
}

int main() {
    thread t1{ create_x };
    thread t2{ use_x };

    t1.join();
    t2.join();
}
