#pragma once
#include <array>
#include <cstddef>
#include <initializer_list>
#include <optional>
#include <set>
#include <vector>
#include <glm/glm.hpp>

namespace engine::tile {

// utility class for manipulating sets of 2D integer coordinates
class CellSet {
public:
    using Cell = glm::ivec2;
    using Container = std::set<glm::ivec2, decltype([](const glm::ivec2& lhs, const glm::ivec2& rhs) {
        return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
    })>;

    static constexpr std::array<Cell, 4> NEIGHBORS_4 = {{
        {0, -1}, {-1, 0}, {1, 0}, {0, 1}
    }};
    static constexpr std::array<Cell, 8> NEIGHBORS_8 = {{
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},          {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    }};

    CellSet() = default;

    // exclusive rectangle from min to max
    static CellSet from_rect(Cell min, Cell max) {
        CellSet s;
        s.insert_rect(min, max);
        return s;
    }

    // -- BASIC SET OPERATIONS --
    bool contains(Cell c) const { 
        return cells.count(c) > 0; 
    }
    bool empty() const { 
        return cells.empty(); 
    }
    std::size_t size() const { 
        return cells.size(); 
    }

    // returns true if c was newly added to the set
    bool insert(Cell c) { 
        return cells.insert(c).second; 
    }
    // returns true if c was present in the set
    bool erase(Cell c) { 
        return cells.erase(c) > 0;
    }
    // removes all elements from the set
    void clear() {
        cells.clear();
    }
    
    // inserts all cells within the rectangle (max exclusive)
    void insert_rect(Cell min, Cell max) {
        for(int x = min.x; x < max.x; x++) {
            for(int y = min.y; y < max.y; y++) {
                cells.insert({x, y});
            }
        }
    }
    // erases all cells in the set that lie within the rectangle (max exclusive)
    void erase_rect(Cell min, Cell max) {
        for(int x = min.x; x < max.x; x++) {
            for(int y = min.y; y < max.y; y++) {
                cells.erase({x, y});
            }
        }
    }

    // iterators
    Container::const_iterator begin() const { return cells.begin(); }
    Container::const_iterator end() const { return cells.end(); }

    // -- SET ALGEBRA --
    CellSet& unite(const CellSet& o) {
        cells.insert(o.cells.begin(), o.cells.end());
        return *this;
    }
    CellSet& subtract(const CellSet& o) {
        for(const Cell& c : o.cells) cells.erase(c);
        return *this;
    }
    CellSet& intersect(const CellSet& o) {
        for(auto it = cells.begin(); it != cells.end();) {
            it = o.contains(*it) ? std::next(it) : cells.erase(it);
        }
        return *this;
    }

    friend CellSet operator|(CellSet a, const CellSet& b) { return a.unite(b); }
    friend CellSet operator-(CellSet a, const CellSet& b) { return a.subtract(b); }
    friend CellSet operator&(CellSet a, const CellSet& b) { return a.intersect(b); }
    friend CellSet operator^(const CellSet& a, const CellSet& b) {
        return (a - b) | (b - a);
    }
    CellSet& operator|=(const CellSet& o) { return unite(o); }
    CellSet& operator-=(const CellSet& o) { return subtract(o); }
    CellSet& operator&=(const CellSet& o) { return intersect(o); }

    bool operator==(const CellSet& o) const { return cells == o.cells; }
    bool operator!=(const CellSet& o) const { return cells != o.cells; }

    bool is_subset_of(const CellSet& o) const {
        for(const Cell& c : cells) {
            if(!o.contains(c)) return false;
        }
        return true;
    }
    bool intersects(const CellSet& o) const {
        const CellSet& small = size() <= o.size() ? *this : o;
        const CellSet& large = size() <= o.size() ? o : *this;
        for(const Cell& c : small.cells) {
            if(large.contains(c)) return true;
        }
        return false;
    }

    // -- TRANSFORMS --
    // returns new set with all cells translated by offset
    CellSet translated(Cell offset) const {
        CellSet out;
        for(const Cell& c : cells) {
            out.cells.insert(out.cells.end(), c + offset);
        }
        return out;
    }
    void translate(Cell offset) { 
        *this = translated(offset); 
    }

    // -- MORPHOLOGY --
    // add all cells outside of the set that are adjacent to a cell in the set
    CellSet dilated(bool eight_way = true) const {
        CellSet out = *this;
        for (const Cell& c : cells) {
            if(eight_way) for(const Cell& d : NEIGHBORS_8) out.insert(c + d);
            else          for(const Cell& d : NEIGHBORS_4) out.insert(c + d);
        }
        return out;
    }
    void dilate(bool eight_way = true) {
        *this = dilated(eight_way);
    }

    // remove all cells in the set that are adjacent to a cell outside of the set
    CellSet eroded(bool eight_way = true) const {
        CellSet out;
        const int need = eight_way ? 8 : 4;
        for(const Cell& c : cells) {
            if(neighbor_count(c, eight_way) == need) out.insert(c);
        }
        return out;
    }
    void erode(bool eight_way = true) {
        *this = eroded(eight_way);
    }

    // set of cells inside the set that are adjacent to a cell outside of the set
    CellSet inner_border(bool eight_way = true) const { 
        return *this - eroded(eight_way); 
    }

    // set of cells not in the set that are adjacent to a cell in the set
    CellSet outer_border(bool eight_way = true) const { 
        return dilated(eight_way) - *this; 
    }
    
    // -- MISC --
    // returns the number of cells adjacent to this one
    int neighbor_count(Cell c, bool eight_way = true) const {
        int n = 0;
        if(eight_way) {
            for(const Cell& d : NEIGHBORS_8) n += contains(c + d);
        } 
        else {
            for(const Cell& d : NEIGHBORS_4) n += contains(c + d);
        }
        return n;
    }

    // returns a set for each connected component of cells
    std::vector<CellSet> connected_components(bool eight_way = false) const {
        std::vector<CellSet> result;
        CellSet unvisited = *this;
        while(!unvisited.empty()) {
            CellSet comp;
            std::vector<Cell> stack{*unvisited.cells.begin()};
            unvisited.erase(stack.back());
            while (!stack.empty()) {
                Cell c = stack.back();
                stack.pop_back();
                comp.insert(c);
                auto visit = [&](const Cell& d) {
                    Cell n = c + d;
                    if (unvisited.erase(n)) stack.push_back(n);
                };
                if(eight_way) for (const Cell& d : NEIGHBORS_8) visit(d);
                else          for (const Cell& d : NEIGHBORS_4) visit(d);
            }
            result.push_back(std::move(comp));
        }
        return result;
    }

private:
    Container cells;
};;

}