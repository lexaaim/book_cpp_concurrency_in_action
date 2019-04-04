#ifndef TEMPLATE_DISPATCHER_H
#define TEMPLATE_DISPATCHER_H

#include "message_queue.h"

namespace Messaging
{
    template<typename PreviousDispatcher,typename Msg,typename Func>
    class TemplateDispatcher
    {
        Queue *              q;
        PreviousDispatcher * prev;
        Func                 f;
        bool                 _chained;

        TemplateDispatcher(TemplateDispatcher const&)=delete;
        TemplateDispatcher& operator=(TemplateDispatcher const&)=delete;

        template<typename Dispatcher,typename OtherMsg,typename OtherFunc>
        friend class TemplateDispatcher;

        void wait_and_dispatch()
        {
            for(;;)
            {
                auto msg=q->wait_and_pop();
                if(dispatch(msg))
                    break;
            }
        }

        bool dispatch(std::shared_ptr<MessageBase> const& msg)
        {
            if(WrappedMessage<Msg>* wrapper=
               dynamic_cast<WrappedMessage<Msg>*>(msg.get()))
            {
                f(wrapper->_contents);
                return true;
            }
            else
            {
                return prev->dispatch(msg);
            }
        }
    public:
        TemplateDispatcher(TemplateDispatcher&& other):
            q(other.q),prev(other.prev),f(std::move(other.f)),
            _chained(other._chained)
        {
            other._chained=true;
        }

        TemplateDispatcher(Queue * q_, PreviousDispatcher* prev_,Func&& f_):
            q(q_),prev(prev_),f(std::forward<Func>(f_)), _chained(false)
        {
            prev_->_chained=true;
        }

        template<typename OtherMsg,typename OtherFunc>
        TemplateDispatcher<TemplateDispatcher,OtherMsg,OtherFunc>
        handle(OtherFunc&& of)
        {
            return TemplateDispatcher<
                TemplateDispatcher,OtherMsg,OtherFunc>(
                    q,this,std::forward<OtherFunc>(of));
        }

        ~TemplateDispatcher() noexcept(false)
        {
            if(!_chained)
            {
                wait_and_dispatch();
            }
        }
    };
}

#endif
