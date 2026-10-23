#pragma once
#include "Follower.h"

namespace engine::util {

// a Snap follower just immediately snaps to the target

template<class T>
class Snap : public Follower<T> {
public:
    Snap(T init_val) : Follower<T>(init_val) {}
    const T& update(const T& target, float dt) override {
        this->val = target;
        return this->val;
    }
};

}