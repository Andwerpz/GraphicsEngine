#pragma once
#include <engine/util/reactive/Reactive.h>
#include <engine/util/follower/Snap.h>
#include <engine/util/follower/ExpSmooth.h>
#include <engine/ui/UINode.h>
#include <engine/ui/UIContext.h>

namespace engine::ui {

// provides a way to easily map a UINode input state to a color

// TODO extend this to be able to handle arbitrary UINode states
//   for example, a toggle button would need an extra 'toggled' state

class NodeStateColor {
    template<typename State, typename T>
    using Reactive = engine::util::Reactive<State, T>;

    template<typename T>
    using Snap = engine::util::Snap<T>;
    template<typename T>
    using ExpSmooth = engine::util::ExpSmooth<T>;

    static inline constexpr glm::vec4 IDLE_COLOR = {0.2, 0.2, 0.2, 0.4};
    static inline constexpr glm::vec4 FOCUSED_COLOR = {0.3, 0.3, 0.3, 0.6};
    static inline constexpr glm::vec4 HOVERED_COLOR = {0.3, 0.3, 0.3, 0.8};
    static inline constexpr glm::vec4 CAPTURED_COLOR = {0.4, 0.4, 0.4, 1.0};

public:
    NodeStateColor(UINode& node) : 
        node(node),
        reactor(
            [&]() -> UINode* { return &node; },
            [&](UINode* node) -> glm::vec4 {
                if(node->is_captured()) return captured_color;
                if(node->is_hovered()) return hovered_color;
                if(node->is_focused()) return focused_color;
                return idle_color;
            },
            std::make_unique<ExpSmooth<glm::vec4>>(idle_color, 0.015f)
        )
    {
        last_get_time = node.get_context().get_ui_time();
    }

    glm::vec4 get_color() {
        double cur_time = node.get_context().get_ui_time();
        reactor.update(cur_time - last_get_time);
        last_get_time = cur_time;
        return reactor.get_val();
    }

private:
    glm::vec4 idle_color = IDLE_COLOR;
    glm::vec4 focused_color = FOCUSED_COLOR;
    glm::vec4 hovered_color = HOVERED_COLOR;
    glm::vec4 captured_color = CAPTURED_COLOR;

    double last_get_time;

    UINode& node;
    Reactive<UINode*, glm::vec4> reactor;
};

}