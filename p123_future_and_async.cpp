#include <future>
#include <iostream>
#include <chrono>
#include <vector>

using namespace std;

size_t fibonacci_slow(size_t index) {
    if (index == 0u) { return 0; }
    if (index == 1u) { return 1; }

    vector<int> fibs(index + 1, 0);
    fibs[1] = 1;

    for (size_t i = 2; i < index + 1; ++i) {
        fibs[i] = fibs[i - 2] + fibs[i - 1];
        this_thread::sleep_for(50ms);
    }
    return fibs[index];
}

int main() {
    future<size_t> result = async(launch::async, fibonacci_slow, 20);
    cout << result.get() << endl;
}
