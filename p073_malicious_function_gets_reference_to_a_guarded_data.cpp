#include <iostream>
#include <mutex>
#include <string>

using namespace std;

class Data {
    int _a;
    string _b;

public:
    void do_something() {
        // within this code block may occur race conditions in the multithreaded program
        cout << "Access to protected code" << endl;
    }
};

class DataWrapper {
private:
    Data  _data;
    mutex _mutex;

public:
    template <typename Function>
    void process_data(Function f) {
        lock_guard<mutex> guard(_mutex);  // protect the class member
        f(_data);
    }
};

Data * malicious_pointer;


void malicious_function(Data & d) {
    malicious_pointer = &d;
}

DataWrapper protected_data;

int main() {
    protected_data.process_data(malicious_function);
    malicious_pointer->do_something();                  // !!unprotected access!!
}
