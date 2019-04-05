#ifndef SENDER_H
#define SENDER_H

#include "messaging_queue.h"

namespace Messaging
{

class Sender {
Queue * _queue;

public:
    Sender() : _queue(nullptr) { }

    explicit Sender(Queue * q) : _queue(q) { }

    template <typename Message>
    void send(const Message & msg) {
        if (_queue != nullptr) {
            _queue->push(msg);
        }
    }
};

}

#endif
