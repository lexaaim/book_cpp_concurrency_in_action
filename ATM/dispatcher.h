#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "message_queue.h"
#include "template_dispatcher.h"

namespace Messaging
{
class CloseQueue{ };

class Dispatcher {
    Queue * _queue;
    bool    _chained;

    Dispatcher(const Dispatcher &) = delete;
    Dispatcher & operator=(const Dispatcher &) = delete;

    template<
        typename Dispatcher,
        typename Msg,
        typename Func>
    friend class TemplateDispatcher;

    void wait_and_dispatch() {
        for (;;) {
            auto msg = _queue->wait_and_pop();
            dispatch(msg);
        }
    }

    bool dispatch(const std::shared_ptr<MessageBase> & msg) {
        if (dynamic_cast<WrappedMessage<CloseQueue> *>(msg.get())) {
            throw CloseQueue();
        }
        return false;
    }
public:
    Dispatcher(Dispatcher && other)
    : _queue(other._queue), _chained(other._chained) {
        other._chained = true;
    }

    explicit Dispatcher(Queue * q)
    : _queue(q), _chained(false) {
    }

    template <typename Message, typename Func>
    TemplateDispatcher<Dispatcher, Message, Func>
    handle(Func && f) {
        return TemplateDispatcher<Dispatcher, Message, Func>(_queue, this, std::forward<Func>(f));
    }

    ~Dispatcher() noexcept(false) { // destructor may generate exceptions
        if (!_chained) {
            wait_and_dispatch();
        }

    }
};
}

#endif
