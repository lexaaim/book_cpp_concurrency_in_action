#include "interface_machine.h"
#include "messages.h"
#include <iostream>

void InterfaceMachine::run() {
    try
    {
        for(;;)
        {
            _incoming.wait()
                .handle<issue_money>(
                    [&](issue_money const& msg)
                    {
                        {
                            std::lock_guard<std::mutex> lk(_iomutex);
                            std::cout<<"Issuing "
                                     <<msg.amount<<std::endl;
                        }
                    }
                    )
                .handle<display_insufficient_funds>(
                    [&](display_insufficient_funds const &)
                    {
                        {
                            std::lock_guard<std::mutex> lk(_iomutex);
                            std::cout<<"Insufficient funds"<<std::endl;
                        }
                    }
                    )
                .handle<display_enter_pin>(
                    [&](display_enter_pin const &)
                    {
                        {
                            std::lock_guard<std::mutex> lk(_iomutex);
                            std::cout
                                <<"Please enter your PIN (0-9)"
                                <<std::endl;
                        }
                    }
                    )
                .handle<display_enter_card>(
                    [&](display_enter_card const &)
                    {
                        {
                            std::lock_guard<std::mutex> lk(_iomutex);
                            std::cout<<"Please enter your card (I)"
                                     <<std::endl;
                        }
                    }
                    )
                .handle<display_balance>(
                    [&](display_balance const & msg)
                    {
                        {
                            std::lock_guard<std::mutex> lk(_iomutex);
                            std::cout
                                <<"The balance of your account is "
                                <<msg.amount<<std::endl;
                        }
                    }
                    )
                .handle<display_withdrawal_options>(
                    [&](display_withdrawal_options const &)
                    {
                        {
                            std::lock_guard<std::mutex> lk(_iomutex);
                            std::cout<<"Withdraw 50? (w)"<<std::endl;
                            std::cout<<"Display Balance? (b)"
                                     <<std::endl;
                            std::cout<<"Cancel? (c)"<<std::endl;
                        }
                    }
                    )
                .handle<display_withdrawal_cancelled>(
                    [&](display_withdrawal_cancelled const &)
                    {
                        {
                            std::lock_guard<std::mutex> lk(_iomutex);
                            std::cout<<"Withdrawal cancelled"
                                     <<std::endl;
                        }
                    }
                    )
                .handle<display_pin_incorrect_message>(
                    [&](display_pin_incorrect_message const &)
                    {
                        {
                            std::lock_guard<std::mutex> lk(_iomutex);
                            std::cout<<"PIN incorrect"<<std::endl;
                        }
                    }
                    )
                .handle<eject_card>(
                    [&](eject_card const &)
                    {
                        {
                            std::lock_guard<std::mutex> lk(_iomutex);
                            std::cout<<"Ejecting card"<<std::endl;
                        }
                    }
                    );
        }
    }
    catch(Messaging::CloseQueue &)
    {
    }
}
