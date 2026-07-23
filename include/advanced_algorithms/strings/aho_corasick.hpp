#pragma once

#include <cstddef>
#include <queue>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace advanced_algorithms {

class AhoCorasick {
  public:
    std::size_t add_pattern(std::string_view pattern) {
        if (built_) {
            throw std::logic_error("Cannot add patterns after AhoCorasick::build");
        }
        if (pattern.empty()) {
            throw std::invalid_argument("Aho-Corasick does not accept empty patterns");
        }

        std::size_t node = 0;
        for (const char character : pattern) {
            auto [iterator, inserted] = nodes_[node].next.emplace(character, nodes_.size());
            if (inserted) {
                nodes_.push_back(Node{});
            }
            node = iterator->second;
        }
        const std::size_t id = pattern_lengths_.size();
        pattern_lengths_.push_back(pattern.size());
        nodes_[node].terminal_pattern_ids.push_back(id);
        return id;
    }

    void build() {
        if (built_) {
            return;
        }
        std::queue<std::size_t> queue;
        for (const auto& [character, child] : nodes_[0].next) {
            (void)character;
            nodes_[child].failure = 0;
            queue.push(child);
        }

        while (!queue.empty()) {
            const std::size_t node = queue.front();
            queue.pop();

            const std::size_t failure = nodes_[node].failure;

            for (const auto& [character, child] : nodes_[node].next) {
                std::size_t candidate = failure;
                while (candidate != 0 && !nodes_[candidate].next.contains(character)) {
                    candidate = nodes_[candidate].failure;
                }
                const auto iterator = nodes_[candidate].next.find(character);
                if (iterator != nodes_[candidate].next.end() && iterator->second != child) {
                    candidate = iterator->second;
                }
                nodes_[child].failure = candidate;
                nodes_[child].output_link = !nodes_[candidate].terminal_pattern_ids.empty()
                                                ? candidate
                                                : nodes_[candidate].output_link;
                queue.push(child);
            }
        }

        built_ = true;
    }

    [[nodiscard]] std::size_t pattern_count() const noexcept {
        return pattern_lengths_.size();
    }

    [[nodiscard]] std::vector<std::size_t> match_counts(std::string_view text) const {
        ensure_built();
        std::vector<std::size_t> counts(pattern_lengths_.size(), 0);
        std::size_t node = 0;
        for (const char character : text) {
            while (node != 0 && !nodes_[node].next.contains(character)) {
                node = nodes_[node].failure;
            }
            const auto iterator = nodes_[node].next.find(character);
            if (iterator != nodes_[node].next.end()) {
                node = iterator->second;
            }
            emit_pattern_ids(node, [&](std::size_t pattern_id) { ++counts[pattern_id]; });
        }
        return counts;
    }

    [[nodiscard]] std::vector<std::pair<std::size_t, std::size_t>>
    find_matches(std::string_view text) const {
        ensure_built();
        std::vector<std::pair<std::size_t, std::size_t>> result;
        std::size_t node = 0;
        for (std::size_t index = 0; index < text.size(); ++index) {
            const char character = text[index];
            while (node != 0 && !nodes_[node].next.contains(character)) {
                node = nodes_[node].failure;
            }
            const auto iterator = nodes_[node].next.find(character);
            if (iterator != nodes_[node].next.end()) {
                node = iterator->second;
            }
            emit_pattern_ids(node, [&](std::size_t pattern_id) {
                const std::size_t start = index + 1U - pattern_lengths_[pattern_id];
                result.emplace_back(pattern_id, start);
            });
        }
        return result;
    }

  private:
    struct Node {
        std::unordered_map<char, std::size_t> next;
        std::size_t failure{};
        std::size_t output_link{no_node()};
        std::vector<std::size_t> terminal_pattern_ids;
    };

    [[nodiscard]] static constexpr std::size_t no_node() noexcept {
        return static_cast<std::size_t>(-1);
    }

    template <class Callback>
    void emit_pattern_ids(std::size_t node, Callback&& callback) const {
        for (const std::size_t pattern_id : nodes_[node].terminal_pattern_ids) {
            callback(pattern_id);
        }
        for (std::size_t output = nodes_[node].output_link; output != no_node();
             output = nodes_[output].output_link) {
            for (const std::size_t pattern_id : nodes_[output].terminal_pattern_ids) {
                callback(pattern_id);
            }
        }
    }

    void ensure_built() const {
        if (!built_) {
            throw std::logic_error("Call AhoCorasick::build before matching");
        }
    }

    std::vector<Node> nodes_{1};
    std::vector<std::size_t> pattern_lengths_;
    bool built_{};
};

} // namespace advanced_algorithms
