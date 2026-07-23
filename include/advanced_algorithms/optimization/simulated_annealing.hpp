#pragma once

#include <chrono>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace advanced_algorithms {

struct SimulatedAnnealingConfig {
    std::size_t iterations{100'000};
    double start_temperature{1.0};
    double end_temperature{1e-3};
    std::uint64_t seed{0xC0FFEEULL};
    std::optional<std::chrono::steady_clock::duration> time_limit;
};

template <class State, class Score>
struct SimulatedAnnealingResult {
    State best_state;
    Score best_score;
    std::size_t iterations_completed{};
    std::size_t accepted_moves{};
};

template <class State, class ScoreFunction, class NeighborFunction>
auto simulated_annealing(State initial_state, ScoreFunction score_function,
                         NeighborFunction neighbor_function,
                         const SimulatedAnnealingConfig& config = {}) {
    using Score = std::remove_cvref_t<std::invoke_result_t<ScoreFunction, const State&>>;
    static_assert(std::is_arithmetic_v<Score>, "Simulated annealing score must be arithmetic");

    if (!(config.start_temperature > 0.0) || !(config.end_temperature > 0.0) ||
        !std::isfinite(config.start_temperature) || !std::isfinite(config.end_temperature)) {
        throw std::invalid_argument("Simulated annealing temperatures must be finite and positive");
    }
    if (config.end_temperature > config.start_temperature) {
        throw std::invalid_argument("End temperature must not exceed start temperature");
    }

    std::mt19937_64 random(config.seed);
    std::uniform_real_distribution<double> probability(0.0, 1.0);

    State current = std::move(initial_state);
    Score current_score = score_function(current);
    State best = current;
    Score best_score = current_score;

    const auto started = std::chrono::steady_clock::now();
    std::size_t completed = 0;
    std::size_t accepted = 0;

    for (std::size_t iteration = 0; iteration < config.iterations; ++iteration) {
        if (config.time_limit.has_value() &&
            std::chrono::steady_clock::now() - started >= *config.time_limit) {
            break;
        }

        const double progress = config.iterations <= 1
                                    ? 0.0
                                    : static_cast<double>(iteration) /
                                          static_cast<double>(config.iterations - 1U);
        const double temperature = config.start_temperature *
                                   std::pow(config.end_temperature / config.start_temperature,
                                            progress);

        State candidate = current;
        neighbor_function(candidate, random);
        const Score candidate_score = score_function(candidate);
        const long double delta = static_cast<long double>(candidate_score) -
                                  static_cast<long double>(current_score);

        bool accept = delta <= 0.0L;
        if (!accept) {
            const long double exponent = -delta / static_cast<long double>(temperature);
            const long double chance = exponent < -700.0L ? 0.0L : std::exp(exponent);
            accept = probability(random) < static_cast<double>(chance);
        }

        if (accept) {
            current = std::move(candidate);
            current_score = candidate_score;
            ++accepted;
            if (current_score < best_score) {
                best = current;
                best_score = current_score;
            }
        }
        ++completed;
    }

    return SimulatedAnnealingResult<State, Score>{std::move(best), best_score, completed, accepted};
}

} // namespace advanced_algorithms
