#ifndef RECEIVER_H
#define RECEIVER_H

#include "message_queue.h"
#include "sender.h"
#include "dispatcher.h"

namespace Messaging
{
class Receiver {
    Queue _queue;

public:
    operator Sender() {
        return Sender(&_queue);
    }

    Dispatcher wait() {
        return Dispatcher(&_queue);
    }
};
}

#endif
