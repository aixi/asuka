#include <iostream>
#include <asuka/RpcError.h>
#include <asuka/Exception.h>
#include <asuka/util.h>
#include <luna/Value.h>

using namespace asuka;
using namespace luna;

int main()
{
    RpcError error(-32700);
    NotifyException exception1(error, "test_error1");
    std::cout << exception1.what() << "-------" << exception1.detail() << '\n';
    Value value(false);
    RequestException exception2(error, value, "test_error2");
    std::cout << exception2.what() << "----" << exception2.detail();
    RpcDoneCallback cb = [](const Value& json){};
    UserDoneCallback user_done_callback(value, cb);
    user_done_callback(Value(true));
    return 0;
}