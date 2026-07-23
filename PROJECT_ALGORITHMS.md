# Algorithms implemented in larger projects

These implementations remain in their original systems because their contracts depend on the surrounding IR,
runtime or problem model.

## UniversalToolchain / Wist2

Repository: <https://github.com/Misha1302/Wist2>

- sparse conditional constant propagation over SSA;
- constant folding and branch folding;
- unreachable-block and dead-code elimination;
- AIR to SSA lowering and SSA to AIR emission;
- structural SSA verification and capability-gated optimization stages.

## x86-64 code generation and register-allocation playground

Repository: <https://github.com/Misha1302/x86-64-codegen-ra-playground>

- backward liveness analysis;
- live-interval construction;
- dominator analysis;
- interference-graph construction;
- linear-scan register allocation;
- simulated-annealing register allocation;
- phi lowering and parallel-move cycle handling.

## PS-form memory-dependence analyzer

Repository: <https://github.com/Misha1302/ps_form_analizer>

- symbolic normalization of address expressions;
- interval, residue and GCD disjointness filters;
- affine integer-lattice reasoning;
- monotonic two-pointer intersection search;
- exact bounded oracle, randomized and metamorphic verification.
