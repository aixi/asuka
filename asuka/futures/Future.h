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
    template<typename U>
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
                       wmutex = std::weak_ptr<std::mutex>(mutex)](typename detail::State<T>::ValueType&& v) {
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
        // no need for while the wait_for deal spurious awakenings
        bool success = cond->wait_for(lock2, timeout, [&ready] { return ready; });
        if (success)
        {
            return std::move(value);
        } else
        {
            throw std::runtime_error("Future wait_for timeout");
        }
    }

    // T is of type Future<InnerType>
    template<typename U = T>
    std::enable_if_t<detail::IsFuture<U>::value, U> Unwrap()
    {
        using Inner = typename detail::IsFuture<U>::Inner;
        static_assert(std::is_same_v<U, Future<Inner>>, "U is same Future<InnerType>");
        Promise<Inner> promise;
        Future<Inner> future = promise.GetFuture();

        std::lock_guard<std::mutex> lock(state_->then_mutex_);
        if (state_->progress_ == detail::Progress::kTimeout)
        {
            throw std::runtime_error("Wrong state: Timeout");
        } else if (state_->progress_ == detail::Progress::kDone)
        {
            try
            {
                auto inner_future = std::move(state_->value_);
                return std::move(inner_future.Value());
            }
            catch (const std::exception& e)
            {
                return MakeExceptionFuture<Inner>(std::current_exception());
            }
        } else
        {
            SetCallback([pm = std::move(promise)](typename TryWrapper<U>::Type&& inner_fut) mutable {
                try
                {
                    U fut = std::move(inner_fut);
                    fut.SetCallback([pm = std::move(pm)](typename TryWrapper<Inner>::Type&& t) mutable {
                        // No need scheduler here, think about following code
                        // outer.Unwrap().Then(sched, func);
                        // outer.Unwrap() is the inner future, the below line
                        // will trigger func in sched thread
                        pm.SetValue(std::move(t));
                    });
                }
                catch (...)
                {
                    pm.SetException(std::current_exception());
                }
            });
        }
        return future;
    }

    template<typename F, typename R = detail::CallableResult<F, T>>
    typename R::ReturnFutureType Then(F&& f)
    {
        using Arguments = typename R::Arg;
        return ThenImpl<F, R>(nullptr, std::forward<F>(f), Arguments());
    }

    // f will be called in sched
    template<typename F, typename R = detail::CallableResult<F, T>>
    typename R::ReturnFutureType Then(Scheduler* sched, F&& f)
    {
        using Arguments = typename R::Arg;
        return ThenImpl<F, R>(sched, std::forward<F>(f), Arguments());
    }

    // 1. F does not return future type
    template<typename F, typename R, typename... Args>
    std::enable_if_t<!R::IsReturnFuture::value, typename R::ReturnFutureType>
    ThenImpl(Scheduler* sched, F&& f, detail::ResultOfWrapper<F, Args...>)
    {
        static_assert(sizeof...(Args) <= 1, "Then must take zero or one argument");
        using FReturnType = typename R::IsReturnFuture::Inner;
        Promise<FReturnType> pm;
        auto next_future = pm.GetFuture();
        using FuncType = std::decay_t<F>;
        std::unique_lock<std::mutex> lock(state_->then_mutex_);
        if (state_->progress_ == detail::Progress::kTimeout)
        {
            throw std::runtime_error("Wrong state: timeout");
        } else if (state_->progress_ == detail::Progress::kDone)
        {
            typename TryWrapper<T>::Type t;
            try
            {
                t = std::move(state_->value_);
            }
            catch (const std::exception& e)
            {
                t = static_cast<typename TryWrapper<T>::Type>(std::current_exception());
            }
            lock.unlock();
            if (sched)
            {
                sched->Schedule([t2 = std::move(t),
                                    f2 = std::forward<FuncType>(f),
                                    pm2 = std::move(pm)]() mutable {
                    auto result = WrapWithTry(f2, std::move(t2));
                    pm2.SetValue(std::move(result));
                });
            } else
            {
                auto result = WrapWithTry(f, std::move(t));
                pm.SetValue(std::move(result));
            }
        } else
        {
            // set this futures's then callback
            SetCallback([sched,
                            func = std::forward<FuncType>(f),
                            prom = std::move(pm)](typename TryWrapper<T>::Type&& t) mutable {
                if (sched)
                {
                    sched->Schedule([func3 = std::move(func),
                                        t3 = std::move(t),
                                        prom3 = std::move(prom)]() mutable {
                        // run callback, T can be void
                        auto result = WrapWithTry(func, std::move(t3));
                        prom3.SetValue(std::move(result));
                    });
                } else
                {
                    // run callback, T can be void
                    auto result = WrapWithTry(func, std::move(t));
                    // set next future's result
                    prom.SetValue(std::move(result));
                }
            });
        }
        return std::move(next_future);
    }

    // 2. F return another future type
    template<typename F, typename R, typename... Args>
    std::enable_if_t<R::IsReturnFuture::value, typename R::ReturnFutureType>
    ThenImpl(Scheduler* sched, F&& f, detail::ResultOfWrapper<F, Args...>)
    {
        static_assert(sizeof...(Args) <= 1, "Then must take zero or one argument");
        using FReturnType = typename R::IsReturnFuture::Inner;
        Promise<FReturnType> pm;
        auto next_future = pm.GetFuture();
        using FuncType = std::decay_t<F>;
        std::unique_lock<std::mutex> lock(state_->then_mutex_);
        if (state_->progress_ == detail::Progress::kTimeout)
        {
            throw std::runtime_error("Wrong state: Timeout");
        }
        else if (state_->progress_ == detail::Progress::kDone)
        {
            typename TryWrapper<T>::Type t;
            try
            {
                t = std::move(state_->value_);
            }
            catch (const std::exception& e)
            {
                t = decltype(t)(std::current_exception());
            }
            lock.unlock();
            auto cb =
                   [res = std::move(t),
                   f2 = std::forward<FuncType>(f),
                   prom = std::move(pm)]() mutable
                   {
                   // because func return another future: innerFuture,
                   // when innerFuture is done, next future can be done
                       decltype(f2(res.template Get<Args>()...)) inner_future;
                       if (res.HasException())
                       {
                           // FIXME: Failed if Args... is void
                           inner_future = f2(typename TryWrapper<std::decay_t<Args>...>::Type(res.Exception()));
                       }
                       else
                       {
                           inner_future = f2(res.template Get<Args>...);
                       }
                       std::unique_lock<std::mutex> lock2(inner_future.state_->then_mutex_);
                       if (inner_future.state_->progress == detail::Progress::kTimeout)
                       {
                           throw std::runtime_error("Wrong state: Timeout");
                       }
                       else if (inner_future.state_->progress == detail::Progress::kDone)
                       {
                           typename TryWrapper<FReturnType>::Type t3;
                           try
                           {
                               t3 = std::move(inner_future.state_->value_);
                           }
                           catch (const std::exception& e)
                           {
                               t3 = decltype(t)(std::current_exception());
                           }
                           lock2.unlock();
                           prom.SetValue(std::move(t3));
                       }
                       else
                       {
                           inner_future.SetCallback([prom = std::move(prom)]
                                        (typename TryWrapper<FReturnType>::Type&& t) mutable
                                        {
                                            prom.SetValue(std::move(t));
                                        });
                       }
               };
            if (sched)
            {
                sched->Schedule(std::move(cb));
            }
            else
            {
                cb();
            }
        }
        else
        {
            // set this future's then callback
            SetCallback([sched = sched, func = std::forward<FuncType>(f), prom = std::move(pm)]
             (typename TryWrapper<T>::Type&& t) mutable
             {
                 auto cb = [func = std::move(func), t = std::move(t), prom = std::move(prom)] () mutable
                 {
                     // because func return another future: innerFuture,
                     // when innerFuture is done, next future can be done
                     decltype(func(t.template Get<Args>()...)) inner_future;
                     if (t.HasException())
                     {
                         // FIXME: Failed if Args... is void
                         inner_future = func(typename TryWrapper<std::decay_t<Args>...>::Type(t.Exception()));
                     }
                     else
                     {
                         inner_future = func(t.template Get<Args>()...);
                     }
                     std::unique_lock<std::mutex> lock3(inner_future.state_->then_mutex_);
                     if (inner_future.state_->progress == detail::Progress::kTimeout)
                     {
                         throw std::runtime_error("Wrong state: Timeout");
                     }
                     else if (inner_future.state_->progress == detail::Progress::kDone)
                     {
                         typename TryWrapper<FReturnType>::Type t3;
                         try
                         {
                             t3 = std::move(inner_future.state_->value_);
                         }
                         catch (const std::exception& e)
                         {
                             t3 = decltype(t3)(std::current_exception());
                         }
                         lock3.unlock();
                         prom.SetValue(std::move(t3));
                     }
                     else
                     {
                         inner_future.SetCallback([prom = std::move(prom)]
                                      (typename TryWrapper<FReturnType>::Type&& t) mutable
                                      {
                                          prom.SetValue(std::move(t));
                                      });
                     }
                 };
                 if (sched)
                 {
                     sched->Schedule(std::move(cb));
                 }
                 else
                 {
                     cb();
                 }
             });
        }
        return std::move(next_future);
    }

    // When register callbacks and timeout for a future like this:
    //               Future<int> f;
    //               f.Then(xx).OnTimeout(yy)
    // There will be one future object created except f, we call f as root future.
    // The yy callback is registered on the last future, here are the possibilities:
    // 1. xx is called, and yy is not called.
    // 2. xx is not called, and yy is called
    // But be careful below !!!!
    //               Future<int> f;
    //               f.Then(xx).Then(yy).OnTimeout(zz);
    // There will be 3 future object created except f, we call f as root future.
    // The zz callback is registered on the last future, here are the possibilities
    // 1. xx is called, zz is called, yy is not called,
    // 2. xx and yy are called, and zz is called, it's rarely happen but...
    // 3. xx and yy are called, it's the normal case.
    // So you may shouldn't use OnTimeout with chained futures !!!

    void OnTimeout(std::chrono::milliseconds duration, detail::TimeoutCallback f, Scheduler* sched)
    {
        sched->SchedulerLater(duration, [state_ = state_, cb = std::move(f)]() mutable
        {
            std::unique_lock<std::mutex> lock(state_->then_mutex_);
            if (state_->progress_ == detail::Progress::kNone)
            {
                return;;
            }
            state_->progress_ = detail::Progress::kTimeout;
            lock.unlock();
            cb();
        });
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

// Make ready future
template <typename T2>
inline Future<std::decay_t<T2>> MakeReadyFuture(T2&& value)
{
    Promise<std::decay_t<T2>> pm;
    auto fut(pm.GetFuture());
    pm.SetValue(std::forward<T2>(value));
    return fut;
}

inline Future<void> MakeReadyFuture()
{
    Promise<void> pm;
    auto fut(pm.GetFuture());
    pm.SetValue();
    return fut;
};

// Make exception future
template <typename T2, typename E>
inline Future<T2> MakeExceptionFuture(E&& e)
{
    Promise<T2> pm;
    pm.SetException(std::make_exception_ptr(std::forward<E>(e)));
    return pm.GetFuture();
}

template <typename T2>
inline Future<T2> MakeExceptionFuture(std::exception_ptr& eptr)
{
    Promise<T2> pm;
    pm.SetException(std::move(eptr));
    return pm.GetFuture();
}

// TODO

// WhenAll

// WhenAny

// WhenN

// WhenIfAny

// WhenIfN



} // namespace asuka

#endif //ASUKA_FUTURE_H
