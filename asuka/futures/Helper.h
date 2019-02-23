//
// Created by xi on 19-2-19.
//

#ifndef ASUKA_HELPER_H
#define ASUKA_HELPER_H

#include <tuple>
#include <vector>
#include <memory>
#include <mutex>

namespace asuka
{

template <typename T>
class Future;

template <typename T>
class Promise;

template <typename T>
class Try;

template <typename T>
struct TryWrapper;

namespace detail
{

// FIXME: std::result_of ?
// why it is needed ?

template <typename F, typename... Args>
using ResultOf = decltype(std::declval<F>()(std::declval<Args>()...));

template <typename F, typename... Args>
struct ResultOfWrapper
{
    using Type = ResultOf<F, Args...>;
};

// test if F can be called with Args type
template <typename F, typename... Args>
struct CanCallWith
{
    // SFINAE Check
    template <typename T, typename Dummy = ResultOf<T, Args...>>
    static constexpr std::true_type Check(std::nullptr_t dummy)
    {
        return std::true_type{};
    }

    template <typename Dummy>
    static constexpr std::false_type Check(...)
    {
        return std::false_type{};
    }

    using type = decltype(Check<F>(nullptr));
    // using std::true_type = std::integral_constant<bool, true>;
    // using std::false_type = std::integral_constant<bool, false>;
    static constexpr bool value = type::value;
};

// simple traits
template <typename T>
struct IsFuture : std::false_type
{
    using Inner = T;
};

// compiler prefer specilization to generic
template <typename T>
struct IsFuture<Future<T>> : std::true_type
{
    using Inner = T;
};

template <typename F, typename T>
struct CallableResult
{
    // Test F call with arg type: void, T&&, T&, but do NOT choose Try (in Try.h) type a args
    using Arg = typename std::conditional_t
        <
        CanCallWith<F>::value, // if true, F can call with void,
        ResultOfWrapper<F>,
        typename std::conditional_t // No, F(void) is invalid
            <
            CanCallWith<F, T&&>::value, // if true, F(T&&) is valid
            ResultOfWrapper<F, T&&>, // Yes, F(T&&) is OK
            ResultOfWrapper<F, T&> // above all failed, resort to F(T&)
            >
        >;

    using Arg_t = typename Arg::Type;
    // if ReturnsFuture::value is true, F returns another future type
    using IsReturnFuture = IsFuture<Arg_t>;

    // Future callback's result must be wrapped in another future
    using ReturnFutureType = Future<typename IsReturnFuture::Inner>;
};

// CallableResult specilization for void type
// I don't know why folly works without this
template <typename F>
struct CallableResult<F, void>
{
    // Test F call with arg type: void or Try(void)
    using Arg = typename std::conditional_t
        <
        CanCallWith<F>::value, // if true, F can call with void
        ResultOfWrapper<F>,
        typename std::conditional_t // No, F(void) is invalid
            <
            CanCallWith<F, Try<void>&&>::type, // if true, F(Try<void>&&) is valid
            ResultOfWrapper<F, Try<void>&&>,  // Yes, F(Try<void>&&) is OK
            ResultOfWrapper<F, const Try<void>&> // above all failed, resort to F(const Try<void>&)
            >
        >;

    using Arg_t = typename Arg::Type;

    // if ReturnsFuture::value is true, F returns another future type
    using IsReturnFuture = IsFuture<Arg_t>;

    // Future callback's result must be wrapped in another future
    using ReturnFutureType = Future<typename IsReturnFuture::Inner>;
};

// For when_all

template <typename... ELEM>
struct CollectAllVariadicContext
{
    CollectAllVariadicContext() {}

    // different from folly: do nothing here
    ~CollectAllVariadicContext() {}

    // non-copyable
    CollectAllVariadicContext(const CollectAllVariadicContext&) = delete;
    CollectAllVariadicContext& operator=(const CollectAllVariadicContext&) = delete;

    // seems like typedef does not work...
    // FIXME: do not use macro use using or typedef
#define _TRYELEM_ typename TryWrapper<ELEM>::Type...
    Promise<std::tuple<_TRYELEM_>> pm;
    std::mutex mutex;
    std::tuple<_TRYELEM_> results;
    std::vector<size_t> collects;

    using FutureType = Future<std::tuple<_TRYELEM_>>;
#undef _TRYELEM_

    template <typename T, size_t I>
    inline void SetPartialResult(typename TryWrapper<T>::Type&& t)
    {
        std::unique_lock<std::mutex> lock(mutex);
        std::get<I>(results) = std::move(t);
        collects.push_back(I);
        if (collects.size() == std::tuple_size_v<decltype(results)>)
        {
            lock.unlock(); // So the lock must be std::unique_lock, not lock_guard
            pm.SetValue(std::move(results));
        }
    }

};

// base template
template <template <typename...> class CTX, typename... Ts>
void CollectVariadicHelper(const std::shared_ptr<CTX<Ts...>>&) {}

// overload
template <template <typename...> class CTX, typename... Ts,
          typename THead, typename... TTail>
void CollectVariadicHelper(const std::shared_ptr<CTX<Ts...>>& ctx,
                           THead&& head, TTail&&... tail)
{
    using InnerTry = typename TryWrapper<typename THead::InnerType>::Type;
    head.Then([ctx](InnerTry&& t) {
        ctx->template SetPartialResult<InnerTry,
                                       sizeof...(Ts) - sizeof...(TTail) - 1>(std::move(t));
    });
    CollectVariadicHelper(ctx, std::forward<TTail>(tail)...);
}

} // namespace detail

} // namespace asuka

#endif //ASUKA_HELPER_H
