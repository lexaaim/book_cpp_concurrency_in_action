#include <chrono>
#include <iostream>

using namespace std;

template <typename D, typename F>
typename D::rep get_duration(F f) {
    auto start = chrono::high_resolution_clock::now();
    f();
    auto stop = chrono::high_resolution_clock::now();
    return chrono::duration_cast<D>(stop - start).count();
}

void fn() {
    for (auto i = 0; i < 20000000; ++i) { }
}

int main() {
    const auto count = get_duration<chrono::milliseconds>(fn);
    cout << count << " ms" << endl;
}
