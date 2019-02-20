//
// Created by xi on 19-2-19.
//

#ifndef ASUKA_TRY_H
#define ASUKA_TRY_H

#include <assert.h>
#include <stdexcept>
#include <exception>

namespace asuka
{

template <typename T>
class Try
{
public:
    enum class State
    {
        kNone,
        kException,
        kValue
    };

    Try() : state_(State::kNone) {}

    explicit Try(const T& t) :
        state_(State::kValue),
        value_(t)
    {}

    explicit Try(T&& t) :
        state_(State::kValue),
        value_(std::move(t))
    {}

    explicit Try(std::exception_ptr e) :
        state_(State::kException),
        exception_(std::move(e))
    {}

    ~Try()
    {
        if (state_ == State::kException)
        {
            exception_.~exception_ptr();
        }
        else if (state_ == State::kValue)
        {
            value_.~T();
        }
    }

    // copy
    Try(const Try<T>& t) :
        state_(t.state_)
    {
        if (state_ == State::kValue)
        {
            new (&value_) T(t.value_);
        }
        else if (state_ == State::kException)
        {
            new (&exception_) std::exception_ptr(t.exception_);
        }
    }

    Try<T>& operator=(const Try<T>& t)
    {
        if (this == &t)
        {
            return *this;
        }
        this->~Try();
        state_ = t.state_;
        if (state_ == State::kValue)
        {
            new (&state_) T(t.value_);
        }
        else if (state_ == State::kException)
        {
            new (&exception_) std::exception_ptr(t.exception_);
        }
        return *this;
    }

    // move
    Try(Try<T>&& t) noexcept:
        state_(t.state_)
    {
        if (state_ == State::kValue)
        {
            new (&value_) T(std::move(t.value_));
        }
        else if (state_ == State::kException)
        {
            new (&exception_) std::exception_ptr(std::move(t.exception_));
        }
    }

    Try<T>& operator=(Try<T>&& t) noexcept
    {
        if (this == &t)
        {
            return *this;
        }
        this->~Try();
        state_ = t.state_;
        if (state_ == State::kValue)
        {
            new (&value_) T(std::move(t.value_));
        }
        else if (state_ == State::kException)
        {
            new (&exception_) std::exception_ptr(std::move(t.exception_));
        }
        return *this;
    }

    // implicitly conversion
    operator const T&() const &
    {
        return Value();
    }

    operator T&() &
    {
        return Value();
    }

    operator T&&() &&
    {
        return std::move(Value());
    }

    // get value_
    const T& Value() const &
    {
        Check();
        return value_;
    }

    T& Value() &
    {
        Check();
        return value_;
    }

    T&& Value() &&
    {
        Check();
        return std::move(value_);
    }

    // get exception_
    const std::exception_ptr& Exception() const &
    {
        if (!HasException())
        {
            throw std::runtime_error("Not exception state");
        }
        return exception_;
    }

    std::exception_ptr& Exception() &
    {
        if (!HasException())
        {
            throw std::runtime_error("Not exception state");
        }
        return exception_;
    }

    std::exception_ptr&& Exception() &&
    {
        if (!HasException())
        {
            throw std::runtime_error("Not exception state");
        }
        return std::move(exception_);
    }

    bool HasValue() const
    {
        return state_ == State::kValue;
    }

    bool HasException() const
    {
        return state_ == State::kException;
    };

    const T& operator*() const
    {
        return Value();
    }

    T& operator*()
    {
        return Value();
    }

    struct UninitializedTry {};

    void Check() const
    {
        // FIXME: hack
        if (state_ == State::kException)
        {
            std::rethrow_exception(exception_);
        }
        else if (state_ == State::kNone)
        {
            throw UninitializedTry();
        }
    }

    // FIXME: amazing hack
    template <typename R>
    R Get()
    {
        return std::forward<R>(Value());
    }

private:
    State state_;
    union
    {
        T value_;
        std::exception_ptr exception_;
    };
};

}

#endif //ASUKA_TRY_H
