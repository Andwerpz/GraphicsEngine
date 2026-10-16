#pragma once
#include <functional>
#include <engine/asset/Font.h>
#include <engine/ui/UINode.h>
#include <engine/ui/node/Label.h>

namespace engine::ui {

// some rectangular region that can be interacted with via mouse press

class Button : public UINode {
    static inline constexpr glm::vec4 NORMAL_COLOR = {0.2, 0.2, 0.2, 1.0};
    static inline constexpr glm::vec4 HOVERED_COLOR = {0.3, 0.3, 0.3, 1.0};
    static inline constexpr glm::vec4 PRESSED_COLOR = {0.4, 0.4, 0.4, 1.0};

public:
    Button(
        std::function<void()> callback,
        UIRect bounds
    );

    void set_callback(std::function<void()> _callback) {
        callback = _callback;
    }

private:
    void update_impl(float dt) override;
    void render_impl(UIRenderer& renderer) override;

    bool on_mouse_button(const MouseButtonEvent& ev) override;

    // user registered callback on mouse click
    std::function<void()> callback;
};

}