#ifndef INTERFACE_MACHINE_H
#define INTERFACE_MACHINE_H

#include "receiver.h"
#include "sender.h"

class InterfaceMachine {
    Messaging::Receiver _incoming;
    std::mutex          _iomutex;

public:
    void done() {
        get_sender().send(Messaging::CloseQueue());
    }

    Messaging::Sender get_sender() {
        return _incoming;
    }

    void run();
};

#endif
