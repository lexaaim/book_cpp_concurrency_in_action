#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void thread_func(const string & str) {
    this_thread::sleep_for(20ms);
    cout << str << endl;
}

int main() {
    {
        string str{ "There is a text" };
        thread t1(thread_func, str);      // copy string value
        thread t2(thread_func, ref(str)); // forward reference to the str
        t1.detach();
        t2.detach();
        str = "Ok, but now there is another text!";  // glitch (read/write data from several threads)
    }

    this_thread::sleep_for(100ms);
}
