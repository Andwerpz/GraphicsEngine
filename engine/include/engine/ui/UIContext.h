#pragma once

#include "UIRenderer.h"
#include "UINode.h"

namespace engine::ui {

// unifies UI graph and rendering

class UIContext {
public:
    UIContext(engine::graphics::GraphicsDevice& device);

private:
    UIRenderer renderer;

    // root node is managed by the UIContext
    UINode root;
};

}