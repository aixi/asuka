//
// Created by xi on 19-2-15.
//

#ifndef ASUKA_RPCERROR_H
#define ASUKA_RPCERROR_H

#include <assert.h>

namespace asuka
{

// NOTE: enum class can not implicitly cast to integer type

template<typename E>
constexpr std::underlying_type_t<E> ToUType(E enumerator) noexcept
{
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

enum class Error
{
    kParseError,
    kInvalidRequest,
    kMethodNotFound,
    kInvalidParams,
    kInternalError
};

class RpcError
{
public:
    // implicit conversion is OK, explicit is not needed
    RpcError(Error error) :
        error_(error)
    {}

    explicit RpcError(int32_t code) :
        RpcError(FromErrorCode(code))
    {}

    const char* AsString() const
    {
        return error_str[ToUType(error_)];
    }

    int32_t AsCode() const
    {
        return error_code[ToUType(error_)];
    }

private:

#pragma GCC diagnostic ignored "-Wreturn-type"
    static Error FromErrorCode(int32_t code)
    {
        switch (code)
        {
            case -32700:
                return Error::kParseError;
            case -32600:
                return Error::kInvalidRequest;
            case -32601:
                return Error::kMethodNotFound;
            case -32602:
                return Error::kInvalidParams;
            case -32603:
                return Error::kInternalError;
        }
    }
#pragma GCC diagnostic error "-Wreturn-type"

private:
    const Error error_;
    static const int32_t error_code[];
    static const char* error_str[];
};

inline const int32_t RpcError::error_code[] = {
    -32700, -32600, -32601, -32602, -32603
};

inline const char* RpcError::error_str[] = {
    "Parse error",
    "Invalid request",
    "Method not found",
    "Invalid params",
    "Internal error"
};

}

#endif //ASUKA_RPCERROR_H
