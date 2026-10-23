#pragma once
#include <functional>
#include <memory>
#include <engine/util/follower/Follower.h>

namespace engine::util {

// a Reactive 

template<typename State, typename T>
class Reactive {
public:
    Reactive(
        std::function<State()> source,
        std::function<T(State)> mapping,
        std::unique_ptr<Follower<T>>&& follower
    ) : source(source), mapping(mapping), follower(std::move(follower)) {}

    const T& get_val() const {
        return follower->get_val();
    }

    void update(float dt) {
        follower->update(mapping(source()), dt);
    }

private:
    std::function<State()> source;
    std::function<T(State)> mapping;
    std::unique_ptr<Follower<T>> follower;
};

}