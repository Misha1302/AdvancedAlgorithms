#include <cstddef>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "advanced_algorithms/trees/centroid_decomposition.hpp"
#include "advanced_algorithms/trees/lca_binary_lifting.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::size_t vertex_count = 0;
    std::size_t query_count = 0;
    if (!(std::cin >> vertex_count >> query_count)) {
        return 0;
    }

    std::vector<std::vector<std::size_t>> tree(vertex_count);
    for (std::size_t edge = 1; edge < vertex_count; ++edge) {
        std::size_t lhs = 0;
        std::size_t rhs = 0;
        std::cin >> lhs >> rhs;
        --lhs;
        --rhs;
        tree[lhs].push_back(rhs);
        tree[rhs].push_back(lhs);
    }

    const advanced_algorithms::CentroidDecomposition decomposition(tree);
    const advanced_algorithms::LcaBinaryLifting lca(tree);
    constexpr std::size_t infinity = std::numeric_limits<std::size_t>::max() / 4U;
    std::vector<std::size_t> best(vertex_count, infinity);

    const auto mark = [&](std::size_t vertex) {
        for (std::size_t centroid = vertex;
             centroid != advanced_algorithms::CentroidDecomposition::no_vertex();
             centroid = decomposition.parent(centroid)) {
            best[centroid] = std::min(best[centroid], lca.distance(vertex, centroid));
        }
    };

    const auto query = [&](std::size_t vertex) {
        std::size_t answer = infinity;
        for (std::size_t centroid = vertex;
             centroid != advanced_algorithms::CentroidDecomposition::no_vertex();
             centroid = decomposition.parent(centroid)) {
            if (best[centroid] != infinity) {
                answer = std::min(answer, best[centroid] + lca.distance(vertex, centroid));
            }
        }
        return answer;
    };

    for (std::size_t query_index = 0; query_index < query_count; ++query_index) {
        std::string command;
        std::size_t vertex = 0;
        std::cin >> command >> vertex;
        --vertex;
        if (command == "mark") {
            mark(vertex);
        } else if (command == "query") {
            const std::size_t answer = query(vertex);
            if (answer == infinity) {
                std::cout << -1 << '\n';
            } else {
                std::cout << answer << '\n';
            }
        }
    }
}
