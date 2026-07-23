#include "test_framework.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

#include "advanced_algorithms/graphs/bridges_articulation.hpp"
#include "advanced_algorithms/graphs/dijkstra.hpp"
#include "advanced_algorithms/graphs/dinic.hpp"
#include "advanced_algorithms/graphs/tarjan_scc.hpp"

using advanced_algorithms::Dinic;
using advanced_algorithms::UndirectedGraph;
using advanced_algorithms::WeightedEdge;
using advanced_algorithms::WeightedGraph;
using advanced_algorithms::bridges_and_articulation_points;
using advanced_algorithms::dijkstra;
using advanced_algorithms::tarjan_scc;

namespace {

std::vector<std::int64_t> bellman_ford(const WeightedGraph& graph, std::size_t source) {
    constexpr std::int64_t infinity = std::numeric_limits<std::int64_t>::max();
    std::vector<std::int64_t> distance(graph.size(), infinity);
    distance[source] = 0;
    for (std::size_t iteration = 1; iteration < graph.size(); ++iteration) {
        bool changed = false;
        for (std::size_t from = 0; from < graph.size(); ++from) {
            if (distance[from] == infinity) {
                continue;
            }
            for (const auto& edge : graph[from]) {
                const std::int64_t candidate = distance[from] + edge.weight;
                if (candidate < distance[edge.to]) {
                    distance[edge.to] = candidate;
                    changed = true;
                }
            }
        }
        if (!changed) {
            break;
        }
    }
    return distance;
}

std::int64_t edmonds_karp(std::vector<std::vector<std::int64_t>> capacity,
                          std::size_t source, std::size_t sink) {
    const std::size_t n = capacity.size();
    std::int64_t answer = 0;
    while (true) {
        std::vector<std::size_t> parent(n, n);
        std::queue<std::size_t> queue;
        parent[source] = source;
        queue.push(source);
        while (!queue.empty() && parent[sink] == n) {
            const std::size_t vertex = queue.front();
            queue.pop();
            for (std::size_t to = 0; to < n; ++to) {
                if (parent[to] == n && capacity[vertex][to] > 0) {
                    parent[to] = vertex;
                    queue.push(to);
                }
            }
        }
        if (parent[sink] == n) {
            return answer;
        }
        std::int64_t pushed = std::numeric_limits<std::int64_t>::max();
        for (std::size_t vertex = sink; vertex != source; vertex = parent[vertex]) {
            pushed = std::min(pushed, capacity[parent[vertex]][vertex]);
        }
        for (std::size_t vertex = sink; vertex != source; vertex = parent[vertex]) {
            capacity[parent[vertex]][vertex] -= pushed;
            capacity[vertex][parent[vertex]] += pushed;
        }
        answer += pushed;
    }
}

std::vector<std::size_t> kosaraju_component_ids(const std::vector<std::vector<std::size_t>>& graph) {
    const std::size_t n = graph.size();
    std::vector<std::vector<std::size_t>> reverse_graph(n);
    for (std::size_t from = 0; from < n; ++from) {
        for (const std::size_t to : graph[from]) {
            reverse_graph[to].push_back(from);
        }
    }

    std::vector<bool> visited(n, false);
    std::vector<std::size_t> order;
    for (std::size_t start = 0; start < n; ++start) {
        if (visited[start]) {
            continue;
        }
        std::vector<std::pair<std::size_t, std::size_t>> stack{{start, 0}};
        visited[start] = true;
        while (!stack.empty()) {
            auto& [vertex, next] = stack.back();
            if (next < graph[vertex].size()) {
                const std::size_t to = graph[vertex][next++];
                if (!visited[to]) {
                    visited[to] = true;
                    stack.emplace_back(to, 0);
                }
            } else {
                order.push_back(vertex);
                stack.pop_back();
            }
        }
    }

    const std::size_t unassigned = n;
    std::vector<std::size_t> component(n, unassigned);
    std::size_t component_id = 0;
    for (auto iterator = order.rbegin(); iterator != order.rend(); ++iterator) {
        if (component[*iterator] != unassigned) {
            continue;
        }
        std::vector<std::size_t> stack{*iterator};
        component[*iterator] = component_id;
        while (!stack.empty()) {
            const std::size_t vertex = stack.back();
            stack.pop_back();
            for (const std::size_t to : reverse_graph[vertex]) {
                if (component[to] == unassigned) {
                    component[to] = component_id;
                    stack.push_back(to);
                }
            }
        }
        ++component_id;
    }
    return component;
}

std::size_t component_count(const UndirectedGraph& graph,
                            std::size_t removed_edge,
                            std::size_t removed_vertex) {
    const std::size_t n = graph.vertex_count();
    std::vector<bool> visited(n, false);
    std::size_t count = 0;
    for (std::size_t start = 0; start < n; ++start) {
        if (start == removed_vertex || visited[start]) {
            continue;
        }
        ++count;
        std::vector<std::size_t> stack{start};
        visited[start] = true;
        while (!stack.empty()) {
            const std::size_t vertex = stack.back();
            stack.pop_back();
            for (const auto& [to, edge_id] : graph.adjacency(vertex)) {
                if (edge_id == removed_edge || to == removed_vertex || visited[to]) {
                    continue;
                }
                visited[to] = true;
                stack.push_back(to);
            }
        }
    }
    return count;
}

} // namespace

AA_TEST(dijkstra_fixed_and_invalid_inputs) {
    WeightedGraph graph(5);
    graph[0] = {{1, 4}, {2, 1}};
    graph[2] = {{1, 2}, {3, 5}};
    graph[1] = {{3, 1}};
    const auto distance = dijkstra(graph, 0);
    AA_REQUIRE_EQ(distance[0], std::int64_t{0});
    AA_REQUIRE_EQ(distance[1], std::int64_t{3});
    AA_REQUIRE_EQ(distance[3], std::int64_t{4});
    AA_REQUIRE_EQ(distance[4], std::numeric_limits<std::int64_t>::max());

    graph[4].push_back(WeightedEdge{0, -1});
    AA_REQUIRE_THROWS_AS(dijkstra(graph, 0), std::invalid_argument);
    AA_REQUIRE_THROWS_AS(dijkstra(graph, 5), std::out_of_range);
}

AA_TEST(dijkstra_randomized_against_bellman_ford) {
    std::mt19937_64 random(0xD1A57AULL);
    for (std::size_t test = 0; test < 1'000; ++test) {
        const std::size_t n = 1U + static_cast<std::size_t>(random() % 20U);
        WeightedGraph graph(n);
        for (std::size_t from = 0; from < n; ++from) {
            for (std::size_t to = 0; to < n; ++to) {
                if ((random() % 7U) == 0U) {
                    graph[from].push_back(WeightedEdge{to,
                        static_cast<std::int64_t>(random() % 100U)});
                }
            }
        }
        const std::size_t source = static_cast<std::size_t>(random() % n);
        AA_REQUIRE(dijkstra(graph, source) == bellman_ford(graph, source));
    }
}

AA_TEST(dinic_fixed_parallel_zero_and_limit) {
    Dinic flow(4);
    const std::size_t first = flow.add_edge(0, 1, 3);
    flow.add_edge(0, 1, 2);
    flow.add_edge(1, 2, 4);
    flow.add_edge(2, 3, 4);
    flow.add_edge(1, 3, 1);
    flow.add_edge(0, 3, 0);
    AA_REQUIRE_EQ(flow.max_flow(0, 3, 3), std::int64_t{3});
    AA_REQUIRE_EQ(flow.max_flow(0, 3), std::int64_t{2});
    AA_REQUIRE_EQ(flow.edge(first).capacity, std::int64_t{3});
    AA_REQUIRE_EQ(flow.max_flow(0, 0), std::int64_t{0});
    AA_REQUIRE_THROWS_AS(flow.add_edge(0, 4, 1), std::out_of_range);
    AA_REQUIRE_THROWS_AS(flow.add_edge(0, 1, -1), std::invalid_argument);
}

AA_TEST(dinic_randomized_against_edmonds_karp) {
    std::mt19937_64 random(0xF10AULL);
    for (std::size_t test = 0; test < 500; ++test) {
        const std::size_t n = 2U + static_cast<std::size_t>(random() % 9U);
        Dinic flow(n);
        std::vector capacity(n, std::vector<std::int64_t>(n, 0));
        const std::size_t edge_count = static_cast<std::size_t>(random() % 60U);
        for (std::size_t edge = 0; edge < edge_count; ++edge) {
            const std::size_t from = static_cast<std::size_t>(random() % n);
            const std::size_t to = static_cast<std::size_t>(random() % n);
            const std::int64_t cap = static_cast<std::int64_t>(random() % 21U);
            flow.add_edge(from, to, cap);
            capacity[from][to] += cap;
        }
        AA_REQUIRE_EQ(flow.max_flow(0, n - 1U), edmonds_karp(capacity, 0, n - 1U));
    }
}

AA_TEST(tarjan_fixed_cases_and_deep_cycle) {
    const std::vector<std::vector<std::size_t>> graph{{1}, {2}, {0, 3}, {4}, {3}, {}};
    const auto result = tarjan_scc(graph);
    AA_REQUIRE_EQ(result.components.size(), std::size_t{3});
    AA_REQUIRE_EQ(result.component_of[0], result.component_of[2]);
    AA_REQUIRE_EQ(result.component_of[3], result.component_of[4]);
    AA_REQUIRE(result.component_of[0] != result.component_of[3]);
    AA_REQUIRE(result.component_of[5] != result.component_of[4]);

    auto invalid = graph;
    invalid[0].push_back(100);
    AA_REQUIRE_THROWS_AS(tarjan_scc(invalid), std::out_of_range);
}

AA_TEST(tarjan_randomized_against_kosaraju) {
    std::mt19937_64 random(0x5CCULL);
    for (std::size_t test = 0; test < 1'000; ++test) {
        const std::size_t n = 1U + static_cast<std::size_t>(random() % 25U);
        std::vector<std::vector<std::size_t>> graph(n);
        for (std::size_t from = 0; from < n; ++from) {
            for (std::size_t to = 0; to < n; ++to) {
                if ((random() % 5U) == 0U) {
                    graph[from].push_back(to);
                }
            }
        }
        const auto tarjan = tarjan_scc(graph).component_of;
        const auto kosaraju = kosaraju_component_ids(graph);
        for (std::size_t lhs = 0; lhs < n; ++lhs) {
            for (std::size_t rhs = 0; rhs < n; ++rhs) {
                AA_REQUIRE_EQ(tarjan[lhs] == tarjan[rhs], kosaraju[lhs] == kosaraju[rhs]);
            }
        }
    }
}

AA_TEST(low_link_handles_parallel_edges_and_self_loops) {
    UndirectedGraph graph(5);
    const std::size_t parallel_a = graph.add_edge(0, 1);
    const std::size_t parallel_b = graph.add_edge(0, 1);
    const std::size_t bridge = graph.add_edge(1, 2);
    graph.add_edge(2, 3);
    graph.add_edge(3, 4);
    graph.add_edge(4, 2);
    graph.add_edge(4, 4);
    const auto result = bridges_and_articulation_points(graph);
    AA_REQUIRE(std::find(result.bridge_edge_ids.begin(), result.bridge_edge_ids.end(), bridge) !=
               result.bridge_edge_ids.end());
    AA_REQUIRE(std::find(result.bridge_edge_ids.begin(), result.bridge_edge_ids.end(), parallel_a) ==
               result.bridge_edge_ids.end());
    AA_REQUIRE(std::find(result.bridge_edge_ids.begin(), result.bridge_edge_ids.end(), parallel_b) ==
               result.bridge_edge_ids.end());
    AA_REQUIRE(result.is_articulation[1]);
    AA_REQUIRE(result.is_articulation[2]);
}

AA_TEST(low_link_randomized_against_removal_oracle) {
    std::mt19937_64 random(0xB21D6EULL);
    constexpr std::size_t none = static_cast<std::size_t>(-1);
    for (std::size_t test = 0; test < 500; ++test) {
        const std::size_t n = 1U + static_cast<std::size_t>(random() % 10U);
        UndirectedGraph graph(n);
        const std::size_t edges = static_cast<std::size_t>(random() % 25U);
        for (std::size_t edge = 0; edge < edges; ++edge) {
            graph.add_edge(static_cast<std::size_t>(random() % n),
                           static_cast<std::size_t>(random() % n));
        }
        const auto result = bridges_and_articulation_points(graph);
        const std::size_t baseline = component_count(graph, none, none);
        std::vector<bool> is_bridge(graph.edge_count(), false);
        for (const std::size_t id : result.bridge_edge_ids) {
            is_bridge[id] = true;
        }
        for (std::size_t id = 0; id < graph.edge_count(); ++id) {
            AA_REQUIRE_EQ(is_bridge[id], component_count(graph, id, none) > baseline);
        }
        for (std::size_t vertex = 0; vertex < n; ++vertex) {
            AA_REQUIRE_EQ(result.is_articulation[vertex],
                          component_count(graph, none, vertex) > baseline);
        }
    }
}
