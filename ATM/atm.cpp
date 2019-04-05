#include "atm.h"
#include "messages.h"

void ATM::process_withdrawal() {
    _incoming.wait()
        .handle<WithdrawOk>(
            [&](const WithdrawOk &)
            {
                _interface_hardware.send(IssueMoney(_withdrawal_amount));
                _bank.send(WithdrawalProcessed(_account, _withdrawal_amount));
                _state = &ATM::done_processing;
            })
        .handle<WithdrawDenied>(
            [&](const WithdrawDenied &)
            {
                _interface_hardware.send(DisplayInsufficientFunds());
                _state = &ATM::done_processing;
            })
        .handle<CancelPressed>(
            [&](const CancelPressed &)
            {
                _bank.send(CancelWithdrawal(_account, _withdrawal_amount));
                _interface_hardware.send(DisplayWithdrawalCancelled());
                _state = &ATM::done_processing;
            });
}

void ATM::process_balance() {
    _incoming.wait()
        .handle<Balance>(
            [&](const Balance & msg)
            {
                _interface_hardware.send(DisplayBalance(msg.amount));
                _state = &ATM::wait_for_action;
            })
        .handle<CancelPressed>(
            [&](const CancelPressed &)
            {
                _state = &ATM::done_processing;
            });
}

void ATM::wait_for_action() {
    _interface_hardware.send(DisplayWithdrawalOptions());
    _incoming.wait()
        .handle<WithdrawPressed>(
            [&](const WithdrawPressed & msg)
            {
                _withdrawal_amount = msg.amount;
                _bank.send(Withdraw(_account, msg.amount, _incoming));
                _state = &ATM::process_withdrawal;
            })
        .handle<BalancePressed>(
            [&](const BalancePressed &)
            {
                _bank.send(GetBalance(_account, _incoming));
                _state = &ATM::process_balance;
            })
        .handle<CancelPressed>(
            [&](const CancelPressed &)
            {
                _state = &ATM::done_processing;
            });
}

void ATM::verifying_pin() {
    _incoming.wait()
        .handle<PinVerified>(
            [&](const PinVerified &)
            {
                _state = &ATM::wait_for_action;
            })
        .handle<PinIncorrect>(
            [&](const PinIncorrect &)
            {
                _interface_hardware.send(DisplayPinIncorrectMessage());
                _state = &ATM::done_processing;
            })
        .handle<CancelPressed>(
            [&](const CancelPressed &)
            {
                _state = &ATM::done_processing;
            });
}

void ATM::getting_pin() {
    _incoming.wait()
        .handle<DigitPressed>(
            [&](const DigitPressed & msg)
            {
                unsigned const pin_length = 4;
                _pin += msg.digit;

                if(_pin.length() == pin_length) {
                    _bank.send(VerifyPin(_account, _pin, _incoming));
                    _state = &ATM::verifying_pin;
                }
            }
            )
        .handle<ClearLastPressed>(
            [&](const ClearLastPressed &)
            {
                if(!_pin.empty()) {
                    _pin.pop_back();
                }
            })
        .handle<CancelPressed>(
            [&](const CancelPressed &)
            {
                _state = &ATM::done_processing;
            });
}

void ATM::waiting_for_card() {
    _interface_hardware.send(DisplayEnterCard());
    _incoming.wait()
        .handle<CardInserted>(
            [&](const CardInserted & msg)
            {
                _account = msg.account;
                _pin = "";
                _interface_hardware.send(DisplayEnterPin());
                _state = &ATM::getting_pin;
            });
}

void ATM::done_processing() {
    _interface_hardware.send(EjectCard());
    _state = &ATM::waiting_for_card;
}

void ATM::run() {
    _state = &ATM::waiting_for_card;
    try {
        while (true) {
            (this->*_state)();
        }
    }
    catch(const Messaging::CloseQueue &) {
    }
}
