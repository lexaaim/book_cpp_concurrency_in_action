#include <vector>
#include <thread>
#include <future>
#include <numeric>
#include <iostream>
#include <chrono>

using namespace std;

void fn_accumulate(vector<int>::iterator first,
                   vector<int>::iterator last,
                   promise<int> accumulate_promise)
{
    int sum = std::accumulate(first, last, 0);
    this_thread::sleep_for(500ms);
    accumulate_promise.set_value(sum);      // Notify future

    this_thread::sleep_for(1000ms);
}

int main()
{
    // Demonstrate using promise<int> to transmit a result between threads.
    vector<int> numbers = { 1, 2, 3, 4, 5, 6 };
    promise<int> accumulate_promise;
    future<int> accumulate_future = accumulate_promise.get_future();

    thread work_thread(fn_accumulate, numbers.begin(), numbers.end(),
                       move(accumulate_promise));

    accumulate_future.wait();               // wait for result
    cout << "result=" << accumulate_future.get() << '\n';

    work_thread.join();                     // wait for thread completion
    cout << "thread completed" << endl;
}
