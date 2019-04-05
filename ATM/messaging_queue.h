#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

namespace Messaging
{

struct MessageBase {
    virtual ~MessageBase() { }
};

template <typename Msg>
struct WrappedMessage: MessageBase {
    Msg _contents;

    explicit WrappedMessage(const Msg & contents)
        : _contents(contents) {
    }
};

class Queue {
    std::mutex                               _mutex;
    std::condition_variable                  _cond;
    std::queue<std::shared_ptr<MessageBase>> _queue;

public:
    Queue() = default;

    template <typename T>
    void push(const T & msg) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(std::make_shared<WrappedMessage<T>>(msg));
        _cond.notify_all();
    }

    std::shared_ptr<MessageBase> wait_and_pop() {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock, [&]{ return !_queue.empty(); });
        auto result = _queue.front();
        _queue.pop();

        return result;
    }
};
}

#endif
