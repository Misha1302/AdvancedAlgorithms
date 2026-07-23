# Testing strategy

The tests are dependency-free and deterministic. Randomized suites use fixed seeds so every failure is reproducible.

## Oracle matrix

| Module | Reference or invariant |
|---|---|
| Centroid decomposition | every selected centroid leaves no part larger than half its active component; parent levels form one tree |
| LCA | repeated parent climbing |
| HLD | exact vertex set on a path; exact descendant set in each subtree interval |
| Dijkstra | Bellman–Ford on small non-negative graphs |
| Dinic | Edmonds–Karp on small random capacitated multigraphs |
| Tarjan SCC | pairwise component equivalence with Kosaraju |
| Bridges/articulation | connected-component count after removing each edge or vertex |
| DSU | explicit component labels |
| Lazy segment tree | plain vector updated and queried directly |
| Sparse table | direct range fold |
| Aho–Corasick | naive substring matching for every pattern |
| Simulated annealing | deterministic replay and problems with a known global optimum |

## Edge cases

The fixed suites include:

- empty and one-element structures;
- disconnected directed graphs;
- unreachable vertices;
- self-loops and parallel edges;
- zero capacities and duplicate patterns;
- full-range, single-element and nested range updates;
- chains, stars and random trees;
- invalid indices, negative Dijkstra weights and malformed trees;
- zero annealing iterations, invalid temperatures and a strict wall-clock limit.

## Large-input and TL checks

`advanced_algorithms_performance_tests` and the dedicated regression executable currently exercise:

- centroid decomposition on a chain of 200,000 vertices;
- LCA and HLD on the same chain;
- iterative Tarjan SCC on a directed path of 500,000 vertices;
- iterative Dinic on a 200,000-vertex path and on 100,000 parallel unit-capacity paths;
- 300,000 mixed lazy-segment-tree operations;
- Dijkstra on a 200,000-vertex sparse graph;
- Aho–Corasick with 20,000 patterns over one million characters;
- simulated annealing with one billion allowed iterations and a 25 ms time limit.

The suite checks broad budgets only. Exact timings depend on compiler, hardware, allocator and sanitizers, so they are
not treated as cross-machine benchmark results.

## Commands

```bash
./scripts/run_all_checks.sh
```

For one suite:

```bash
ctest --test-dir build -L correctness --output-on-failure
ctest --test-dir build-release -L performance --output-on-failure
```
