#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "advanced_algorithms/trees/detail/tree_validation.hpp"

namespace advanced_algorithms {

class CentroidDecomposition {
  public:
    explicit CentroidDecomposition(const std::vector<std::vector<std::size_t>>& tree)
        : tree_(tree), parent_(tree.size(), no_vertex()), level_(tree.size(), 0),
          component_size_at_selection_(tree.size(), 0), largest_part_at_selection_(tree.size(), 0),
          removed_(tree.size(), false), traversal_parent_(tree.size(), no_vertex()),
          subtree_size_(tree.size(), 0) {
        detail::validate_tree(tree_);
        build();
    }

    [[nodiscard]] static constexpr std::size_t no_vertex() noexcept {
        return static_cast<std::size_t>(-1);
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return tree_.size();
    }

    [[nodiscard]] std::size_t parent(std::size_t vertex) const {
        check_vertex(vertex);
        return parent_[vertex];
    }

    [[nodiscard]] std::size_t level(std::size_t vertex) const {
        check_vertex(vertex);
        return level_[vertex];
    }

    [[nodiscard]] std::size_t root() const noexcept {
        return root_;
    }

    [[nodiscard]] const std::vector<std::size_t>& parents() const noexcept {
        return parent_;
    }

    [[nodiscard]] const std::vector<std::size_t>& levels() const noexcept {
        return level_;
    }

    [[nodiscard]] std::size_t component_size_at_selection(std::size_t vertex) const {
        check_vertex(vertex);
        return component_size_at_selection_[vertex];
    }

    [[nodiscard]] std::size_t largest_part_at_selection(std::size_t vertex) const {
        check_vertex(vertex);
        return largest_part_at_selection_[vertex];
    }

  private:
    struct Task {
        std::size_t start{};
        std::size_t centroid_parent{};
        std::size_t level{};
    };

    void check_vertex(std::size_t vertex) const {
        if (vertex >= tree_.size()) {
            throw std::out_of_range("CentroidDecomposition vertex is out of range");
        }
    }

    std::size_t find_centroid(std::size_t start, std::vector<std::size_t>& component) {
        component.clear();
        std::vector<std::size_t> stack{start};
        traversal_parent_[start] = start;

        while (!stack.empty()) {
            const std::size_t vertex = stack.back();
            stack.pop_back();
            component.push_back(vertex);
            for (const std::size_t to : tree_[vertex]) {
                if (removed_[to] || to == traversal_parent_[vertex]) {
                    continue;
                }
                traversal_parent_[to] = vertex;
                stack.push_back(to);
            }
        }

        for (auto iterator = component.rbegin(); iterator != component.rend(); ++iterator) {
            const std::size_t vertex = *iterator;
            subtree_size_[vertex] = 1;
            for (const std::size_t to : tree_[vertex]) {
                if (!removed_[to] && traversal_parent_[to] == vertex) {
                    subtree_size_[vertex] += subtree_size_[to];
                }
            }
        }

        const std::size_t component_size = component.size();
        std::size_t best = component.front();
        std::size_t best_largest_part = component_size;
        for (const std::size_t vertex : component) {
            std::size_t largest_part = component_size - subtree_size_[vertex];
            for (const std::size_t to : tree_[vertex]) {
                if (!removed_[to] && traversal_parent_[to] == vertex) {
                    largest_part = std::max(largest_part, subtree_size_[to]);
                }
            }
            if (largest_part < best_largest_part ||
                (largest_part == best_largest_part && vertex < best)) {
                best = vertex;
                best_largest_part = largest_part;
            }
        }
        component_size_at_selection_[best] = component_size;
        largest_part_at_selection_[best] = best_largest_part;
        return best;
    }

    void build() {
        if (tree_.empty()) {
            root_ = no_vertex();
            return;
        }

        std::vector<Task> tasks{Task{0, no_vertex(), 0}};
        std::vector<std::size_t> component;
        component.reserve(tree_.size());

        while (!tasks.empty()) {
            const Task task = tasks.back();
            tasks.pop_back();
            if (removed_[task.start]) {
                continue;
            }

            const std::size_t centroid = find_centroid(task.start, component);
            removed_[centroid] = true;
            parent_[centroid] = task.centroid_parent;
            level_[centroid] = task.level;
            if (task.centroid_parent == no_vertex()) {
                root_ = centroid;
            }

            for (const std::size_t to : tree_[centroid]) {
                if (!removed_[to]) {
                    tasks.push_back(Task{to, centroid, task.level + 1U});
                }
            }
        }
    }

    const std::vector<std::vector<std::size_t>>& tree_;
    std::vector<std::size_t> parent_;
    std::vector<std::size_t> level_;
    std::vector<std::size_t> component_size_at_selection_;
    std::vector<std::size_t> largest_part_at_selection_;
    std::vector<bool> removed_;
    std::vector<std::size_t> traversal_parent_;
    std::vector<std::size_t> subtree_size_;
    std::size_t root_{no_vertex()};
};

} // namespace advanced_algorithms
