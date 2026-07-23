#!/usr/bin/env bash
set -euo pipefail

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DADVANCED_ALGORITHMS_ENABLE_SANITIZERS=ON
cmake --build build -j2
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=print_stacktrace=1 \
  ctest --test-dir build -L correctness --output-on-failure

cmake -S . -B build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DADVANCED_ALGORITHMS_ENABLE_SANITIZERS=OFF
cmake --build build-release -j2
ctest --test-dir build-release -L correctness --output-on-failure
ctest --test-dir build-release -L performance --output-on-failure
