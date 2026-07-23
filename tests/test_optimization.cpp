#include "test_framework.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <random>
#include <stdexcept>

#include "advanced_algorithms/optimization/simulated_annealing.hpp"

using advanced_algorithms::SimulatedAnnealingConfig;
using advanced_algorithms::simulated_annealing;

AA_TEST(simulated_annealing_is_reproducible_for_fixed_seed) {
    const auto score = [](const int value) {
        const std::int64_t delta = static_cast<std::int64_t>(value) - 137;
        return delta * delta;
    };
    const auto neighbor = [](int& value, std::mt19937_64& random) {
        const int step = 1 + static_cast<int>(random() % 25U);
        value += (random() & 1U) == 0U ? step : -step;
    };

    SimulatedAnnealingConfig config;
    config.iterations = 50'000;
    config.start_temperature = 10'000.0;
    config.end_temperature = 0.001;
    config.seed = 42;

    const auto first = simulated_annealing(5'000, score, neighbor, config);
    const auto second = simulated_annealing(5'000, score, neighbor, config);
    AA_REQUIRE_EQ(first.best_state, second.best_state);
    AA_REQUIRE_EQ(first.best_score, second.best_score);
    AA_REQUIRE_EQ(first.accepted_moves, second.accepted_moves);
    AA_REQUIRE_EQ(first.best_state, 137);
    AA_REQUIRE_EQ(first.best_score, std::int64_t{0});
}

AA_TEST(simulated_annealing_handles_zero_iterations_and_invalid_config) {
    const auto score = [](const int value) { return value * value; };
    const auto neighbor = [](int& value, std::mt19937_64&) { ++value; };

    SimulatedAnnealingConfig config;
    config.iterations = 0;
    const auto result = simulated_annealing(7, score, neighbor, config);
    AA_REQUIRE_EQ(result.best_state, 7);
    AA_REQUIRE_EQ(result.best_score, 49);
    AA_REQUIRE_EQ(result.iterations_completed, std::size_t{0});

    config.start_temperature = 0.0;
    AA_REQUIRE_THROWS_AS(simulated_annealing(7, score, neighbor, config), std::invalid_argument);
    config.start_temperature = 1.0;
    config.end_temperature = 2.0;
    AA_REQUIRE_THROWS_AS(simulated_annealing(7, score, neighbor, config), std::invalid_argument);
}

AA_TEST(simulated_annealing_respects_time_limit) {
    const auto score = [](const std::int64_t value) { return std::abs(value); };
    const auto neighbor = [](std::int64_t& value, std::mt19937_64& random) {
        value += (random() & 1U) == 0U ? 1 : -1;
    };

    SimulatedAnnealingConfig config;
    config.iterations = 1'000'000'000U;
    config.start_temperature = 10.0;
    config.end_temperature = 0.01;
    config.time_limit = std::chrono::milliseconds(10);

    const auto started = std::chrono::steady_clock::now();
    const auto result = simulated_annealing<std::int64_t>(1'000, score, neighbor, config);
    const auto elapsed = std::chrono::steady_clock::now() - started;
    AA_REQUIRE(result.iterations_completed < config.iterations);
    AA_REQUIRE(elapsed < std::chrono::milliseconds(500));
}
