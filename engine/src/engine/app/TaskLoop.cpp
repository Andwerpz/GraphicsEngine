#include <engine/app/TaskLoop.h>
#include <iostream>

namespace engine::app {

void TaskLoop::run() {
    while(!should_stop) {
        auto current_time = std::chrono::steady_clock::now();
        auto current_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time.time_since_epoch());

        // triggered tasks
        while(triggered_tasks.size() != 0) {
            Task* task = triggered_tasks.front();
            triggered_tasks.pop();
            assert(task->is_pending);
            task->is_pending = false;
            task->run();
        }

        // periodic tasks
        while(periodic_tasks.size() != 0 && periodic_tasks.top()->wakeup_time <= current_time_ns) {
            Task* task = periodic_tasks.top();
            periodic_tasks.pop();
            task->run();
            task->wakeup_time = std::max(current_time_ns, task->wakeup_time + task->period);
            periodic_tasks.push(task);
        }
    }
}

void TaskLoop::stop() {
    should_stop = true;
}

uint64_t TaskLoop::add_periodic_task(
    std::string name,
    std::function<void()> func,
    std::chrono::nanoseconds period
) {
    auto current_time = std::chrono::steady_clock::now();
    auto current_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time.time_since_epoch());
    uint64_t id = tasks.size();

    auto task = std::make_unique<Task>();
    task->name = name;
    task->id = id;
    task->func = func;
    task->is_periodic = true;
    task->period = period;
    task->wakeup_time = current_time_ns;
    tasks.emplace_back(std::move(task));
    periodic_tasks.push(tasks[id].get());

    return id;
}

uint64_t TaskLoop::add_reactive_task(
    std::string name,
    std::function<void()> func
) {
    auto current_time = std::chrono::steady_clock::now();
    auto current_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time.time_since_epoch());
    uint64_t id = tasks.size();

    auto task = std::make_unique<Task>();
    task->name = name;
    task->id = id;
    task->func = func;
    tasks.emplace_back(std::move(task));

    return id;
}

void TaskLoop::trigger_task(uint64_t id) {
    assert(id < tasks.size());
    Task* task = tasks[id].get();
    if(!task->is_pending) {
        task->is_pending = true;
        triggered_tasks.push(task);
    }
}

}