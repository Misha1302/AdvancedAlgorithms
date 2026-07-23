#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

namespace advanced_algorithms {

struct WeightedEdge {
    std::size_t to{};
    std::int64_t weight{};
};

using WeightedGraph = std::vector<std::vector<WeightedEdge>>;

inline std::vector<std::int64_t> dijkstra(const WeightedGraph& graph, std::size_t source) {
    if (source >= graph.size()) {
        throw std::out_of_range("Dijkstra source is out of range");
    }
    for (const auto& edges : graph) {
        for (const auto& edge : edges) {
            if (edge.to >= graph.size()) {
                throw std::out_of_range("Dijkstra edge endpoint is out of range");
            }
            if (edge.weight < 0) {
                throw std::invalid_argument("Dijkstra requires non-negative edge weights");
            }
        }
    }

    constexpr std::int64_t infinity = std::numeric_limits<std::int64_t>::max();
    using QueueItem = std::pair<std::int64_t, std::size_t>;

    std::vector<std::int64_t> distance(graph.size(), infinity);
    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<>> queue;
    distance[source] = 0;
    queue.emplace(0, source);

    while (!queue.empty()) {
        const auto [current_distance, vertex] = queue.top();
        queue.pop();
        if (current_distance != distance[vertex]) {
            continue;
        }
        for (const auto& edge : graph[vertex]) {
            if (current_distance > infinity - edge.weight) {
                continue;
            }
            const std::int64_t candidate = current_distance + edge.weight;
            if (candidate < distance[edge.to]) {
                distance[edge.to] = candidate;
                queue.emplace(candidate, edge.to);
            }
        }
    }

    return distance;
}

} // namespace advanced_algorithms
