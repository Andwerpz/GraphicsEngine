#pragma once
#include <optional>
#include <vector>
#include <memory>

namespace engine::ui {

// some rectangular region 

class UIContext;

class UINode {
    friend class UIContext;

public:
    struct Rect {
        float x;
        float y;
        float width;
        float height;
    };
    
    


private:
    // context that this node belongs to
    UIContext* context;

    // if this is the root, parent is nullopt
    std::optional<UINode*> parent;

    // all children of this node
    std::vector<std::unique_ptr<UINode>> children;

    // rectangular bounds of this node
    // offset is relative to parent node
    Rect bounds;
};

}