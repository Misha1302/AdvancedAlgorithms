#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "advanced_algorithms/graphs/dinic.hpp"

int main() {
    constexpr std::size_t vertex_count = 200'000;
    advanced_algorithms::Dinic flow(vertex_count);
    for (std::size_t vertex = 1; vertex < vertex_count; ++vertex) {
        flow.add_edge(vertex - 1U, vertex, 1);
    }

    const std::int64_t result = flow.max_flow(0, vertex_count - 1U);
    if (result != 1) {
        throw std::runtime_error("unexpected max flow on a deep path");
    }
    std::cout << "iterative Dinic deep-path regression: OK\n";
    return 0;
}
