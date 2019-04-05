#include "messaging_sender.h"
#include "messages.h"
#include "bank_machine.h"
#include "interface_machine.h"
#include "atm.h"

#include <thread>

using namespace std;

int main()
{
    BankMachine       bank;
    InterfaceMachine  interface_hardware;
    ATM               machine(bank.get_sender(), interface_hardware.get_sender());

    thread            bank_thread(&BankMachine::run, &bank);
    thread            if_thread  (&InterfaceMachine::run, &interface_hardware);
    thread            atm_thread (&ATM::run, &machine);

    Messaging::Sender atmqueue(machine.get_sender());

    bool quit_pressed = false;
    while(!quit_pressed)
    {
        char c = static_cast<char>(getchar());
        switch(c)
        {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            atmqueue.send(DigitPressed(c));
            break;
        case 'b':
            atmqueue.send(BalancePressed());
            break;
        case 'w':
            atmqueue.send(WithdrawPressed(50));
            break;
        case 'c':
            atmqueue.send(CancelPressed());
            break;
        case 'q':
            quit_pressed = true;
            break;
        case 'i':
            atmqueue.send(CardInserted("acc1234"));
            break;
        }
    }

    bank.done();
    machine.done();
    interface_hardware.done();

    atm_thread.join();
    bank_thread.join();
    if_thread.join();
}
