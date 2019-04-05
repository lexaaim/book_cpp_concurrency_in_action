#ifndef TEMPLATE_DISPATCHER_H
#define TEMPLATE_DISPATCHER_H

#include "messaging_queue.h"

namespace Messaging
{

template <typename PreviousDispatcher, typename Msg, typename Func>
class TemplateDispatcher
{
    Queue *              _queue;
    PreviousDispatcher * _prev;
    Func                 _function;
    bool                 _chained;

    TemplateDispatcher(const TemplateDispatcher &) = delete;
    TemplateDispatcher & operator=(const TemplateDispatcher &) = delete;

    template <typename Dispatcher, typename OtherMsg, typename OtherFunc>
    friend class TemplateDispatcher;

    void wait_and_dispatch() {
        while (true) {
            auto msg = _queue->wait_and_pop();
            if (dispatch(msg)) { break; }
        }
    }

    bool dispatch(const std::shared_ptr<MessageBase> & msg) {
        if(WrappedMessage<Msg> * wrapper = dynamic_cast<WrappedMessage<Msg> *>(msg.get())) {
            _function(wrapper->_contents);
            return true;
        } else {
            return _prev->dispatch(msg);
        }
    }

public:
    TemplateDispatcher(TemplateDispatcher && other) :
        _queue(other._queue), _prev(other._prev),
        _function(std::move(other._function)),
        _chained(other._chained) {
        other._chained = true;
    }

    TemplateDispatcher(Queue * q, PreviousDispatcher * prev, Func && f) :
        _queue(q), _prev(prev),
        _function(std::forward<Func>(f)),
        _chained(false) {
        prev->_chained = true;
    }

    template <typename OtherMsg, typename OtherFunc>
    TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>
    handle(OtherFunc && of)
    {
        return TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>(
                _queue, this, std::forward<OtherFunc>(of));
    }

    ~TemplateDispatcher() noexcept(false) {
        if (!_chained) {
            wait_and_dispatch();
        }
    }
};

}

#endif
