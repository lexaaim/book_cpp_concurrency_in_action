#include "bank_machine.h"
#include "messages.h"

void BankMachine::run() {
    try {
        while (true) {
            _incoming.wait()
                .handle<VerifyPin>(
                    [&](const VerifyPin & msg)
                    {
                        if(msg.pin == "1937") {
                            msg.atm_queue.send(PinVerified());
                        } else {
                            msg.atm_queue.send(PinIncorrect());
                        }
                    })
                .handle<Withdraw>(
                    [&](const Withdraw & msg)
                    {
                        if(_balance >= msg.amount) {
                            msg.atm_queue.send(WithdrawOk());
                            _balance -= msg.amount;
                        } else {
                            msg.atm_queue.send(WithdrawDenied());
                        }
                    })
                .handle<GetBalance>(
                    [&](const GetBalance & msg)
                    {
                        msg.atm_queue.send(Balance(_balance));
                    })
                .handle<WithdrawalProcessed>(
                    [&](const WithdrawalProcessed &) { })
                .handle<CancelWithdrawal>(
                    [&](const CancelWithdrawal &) { });
        }
    }
    catch(Messaging::CloseQueue const&)
    {
    }
}

