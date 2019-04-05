#ifndef MESSAGES_H
#define MESSAGES_H

#include "messaging_sender.h"

struct Withdraw {
    std::string               account;
    unsigned                  amount;
    mutable Messaging::Sender atm_queue;

    Withdraw(const std::string & account_, unsigned amount_, Messaging::Sender atm_queue_)
    : account(account_), amount(amount_), atm_queue(atm_queue_)
    { }
};

struct WithdrawOk { };

struct WithdrawDenied { };

struct CancelWithdrawal {
    std::string account;
    unsigned    amount;

    CancelWithdrawal(const std::string & account_, unsigned amount_)
    : account(account_), amount(amount_)
    { }
};

struct WithdrawalProcessed {
    std::string account;
    unsigned    amount;

    WithdrawalProcessed(const std::string & account_, unsigned amount_)
    : account(account_), amount(amount_)
    { }
};

struct CardInserted {
    std::string account;

    explicit CardInserted(const std::string & account_)
    : account(account_)
    { }
};

struct DigitPressed {
    char digit;

    explicit DigitPressed(char digit_) : digit(digit_)
    { }
};

struct ClearLastPressed { };

struct EjectCard { };

struct WithdrawPressed {
    unsigned amount;

    explicit WithdrawPressed(unsigned amount_)
    : amount(amount_)
    { }
};

struct CancelPressed { };

struct IssueMoney {
    unsigned amount;

    IssueMoney(unsigned amount_) : amount(amount_)
    { }
};

struct VerifyPin {
    std::string               account;
    std::string               pin;
    mutable Messaging::Sender atm_queue;

    VerifyPin(const std::string & account_, const std::string & pin_,
               Messaging::Sender atm_queue_)
    : account(account_), pin(pin_), atm_queue(atm_queue_)
    { }
};

struct PinVerified { };

struct PinIncorrect { };

struct DisplayEnterPin { };

struct DisplayEnterCard { };

struct DisplayInsufficientFunds { };

struct DisplayWithdrawalCancelled { };

struct DisplayPinIncorrectMessage { };

struct DisplayWithdrawalOptions { };

struct GetBalance {
    std::string account;
    mutable Messaging::Sender atm_queue;

    GetBalance(const std::string & account_, Messaging::Sender atm_queue_)
    : account(account_), atm_queue(atm_queue_)
    { }
};

struct Balance {
    unsigned amount;

    explicit Balance(unsigned amount_) : amount(amount_)
    { }
};

struct DisplayBalance {
    unsigned amount;

    explicit DisplayBalance(unsigned amount_) : amount(amount_)
    { }
};

struct BalancePressed { };

#endif
