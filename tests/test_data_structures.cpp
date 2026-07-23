#include "test_framework.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

#include "advanced_algorithms/data_structures/disjoint_set_union.hpp"
#include "advanced_algorithms/data_structures/lazy_segment_tree.hpp"
#include "advanced_algorithms/data_structures/sparse_table.hpp"

using advanced_algorithms::DisjointSetUnion;
using advanced_algorithms::LazySegmentTree;
using advanced_algorithms::SparseTable;

AA_TEST(dsu_basic_contract) {
    DisjointSetUnion dsu(6);
    AA_REQUIRE_EQ(dsu.components(), std::size_t{6});
    AA_REQUIRE(dsu.unite(0, 1));
    AA_REQUIRE(dsu.unite(1, 2));
    AA_REQUIRE(!dsu.unite(0, 2));
    AA_REQUIRE(dsu.same(0, 2));
    AA_REQUIRE(!dsu.same(0, 3));
    AA_REQUIRE_EQ(dsu.component_size(1), std::size_t{3});
    AA_REQUIRE_EQ(dsu.components(), std::size_t{4});
    AA_REQUIRE_THROWS_AS(dsu.find(6), std::out_of_range);
}

AA_TEST(dsu_randomized_against_naive_components) {
    constexpr std::size_t n = 64;
    DisjointSetUnion dsu(n);
    std::vector<std::size_t> label(n);
    std::iota(label.begin(), label.end(), std::size_t{0});
    std::mt19937_64 random(0xD5U);

    for (std::size_t step = 0; step < 20'000; ++step) {
        const std::size_t lhs = static_cast<std::size_t>(random() % n);
        const std::size_t rhs = static_cast<std::size_t>(random() % n);
        if ((random() & 1U) == 0U) {
            const bool expected = label[lhs] != label[rhs];
            const std::size_t old_label = label[rhs];
            const std::size_t new_label = label[lhs];
            const bool merged = dsu.unite(lhs, rhs);
            AA_REQUIRE_EQ(merged, expected);
            if (expected) {
                for (auto& value : label) {
                    if (value == old_label) {
                        value = new_label;
                    }
                }
            }
        } else {
            AA_REQUIRE_EQ(dsu.same(lhs, rhs), label[lhs] == label[rhs]);
        }
    }
}

AA_TEST(lazy_segment_tree_fixed_edge_cases) {
    LazySegmentTree tree({5, -2, 7, 0});
    AA_REQUIRE_EQ(tree.range_sum(0, 4), std::int64_t{10});
    AA_REQUIRE_EQ(tree.range_min(0, 4), std::int64_t{-2});
    tree.range_add(0, 4, 3);
    AA_REQUIRE_EQ(tree.range_sum(0, 4), std::int64_t{22});
    AA_REQUIRE_EQ(tree.range_min(0, 4), std::int64_t{1});
    tree.range_add(1, 3, -10);
    AA_REQUIRE_EQ(tree.range_sum(1, 3), std::int64_t{-9});
    AA_REQUIRE_EQ(tree.range_min(0, 4), std::int64_t{-9});
    AA_REQUIRE_THROWS_AS(tree.range_sum(0, 0), std::out_of_range);
    AA_REQUIRE_THROWS_AS(tree.range_add(0, 5, 1), std::out_of_range);
}

AA_TEST(lazy_segment_tree_randomized_against_vector) {
    constexpr std::size_t n = 257;
    std::mt19937_64 random(0x5E67U);
    std::vector<std::int64_t> values(n);
    for (auto& value : values) {
        value = static_cast<std::int64_t>(random() % 2001U) - 1000;
    }
    LazySegmentTree tree(values);

    for (std::size_t step = 0; step < 60'000; ++step) {
        std::size_t left = static_cast<std::size_t>(random() % n);
        std::size_t right = static_cast<std::size_t>(random() % n);
        if (left > right) {
            std::swap(left, right);
        }
        ++right;
        if ((random() % 3U) == 0U) {
            const std::int64_t delta = static_cast<std::int64_t>(random() % 101U) - 50;
            tree.range_add(left, right, delta);
            for (std::size_t index = left; index < right; ++index) {
                values[index] += delta;
            }
        } else {
            const auto expected_sum = std::accumulate(values.begin() + static_cast<std::ptrdiff_t>(left),
                                                      values.begin() + static_cast<std::ptrdiff_t>(right),
                                                      std::int64_t{0});
            const auto expected_min = *std::min_element(
                values.begin() + static_cast<std::ptrdiff_t>(left),
                values.begin() + static_cast<std::ptrdiff_t>(right));
            AA_REQUIRE_EQ(tree.range_sum(left, right), expected_sum);
            AA_REQUIRE_EQ(tree.range_min(left, right), expected_min);
        }
    }
}

AA_TEST(sparse_table_minimum_and_gcd) {
    const std::vector<std::int64_t> values{18, 24, 6, 15, 9, 27, 3, 12};
    SparseTable<std::int64_t> minimum(values);
    AA_REQUIRE_EQ(minimum.query(0, 8), std::int64_t{3});
    AA_REQUIRE_EQ(minimum.query(1, 4), std::int64_t{6});
    AA_REQUIRE_EQ(minimum.query(3, 4), std::int64_t{15});

    struct GcdOperation {
        std::int64_t operator()(std::int64_t lhs, std::int64_t rhs) const {
            return std::gcd(lhs, rhs);
        }
    };
    SparseTable<std::int64_t, GcdOperation> gcd_table(values, GcdOperation{});
    AA_REQUIRE_EQ(gcd_table.query(0, 2), std::int64_t{6});
    AA_REQUIRE_EQ(gcd_table.query(1, 5), std::int64_t{3});
    AA_REQUIRE_THROWS_AS(minimum.query(2, 2), std::out_of_range);
}

AA_TEST(sparse_table_randomized_against_naive_minimum) {
    constexpr std::size_t n = 1'000;
    std::mt19937_64 random(0x5A1EULL);
    std::vector<std::int64_t> values(n);
    for (auto& value : values) {
        value = static_cast<std::int64_t>(random());
    }
    SparseTable<std::int64_t> table(values);
    for (std::size_t query = 0; query < 20'000; ++query) {
        std::size_t left = static_cast<std::size_t>(random() % n);
        std::size_t right = static_cast<std::size_t>(random() % n);
        if (left > right) {
            std::swap(left, right);
        }
        ++right;
        const auto expected = *std::min_element(values.begin() + static_cast<std::ptrdiff_t>(left),
                                                values.begin() + static_cast<std::ptrdiff_t>(right));
        AA_REQUIRE_EQ(table.query(left, right), expected);
    }
}
