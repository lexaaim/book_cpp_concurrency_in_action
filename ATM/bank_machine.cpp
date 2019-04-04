#include "bank_machine.h"
#include "messages.h"

void BankMachine::run() {
    try
    {
        for(;;)
        {
            _incoming.wait()
                .handle<verify_pin>(
                    [&](verify_pin const& msg)
                    {
                        if(msg.pin=="1937")
                        {
                            msg.atm_queue.send(pin_verified());
                        }
                        else
                        {
                            msg.atm_queue.send(pin_incorrect());
                        }
                    }
                    )
                .handle<withdraw>(
                    [&](withdraw const& msg)
                    {
                        if(_balance>=msg.amount)
                        {
                            msg.atm_queue.send(withdraw_ok());
                            _balance-=msg.amount;
                        }
                        else
                        {
                            msg.atm_queue.send(withdraw_denied());
                        }
                    }
                    )
                .handle<get_balance>(
                    [&](get_balance const& msg)
                    {
                        msg.atm_queue.send(::balance(_balance));
                    }
                    )
                .handle<withdrawal_processed>(
                    [&](withdrawal_processed const &)
                    {
                    }
                    )
                .handle<cancel_withdrawal>(
                    [&](cancel_withdrawal const &)
                    {
                    }
                    );
        }
    }
    catch(Messaging::CloseQueue const&)
    {
    }
}

