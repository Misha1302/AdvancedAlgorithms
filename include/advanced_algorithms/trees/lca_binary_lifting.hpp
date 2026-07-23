#pragma once

#include <bit>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "advanced_algorithms/trees/detail/tree_validation.hpp"

namespace advanced_algorithms {

class LcaBinaryLifting {
  public:
    explicit LcaBinaryLifting(const std::vector<std::vector<std::size_t>>& tree,
                              std::size_t root = 0) {
        const auto traversal = detail::root_tree(tree, root);
        parent_ = traversal.parent;
        depth_ = traversal.depth;
        if (tree.empty()) {
            return;
        }

        const std::size_t levels = static_cast<std::size_t>(std::bit_width(tree.size()));
        up_.assign(levels, std::vector<std::size_t>(tree.size(), root));
        up_[0] = parent_;
        for (std::size_t level = 1; level < levels; ++level) {
            for (std::size_t vertex = 0; vertex < tree.size(); ++vertex) {
                up_[level][vertex] = up_[level - 1][up_[level - 1][vertex]];
            }
        }
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return parent_.size();
    }

    [[nodiscard]] std::size_t depth(std::size_t vertex) const {
        check_vertex(vertex);
        return depth_[vertex];
    }

    [[nodiscard]] std::size_t parent(std::size_t vertex) const {
        check_vertex(vertex);
        return parent_[vertex];
    }

    [[nodiscard]] std::size_t kth_ancestor(std::size_t vertex, std::size_t steps) const {
        check_vertex(vertex);
        if (steps > depth_[vertex]) {
            throw std::out_of_range("Requested ancestor is above the root");
        }
        std::size_t bit = 0;
        while (steps != 0) {
            if ((steps & 1U) != 0) {
                vertex = up_[bit][vertex];
            }
            steps >>= 1U;
            ++bit;
        }
        return vertex;
    }

    [[nodiscard]] std::size_t lca(std::size_t lhs, std::size_t rhs) const {
        check_vertex(lhs);
        check_vertex(rhs);
        if (depth_[lhs] < depth_[rhs]) {
            std::swap(lhs, rhs);
        }
        lhs = kth_ancestor(lhs, depth_[lhs] - depth_[rhs]);
        if (lhs == rhs) {
            return lhs;
        }
        for (std::size_t level = up_.size(); level-- > 0;) {
            if (up_[level][lhs] != up_[level][rhs]) {
                lhs = up_[level][lhs];
                rhs = up_[level][rhs];
            }
        }
        return parent_[lhs];
    }

    [[nodiscard]] std::size_t distance(std::size_t lhs, std::size_t rhs) const {
        const std::size_t ancestor = lca(lhs, rhs);
        return depth_[lhs] + depth_[rhs] - 2U * depth_[ancestor];
    }

  private:
    void check_vertex(std::size_t vertex) const {
        if (vertex >= parent_.size()) {
            throw std::out_of_range("LcaBinaryLifting vertex is out of range");
        }
    }

    std::vector<std::size_t> parent_;
    std::vector<std::size_t> depth_;
    std::vector<std::vector<std::size_t>> up_;
};

} // namespace advanced_algorithms
