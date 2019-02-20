//
// Created by xi on 19-2-19.
//

#ifndef ASUKA_COROUTINE_H
#define ASUKA_COROUTINE_H

#include <ucontext.h> // NOTE: Linux only

#include <vector>
#include <map>
#include <memory>
#include <functional>

// a Python like Coroutine class

namespace asuka
{

using VoidPtr = std::shared_ptr<void>;

class Coroutine;
using CoroutinePtr = std::shared_ptr<Coroutine>;

class Coroutine
{
public:

    enum class State
    {
        kNotInit,
        kRunning,
        kFinished
    };

    // works like python decorator: warp the func_ to a Coroutine
    template <typename F, typename... Args>
    static CoroutinePtr CreateCoroutine(F&& f, Args&&... args)
    {
        return std::make_shared<Coroutine>(std::forward<F>(f), std::forward<Args>(args)...);
    }

    // schedule coroutine

    // like Python generator's send method
    static VoidPtr Send(const CoroutinePtr& co, VoidPtr args = VoidPtr(nullptr));
    static VoidPtr Yield(const VoidPtr& args = VoidPtr(nullptr));
    static VoidPtr Next(const CoroutinePtr& co);

    // NOTE: user shall use CreateCoroutine, not constructor
    // the Constructor should be private
    // but Compiler does NOT allow private template constructor

    explicit Coroutine(size_t stack_size = 0);

    // if F return void
    template <typename F, typename... Args,
              typename = typename std::enable_if_t<std::is_void_v<std::result_of_t<F(Args...)>>, void>,
              typename Dummy = void>
    Coroutine(F&& f, Args&&... args) : Coroutine(kDefaultStackSize)
    {
        func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    }

    // if F return non-void
    template <typename F, typename... Args,
              typename = typename std::enable_if_t<!std::is_void_v<std::result_of_t<F(Args...)>>, void>>
    Coroutine(F&& f, Args&&... args) : Coroutine(kDefaultStackSize)
    {
        using ResultType = std::result_of_t<F(Args...)>;
        auto temp = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        func_ = [temp, this] () mutable {
            this->result_ = std::make_shared<ResultType>(temp());
        };
    }

    ~Coroutine() = default;

    unsigned int id() const
    {
        return id_;
    }

    static unsigned int GetCurrentId()
    {
        return current_->id_;
    }

    // non copyable
    Coroutine(const Coroutine&) = delete;
    Coroutine& operator=(const Coroutine&) = delete;
    // non movable
    Coroutine(Coroutine&&) = delete;
    Coroutine& operator=(Coroutine&&) = delete;

private:
    VoidPtr SendImpl(Coroutine* co_ptr, VoidPtr args = VoidPtr(nullptr)); // pass by value and move

    VoidPtr YieldImpl(VoidPtr args = VoidPtr(nullptr));

    static void Run(Coroutine* co_ptr);

private:
    unsigned int id_; // 1: main

    State state_;

    std::vector<char> stack_;

    ucontext_t uctx_;

    std::function<void ()> func_;

    VoidPtr result_;

    VoidPtr yield_value_;

    static const size_t kDefaultStackSize;
    static Coroutine main_;
    static Coroutine* current_;
    static unsigned int s_id_;
};
}

#endif //ASUKA_COROUTINE_H
