//
// Created by xi on 19-2-19.
//

#include <assert.h>
#include <string>
#include <stdexcept>
#include <asuka/utils/Types.h>
#include <asuka/coroutine/Coroutine.h>

namespace asuka
{

const size_t Coroutine::kDefaultStackSize = 8 * 1024; // 8 KB
Coroutine Coroutine::main_;
Coroutine* Coroutine::current_ = nullptr;
unsigned int Coroutine::s_id_ = 0;

Coroutine::Coroutine(size_t stack_size) :
    id_(++s_id_),
    state_(State::kInit),
    stack_(std::max(stack_size, kDefaultStackSize))
{
    if (this == &main_)
    {
        std::vector<char>().swap(stack_);
        return;
    }
    if (id_ == main_.id_)
    {
        id_ = ++s_id_; // when s_id_ overflow
    }
    int ret = ::getcontext(&uctx_);
    // FIXME: check ret
    assert(ret == 0);
    UnusedVariable(ret);
    uctx_.uc_stack.ss_sp = stack_.data();
    uctx_.uc_stack.ss_size = stack_.size();
    uctx_.uc_link = nullptr;
    ::makecontext(&uctx_, reinterpret_cast<void(*)()>(&Coroutine::Run), 1, this);
}

VoidPtr Coroutine::Send(const CoroutinePtr& co, VoidPtr args)
{
    if (co->state_ == Coroutine::State::kFinish)
    {
        throw std::runtime_error("Send value to finished coroutine");
    }
    if (!Coroutine::current_)
    {
        Coroutine::current_ = &Coroutine::main_;
    }
    return Coroutine::current_->SendImpl(get_pointer(co), std::move(args));
}

VoidPtr Coroutine::Yield(const VoidPtr& args)
{
    return Coroutine::current_->YieldImpl(args);
}

VoidPtr Coroutine::Next(const CoroutinePtr& co)
{
    return Send(co);
}

VoidPtr Coroutine::SendImpl(Coroutine* co_ptr, VoidPtr args)
{
    assert(co_ptr);
    assert(this == current_);
    assert(this != co_ptr);
    current_ = co_ptr;
    if (args)
    {
        // behave like Python generator
        if (co_ptr->state_ == State::kInit && co_ptr != &Coroutine::main_)
        {
            throw std::runtime_error("Can't send non-void value to a just-created coroutine");
        }
        // set old coroutine's yield value
        this->yield_value_ = std::move(args);
    }
    int ret = ::swapcontext(&uctx_, &co_ptr->uctx_);
    if (ret != 0)
    {
        perror("FATAL ERROR: ::swapcontext");
        throw std::runtime_error("FATAL ERROR: swapcontext failed");
    }
    return co_ptr->yield_value_;
}

VoidPtr Coroutine::YieldImpl(VoidPtr args)
{
    return SendImpl(&main_, std::move(args));
}

void Coroutine::Run(Coroutine* co_ptr)
{
    assert(&Coroutine::main_ != co_ptr);
    assert(Coroutine::current_ == co_ptr);
    co_ptr->state_ = State::kRunning;
    if (co_ptr->func_)
    {
        co_ptr->func_();
    }
    co_ptr->state_ = State::kFinish;
    co_ptr->Yield(co_ptr->result_);
}

} // namespace asuka
