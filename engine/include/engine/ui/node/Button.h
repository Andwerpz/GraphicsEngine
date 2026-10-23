#pragma once
#include <functional>
#include <engine/ui/node/Container.h>

namespace engine::ui {

// some rectangular region that can be interacted with via mouse press

class Button : public Container {
    static inline constexpr glm::vec4 NORMAL_COLOR = {0.2, 0.2, 0.2, 0.4};
    static inline constexpr glm::vec4 HOVERED_COLOR = {0.3, 0.3, 0.3, 0.8};
    static inline constexpr glm::vec4 PRESSED_COLOR = {0.4, 0.4, 0.4, 1.0};

public:
    Button(UIContext& context) : Container(context) {}
    Button(UIContext& context, std::function<void()> callback) : 
        Container(context), callback(callback) {}

    void set_callback(std::function<void()> _callback) {
        callback = _callback;
    }

private:
    bool on_mouse_button(const MouseButtonEvent& ev) override {
        if(ev.action == engine::app::MouseButtonAction::Release && is_hovered()) {
            if(callback) callback();
        }
        return true;
    }

    // user registered callback on mouse click
    std::function<void()> callback;
};

}