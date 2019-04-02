#include <iostream>
#include <thread>

using namespace std;

void hello() {
    cout << "Parallel Hello World!" << endl;
}

int main() {
    thread thr(hello);
    thr.join();
}
