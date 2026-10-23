#pragma once

namespace engine::util {

// a Follower is a container for a value that evolves over time given some other target value 

template<typename T>
class Follower {
public:
    Follower(T init_val) : val(init_val) {}
    virtual ~Follower() = default;
    const T& get_val() const { return val; }

    // should return the value after the step
    virtual const T& update(const T& target, float dt) = 0;

protected:
    T val;
};

}