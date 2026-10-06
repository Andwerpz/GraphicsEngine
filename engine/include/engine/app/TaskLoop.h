#pragma once
#include <chrono>
#include <vector>
#include <functional>
#include <queue>
#include <string>
#include <cassert>

namespace engine::app {

// an TaskLoop is the manager for periodic and reactive tasks
// it's essentially a container for a bunch of methods that can be grouped into a few categories:
// - startup : these tasks run once when the task loop starts up
// - periodic : these tasks are supposed to run at a fixed wall frequency
// - reactive : these tasks will only run when something external triggers them
// - cleanup : these tasks run once when the task loop terminates

struct Task {
    std::string name;
    uint64_t id;                            // unique task identifier
    std::function<void()> func;
    bool is_pending = false;                // is this task waiting to run?

    // periodic
    bool is_periodic = false;
    std::chrono::nanoseconds period;        // this task will try to run every period nanoseconds
    std::chrono::nanoseconds wakeup_time;   // wall time at which this task should wake to run again

    void run() {
        try {
            func();
        } catch(const std::runtime_error& e) {
            throw std::runtime_error("Uncaught runtime error in task \"" + name + "\" : " + e.what());
        }
    }
};

class TaskLoop {
public:
    TaskLoop() = default;
    TaskLoop(TaskLoop&&) = delete;
    TaskLoop& operator=(const TaskLoop&) = delete;

    // entry point
    void run();

    // sets should_stop to true
    void stop();

    // add tasks
    uint64_t add_periodic_task(
        std::string name,
        std::function<void()> func,
        std::chrono::nanoseconds period
    );
    uint64_t add_reactive_task(
        std::string name,
        std::function<void()> func
    );

    // it is guaranteed that after calling this, the associated task will run sometime in the future
    void trigger_task(uint64_t id);

private:
    // run() stops when this is true
    bool should_stop = false;

    // list of all tasks by ID
    std::vector<std::unique_ptr<Task>> tasks;

    // sort periodic tasks in descending order by wakeup time
    struct ComparePeriodicTask {
        bool operator()(const Task* a, const Task* b) const {
            assert(a->is_periodic && b->is_periodic);
            return a->wakeup_time > b->wakeup_time;
        }
    };
    std::priority_queue<Task*, std::vector<Task*>, ComparePeriodicTask> periodic_tasks;

    // queue of triggered tasks to run
    // all tasks in this queue should have is_pending set 
    std::queue<Task*> triggered_tasks;
};

}