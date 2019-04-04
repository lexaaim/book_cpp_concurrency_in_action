#include <map>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <thread>
#include <iostream>

using namespace std;

class DNSEntry {
    string _data;

public:
    DNSEntry() = default;
    DNSEntry(const string & str) : _data{ str } { }

    string data() const { return _data; }
};

ostream & operator<<(ostream & stream, const DNSEntry & dnse) {
    return stream << dnse.data();
}

class DNSCache {
    map<string, DNSEntry> _entries;
    mutable shared_mutex _entry_mutex;

public:
    DNSEntry find_entry(const string & domain) const {
        shared_lock lock(_entry_mutex);
        auto it = _entries.find(domain);
        if (it == _entries.end()) { return DNSEntry(); }
        else { return it->second; }
    }

    void update_entry(const string & domain, const DNSEntry & dnse) {
        unique_lock lock(_entry_mutex);
        _entries[domain] = dnse;
    }
};

DNSCache dnscache;
mutex coutmutex;

void reader_routine() {
    for (int i = 0; i < 10; i++) {
        unique_lock lock(coutmutex);
        cout << "1: " << dnscache.find_entry("google.com") << endl;
        lock.unlock();

        this_thread::sleep_for(20ms);
        lock.lock();
        cout << "2: " << dnscache.find_entry("ya.ru") << endl;
        lock.unlock();

        this_thread::sleep_for(20ms);
        lock.lock();
        cout << "3: " << dnscache.find_entry("cppreference.com") << endl;
        lock.unlock();
        this_thread::sleep_for(20ms);
    }
}

void writer_routine() {
    this_thread::sleep_for(100ms);
    dnscache.update_entry("google.com", DNSEntry("01.02.03.04"));

    this_thread::sleep_for(200ms);
    dnscache.update_entry("ya.ru", DNSEntry("98.125.80.204"));

    this_thread::sleep_for(200ms);
    dnscache.update_entry("cppreference.com", DNSEntry("114.87.113.139"));
}

int main() {
    thread reader1{ reader_routine };
    thread reader2{ reader_routine };

    thread writer{ writer_routine };

    reader1.join();
    reader2.join();
    writer.join();
}
