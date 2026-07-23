#pragma once

#include <algorithm>
#include <cstddef>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

namespace advanced_algorithms::detail {

inline void validate_tree(const std::vector<std::vector<std::size_t>>& tree) {
    const std::size_t n = tree.size();
    if (n == 0) {
        return;
    }

    std::vector<std::pair<std::size_t, std::size_t>> directed_edges;
    for (std::size_t from = 0; from < n; ++from) {
        for (const std::size_t to : tree[from]) {
            if (to >= n) {
                throw std::out_of_range("Tree edge endpoint is out of range");
            }
            if (from == to) {
                throw std::invalid_argument("Tree must not contain self-loops");
            }
            directed_edges.emplace_back(from, to);
        }
    }

    std::sort(directed_edges.begin(), directed_edges.end());
    if (std::adjacent_find(directed_edges.begin(), directed_edges.end()) != directed_edges.end()) {
        throw std::invalid_argument("Tree adjacency must not contain duplicate edges");
    }
    for (const auto& [from, to] : directed_edges) {
        if (!std::binary_search(directed_edges.begin(), directed_edges.end(), std::pair{to, from})) {
            throw std::invalid_argument("Tree adjacency must be symmetric");
        }
    }
    if (directed_edges.size() != (n - 1U) * 2U) {
        throw std::invalid_argument("Tree must contain exactly n - 1 undirected edges");
    }

    std::vector<bool> visited(n, false);
    std::queue<std::size_t> queue;
    queue.push(0);
    visited[0] = true;
    std::size_t reached = 0;
    while (!queue.empty()) {
        const std::size_t vertex = queue.front();
        queue.pop();
        ++reached;
        for (const std::size_t to : tree[vertex]) {
            if (!visited[to]) {
                visited[to] = true;
                queue.push(to);
            }
        }
    }
    if (reached != n) {
        throw std::invalid_argument("Tree must be connected");
    }
}

struct RootedTreeTraversal {
    std::vector<std::size_t> parent;
    std::vector<std::size_t> depth;
    std::vector<std::size_t> order;
};

inline RootedTreeTraversal root_tree(const std::vector<std::vector<std::size_t>>& tree,
                                     std::size_t root) {
    validate_tree(tree);
    if (tree.empty()) {
        if (root != 0) {
            throw std::out_of_range("Empty tree only accepts root 0");
        }
        return {};
    }
    if (root >= tree.size()) {
        throw std::out_of_range("Tree root is out of range");
    }

    const std::size_t n = tree.size();
    std::vector<std::size_t> parent(n, n);
    std::vector<std::size_t> depth(n, 0);
    std::vector<std::size_t> order;
    order.reserve(n);

    std::vector<std::size_t> stack{root};
    parent[root] = root;
    while (!stack.empty()) {
        const std::size_t vertex = stack.back();
        stack.pop_back();
        order.push_back(vertex);
        for (const std::size_t to : tree[vertex]) {
            if (parent[to] != n) {
                continue;
            }
            parent[to] = vertex;
            depth[to] = depth[vertex] + 1U;
            stack.push_back(to);
        }
    }

    return RootedTreeTraversal{std::move(parent), std::move(depth), std::move(order)};
}

} // namespace advanced_algorithms::detail
