#ifndef RECEIVER_H
#define RECEIVER_H

#include "messaging_queue.h"
#include "messaging_sender.h"
#include "messaging_dispatcher.h"

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
