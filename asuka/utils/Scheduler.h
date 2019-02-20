//
// Created by xi on 19-2-20.
//

#ifndef ASUKA_SCHEDULER_H
#define ASUKA_SCHEDULER_H

#include <chrono>
#include <functional>

namespace asuka
{

class Scheduler
{
public:
    Scheduler() = default;
    virtual ~Scheduler() = default;

    // non-copyable
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    // non-movable
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

    // NOTE: The following function need not to be thread safe
    // always schedule callback and submit request in same thread

    virtual void SchedulerLater(std::chrono::milliseconds duration, std::function<void ()> func) = 0;
    virtual void Schedule(std::function<void ()> func) = 0;
};

} // namespace asuka

#endif //ASUKA_SCHEDULER_H
