#pragma once
#include <functional>
#include <engine/ui/UINode.h>

namespace engine::ui {

// some rectangular region that can be interacted with via mouse press

class Button : public UINode {
public:
    Button(
        UIRect bounds, 
        UINode* parent
    );

    // TODO have some way for user to register callback

private:
    void update_impl(float dt) override;
    void render_impl(UIRenderer& renderer) override;
};

}