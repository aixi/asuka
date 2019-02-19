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
        kInit,
        kRunning,
        kFinish
    };

    // works like python decorator: warp the func_ to a Coroutine
    template <typename F, typename... Args>
    static CoroutinePtr CreateCoroutine(F&& f, Args&&... args)
    {
        return std::make_shared<Coroutine>(std::forward<F>(f), std::forward<Args>(args)...);
    }

private:
    VoidPtr Send(Coroutine* co, VoidPtr params = VoidPtr(nullptr)); // it will ++ref_count;
    VoidPtr Yield(const VoidPtr& params = VoidPtr(nullptr));
    static void Run(Coroutine* co);

private:
    unsigned int id_; // 1: main
    State state_;
    VoidPtr yield_value_;

    static const size_t kDefaultStackSize;

    std::vector<char> stack_;

    ucontext_t uctx_;

    std::function<void ()> func_;

    VoidPtr result_;

    static Coroutine main_;
    static Coroutine current_;
    static unsigned int sid_;
};
}

#endif //ASUKA_COROUTINE_H
