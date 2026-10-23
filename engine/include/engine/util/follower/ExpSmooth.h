#pragma once
#include "Follower.h"
#include <glm/glm.hpp>

namespace engine::util {

// a ExpSmooth follower exponentially decays towards the target value

template<class T>
class ExpSmooth : public Follower<T> {
public:
    ExpSmooth(T init_val, float half_life) : Follower<T>(init_val), half_life(half_life) {}
    const T& update(const T& target, float dt) override {
        float k = 1.0f - std::exp2(-dt / half_life);
        this->val = glm::mix(this->val, target, k);
        return this->val;
    }

    void set_half_life(float val) {
        half_life = val;
    }

private:
    float half_life;
};

}