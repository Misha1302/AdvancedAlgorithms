#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace advanced_algorithms {

struct StronglyConnectedComponents {
    std::vector<std::size_t> component_of;
    std::vector<std::vector<std::size_t>> components;
};

inline StronglyConnectedComponents tarjan_scc(const std::vector<std::vector<std::size_t>>& graph) {
    const std::size_t n = graph.size();
    for (const auto& edges : graph) {
        for (const std::size_t to : edges) {
            if (to >= n) {
                throw std::out_of_range("Tarjan SCC edge endpoint is out of range");
            }
        }
    }

    struct Frame {
        std::size_t vertex{};
        std::size_t next_edge{};
        std::size_t parent{};
        bool has_parent{};
    };

    constexpr std::size_t unvisited = static_cast<std::size_t>(-1);
    std::vector<std::size_t> index(n, unvisited);
    std::vector<std::size_t> low(n, 0);
    std::vector<bool> on_stack(n, false);
    std::vector<std::size_t> active;
    std::vector<std::size_t> component_of(n, unvisited);
    std::vector<std::vector<std::size_t>> components;
    std::size_t timer = 0;

    for (std::size_t start = 0; start < n; ++start) {
        if (index[start] != unvisited) {
            continue;
        }

        std::vector<Frame> dfs;
        index[start] = low[start] = timer++;
        active.push_back(start);
        on_stack[start] = true;
        dfs.push_back(Frame{start, 0, 0, false});

        while (!dfs.empty()) {
            Frame& frame = dfs.back();
            const std::size_t vertex = frame.vertex;
            if (frame.next_edge < graph[vertex].size()) {
                const std::size_t to = graph[vertex][frame.next_edge++];
                if (index[to] == unvisited) {
                    index[to] = low[to] = timer++;
                    active.push_back(to);
                    on_stack[to] = true;
                    dfs.push_back(Frame{to, 0, vertex, true});
                } else if (on_stack[to]) {
                    low[vertex] = std::min(low[vertex], index[to]);
                }
                continue;
            }

            const Frame finished = frame;
            dfs.pop_back();
            if (finished.has_parent) {
                low[finished.parent] = std::min(low[finished.parent], low[finished.vertex]);
            }
            if (low[finished.vertex] == index[finished.vertex]) {
                const std::size_t component_id = components.size();
                components.emplace_back();
                while (true) {
                    const std::size_t current = active.back();
                    active.pop_back();
                    on_stack[current] = false;
                    component_of[current] = component_id;
                    components.back().push_back(current);
                    if (current == finished.vertex) {
                        break;
                    }
                }
            }
        }
    }

    return StronglyConnectedComponents{std::move(component_of), std::move(components)};
}

} // namespace advanced_algorithms
