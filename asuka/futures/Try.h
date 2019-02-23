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
    // if T is not a trivial class you need a destructor and constructor
    union
    {
        T value_;
        std::exception_ptr exception_;
    };
};


template <>
class Try<void>
{
public:
    enum class State
    {
        kException,
        kValue
    };

    Try() : state_(State::kValue) {}

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
    }

    // copy

    Try(const Try<void>& t) :
        state_(t.state_)
    {
        if (state_ == State::kException)
        {
            new (&exception_) std::exception_ptr(t.exception_);
        }
    }

    Try& operator=(const Try<void>& t)
    {
        if (this == &t)
        {
            return *this;
        }
        this->~Try();
        state_ = t.state_;
        if (state_ == State::kException)
        {
            new (&exception_) std::exception_ptr(t.exception_);
        }
        return *this;
    }

    // move

    Try(Try<void>&& t) noexcept :
        state_(t.state_)
    {
        if (state_ == State::kException)
        {
            new (&exception_) std::exception_ptr(std::move(t.exception_));
        }
    }

    Try& operator=(Try<void>&& t) noexcept
    {
        if (this == &t)
        {
            return *this;
        }
        this->~Try();
        state_ = t.state_;
        if (state_ == State::kException)
        {
            new (&exception_) std::exception_ptr(std::move(t.exception_));
        }
        return *this;
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
    }

    void Check() const
    {
        if (state_ == State::kException)
        {
            std::rethrow_exception(exception_);
        }
    }

    // FIXME: amazing hack
    template <typename R>
    R Get()
    {
        return std::forward<R>(*this);
    }

private:
    State state_;
    std::exception_ptr exception_;

};

// TryWrapper<T> : if T is Try type, then Type is T otherwise is Try<T>
template <typename T>
struct TryWrapper
{
    using Type = Try<T>;
};

// SFINAE

// Warp function f(...) return by Try<T>
template <typename F, typename... Args>
std::enable_if_t<!std::is_same_v<std::result_of_t<F(Args...)>, void>,
         typename TryWrapper<std::result_of_t<F(Args...)>>::Type>
WarpWithTry(F&& f, Args&&... args)
{
    using Type = std::result_of_t<F(Args...)>;
    try
    {
        return typename TryWrapper<Type>::Type(std::forward<F>(f)(std::forward<Args>(args)...));
    }
    catch (std::exception& e)
    {
        return typename TryWrapper<Type>::Type(std::current_exception());
    }
}

// Wrap void function f(...) return by Try<void>
template <typename F, typename... Args>
std::enable_if_t<std::is_same_v<std::result_of_t<F(Args...)>, void>, Try<void>>
WarpWithTry(F&& f, Args&&... args)
{
    try
    {
        std::forward<F>(f)(std::forward<Args>(args)...);
        return Try<void>();
    }
    catch (std::exception& e)
    {
        return Try<void>(std::current_exception());
    }
}

// f's arg is void, but return Type
// Warp return value of function Type f(void) by Try<Type>
template <typename F>
std::enable_if_t<!std::is_same_v<std::result_of_t<F()>, void>,
typename TryWrapper<std::result_of_t<F()>>::Type>
WarpWithTry(F&& f, Try<void>&& arg)
{
    using Type = std::result_of_t<F()>;
    try
    {
        return typename TryWrapper<Type>::Type(std::forward<F>(f)());
    }
    catch (std::exception& e)
    {
        return typename TryWrapper<Type>::Type(std::current_exception());
    }
}

// Wrap return value of function void f(void) by Try<void>
template <typename F>
std::enable_if_t<std::is_same_v<std::result_of_t<F()>, void>,
Try<std::result_of_t<F()>>>
WrapWithTry(F&& f, Try<void>&& arg)
{
    try
    {
        std::forward<F>(f)();
        return Try<void>();
    }
    catch (std::exception& e)
    {
        return Try<void>(std::current_exception());
    }
}

} // namespace asuka

#endif //ASUKA_TRY_H
