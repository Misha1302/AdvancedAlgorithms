# Centroid decomposition

## Contract

Input:

- an undirected adjacency list;
- vertices numbered from `0` to `n - 1`;
- exactly `n - 1` undirected edges;
- no self-loops, duplicate adjacency entries or asymmetric edges.

For an empty tree the decomposition root is `no_vertex()`.

Output:

- one centroid-tree parent per original vertex;
- one decomposition level per vertex;
- the active component size when the vertex was selected;
- the size of the largest part left by removing that centroid.

## Invariant

For every selected centroid `c`:

```text
2 * largest_part_at_selection(c) <= component_size_at_selection(c)
```

The tests verify this property for single vertices, chains, stars and random trees.

## Implementation

Each active component is collected iteratively. Subtree sizes are calculated in reverse traversal order, then the
vertex minimizing the largest resulting part is selected. The decomposition itself uses an explicit task stack.

No DFS recurses along the original tree, so a chain with hundreds of thousands of vertices does not depend on the
process call-stack limit.

## Complexity

- build time: `O(n log n)`;
- stored result: `O(n)`;
- temporary working memory: `O(n)`;
- centroid-tree height: `O(log n)`.

## Typical application

To support activation and nearest-active queries:

1. build an LCA structure for original-tree distances;
2. keep the best known distance for each centroid;
3. on update or query, walk through centroid-tree ancestors.

See [`../examples/centroid_nearest_marked.cpp`](../examples/centroid_nearest_marked.cpp).
