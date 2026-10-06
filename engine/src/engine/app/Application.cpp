#include <engine/app/Application.h>

namespace engine::app {

Application::Application() :
    task_loop(), 
    input_handler(task_loop)
{
    // do nothing
}

void Application::run() {
    // enter task loop
    task_loop.run();
}

}