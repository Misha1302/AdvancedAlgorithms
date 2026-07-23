#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "advanced_algorithms/trees/detail/tree_validation.hpp"

namespace advanced_algorithms {

class HeavyLightDecomposition {
  public:
    explicit HeavyLightDecomposition(const std::vector<std::vector<std::size_t>>& tree,
                                     std::size_t root = 0) {
        const auto traversal = detail::root_tree(tree, root);
        parent_ = traversal.parent;
        depth_ = traversal.depth;
        if (tree.empty()) {
            return;
        }

        const std::size_t n = tree.size();
        subtree_size_.assign(n, 1);
        heavy_child_.assign(n, no_vertex());
        for (auto iterator = traversal.order.rbegin(); iterator != traversal.order.rend(); ++iterator) {
            const std::size_t vertex = *iterator;
            std::size_t largest_child_size = 0;
            for (const std::size_t to : tree[vertex]) {
                if (parent_[to] != vertex) {
                    continue;
                }
                subtree_size_[vertex] += subtree_size_[to];
                if (subtree_size_[to] > largest_child_size) {
                    largest_child_size = subtree_size_[to];
                    heavy_child_[vertex] = to;
                }
            }
        }

        head_.assign(n, root);
        position_.assign(n, 0);
        vertex_at_position_.assign(n, 0);
        std::size_t timer = 0;
        std::vector<std::pair<std::size_t, std::size_t>> chains{{root, root}};
        while (!chains.empty()) {
            const auto [chain_start, chain_head] = chains.back();
            chains.pop_back();
            std::size_t vertex = chain_start;
            while (vertex != no_vertex()) {
                head_[vertex] = chain_head;
                position_[vertex] = timer;
                vertex_at_position_[timer] = vertex;
                ++timer;

                for (const std::size_t to : tree[vertex]) {
                    if (parent_[to] == vertex && to != heavy_child_[vertex]) {
                        chains.emplace_back(to, to);
                    }
                }
                vertex = heavy_child_[vertex];
            }
        }
    }

    [[nodiscard]] static constexpr std::size_t no_vertex() noexcept {
        return static_cast<std::size_t>(-1);
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return parent_.size();
    }

    [[nodiscard]] std::size_t position(std::size_t vertex) const {
        check_vertex(vertex);
        return position_[vertex];
    }

    [[nodiscard]] std::size_t vertex_at_position(std::size_t position) const {
        if (position >= vertex_at_position_.size()) {
            throw std::out_of_range("HLD position is out of range");
        }
        return vertex_at_position_[position];
    }

    [[nodiscard]] std::pair<std::size_t, std::size_t> subtree_range(std::size_t vertex) const {
        check_vertex(vertex);
        return {position_[vertex], position_[vertex] + subtree_size_[vertex]};
    }

    // Covers every path vertex exactly once. Segment order and direction are unspecified,
    // so this overload is intended for commutative aggregates.
    template <class Callback>
    void for_each_path_segment(std::size_t lhs, std::size_t rhs, Callback&& callback) const {
        check_vertex(lhs);
        check_vertex(rhs);
        while (head_[lhs] != head_[rhs]) {
            if (depth_[head_[lhs]] < depth_[head_[rhs]]) {
                std::swap(lhs, rhs);
            }
            callback(position_[head_[lhs]], position_[lhs] + 1U);
            lhs = parent_[head_[lhs]];
        }
        if (depth_[lhs] > depth_[rhs]) {
            std::swap(lhs, rhs);
        }
        callback(position_[lhs], position_[rhs] + 1U);
    }

    // Emits segments in path order from lhs to rhs. If reversed is true, consume
    // positions in [left, right) from right - 1 down to left; otherwise consume
    // them from left up to right - 1.
    template <class Callback>
    void for_each_ordered_path_segment(std::size_t lhs, std::size_t rhs,
                                       Callback&& callback) const {
        check_vertex(lhs);
        check_vertex(rhs);

        struct Segment {
            std::size_t left{};
            std::size_t right{};
            bool reversed{};
        };
        std::vector<Segment> rhs_segments;

        while (head_[lhs] != head_[rhs]) {
            if (depth_[head_[lhs]] >= depth_[head_[rhs]]) {
                callback(position_[head_[lhs]], position_[lhs] + 1U, true);
                lhs = parent_[head_[lhs]];
            } else {
                rhs_segments.push_back(Segment{position_[head_[rhs]], position_[rhs] + 1U, false});
                rhs = parent_[head_[rhs]];
            }
        }

        if (depth_[lhs] >= depth_[rhs]) {
            callback(position_[rhs], position_[lhs] + 1U, true);
        } else {
            rhs_segments.push_back(Segment{position_[lhs], position_[rhs] + 1U, false});
        }

        for (auto iterator = rhs_segments.rbegin(); iterator != rhs_segments.rend(); ++iterator) {
            callback(iterator->left, iterator->right, iterator->reversed);
        }
    }

    [[nodiscard]] std::size_t parent(std::size_t vertex) const {
        check_vertex(vertex);
        return parent_[vertex];
    }

    [[nodiscard]] std::size_t depth(std::size_t vertex) const {
        check_vertex(vertex);
        return depth_[vertex];
    }

    [[nodiscard]] std::size_t head(std::size_t vertex) const {
        check_vertex(vertex);
        return head_[vertex];
    }

  private:
    void check_vertex(std::size_t vertex) const {
        if (vertex >= parent_.size()) {
            throw std::out_of_range("HeavyLightDecomposition vertex is out of range");
        }
    }

    std::vector<std::size_t> parent_;
    std::vector<std::size_t> depth_;
    std::vector<std::size_t> subtree_size_;
    std::vector<std::size_t> heavy_child_;
    std::vector<std::size_t> head_;
    std::vector<std::size_t> position_;
    std::vector<std::size_t> vertex_at_position_;
};

} // namespace advanced_algorithms
