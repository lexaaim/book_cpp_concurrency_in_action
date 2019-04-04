#ifndef ATM_H
#define ATM_H

#include "receiver.h"
#include "sender.h"
#include <string>

class ATM
{
    void (ATM::*_state)();

    Messaging::Receiver _incoming;
    Messaging::Sender   _bank;
    Messaging::Sender   _interface_hardware;

    std::string         _account;
    unsigned            _withdrawal_amount;
    std::string         _pin;

    void process_withdrawal();
    void process_balance();
    void wait_for_action();
    void verifying_pin();
    void getting_pin();
    void waiting_for_card();
    void done_processing();

    ATM(ATM const&)=delete;
    ATM& operator=(ATM const&)=delete;

public:
    ATM(Messaging::Sender bank_,
        Messaging::Sender interface_hardware_):
        _bank(bank_),_interface_hardware(interface_hardware_)
    {}

    void done() {
        get_sender().send(Messaging::CloseQueue());
    }

    Messaging::Sender get_sender() {
        return _incoming;
    }

    void run();
};

#endif
