#pragma once
#include <engine/ui/UINode.h>

namespace engine::ui {

// a Container is just a UINode that outside users can add arbitrary UINodes to
// Containers are useful as building blocks that other nodes can use since they have control
//   over the children of a Container 

class Container : public UINode {
public:
    Container(UIContext& context) : UINode(context) {}

    // make adding children public
    using UINode::emplace_child;
};

}