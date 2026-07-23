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
        const std::size_t reverse_index = graph_[to].size();
        graph_[from].push_back(ResidualEdge{to, reverse_index, capacity, id, true});
        graph_[to].push_back(ResidualEdge{from, forward_index, 0, id, false});
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
        std::size_t original_id{};
        bool forward{};
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

    Capacity send_flow(std::size_t vertex, std::size_t sink, Capacity pushed) {
        if (vertex == sink) {
            return pushed;
        }
        for (std::size_t& index = next_edge_[vertex]; index < graph_[vertex].size(); ++index) {
            ResidualEdge& edge = graph_[vertex][index];
            if (edge.capacity <= 0 || level_[edge.to] != level_[vertex] + 1) {
                continue;
            }
            const Capacity sent = send_flow(edge.to, sink, std::min(pushed, edge.capacity));
            if (sent == 0) {
                continue;
            }
            edge.capacity -= sent;
            graph_[edge.to][edge.reverse_index].capacity += sent;
            return sent;
        }
        return 0;
    }

    std::vector<std::vector<ResidualEdge>> graph_;
    std::vector<int> level_;
    std::vector<std::size_t> next_edge_;
    std::vector<OriginalEdge> original_edges_;
};

} // namespace advanced_algorithms
