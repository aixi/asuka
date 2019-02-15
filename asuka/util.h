//
// Created by xi on 19-2-15.
//

#ifndef ASUKA_UTIL_H
#define ASUKA_UTIL_H

#include <functional>
#include <blaze/utils/noncopyable.h>
#include <luna/Value.h>

namespace asuka
{

// response pass by value ?
using RpcDoneCallback = std::function<void (const luna::Value& response)>;

class UserDoneCallback : public blaze::noncopyable
{
public:
    UserDoneCallback(luna::Value& request, RpcDoneCallback callback) :
        request_(request),
        callback_(std::move(callback))
    {}

    void operator()(luna::Value&& result) const
    {
        luna::Value response(luna::ValueType::kObject);
        response.AddObjectElement("jsonrpc", "2.0");
        response.AddObjectElement("id", request_["id"]);
        response.AddObjectElement("result", result);
        callback_(response);
    }

private:
    mutable luna::Value request_;
    RpcDoneCallback callback_;
};

} // namespace asuka

#endif //ASUKA_UTIL_H
