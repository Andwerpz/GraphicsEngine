#pragma once
#include <engine/graphics/GraphicsDevice.h>
#include <memory>

#include "TaskLoop.h"
#include "input/InputHandler.h"

namespace engine::app {

// an Application is the top level container for everything an app needs to run

class Application {
public:
    Application();

    // application entry point
    void run();

    TaskLoop& get_task_loop() {
        return task_loop;
    }

    InputHandler& get_input_handler() {
        return input_handler;
    }

private:    
    TaskLoop task_loop;
    InputHandler input_handler;
};

}