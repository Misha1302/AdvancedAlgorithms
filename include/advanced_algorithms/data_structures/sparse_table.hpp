#pragma once

#include <bit>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace advanced_algorithms {

template <class Value>
struct MinimumOperation {
    [[nodiscard]] Value operator()(const Value& lhs, const Value& rhs) const {
        return lhs < rhs ? lhs : rhs;
    }
};

template <class Value, class Operation = MinimumOperation<Value>>
class SparseTable {
  public:
    SparseTable() = default;

    explicit SparseTable(std::vector<Value> values)
        : SparseTable(std::move(values), Operation{}) {}

    SparseTable(std::vector<Value> values, Operation operation)
        : operation_(std::move(operation)), size_(values.size()) {
        if (values.empty()) {
            return;
        }
        const std::size_t levels = static_cast<std::size_t>(std::bit_width(values.size()));
        table_.resize(levels);
        table_[0] = std::move(values);
        for (std::size_t level = 1; level < levels; ++level) {
            const std::size_t length = std::size_t{1} << level;
            const std::size_t half = length >> 1U;
            const std::size_t count = size_ >= length ? size_ - length + 1 : 0;
            table_[level].reserve(count);
            for (std::size_t index = 0; index < count; ++index) {
                table_[level].push_back(operation_(table_[level - 1][index],
                                                   table_[level - 1][index + half]));
            }
        }
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] Value query(std::size_t left, std::size_t right) const {
        if (left >= right || right > size_) {
            throw std::out_of_range("SparseTable query must satisfy 0 <= left < right <= size");
        }
        const std::size_t length = right - left;
        const std::size_t level = static_cast<std::size_t>(std::bit_width(length)) - 1U;
        const std::size_t block = std::size_t{1} << level;
        return operation_(table_[level][left], table_[level][right - block]);
    }

  private:
    Operation operation_{};
    std::size_t size_{};
    std::vector<std::vector<Value>> table_;
};

} // namespace advanced_algorithms
