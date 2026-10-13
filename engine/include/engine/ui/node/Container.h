#pragma once
#include <engine/ui/UINode.h>

namespace engine::ui {

// a Container is just a UINode that outside users can add arbitrary UINodes to

class Container : public UINode {
public:
    Container(UIRect bounds) : UINode(bounds) {}

    // make adding children public
    using UINode::add_child;
    using UINode::emplace_child;
};

}