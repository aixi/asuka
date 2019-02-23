//
// Created by xi on 19-2-19.
//

#ifndef ASUKA_FUTURE_H
#define ASUKA_FUTURE_H

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <type_traits>

#include <asuka/utils/Scheduler.h>
#include <asuka/futures/Helper.h>
#include <asuka/futures/Try.h>

namespace asuka
{

namespace detail
{

enum class Progress
{
    kNone,
    kTimeout,
    kDone,
    kRetrieved
};

using TimeoutCallback = std::function<void ()>;

template <typename T>
struct State
{
    static_assert(std::is_same_v<T, void> ||
                  std::is_copy_constructible_v<T> ||
                  std::is_move_constructible_v<T>,
                  "T must be copyable or movable or void");

    using ValueType = typename TryWrapper<T>::Type;

    State() :
        progress_(Progress::kNone),
        retrieved_(false)
    {}

    Progress progress_;

    std::atomic<bool> retrieved_;

    ValueType value_;

    std::mutex then_mutex_;

    std::function<void (TimeoutCallback&& )> on_timeout_;

    std::function<void (ValueType&& )> then_;

};

} // namespace detail

template <typename T>
class Future;

template <typename T>
class Promise
{
public:
    Promise() :
        state_(std::make_shared<detail::State<T>>())
    {}

    // The lambda with movable capture can not be stored in
    // std::function, just for compiler, do NOT copy Promise !!!!!!!!
    Promise(const Promise&) = default;
    Promise& operator=(const Promise&) = default;

    // Not always noexcept
    Promise(Promise&& promise) = default;
    Promise& operator=(Promise&& promise) = default;

    void SetException(std::exception_ptr e)
    {
        {
            std::lock_guard<std::mutex> lock(state_->then_mutex_);
            if (state_->progress_ != detail::Progress::kNone)
            {
                return;
            }
            state_->progress_ = detail::Progress::kDone;
            state_->value_ = typename detail::State<T>::ValueType(std::move(e));
        }
        if (state_->then_)
        {
            state_->then_(std::move(state_->value_));
        }
    }

    template <typename U = T>
    std::enable_if_t<!std::is_void_v<U>, void> SetValue(U&& u)
    {
        // if ThenImpl is running, wait for the lock.
        // After set then_, ThenImpl will release lock.
        // and this func get lock, it will call then_.
        {
            std::lock_guard<std::mutex> lock(state_->then_mutex_);
            if (state_->progress_ != detail::Progress::kNone)
            {
                return;
            }
            state_->progress_ = detail::Progress::kDone;
            state_->value_ = std::forward<U>(u);
        }
        // When reach here, state_ has been set, so mutex is useless
        // if ThenImpl func runs, it'll see KDone state and
        // call user callback there, not assign to then_
        if (state_->then_)
        {
            state_->then_(std::move(state_->value_));
        }
    }

    template <typename U = T>
    std::enable_if_t<!std::is_same_v<U, void>, void> SetValue(const U& u)
    {
        // if ThenImpl is running, wait for the lock.
        // After set then_, ThenImpl will release lock.
        // and this func get lock, it will call then_.
        {
            std::lock_guard<std::mutex> lock(state_->then_mutex_);
            if (state_->progress_ != detail::Progress::kNone)
            {
                return;
            }
            state_->progress_ = detail::Progress::kDone;
            state_->value_ = u;
        }
        // When reach here, state_ has been set, so mutex is useless
        // if ThenImpl func runs, it'll see KDone state and
        // call user callback there, not assign to then_
        if (state_->then_)
        {
            state_->then_(std::move(state_->value_));
        }
    }

    template <typename U = T>
    std::enable_if_t<!std::is_same_v<U, void>, void> SetValue(Try<U>&& t)
    {
        // if ThenImpl is running, wait for the lock.
        // After set then_, ThenImpl will release lock.
        // and this func get lock, it will call then_.
        {
            std::lock_guard<std::mutex> lock(state_->then_mutex_);
            if (state_->progress_ != detail::Progress::kNone)
            {
                return;
            }
            state_->progress_ = detail::Progress::kDone;
            state_->value_ = std::forward<Try<U>>(t);
        }
        // When reach here, state_ has been set, so mutex is useless
        // if ThenImpl func runs, it'll see KDone state and
        // call user callback there, not assign to then_
        if (state_->then_)
        {
            state_->then_(std::move(state_->value_));
        }
    }

    template <typename U = T>
    std::enable_if_t<!std::is_same_v<U, void>, void> SetValue(const Try<U>& t)
    {
        // if ThenImpl is running, wait for the lock.
        // After set then_, ThenImpl will release lock.
        // and this func get lock, it will call then_.
        {
            std::lock_guard<std::mutex> lock(state_->then_mutex_);
            if (state_->progress_ != detail::Progress::kNone)
            {
                return;
            }
            state_->progress_ = detail::Progress::kDone;
            state_->value_ = t;
        }
        // When reach here, state_ has been set, so mutex is useless
        // if ThenImpl func runs, it'll see KDone state and
        // call user callback there, not assign to then_
        if (state_->then_)
        {
            state_->then_(std::move(state_->value_));
        }
    }

    template <typename U = T>
    std::enable_if_t<std::is_same_v<U, void>, void> SetValue(Try<void>&& )
    {
        // if ThenImpl is running, wait for the lock.
        // After set then_, ThenImpl will release lock.
        // and this func get lock, it will call then_.
        {
            std::lock_guard<std::mutex> lock(state_->then_mutex_);
            if (state_->progress_ != detail::Progress::kNone)
            {
                return;
            }
            state_->progress_ = detail::Progress::kDone;
            state_->value_ = Try<void>();
        }
        // When reach here, state_ has been set, so mutex is useless
        // if ThenImpl func runs, it'll see KDone state and
        // call user callback there, not assign to then_
        if (state_->then_)
        {
            state_->then_(std::move(state_->value_));
        }
    }

    template <typename U = T>
    std::enable_if_t<std::is_same_v<U, void>, void> SetValue(const Try<void>& )
    {
        // if ThenImpl is running, wait for the lock.
        // After set then_, ThenImpl will release lock.
        // and this func get lock, it will call then_.
        {
            std::lock_guard<std::mutex> lock(state_->then_mutex_);
            if (state_->progress_ != detail::Progress::kNone)
            {
                return;
            }
            state_->progress_ = detail::Progress::kDone;
            state_->value_ = Try<void>();
        }
        // When reach here, state_ has been set, so mutex is useless
        // if ThenImpl func runs, it'll see KDone state and
        // call user callback there, not assign to then_
        if (state_->then_)
        {
            state_->then_(std::move(state_->value_));
        }
    }

    template <typename U = T>
    std::enable_if_t<std::is_same_v<U, void>, void> SetValue()
    {
        // if ThenImpl is running, wait for the lock.
        // After set then_, ThenImpl will release lock.
        // and this func get lock, it will call then_.
        {
            std::lock_guard<std::mutex> lock(state_->then_mutex_);
            if (state_->progress_ != detail::Progress::kNone)
            {
                return;
            }
            state_->progress_ = detail::Progress::kDone;
            state_->value_ = Try<void>();
        }
        // When reach here, state_ has been set, so mutex is useless
        // if ThenImpl func runs, it'll see KDone state and
        // call user callback there, not assign to then_
        if (state_->then_)
        {
            state_->then_(std::move(state_->value_));
        }
    }

    Future<T> GetFuture()
    {
        bool expect = false;
        // FIXME: memory order relaxed ?
        if (!state_->retrieved_.compare_exchange_strong(expect, true))
        {
            throw std::runtime_error("Future already retrieved");
        }
        return Future<T>(state_);
    }

    bool IsReady() const
    {
        return state_->progress_ != detail::Progress::kNone;
    }

private:
    std::shared_ptr<detail::State<T>> state_;
};

template <typename T2>
Future<T2> MakeExceptionFuture(std::exception_ptr&& e);

template <typename T>
class Future
{
public:
    template <typename U>
    friend class Future;

    using InnerType = T;

    Future() = default;

    // non-copyable
    Future(const Future&) = delete;
    Future& operator=(const Future&) = delete;

    // movable
    Future(Future&& fut) = default;
    Future& operator=(Future&& fut) = default;

    explicit Future(std::shared_ptr<detail::State<T>> state) :
        state_(std::move(state))
    {}

    // Attention: deadlock !!!
    // Wait thread shall NOT be same Promise thread !!!
    typename detail::State<T>::ValueType
    Wait(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(24 * 3600 * 1000))
    {
        {
            std::lock_guard<std::mutex> lock(state_->then_mutex_);
            switch (state_->progress_)
            {
                case detail::Progress::kNone:
                    break;
                case detail::Progress::kTimeout:
                    throw std::runtime_error("Future timeout");
                case detail::Progress::kDone:
                    state_->progress_ = detail::Progress::kRetrieved;
                    return std::move(state_->value_);
                default:
                    throw std::runtime_error("Future already retrieved");
            }
        }
        // avoid invalid reference in Then lambda capture list
        std::shared_ptr<std::condition_variable> cond(std::make_shared<std::condition_variable>());
        std::shared_ptr<std::mutex> mutex(std::make_shared<std::mutex>());
        bool ready = false;
        typename detail::State<T>::ValueType value;
        this->Then([&value, &ready,
                    wcond = std::weak_ptr<std::condition_variable>(cond),
                    wmutex = std::weak_ptr<std::mutex>(mutex)] (typename detail::State<T>::ValueType&& v)
                   {
                        std::shared_ptr<std::condition_variable> cond1 = wcond.lock();
                        std::shared_ptr<std::mutex> mutex1 = wmutex.lock();
                        if (!cond1 || !mutex1)
                        {
                            return;
                        }
                        std::lock_guard<std::mutex> lock(*mutex1);
                        value = std::move(v);
                        ready = true;
                        cond1->notify_one();
                   });
        std::unique_lock<std::mutex> lock2(*mutex);
        bool success = cond->wait_for(lock2, timeout, [&ready]{ return ready;});
        if (success)
        {
            return std::move(value);
        }
        else
        {
            throw std::runtime_error("Future wait_for timeout");
        }
    }

private:

    void SetCallback(std::function<void (typename TryWrapper<T>::Type&& )>&& func)
    {
        state_->then_ = std::move(func);
    }

    void SetOnTimeout(std::function<void (detail::TimeoutCallback&& )>&& func)
    {
        state_->on_timeout_ = std::move(func);
    }


private:
    std::shared_ptr<detail::State<T>> state_;
}; // class Future

} // namespace asuka

#endif //ASUKA_FUTURE_H
