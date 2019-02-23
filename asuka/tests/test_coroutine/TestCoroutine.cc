//
// Created by xi on 19-2-19.
//

#include <assert.h>
#include <iostream>
#include <string>

#include <asuka/coroutine/Coroutine.h>

using namespace asuka;

// A coroutine: simply twice the input integer and return

int TimesTwo(int input)
{
    std::cout << "Coroutine TimesTwo--- input: " << input << std::endl;
    std::cout << "Coroutine TimesTwo: return to main" << std::endl;
    std::shared_ptr<void> msg = Coroutine::Yield(std::make_shared<std::string>("I am calculating, please wait..."));
    std::cout << "Coroutine TimesTwo is resumed from main\n";
    std::cout << "Coroutine TimesTwo receive: " << *std::static_pointer_cast<std::string>(msg) << std::endl;
    std::cout << "Exit " << __FUNCTION__ << std::endl;
    return input * 2;
}

int main()
{
    const int input = 42;

    CoroutinePtr coroutine(Coroutine::CreateCoroutine(TimesTwo, input));
    VoidPtr reply = Coroutine::Send(coroutine);
    std::cout << "main()-- got reply message: " << *std::static_pointer_cast<std::string>(reply) << "----" << std::endl;
    VoidPtr final_result = Coroutine::Send(coroutine, std::make_shared<std::string>("final result"));
    std::cout << "the answer is twice of " << input << "--- " << *std::static_pointer_cast<int>(final_result) << std::endl;
    return 0;
}