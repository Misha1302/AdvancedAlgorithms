#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace advanced_algorithms {

class UndirectedGraph {
  public:
    explicit UndirectedGraph(std::size_t vertex_count) : adjacency_(vertex_count) {}

    std::size_t add_edge(std::size_t lhs, std::size_t rhs) {
        check_vertex(lhs);
        check_vertex(rhs);
        const std::size_t id = edges_.size();
        edges_.emplace_back(lhs, rhs);
        adjacency_[lhs].emplace_back(rhs, id);
        adjacency_[rhs].emplace_back(lhs, id);
        return id;
    }

    [[nodiscard]] std::size_t vertex_count() const noexcept {
        return adjacency_.size();
    }

    [[nodiscard]] const std::vector<std::pair<std::size_t, std::size_t>>&
    adjacency(std::size_t vertex) const {
        check_vertex(vertex);
        return adjacency_[vertex];
    }

    [[nodiscard]] const std::pair<std::size_t, std::size_t>& edge(std::size_t id) const {
        if (id >= edges_.size()) {
            throw std::out_of_range("UndirectedGraph edge id is out of range");
        }
        return edges_[id];
    }

    [[nodiscard]] std::size_t edge_count() const noexcept {
        return edges_.size();
    }

  private:
    void check_vertex(std::size_t vertex) const {
        if (vertex >= adjacency_.size()) {
            throw std::out_of_range("UndirectedGraph vertex is out of range");
        }
    }

    std::vector<std::vector<std::pair<std::size_t, std::size_t>>> adjacency_;
    std::vector<std::pair<std::size_t, std::size_t>> edges_;
};

struct LowLinkResult {
    std::vector<std::size_t> bridge_edge_ids;
    std::vector<bool> is_articulation;
};

inline LowLinkResult bridges_and_articulation_points(const UndirectedGraph& graph) {
    const std::size_t n = graph.vertex_count();
    constexpr std::size_t unvisited = static_cast<std::size_t>(-1);

    struct Frame {
        std::size_t vertex{};
        std::size_t parent{};
        std::size_t parent_edge{};
        std::size_t next_edge{};
        std::size_t child_count{};
        bool has_parent{};
    };

    std::vector<std::size_t> tin(n, unvisited);
    std::vector<std::size_t> low(n, 0);
    std::vector<bool> articulation(n, false);
    std::vector<std::size_t> bridges;
    std::size_t timer = 0;

    for (std::size_t root = 0; root < n; ++root) {
        if (tin[root] != unvisited) {
            continue;
        }
        tin[root] = low[root] = timer++;
        std::vector<Frame> dfs;
        dfs.push_back(Frame{root, 0, 0, 0, 0, false});

        while (!dfs.empty()) {
            Frame& frame = dfs.back();
            const auto& edges = graph.adjacency(frame.vertex);
            if (frame.next_edge < edges.size()) {
                const auto [to, edge_id] = edges[frame.next_edge++];
                if (frame.has_parent && edge_id == frame.parent_edge) {
                    continue;
                }
                if (tin[to] == unvisited) {
                    ++frame.child_count;
                    tin[to] = low[to] = timer++;
                    dfs.push_back(Frame{to, frame.vertex, edge_id, 0, 0, true});
                } else {
                    low[frame.vertex] = std::min(low[frame.vertex], tin[to]);
                }
                continue;
            }

            const Frame finished = frame;
            dfs.pop_back();
            if (!finished.has_parent) {
                articulation[finished.vertex] = finished.child_count > 1;
                continue;
            }

            low[finished.parent] = std::min(low[finished.parent], low[finished.vertex]);
            if (low[finished.vertex] > tin[finished.parent]) {
                bridges.push_back(finished.parent_edge);
            }
            if (!dfs.empty() && dfs.back().has_parent &&
                low[finished.vertex] >= tin[finished.parent]) {
                articulation[finished.parent] = true;
            }
        }
    }

    std::sort(bridges.begin(), bridges.end());
    return LowLinkResult{std::move(bridges), std::move(articulation)};
}

} // namespace advanced_algorithms
