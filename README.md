# AdvancedAlgorithms

[![CI](https://github.com/Misha1302/AdvancedAlgorithms/actions/workflows/ci.yml/badge.svg)](https://github.com/Misha1302/AdvancedAlgorithms/actions/workflows/ci.yml)
[![Performance tests](https://github.com/Misha1302/AdvancedAlgorithms/actions/workflows/performance.yml/badge.svg)](https://github.com/Misha1302/AdvancedAlgorithms/actions/workflows/performance.yml)

Header-only C++23 implementations of graph, tree, string, data-structure and optimization algorithms.
The repository is built around explicit contracts and reproducible tests rather than copy-paste contest snippets.

## Included algorithms

| Area | Algorithm | Main complexity | Verification |
|---|---|---:|---|
| Trees | Centroid decomposition | `O(n log n)` build | balance invariant, random trees, 200k-chain stress |
| Trees | Heavy-light decomposition | `O(n)` core build after validation, `O(log n)` path segments | ordered/unordered path segments and subtree ranges vs naive oracle |
| Trees | LCA by binary lifting | `O(n log n)` build, `O(log n)` query | random queries vs parent-walk oracle |
| Graphs | Dinic maximum flow | `O(V²E)` general bound | random differential tests vs Edmonds–Karp |
| Graphs | Iterative Tarjan SCC | `O(V + E)` | random differential tests vs Kosaraju, 500k path |
| Graphs | Bridges and articulation points | `O(V + E)` | edge/vertex-removal oracle, parallel edges, self-loops |
| Graphs | Dijkstra | `O((V + E) log V)` | random differential tests vs Bellman–Ford |
| Data structures | DSU with path compression and union by size | inverse Ackermann amortized | randomized component oracle |
| Data structures | Lazy segment tree: range add, sum and minimum | `O(log n)` per operation | 60k mixed random operations vs vector |
| Data structures | Sparse table for idempotent operations | `O(n log n)` build, `O(1)` query | random minimum and GCD queries |
| Strings | Aho–Corasick | `O(total patterns + text + matches)` | overlaps, suffixes, duplicates, random naive oracle |
| Optimization | Generic simulated annealing minimizer | user-defined state cost per iteration | fixed-seed reproducibility, convergence and time-limit tests |

## Why the centroid implementation was rewritten

The initial problem-specific version solved nearest-colour queries with fixed arrays, recursive traversals and a
per-vertex collection of hash maps. That shape is useful for one task but unsafe as a reusable component: a long chain
can exhaust the process stack, and the colour maps can consume `O(n log n)` entries.

The repository version only builds the centroid tree. It:

- accepts a normal adjacency list;
- validates that the input is an undirected tree;
- uses iterative component traversals;
- supports empty, one-vertex, chain and star trees;
- exposes the selected component size and largest remaining part for invariant checks;
- uses `O(n)` working memory in addition to the input tree.

A nearest-marked-vertex application is provided in
[`examples/centroid_nearest_marked.cpp`](examples/centroid_nearest_marked.cpp).

## Build and test

Requirements:

- CMake 3.20 or newer;
- GCC 13+, Clang 17+ or another compiler with C++23 support.

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DADVANCED_ALGORITHMS_ENABLE_SANITIZERS=ON
cmake --build build -j
ctest --test-dir build -L correctness --output-on-failure
```

Run the large-input checks separately in an optimized build:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j
ctest --test-dir build-release -L performance --output-on-failure
```

The performance suite uses generous non-flaky budgets and a CTest timeout. It is intended to catch accidental
quadratic behaviour, recursion failures and ignored time limits, not to publish portable benchmark claims.

## Example

```cpp
#include <cstddef>
#include <vector>

#include <advanced_algorithms/trees/centroid_decomposition.hpp>

int main() {
    std::vector<std::vector<std::size_t>> tree{
        {1, 2},
        {0, 3, 4},
        {0},
        {1},
        {1},
    };

    advanced_algorithms::CentroidDecomposition decomposition(tree);
    const std::size_t root = decomposition.root();
    const auto& parent = decomposition.parents();
    (void)root;
    (void)parent;
}
```

## Test strategy

The correctness suite combines:

1. fixed edge cases;
2. randomized differential tests against slower reference algorithms;
3. structural invariants that do not depend on one expected output;
4. invalid-input and API-contract checks;
5. ASan/UBSan runs;
6. large-input tests aimed at stack overflow, accidental `O(n²)` behaviour and time-limit regressions.

The exact oracle for each module is listed in [`docs/testing.md`](docs/testing.md).

## Repository layout

```text
include/advanced_algorithms/   header-only library
examples/                      small complete programs
tests/                         correctness and performance suites
docs/                          contracts and verification notes
scripts/                       local check commands
```

Algorithms implemented inside larger compiler projects are catalogued in
[`PROJECT_ALGORITHMS.md`](PROJECT_ALGORITHMS.md) instead of being duplicated here.

## Scope

This repository is an educational and interview-oriented library. It does not claim constant-time hardening,
formal verification or production suitability for untrusted input without an application-specific review.

## License

MIT.
