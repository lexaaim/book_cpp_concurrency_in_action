#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <functional>
#include <vector>
#include <numeric>

using namespace std;

template <typename Iterator, typename T>
struct accumulate_block {
    void operator()(Iterator first, Iterator last, T & result) {
        result = accumulate(first, last, result);
    }
};

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    const size_t length = distance(first, last);
    if (length == 0) { return init; }

    const size_t min_per_thread = 25;
    const size_t max_threads = (length + min_per_thread - 1) / min_per_thread; // ceiling
    const size_t hardware_threads = thread::hardware_concurrency();

    const size_t num_threads = min<size_t>(max<size_t>(hardware_threads, 2), max_threads);
    const size_t block_size = length / num_threads;

    vector<T> results(num_threads);                     // initialized with T()
    vector<thread> threads(num_threads - 1);

    Iterator new_begin = first;
    for (size_t i = 0; i < num_threads - 1; i++) {
        Iterator new_end = new_begin;
        advance(new_end, block_size);
        threads[i] = thread{accumulate_block<Iterator, T>(), new_begin, new_end, ref(results[i])};
        new_begin = new_end;
    }
    accumulate_block<Iterator, T>()(new_begin, last, ref(results[num_threads - 1]));

    for_each (begin(threads), end(threads), mem_fn(&thread::join));
    return accumulate(begin(results), end(results), init);
}

int main() {
    vector<int> ints(10000);
    iota(begin(ints), end(ints), 1);
    auto result = parallel_accumulate(begin(ints), end(ints), 0);
    cout << result << endl;
}
