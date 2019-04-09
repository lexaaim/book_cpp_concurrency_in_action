#include <list>
#include <vector>
#include <shared_mutex>
#include <algorithm>
#include <map>
#include <mutex>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;

template <typename Key, typename Value, typename Hash=hash<Key>>
class LookupTable {
private:

    class BucketType {
    private:
        typedef pair<Key, Value>                    BucketValue;
        typedef list<BucketValue>                   BucketData;
        typedef typename BucketData::iterator       BucketIterator;
        typedef typename BucketData::const_iterator ConstBucketIterator;

        BucketData           _data;
        mutable shared_mutex _mutex;

        ConstBucketIterator find_entry_for(const Key & key) const {
            return find_if(_data.cbegin(), _data.cend(),
                           [&key](const BucketValue & item) { return item.first == key; });
        }

        BucketIterator find_entry_for(const Key & key) {
            return find_if(_data.begin(), _data.end(),
                           [&key](const BucketValue & item) { return item.first == key; });
        }

    public:
        Value value_for(const Key & key, const Value & default_value) const {
            shared_lock<shared_mutex> lock(_mutex);
            ConstBucketIterator found_entry = find_entry_for(key);

            return (found_entry == _data.end()) ? default_value
                                                : found_entry->second;
        }

        void add_or_update_mapping(const Key & key, const Value & value) {
            unique_lock<shared_mutex> lock(_mutex);
            BucketIterator found_entry = find_entry_for(key);

            if (found_entry == _data.end()) {
                _data.push_back(BucketValue(key, value));
            } else {
                found_entry->second = value;
            }
        }

        void remove_mapping(const Key & key) {
            unique_lock<shared_mutex> lock(mutex);
            ConstBucketIterator found_entry = find_entry_for(key);

            if (found_entry != _data.end()) {
                _data.erase(found_entry);
            }
        }
    };

    vector<unique_ptr<BucketType>> _buckets;
    Hash                           _hasher;

    BucketType & get_bucket(const Key & key) const {
        const size_t bucket_index = _hasher(key) % _buckets.size();
        return *_buckets[bucket_index];
    }

public:
    typedef Key   key_type;
    typedef Value mapped_type;
    typedef Hash  hash_type;

    LookupTable(unsigned num_buckets = 19, const Hash & hasher = Hash())
    : _buckets(num_buckets), _hasher(hasher) {
        for (unsigned i = 0; i < num_buckets; ++i) {
            _buckets[i].reset(new BucketType);
        }
    }

    LookupTable(const LookupTable &) = delete;
    LookupTable & operator=(const LookupTable &) = delete;

    Value value_for(const Key & key, const Value & default_value = Value()) const {
        return get_bucket(key).value_for(key, default_value);
    }

    void add_or_update_mapping(const Key & key, const Value & value) {
        get_bucket(key).add_or_update_mapping(key, value);
    }

    void remove_mapping(const Key & key) {
        get_bucket(key).remove_mapping(key);
    }

    map<Key, Value> get_map() const {
        // для создания копии словаря необходимо заблокировать все кластеры
        vector<unique_lock<shared_mutex>> locks;
        for (unsigned i = 0; i < _buckets.size(); i++) {
            locks.push_back(unique_lock<shared_mutex>(_buckets[i].mutex));
        }

        map<Key, Value> result;
        for (unsigned i = 0; i < _buckets.size(); ++i) {
            for (typename BucketType::BucketIterator it = _buckets[i].data.begin();
                 it != _buckets[i].data.end(); ++it) {
                result.insert(*it);
            }
        }

        return result;
    }
};

LookupTable<string, int> lt;
mutex                    coutmutex;

void thread_reader() {
    this_thread::sleep_for(100ms);

    while (true) {
        auto val = lt.value_for("first");
        unique_lock coutlock(coutmutex);
        cout << "READ first: "  << val << endl;
        coutlock.unlock();
        this_thread::sleep_for(95ms);

        val = lt.value_for("second");
        coutlock.lock();
        cout << "READ second: " << val << endl;
        coutlock.unlock();
        this_thread::sleep_for(95ms);

        val = lt.value_for("third");
        coutlock.lock();
        cout << "READ third: "  << val << endl;
        coutlock.unlock();
        this_thread::sleep_for(95ms);
    }
}

void thread_writer() {
    while (true) {
        lt.add_or_update_mapping("first",  1);
        unique_lock coutlock(coutmutex);
        cout << "WRITE first: "  << endl;
        coutlock.unlock();
        this_thread::sleep_for(90ms);

        lt.add_or_update_mapping("second", 2);
        coutlock.lock();
        cout << "WRITE second: " << endl;
        coutlock.unlock();
        this_thread::sleep_for(90ms);

        lt.add_or_update_mapping("third",  3);
        coutlock.lock();
        cout << "WRITE third: "  << endl;
        coutlock.unlock();
        this_thread::sleep_for(90ms);
    }
}

void thread_remover() {
    this_thread::sleep_for(200ms);

    while (true) {
        lt.remove_mapping("first");
        unique_lock coutlock(coutmutex);
        cout << "REMOVE first"  << endl;
        coutlock.unlock();
        this_thread::sleep_for(85ms);

        lt.remove_mapping("second");
        coutlock.lock();
        cout << "REMOVE second" << endl;
        coutlock.unlock();
        this_thread::sleep_for(85ms);

        lt.remove_mapping("third");
        coutlock.lock();
        cout << "REMOVE third" << endl;
        coutlock.unlock();
        this_thread::sleep_for(85ms);
    }
}

int main() {
    thread t1{ thread_reader  };
    thread t2{ thread_writer  };
    thread t3{ thread_remover };

    t1.join();
    t2.join();
    t3.join();
}
