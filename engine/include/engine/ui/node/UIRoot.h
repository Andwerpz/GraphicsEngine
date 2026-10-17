#pragma once
#include <engine/ui/node/Container.h>

namespace engine::ui {

// a UIRoot is just a Container that also holds a reference to the managing UIContext

class UIContext;

class UIRoot : public Container {
public:
    UIRoot(
        UIContext& context,
        UIRect bounds
    ) : Container(context, bounds) {}
};

}