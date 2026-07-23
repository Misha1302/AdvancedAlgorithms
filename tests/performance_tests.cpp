#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "advanced_algorithms/data_structures/lazy_segment_tree.hpp"
#include "advanced_algorithms/graphs/dijkstra.hpp"
#include "advanced_algorithms/graphs/dinic.hpp"
#include "advanced_algorithms/graphs/tarjan_scc.hpp"
#include "advanced_algorithms/optimization/simulated_annealing.hpp"
#include "advanced_algorithms/strings/aho_corasick.hpp"
#include "advanced_algorithms/trees/centroid_decomposition.hpp"
#include "advanced_algorithms/trees/heavy_light_decomposition.hpp"
#include "advanced_algorithms/trees/lca_binary_lifting.hpp"

namespace {

using Clock = std::chrono::steady_clock;

template <class Function>
auto measure(const std::string& name, std::chrono::milliseconds limit, Function&& function) {
    const auto started = Clock::now();
    auto result = function();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - started);
    std::cout << name << ": " << elapsed.count() << " ms\n";
    if (elapsed > limit) {
        throw std::runtime_error(name + " exceeded the non-flaky performance budget");
    }
    return result;
}

std::vector<std::vector<std::size_t>> make_chain(std::size_t n) {
    std::vector<std::vector<std::size_t>> tree(n);
    for (std::size_t vertex = 1; vertex < n; ++vertex) {
        tree[vertex - 1U].push_back(vertex);
        tree[vertex].push_back(vertex - 1U);
    }
    return tree;
}

} // namespace

int main() {
    constexpr std::size_t tree_size = 200'000;
    const auto tree = make_chain(tree_size);

    const auto centroid_root = measure("centroid decomposition on a 200k chain",
                                       std::chrono::seconds(15), [&] {
        const advanced_algorithms::CentroidDecomposition decomposition(tree);
        return decomposition.root();
    });
    if (centroid_root >= tree_size) {
        throw std::runtime_error("invalid centroid root");
    }

    const auto lca_distance = measure("LCA build and query on a 200k chain",
                                      std::chrono::seconds(8), [&] {
        const advanced_algorithms::LcaBinaryLifting lca(tree);
        return lca.distance(0, tree_size - 1U);
    });
    if (lca_distance != tree_size - 1U) {
        throw std::runtime_error("invalid LCA distance");
    }

    const auto hld_segments = measure("HLD build and path split on a 200k chain",
                                      std::chrono::seconds(8), [&] {
        const advanced_algorithms::HeavyLightDecomposition hld(tree);
        std::size_t segments = 0;
        hld.for_each_path_segment(0, tree_size - 1U,
                                  [&](std::size_t, std::size_t) { ++segments; });
        return segments;
    });
    if (hld_segments != 1) {
        throw std::runtime_error("chain must be represented by one HLD segment");
    }

    measure("iterative Tarjan SCC on a 500k path", std::chrono::seconds(8), [&] {
        constexpr std::size_t n = 500'000;
        std::vector<std::vector<std::size_t>> graph(n);
        for (std::size_t vertex = 1; vertex < n; ++vertex) {
            graph[vertex - 1U].push_back(vertex);
        }
        const auto result = advanced_algorithms::tarjan_scc(graph);
        if (result.components.size() != n) {
            throw std::runtime_error("unexpected SCC count");
        }
        return result.components.size();
    });

    measure("lazy segment tree 300k mixed operations", std::chrono::seconds(8), [&] {
        constexpr std::size_t n = 200'000;
        std::vector<std::int64_t> values(n, 0);
        advanced_algorithms::LazySegmentTree segment_tree(values);
        std::uint64_t checksum = 0;
        for (std::size_t operation = 0; operation < 300'000; ++operation) {
            const std::size_t left = operation % n;
            const std::size_t right = std::min(n, left + 1U + operation % 1'000U);
            if ((operation & 1U) == 0U) {
                segment_tree.range_add(left, right, 1);
            } else {
                checksum ^= static_cast<std::uint64_t>(segment_tree.range_sum(left, right));
            }
        }
        return checksum;
    });

    measure("Dinic on 100k parallel unit paths", std::chrono::seconds(8), [&] {
        constexpr std::size_t paths = 100'000;
        const std::size_t sink = paths + 1U;
        advanced_algorithms::Dinic flow(sink + 1U);
        for (std::size_t path = 0; path < paths; ++path) {
            const std::size_t middle = path + 1U;
            flow.add_edge(0, middle, 1);
            flow.add_edge(middle, sink, 1);
        }
        const auto result = flow.max_flow(0, sink);
        if (result != static_cast<std::int64_t>(paths)) {
            throw std::runtime_error("unexpected max flow");
        }
        return result;
    });

    measure("Dijkstra on a 200k sparse graph", std::chrono::seconds(8), [&] {
        constexpr std::size_t n = 200'000;
        advanced_algorithms::WeightedGraph graph(n);
        for (std::size_t vertex = 1; vertex < n; ++vertex) {
            graph[vertex - 1U].push_back({vertex, 1});
            if (vertex > 1) {
                graph[vertex - 2U].push_back({vertex, 3});
            }
        }
        const auto distance = advanced_algorithms::dijkstra(graph, 0);
        if (distance.back() != static_cast<std::int64_t>(n - 1U)) {
            throw std::runtime_error("unexpected shortest path");
        }
        return distance.back();
    });

    measure("Aho-Corasick 20k patterns over 1M characters", std::chrono::seconds(10), [&] {
        advanced_algorithms::AhoCorasick automaton;
        for (std::size_t pattern = 0; pattern < 20'000; ++pattern) {
            automaton.add_pattern("p" + std::to_string(pattern) + "x");
        }
        automaton.build();
        std::string text;
        text.reserve(1'000'000);
        while (text.size() < 1'000'000) {
            text += "p19999x-noise-";
        }
        text.resize(1'000'000);
        return automaton.match_counts(text)[19'999];
    });

    measure("simulated annealing time limit", std::chrono::milliseconds(500), [&] {
        advanced_algorithms::SimulatedAnnealingConfig config;
        config.iterations = 1'000'000'000U;
        config.start_temperature = 100.0;
        config.end_temperature = 0.001;
        config.time_limit = std::chrono::milliseconds(25);
        const auto result = advanced_algorithms::simulated_annealing(
            std::int64_t{10'000},
            [](std::int64_t value) { return value * value; },
            [](std::int64_t& value, std::mt19937_64& random) {
                value += (random() & 1U) == 0U ? 1 : -1;
            },
            config);
        if (result.iterations_completed >= config.iterations) {
            throw std::runtime_error("time limit was ignored");
        }
        return result.best_score;
    });

    return 0;
}
