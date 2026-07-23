#include "test_framework.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "advanced_algorithms/graphs/dinic.hpp"
#include "advanced_algorithms/trees/centroid_decomposition.hpp"
#include "advanced_algorithms/trees/heavy_light_decomposition.hpp"
#include "advanced_algorithms/trees/lca_binary_lifting.hpp"

using advanced_algorithms::CentroidDecomposition;
using advanced_algorithms::Dinic;
using advanced_algorithms::HeavyLightDecomposition;
using advanced_algorithms::LcaBinaryLifting;

namespace {

std::vector<std::vector<std::size_t>> make_temporary_tree() {
    std::vector<std::vector<std::size_t>> tree(5);
    const auto add_edge = [&](std::size_t lhs, std::size_t rhs) {
        tree[lhs].push_back(rhs);
        tree[rhs].push_back(lhs);
    };
    add_edge(0, 1);
    add_edge(1, 2);
    add_edge(1, 3);
    add_edge(3, 4);
    return tree;
}

} // namespace

AA_TEST(dinic_self_loop_keeps_valid_reverse_indices) {
    Dinic flow(3);
    const std::size_t self_loop = flow.add_edge(1, 1, 9);
    flow.add_edge(0, 1, 4);
    flow.add_edge(1, 2, 4);

    AA_REQUIRE_EQ(flow.edge(self_loop).capacity, std::int64_t{9});
    AA_REQUIRE_EQ(flow.edge(self_loop).flow, std::int64_t{0});
    AA_REQUIRE_EQ(flow.max_flow(0, 2), std::int64_t{4});
    AA_REQUIRE_EQ(flow.edge(self_loop).flow, std::int64_t{0});
}

AA_TEST(tree_structures_do_not_borrow_constructor_input) {
    const CentroidDecomposition centroid(make_temporary_tree());
    const LcaBinaryLifting lca(make_temporary_tree());
    const HeavyLightDecomposition hld(make_temporary_tree());

    AA_REQUIRE(centroid.root() < centroid.size());
    AA_REQUIRE_EQ(lca.lca(2, 4), std::size_t{1});
    AA_REQUIRE_EQ(lca.distance(2, 4), std::size_t{3});

    std::vector<std::size_t> path;
    hld.for_each_path_segment(2, 4, [&](std::size_t left, std::size_t right) {
        for (std::size_t position = left; position < right; ++position) {
            path.push_back(hld.vertex_at_position(position));
        }
    });
    std::sort(path.begin(), path.end());
    AA_REQUIRE(path == std::vector<std::size_t>({1, 2, 3, 4}));

    std::vector<std::size_t> ordered_path;
    hld.for_each_ordered_path_segment(
        2, 4, [&](std::size_t left, std::size_t right, bool reversed) {
            if (reversed) {
                for (std::size_t position = right; position-- > left;) {
                    ordered_path.push_back(hld.vertex_at_position(position));
                }
            } else {
                for (std::size_t position = left; position < right; ++position) {
                    ordered_path.push_back(hld.vertex_at_position(position));
                }
            }
        });
    AA_REQUIRE(ordered_path == std::vector<std::size_t>({2, 1, 3, 4}));
}
