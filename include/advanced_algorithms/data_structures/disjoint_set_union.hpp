#pragma once

#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace advanced_algorithms {

class DisjointSetUnion {
  public:
    explicit DisjointSetUnion(std::size_t size)
        : parent_(size), component_size_(size, 1), components_(size) {
        std::iota(parent_.begin(), parent_.end(), std::size_t{0});
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return parent_.size();
    }

    [[nodiscard]] std::size_t components() const noexcept {
        return components_;
    }

    std::size_t find(std::size_t vertex) {
        check_vertex(vertex);
        std::size_t root = vertex;
        while (parent_[root] != root) {
            root = parent_[root];
        }
        while (parent_[vertex] != vertex) {
            const std::size_t next = parent_[vertex];
            parent_[vertex] = root;
            vertex = next;
        }
        return root;
    }

    bool unite(std::size_t lhs, std::size_t rhs) {
        lhs = find(lhs);
        rhs = find(rhs);
        if (lhs == rhs) {
            return false;
        }
        if (component_size_[lhs] < component_size_[rhs]) {
            std::swap(lhs, rhs);
        }
        parent_[rhs] = lhs;
        component_size_[lhs] += component_size_[rhs];
        --components_;
        return true;
    }

    bool same(std::size_t lhs, std::size_t rhs) {
        return find(lhs) == find(rhs);
    }

    std::size_t component_size(std::size_t vertex) {
        return component_size_[find(vertex)];
    }

  private:
    void check_vertex(std::size_t vertex) const {
        if (vertex >= parent_.size()) {
            throw std::out_of_range("DisjointSetUnion vertex is out of range");
        }
    }

    std::vector<std::size_t> parent_;
    std::vector<std::size_t> component_size_;
    std::size_t components_;
};

} // namespace advanced_algorithms
