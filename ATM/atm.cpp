#include "atm.h"
#include "messages.h"

void ATM::process_withdrawal() {
    _incoming.wait()
        .handle<withdraw_ok>(
            [&](withdraw_ok const &)
            {
                _interface_hardware.send(
                    issue_money(_withdrawal_amount));
                _bank.send(
                    withdrawal_processed(_account,_withdrawal_amount));
                _state=&ATM::done_processing;
            }
            )
        .handle<withdraw_denied>(
            [&](withdraw_denied const &)
            {
                _interface_hardware.send(display_insufficient_funds());
                _state=&ATM::done_processing;
            }
            )
        .handle<cancel_pressed>(
            [&](cancel_pressed const &)
            {
                _bank.send(
                    cancel_withdrawal(_account,_withdrawal_amount));
                _interface_hardware.send(
                    display_withdrawal_cancelled());
                _state=&ATM::done_processing;
            }
            );
}

void ATM::process_balance() {
    _incoming.wait()
        .handle<balance>(
            [&](balance const& msg)
            {
                _interface_hardware.send(display_balance(msg.amount));
                _state=&ATM::wait_for_action;
            }
            )
        .handle<cancel_pressed>(
            [&](cancel_pressed const &)
            {
                _state=&ATM::done_processing;
            }
            );
}

void ATM::wait_for_action() {
    _interface_hardware.send(display_withdrawal_options());
    _incoming.wait()
        .handle<withdraw_pressed>(
            [&](withdraw_pressed const& msg)
            {
                _withdrawal_amount=msg.amount;
                _bank.send(withdraw(_account,msg.amount,_incoming));
                _state=&ATM::process_withdrawal;
            }
            )
        .handle<balance_pressed>(
            [&](balance_pressed const &)
            {
                _bank.send(get_balance(_account,_incoming));
                _state=&ATM::process_balance;
            }
            )
        .handle<cancel_pressed>(
            [&](cancel_pressed const &)
            {
                _state=&ATM::done_processing;
            }
            );
}

void ATM::verifying_pin() {
    _incoming.wait()
        .handle<pin_verified>(
            [&](pin_verified const &)
            {
                _state=&ATM::wait_for_action;
            }
            )
        .handle<pin_incorrect>(
            [&](pin_incorrect const &)
            {
                _interface_hardware.send(
                    display_pin_incorrect_message());
                _state=&ATM::done_processing;
            }
            )
        .handle<cancel_pressed>(
            [&](cancel_pressed const &)
            {
                _state=&ATM::done_processing;
            }
            );
}

void ATM::getting_pin() {
    _incoming.wait()
        .handle<digit_pressed>(
            [&](digit_pressed const& msg)
            {
                unsigned const pin_length=4;
                _pin+=msg.digit;
                if(_pin.length()==pin_length)
                {
                    _bank.send(verify_pin(_account,_pin,_incoming));
                    _state=&ATM::verifying_pin;
                }
            }
            )
        .handle<clear_last_pressed>(
            [&](clear_last_pressed const &)
            {
                if(!_pin.empty())
                {
                    _pin.pop_back();
                }
            }
            )
        .handle<cancel_pressed>(
            [&](cancel_pressed const &)
            {
                _state=&ATM::done_processing;
            }
            );
}

void ATM::waiting_for_card() {
    _interface_hardware.send(display_enter_card());
    _incoming.wait()
        .handle<card_inserted>(
            [&](card_inserted const& msg)
            {
                _account = msg.account;
                _pin = "";
                _interface_hardware.send(display_enter_pin());
                _state=&ATM::getting_pin;
            }
            );
}

void ATM::done_processing() {
    _interface_hardware.send(eject_card());
    _state = &ATM::waiting_for_card;
}

void ATM::run() {
    _state = &ATM::waiting_for_card;
    try
    {
        for(;;)
        {
            (this->*_state)();
        }
    }
    catch(Messaging::CloseQueue const&)
    {
    }
}
