//
// Created by xi on 19-2-15.
//

#ifndef ASUKA_EXCEPTION_H
#define ASUKA_EXCEPTION_H

#include <exception>
#include <memory>

#include <luna/Value.h>

#include <asuka/RpcError.h>

namespace asuka
{

class NotifyException : public std::exception
{
public:
    // sizeof(error) == 4, because default underlying type of scoped enum is int
    // so RpcError pass by value is OK
    NotifyException(RpcError error, const char* detail) :
        error_(error),
        detail_(detail)
    {}

    const char* what() const noexcept override
    {
        return error_.AsString();
    }

    RpcError error() const
    {
        return error_;
    }

    const char* detail() const
    {
        return detail_;
    }

private:
    RpcError error_;
    const char* detail_;
};

class RequestException : public std::exception
{
public:
    RequestException(RpcError error, const luna::Value& id, const char* detail) :
        error_(error),
        id_(new luna::Value(id)),
        detail_(detail)
    {}

    const char* what() const noexcept override
    {
        return error_.AsString();
    }

    RpcError error() const
    {
        return error_;
    }

    luna::Value& id()
    {
        return *id_;
    }

    const char* detail() const
    {
        return detail_;
    }

private:
    RpcError error_;
    std::unique_ptr<luna::Value> id_;
    const char* detail_;
};

class ResponseException : public std::exception
{
public:
    explicit ResponseException(const char* msg) :
        has_id_(false),
        id_(-1),
        msg_(msg)
    {}

    ResponseException(const char* msg, int32_t id) :
        has_id_(true),
        id_(id),
        msg_(msg)
    {}

    bool HasId() const
    {
        return has_id_;
    }

    int32_t id() const
    {
        return id_;
    }

private:
    const bool has_id_;
    const int32_t id_;
    const char* msg_;
};

class StubException : std::exception
{
public:
    explicit StubException(const char* msg) :
        msg_(msg)
    {}

    const char* what() const noexcept override
    {
        return msg_;
    }

private:
    const char* msg_;
};

} // namespace asuka

#endif //ASUKA_EXCEPTION_H
