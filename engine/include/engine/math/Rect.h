#pragma once
#include <glm/glm.hpp>

namespace engine::math {

// represents some axis aligned rectangle

template <glm::length_t D, typename T>
struct Rect {
    using Vec = glm::vec<D, T>;

    // position of minimum corner
    // position of maximum corner given by pos + dim
    glm::vec<D, T> pos;

    // dimensions
    // any dimension <= 0 indicates an empty rect
    glm::vec<D, T> dim;

    constexpr Rect(Vec pos, Vec dim) : pos(pos), dim(dim) {}
    constexpr Rect() : pos(0), dim(0) {}

    static Rect from_min_max(Vec lo, Vec hi) {
        return {lo, hi - lo};
    }
    Vec min() const { return this->pos; }
    Vec max() const { return this->pos + this->dim; }

    static Rect empty() {
        return {Vec(0), Vec(0)};
    }
    bool is_empty() const {
        return glm::any(glm::lessThanEqual(this->dim, Vec(0)));
    }

    // returns the smallest rect that contains this and the other rect
    // empty rects are ignored
    Rect unite(const Rect& other) const {
        if(is_empty()) return other.is_empty() ? empty() : other;
        if(other.is_empty()) return is_empty() ? empty() : *this;
        return from_min_max(
            glm::min(this->min(), other.min()),
            glm::max(this->max(), other.max())
        );
    }

    // returns the intersection between this rect and another rect
    // if they don't overlap, an empty rect is returned
    Rect intersect(const Rect& other) const {
        Vec lo = glm::max(this->min(), other.min());
        Vec hi = glm::min(this->max(), other.max());
        if(glm::any(glm::lessThanEqual(hi, lo))) return empty();
        return from_min_max(lo, hi);
    }
};

using Rect2  = Rect<2, float>;
using Rect2i = Rect<2, int>;

using Rect3  = Rect<3, float>;
using Rect3i = Rect<3, int>;

}