#pragma once

#include <engine/graphics/GraphicsDevice.h>

namespace engine::ui {

// responsible for efficiently rendering the UI to some rendering target
// for now, the rendering target is just the OS window 
// should handle all UI rendering details like batching and stuff

class UIRenderer {
public:


private:
    // underlying graphics device used for rendering
    engine::graphics::GraphicsDevice& device;
};

}