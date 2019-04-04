#ifndef BANK_MACHINE_H
#define BANK_MACHINE_H

#include "receiver.h"

class BankMachine {
    Messaging::Receiver _incoming;
    unsigned            _balance;

public:
    BankMachine() : _balance(199) { }

    void done() {
        get_sender().send(Messaging::CloseQueue());
    }

    Messaging::Sender get_sender() {
        return _incoming;
    }

    void run();
};

#endif
