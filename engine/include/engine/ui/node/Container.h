#pragma once
#include <engine/ui/UINode.h>

namespace engine::ui {

// a Container is just a UINode that outside users can add arbitrary UINodes to

class Container : public UINode {
public:
    Container(UIContext& context, UIRect bounds) : UINode(context, bounds) {}

    // make adding children public
    using UINode::emplace_child;
};

}