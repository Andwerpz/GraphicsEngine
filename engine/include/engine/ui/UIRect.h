#pragma once
#include <glm/glm.hpp>

namespace engine::ui {

struct UIRect {
    // position of top left corner
    glm::vec2 pos;

    // dimensions
    glm::vec2 dim;
};

}