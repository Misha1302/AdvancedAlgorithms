#include "test_framework.hpp"

#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "advanced_algorithms/strings/aho_corasick.hpp"

using advanced_algorithms::AhoCorasick;

namespace {

std::size_t naive_count(const std::string& text, const std::string& pattern) {
    std::size_t count = 0;
    if (pattern.size() > text.size()) {
        return 0;
    }
    for (std::size_t start = 0; start + pattern.size() <= text.size(); ++start) {
        if (text.compare(start, pattern.size(), pattern) == 0) {
            ++count;
        }
    }
    return count;
}

std::string random_string(std::size_t length, std::mt19937_64& random) {
    std::string result(length, 'a');
    for (char& character : result) {
        character = static_cast<char>('a' + static_cast<char>(random() % 4U));
    }
    return result;
}

} // namespace

AA_TEST(aho_corasick_fixed_overlaps_suffixes_and_duplicates) {
    AhoCorasick automaton;
    const std::size_t he = automaton.add_pattern("he");
    const std::size_t she = automaton.add_pattern("she");
    const std::size_t hers = automaton.add_pattern("hers");
    const std::size_t his = automaton.add_pattern("his");
    const std::size_t duplicate_he = automaton.add_pattern("he");
    automaton.build();

    const auto counts = automaton.match_counts("ahishershe");
    AA_REQUIRE_EQ(counts[he], std::size_t{2});
    AA_REQUIRE_EQ(counts[duplicate_he], std::size_t{2});
    AA_REQUIRE_EQ(counts[she], std::size_t{2});
    AA_REQUIRE_EQ(counts[hers], std::size_t{1});
    AA_REQUIRE_EQ(counts[his], std::size_t{1});

    const auto matches = automaton.find_matches("she");
    bool found_she = false;
    bool found_he = false;
    for (const auto& [pattern, start] : matches) {
        if (pattern == she && start == 0) {
            found_she = true;
        }
        if (pattern == he && start == 1) {
            found_he = true;
        }
    }
    AA_REQUIRE(found_she);
    AA_REQUIRE(found_he);
    AA_REQUIRE_THROWS_AS(automaton.add_pattern("x"), std::logic_error);
}

AA_TEST(aho_corasick_contract_errors) {
    AhoCorasick automaton;
    AA_REQUIRE_THROWS_AS(automaton.add_pattern(""), std::invalid_argument);
    automaton.add_pattern("a");
    AA_REQUIRE_THROWS_AS(automaton.match_counts("a"), std::logic_error);
}

AA_TEST(aho_corasick_randomized_against_naive_matching) {
    std::mt19937_64 random(0xACACULL);
    for (std::size_t test = 0; test < 500; ++test) {
        const std::size_t pattern_count = 1U + static_cast<std::size_t>(random() % 30U);
        std::vector<std::string> patterns;
        AhoCorasick automaton;
        for (std::size_t index = 0; index < pattern_count; ++index) {
            patterns.push_back(random_string(1U + static_cast<std::size_t>(random() % 10U), random));
            automaton.add_pattern(patterns.back());
        }
        automaton.build();
        const std::string text = random_string(static_cast<std::size_t>(random() % 200U), random);
        const auto counts = automaton.match_counts(text);
        for (std::size_t index = 0; index < patterns.size(); ++index) {
            AA_REQUIRE_EQ(counts[index], naive_count(text, patterns[index]));
        }
    }
}
