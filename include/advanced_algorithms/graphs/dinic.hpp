#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>
#include <vector>

namespace advanced_algorithms {

class Dinic {
  public:
    using Capacity = std::int64_t;

    struct EdgeView {
        std::size_t from{};
        std::size_t to{};
        Capacity capacity{};
        Capacity flow{};
    };

    explicit Dinic(std::size_t vertex_count)
        : graph_(vertex_count), level_(vertex_count), next_edge_(vertex_count) {}

    [[nodiscard]] std::size_t vertex_count() const noexcept {
        return graph_.size();
    }

    std::size_t add_edge(std::size_t from, std::size_t to, Capacity capacity) {
        check_vertex(from);
        check_vertex(to);
        if (capacity < 0) {
            throw std::invalid_argument("Dinic capacity must be non-negative");
        }
        const std::size_t id = original_edges_.size();
        const std::size_t forward_index = graph_[from].size();
        const std::size_t reverse_index = graph_[to].size() + (from == to ? 1U : 0U);
        graph_[from].push_back(ResidualEdge{to, reverse_index, capacity});
        graph_[to].push_back(ResidualEdge{from, forward_index, 0});
        original_edges_.push_back(OriginalEdge{from, forward_index, capacity});
        return id;
    }

    Capacity max_flow(std::size_t source, std::size_t sink,
                      Capacity limit = std::numeric_limits<Capacity>::max()) {
        check_vertex(source);
        check_vertex(sink);
        if (limit < 0) {
            throw std::invalid_argument("Dinic flow limit must be non-negative");
        }
        if (source == sink || limit == 0) {
            return 0;
        }

        Capacity total = 0;
        while (total < limit && build_levels(source, sink)) {
            std::fill(next_edge_.begin(), next_edge_.end(), std::size_t{0});
            while (total < limit) {
                const Capacity pushed = send_flow(source, sink, limit - total);
                if (pushed == 0) {
                    break;
                }
                total += pushed;
            }
        }
        return total;
    }

    [[nodiscard]] EdgeView edge(std::size_t id) const {
        if (id >= original_edges_.size()) {
            throw std::out_of_range("Dinic edge id is out of range");
        }
        const auto& original = original_edges_[id];
        const auto& residual = graph_[original.from][original.index];
        return EdgeView{original.from, residual.to, original.capacity,
                        original.capacity - residual.capacity};
    }

    [[nodiscard]] std::vector<EdgeView> edges() const {
        std::vector<EdgeView> result;
        result.reserve(original_edges_.size());
        for (std::size_t id = 0; id < original_edges_.size(); ++id) {
            result.push_back(edge(id));
        }
        return result;
    }

  private:
    struct ResidualEdge {
        std::size_t to{};
        std::size_t reverse_index{};
        Capacity capacity{};
    };

    struct OriginalEdge {
        std::size_t from{};
        std::size_t index{};
        Capacity capacity{};
    };

    void check_vertex(std::size_t vertex) const {
        if (vertex >= graph_.size()) {
            throw std::out_of_range("Dinic vertex is out of range");
        }
    }

    bool build_levels(std::size_t source, std::size_t sink) {
        std::fill(level_.begin(), level_.end(), -1);
        std::queue<std::size_t> queue;
        level_[source] = 0;
        queue.push(source);
        while (!queue.empty()) {
            const std::size_t vertex = queue.front();
            queue.pop();
            for (const auto& edge : graph_[vertex]) {
                if (edge.capacity > 0 && level_[edge.to] == -1) {
                    level_[edge.to] = level_[vertex] + 1;
                    queue.push(edge.to);
                }
            }
        }
        return level_[sink] != -1;
    }

    Capacity send_flow(std::size_t source, std::size_t sink, Capacity pushed) {
        path_vertices_.clear();
        path_edges_.clear();
        path_capacities_.clear();

        path_vertices_.push_back(source);
        path_capacities_.push_back(pushed);

        while (!path_vertices_.empty()) {
            const std::size_t vertex = path_vertices_.back();
            if (vertex == sink) {
                const Capacity sent = path_capacities_.back();
                for (std::size_t depth = 0; depth < path_edges_.size(); ++depth) {
                    const std::size_t from = path_vertices_[depth];
                    ResidualEdge& edge = graph_[from][path_edges_[depth]];
                    edge.capacity -= sent;
                    graph_[edge.to][edge.reverse_index].capacity += sent;
                }
                return sent;
            }

            std::size_t& edge_index = next_edge_[vertex];
            while (edge_index < graph_[vertex].size()) {
                const ResidualEdge& edge = graph_[vertex][edge_index];
                if (edge.capacity > 0 && level_[edge.to] == level_[vertex] + 1) {
                    break;
                }
                ++edge_index;
            }

            if (edge_index == graph_[vertex].size()) {
                level_[vertex] = -1;
                path_vertices_.pop_back();
                path_capacities_.pop_back();
                if (!path_edges_.empty()) {
                    path_edges_.pop_back();
                    if (!path_vertices_.empty()) {
                        ++next_edge_[path_vertices_.back()];
                    }
                }
                continue;
            }

            const ResidualEdge& edge = graph_[vertex][edge_index];
            path_edges_.push_back(edge_index);
            path_vertices_.push_back(edge.to);
            path_capacities_.push_back(std::min(path_capacities_.back(), edge.capacity));
        }

        return 0;
    }

    std::vector<std::vector<ResidualEdge>> graph_;
    std::vector<int> level_;
    std::vector<std::size_t> next_edge_;
    std::vector<OriginalEdge> original_edges_;
    std::vector<std::size_t> path_vertices_;
    std::vector<std::size_t> path_edges_;
    std::vector<Capacity> path_capacities_;
};

} // namespace advanced_algorithms
