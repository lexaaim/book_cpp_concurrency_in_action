#include "interface_machine.h"
#include "messages.h"
#include <iostream>

using namespace std;

void InterfaceMachine::run() {
    try {
        while (true) {
            _incoming.wait()            // constructs and returns new Dispatcher
                .handle<IssueMoney>(    // constructs and returns the TemplateDispatcher instance,
                                        // saves lambda and a pointer to the parent dispatcher
                    [&](const IssueMoney & msg)
                    {
                        lock_guard<mutex> lk(_iomutex);
                        cout << "Issuing " << msg.amount << endl;
                    })
                .handle<DisplayInsufficientFunds>(
                    [&](const DisplayInsufficientFunds &)
                    {
                        lock_guard<mutex> lk(_iomutex);
                        cout << "Insufficient funds" << endl;
                    })
                .handle<DisplayEnterPin>(
                    [&](const DisplayEnterPin &)
                    {
                        lock_guard<mutex> lk(_iomutex);
                        cout << "Please enter your PIN (0-9)" << endl;
                    })
                .handle<DisplayEnterCard>(
                    [&](const DisplayEnterCard &)
                    {
                        lock_guard<mutex> lk(_iomutex);
                        cout << "Please enter your card (I)" << endl;
                    })
                .handle<DisplayBalance>(
                    [&](const DisplayBalance & msg)
                    {
                        lock_guard<mutex> lk(_iomutex);
                        cout << "The balance of your account is " << msg.amount << endl;
                    })
                .handle<DisplayWithdrawalOptions>(
                    [&](const DisplayWithdrawalOptions &)
                    {
                        lock_guard<mutex> lk(_iomutex);
                        cout << "Withdraw 50? (w)" << endl;
                        cout << "Display Balance? (b)" << endl;
                        cout << "Cancel? (c)" << endl;
                    })
                .handle<DisplayWithdrawalCancelled>(
                    [&](const DisplayWithdrawalCancelled &)
                    {
                        lock_guard<mutex> lk(_iomutex);
                        cout << "Withdrawal cancelled" << endl;
                    })
                .handle<DisplayPinIncorrectMessage>(
                    [&](const DisplayPinIncorrectMessage &)
                    {
                        lock_guard<mutex> lk(_iomutex);
                        cout << "PIN incorrect" << endl;
                    })
                .handle<EjectCard>(
                    [&](const EjectCard &)
                    {
                        lock_guard<mutex> lk(_iomutex);
                        cout << "Ejecting card" << endl;
                    });

        // Every of TemplateDispathers are chained, except for last one.
        // Calls Dispatcher's destructor, where it reads a new message from the queue,
        // and dispatches it from last to first
        }
    }
    catch(Messaging::CloseQueue &) {
    }
}
