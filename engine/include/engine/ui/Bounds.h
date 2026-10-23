#pragma once
#include <glm/glm.hpp>

namespace engine::ui {

// represents some rectangular 2D region

struct Bounds {
    // position of top left corner
    glm::vec2 pos;

    // dimensions
    glm::vec2 dim;
};

}