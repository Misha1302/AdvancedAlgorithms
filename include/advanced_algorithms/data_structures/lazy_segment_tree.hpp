#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace advanced_algorithms {

class LazySegmentTree {
  public:
    using Value = std::int64_t;

    LazySegmentTree() = default;

    explicit LazySegmentTree(const std::vector<Value>& values)
        : size_(values.size()), tree_(std::max<std::size_t>(1, size_ * 4U + 5U)) {
        if (!values.empty()) {
            build(1, 0, size_, values);
        }
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    void range_add(std::size_t left, std::size_t right, Value delta) {
        check_range(left, right);
        range_add(1, 0, size_, left, right, delta);
    }

    [[nodiscard]] Value range_sum(std::size_t left, std::size_t right) {
        check_range(left, right);
        return range_sum(1, 0, size_, left, right);
    }

    [[nodiscard]] Value range_min(std::size_t left, std::size_t right) {
        check_range(left, right);
        return range_min(1, 0, size_, left, right);
    }

  private:
    struct Node {
        Value sum{};
        Value minimum{};
        Value lazy{};
    };

    void check_range(std::size_t left, std::size_t right) const {
        if (left >= right || right > size_) {
            throw std::out_of_range("LazySegmentTree range must satisfy 0 <= left < right <= size");
        }
    }

    void build(std::size_t node, std::size_t left, std::size_t right,
               const std::vector<Value>& values) {
        if (right - left == 1) {
            tree_[node].sum = values[left];
            tree_[node].minimum = values[left];
            return;
        }
        const std::size_t middle = left + (right - left) / 2U;
        build(node * 2U, left, middle, values);
        build(node * 2U + 1U, middle, right, values);
        pull(node);
    }

    void apply(std::size_t node, std::size_t segment_size, Value delta) {
        tree_[node].sum += delta * static_cast<Value>(segment_size);
        tree_[node].minimum += delta;
        tree_[node].lazy += delta;
    }

    void push(std::size_t node, std::size_t left, std::size_t right) {
        if (tree_[node].lazy == 0 || right - left == 1) {
            return;
        }
        const std::size_t middle = left + (right - left) / 2U;
        apply(node * 2U, middle - left, tree_[node].lazy);
        apply(node * 2U + 1U, right - middle, tree_[node].lazy);
        tree_[node].lazy = 0;
    }

    void pull(std::size_t node) {
        tree_[node].sum = tree_[node * 2U].sum + tree_[node * 2U + 1U].sum;
        tree_[node].minimum = std::min(tree_[node * 2U].minimum,
                                      tree_[node * 2U + 1U].minimum);
    }

    void range_add(std::size_t node, std::size_t left, std::size_t right,
                   std::size_t query_left, std::size_t query_right, Value delta) {
        if (query_left <= left && right <= query_right) {
            apply(node, right - left, delta);
            return;
        }
        push(node, left, right);
        const std::size_t middle = left + (right - left) / 2U;
        if (query_left < middle) {
            range_add(node * 2U, left, middle, query_left, query_right, delta);
        }
        if (middle < query_right) {
            range_add(node * 2U + 1U, middle, right, query_left, query_right, delta);
        }
        pull(node);
    }

    Value range_sum(std::size_t node, std::size_t left, std::size_t right,
                    std::size_t query_left, std::size_t query_right) {
        if (query_left <= left && right <= query_right) {
            return tree_[node].sum;
        }
        push(node, left, right);
        const std::size_t middle = left + (right - left) / 2U;
        Value answer = 0;
        if (query_left < middle) {
            answer += range_sum(node * 2U, left, middle, query_left, query_right);
        }
        if (middle < query_right) {
            answer += range_sum(node * 2U + 1U, middle, right, query_left, query_right);
        }
        return answer;
    }

    Value range_min(std::size_t node, std::size_t left, std::size_t right,
                    std::size_t query_left, std::size_t query_right) {
        if (query_left <= left && right <= query_right) {
            return tree_[node].minimum;
        }
        push(node, left, right);
        const std::size_t middle = left + (right - left) / 2U;
        Value answer = std::numeric_limits<Value>::max();
        if (query_left < middle) {
            answer = std::min(answer,
                              range_min(node * 2U, left, middle, query_left, query_right));
        }
        if (middle < query_right) {
            answer = std::min(answer,
                              range_min(node * 2U + 1U, middle, right, query_left, query_right));
        }
        return answer;
    }

    std::size_t size_{};
    std::vector<Node> tree_;
};

} // namespace advanced_algorithms
