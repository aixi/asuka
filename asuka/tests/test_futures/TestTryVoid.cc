//
// Created by xi on 19-2-21.
//

#include <iostream>
#include <asuka/futures/Try.h>

using namespace asuka;

int func()
{
    return 1;
}

template <typename T, typename... Args>
int test(Try<T> t)
{
    return func(t.template Get<Args>()...);
}

int main()
{
    Try<void> t;
    std::cout << test<void>(t) << std::endl;
    return 0;
}

