#include <iostream>
#include <thread>

using namespace std;

class Object {
public:
    void Method() {
        cout << "From Method" << endl;
    }
};

int main() {
    Object obj;
    thread t(&Object::Method, &obj);
    t.join();
}
