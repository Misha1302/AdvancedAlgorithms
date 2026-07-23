#include "test_framework.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

#include "advanced_algorithms/trees/centroid_decomposition.hpp"
#include "advanced_algorithms/trees/heavy_light_decomposition.hpp"
#include "advanced_algorithms/trees/lca_binary_lifting.hpp"

using advanced_algorithms::CentroidDecomposition;
using advanced_algorithms::HeavyLightDecomposition;
using advanced_algorithms::LcaBinaryLifting;

namespace {

std::vector<std::vector<std::size_t>> make_tree_from_parents(const std::vector<std::size_t>& parent) {
    std::vector<std::vector<std::size_t>> tree(parent.size());
    for (std::size_t vertex = 1; vertex < parent.size(); ++vertex) {
        tree[vertex].push_back(parent[vertex]);
        tree[parent[vertex]].push_back(vertex);
    }
    return tree;
}

std::vector<std::vector<std::size_t>> random_tree(std::size_t n, std::mt19937_64& random) {
    std::vector<std::size_t> parent(n, 0);
    for (std::size_t vertex = 1; vertex < n; ++vertex) {
        parent[vertex] = static_cast<std::size_t>(random() % vertex);
    }
    return make_tree_from_parents(parent);
}

std::size_t naive_lca(std::size_t lhs, std::size_t rhs,
                      const std::vector<std::size_t>& parent,
                      const std::vector<std::size_t>& depth) {
    while (depth[lhs] > depth[rhs]) {
        lhs = parent[lhs];
    }
    while (depth[rhs] > depth[lhs]) {
        rhs = parent[rhs];
    }
    while (lhs != rhs) {
        lhs = parent[lhs];
        rhs = parent[rhs];
    }
    return lhs;
}

std::vector<std::size_t> naive_path(std::size_t lhs, std::size_t rhs,
                                    const std::vector<std::size_t>& parent,
                                    const std::vector<std::size_t>& depth) {
    const std::size_t ancestor = naive_lca(lhs, rhs, parent, depth);
    std::vector<std::size_t> result;
    for (std::size_t vertex = lhs; vertex != ancestor; vertex = parent[vertex]) {
        result.push_back(vertex);
    }
    result.push_back(ancestor);
    std::vector<std::size_t> suffix;
    for (std::size_t vertex = rhs; vertex != ancestor; vertex = parent[vertex]) {
        suffix.push_back(vertex);
    }
    result.insert(result.end(), suffix.rbegin(), suffix.rend());
    return result;
}

void rooted_parent_depth(const std::vector<std::vector<std::size_t>>& tree,
                         std::vector<std::size_t>& parent,
                         std::vector<std::size_t>& depth) {
    const std::size_t n = tree.size();
    parent.assign(n, n);
    depth.assign(n, 0);
    parent[0] = 0;
    std::vector<std::size_t> stack{0};
    while (!stack.empty()) {
        const std::size_t vertex = stack.back();
        stack.pop_back();
        for (const std::size_t to : tree[vertex]) {
            if (parent[to] == n) {
                parent[to] = vertex;
                depth[to] = depth[vertex] + 1U;
                stack.push_back(to);
            }
        }
    }
}

void verify_centroid_decomposition(const std::vector<std::vector<std::size_t>>& tree) {
    const CentroidDecomposition decomposition(tree);
    if (tree.empty()) {
        AA_REQUIRE_EQ(decomposition.root(), CentroidDecomposition::no_vertex());
        return;
    }

    std::size_t roots = 0;
    for (std::size_t vertex = 0; vertex < tree.size(); ++vertex) {
        AA_REQUIRE(decomposition.largest_part_at_selection(vertex) * 2U <=
                   decomposition.component_size_at_selection(vertex));
        const std::size_t parent = decomposition.parent(vertex);
        if (parent == CentroidDecomposition::no_vertex()) {
            ++roots;
            AA_REQUIRE_EQ(vertex, decomposition.root());
            AA_REQUIRE_EQ(decomposition.level(vertex), std::size_t{0});
        } else {
            AA_REQUIRE(parent < tree.size());
            AA_REQUIRE_EQ(decomposition.level(vertex), decomposition.level(parent) + 1U);
        }
    }
    AA_REQUIRE_EQ(roots, std::size_t{1});
}

} // namespace

AA_TEST(centroid_decomposition_single_chain_star_and_random) {
    verify_centroid_decomposition({{}});

    constexpr std::size_t chain_size = 2'001;
    std::vector<std::size_t> chain_parent(chain_size, 0);
    for (std::size_t vertex = 1; vertex < chain_size; ++vertex) {
        chain_parent[vertex] = vertex - 1U;
    }
    const auto chain = make_tree_from_parents(chain_parent);
    verify_centroid_decomposition(chain);
    const CentroidDecomposition chain_decomposition(chain);
    const std::size_t maximum_expected_level = std::bit_width(chain_size) + 1U;
    for (const std::size_t level : chain_decomposition.levels()) {
        AA_REQUIRE(level <= maximum_expected_level);
    }

    constexpr std::size_t star_size = 1'000;
    std::vector<std::size_t> star_parent(star_size, 0);
    verify_centroid_decomposition(make_tree_from_parents(star_parent));

    std::mt19937_64 random(0xCE47A01DULL);
    for (std::size_t test = 0; test < 100; ++test) {
        verify_centroid_decomposition(random_tree(1U + static_cast<std::size_t>(random() % 500U), random));
    }
}

AA_TEST(tree_validation_rejects_non_trees) {
    AA_REQUIRE_THROWS_AS(CentroidDecomposition({{1}, {}}), std::invalid_argument);
    AA_REQUIRE_THROWS_AS(CentroidDecomposition({{1, 2}, {0, 2}, {0, 1}}), std::invalid_argument);
    AA_REQUIRE_THROWS_AS(CentroidDecomposition({{0}}), std::invalid_argument);
}

AA_TEST(lca_binary_lifting_randomized_against_naive) {
    std::mt19937_64 random(0x1CAULL);
    for (std::size_t test = 0; test < 100; ++test) {
        const std::size_t n = 1U + static_cast<std::size_t>(random() % 1'000U);
        const auto tree = random_tree(n, random);
        const LcaBinaryLifting lca(tree);
        std::vector<std::size_t> parent;
        std::vector<std::size_t> depth;
        rooted_parent_depth(tree, parent, depth);

        for (std::size_t query = 0; query < 5'000; ++query) {
            const std::size_t lhs = static_cast<std::size_t>(random() % n);
            const std::size_t rhs = static_cast<std::size_t>(random() % n);
            const std::size_t expected = naive_lca(lhs, rhs, parent, depth);
            AA_REQUIRE_EQ(lca.lca(lhs, rhs), expected);
            AA_REQUIRE_EQ(lca.distance(lhs, rhs),
                          depth[lhs] + depth[rhs] - 2U * depth[expected]);
            const std::size_t steps = static_cast<std::size_t>(random() % (depth[lhs] + 1U));
            std::size_t ancestor = lhs;
            for (std::size_t step = 0; step < steps; ++step) {
                ancestor = parent[ancestor];
            }
            AA_REQUIRE_EQ(lca.kth_ancestor(lhs, steps), ancestor);
        }
    }
}

AA_TEST(heavy_light_path_segments_and_subtree_ranges) {
    std::mt19937_64 random(0x41DULL);
    for (std::size_t test = 0; test < 50; ++test) {
        const std::size_t n = 1U + static_cast<std::size_t>(random() % 600U);
        const auto tree = random_tree(n, random);
        const HeavyLightDecomposition hld(tree);
        std::vector<std::size_t> parent;
        std::vector<std::size_t> depth;
        rooted_parent_depth(tree, parent, depth);

        std::vector<bool> position_seen(n, false);
        for (std::size_t vertex = 0; vertex < n; ++vertex) {
            const std::size_t position = hld.position(vertex);
            AA_REQUIRE(position < n);
            AA_REQUIRE(!position_seen[position]);
            position_seen[position] = true;
            AA_REQUIRE_EQ(hld.vertex_at_position(position), vertex);
        }

        for (std::size_t query = 0; query < 3'000; ++query) {
            const std::size_t lhs = static_cast<std::size_t>(random() % n);
            const std::size_t rhs = static_cast<std::size_t>(random() % n);
            auto expected = naive_path(lhs, rhs, parent, depth);
            std::vector<std::size_t> actual;
            hld.for_each_path_segment(lhs, rhs, [&](std::size_t left, std::size_t right) {
                AA_REQUIRE(left < right);
                AA_REQUIRE(right <= n);
                for (std::size_t position = left; position < right; ++position) {
                    actual.push_back(hld.vertex_at_position(position));
                }
            });
            std::sort(expected.begin(), expected.end());
            std::sort(actual.begin(), actual.end());
            AA_REQUIRE(actual == expected);
        }

        for (std::size_t root = 0; root < n; ++root) {
            const auto [left, right] = hld.subtree_range(root);
            std::vector<std::size_t> expected;
            for (std::size_t vertex = 0; vertex < n; ++vertex) {
                std::size_t current = vertex;
                while (depth[current] > depth[root]) {
                    current = parent[current];
                }
                if (current == root) {
                    expected.push_back(vertex);
                }
            }
            std::vector<std::size_t> actual;
            for (std::size_t position = left; position < right; ++position) {
                actual.push_back(hld.vertex_at_position(position));
            }
            std::sort(expected.begin(), expected.end());
            std::sort(actual.begin(), actual.end());
            AA_REQUIRE(actual == expected);
        }
    }
}
