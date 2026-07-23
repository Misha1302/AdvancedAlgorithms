#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace test_framework {

using TestFunction = void (*)();

struct TestCase {
    std::string name;
    TestFunction function{};
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

class Registrar {
  public:
    Registrar(std::string name, TestFunction function) {
        registry().push_back(TestCase{std::move(name), function});
    }
};

[[noreturn]] inline void fail(const char* expression, const char* file, int line,
                              const std::string& details = {}) {
    std::ostringstream message;
    message << file << ':' << line << ": requirement failed: " << expression;
    if (!details.empty()) {
        message << " (" << details << ')';
    }
    throw std::runtime_error(message.str());
}

} // namespace test_framework

#define AA_TEST(name)                                                                              \
    static void name();                                                                            \
    static const ::test_framework::Registrar registrar_##name(#name, &name);                       \
    static void name()

#define AA_REQUIRE(expression)                                                                     \
    do {                                                                                           \
        if (!(expression)) {                                                                       \
            ::test_framework::fail(#expression, __FILE__, __LINE__);                              \
        }                                                                                          \
    } while (false)

#define AA_REQUIRE_EQ(lhs, rhs)                                                                    \
    do {                                                                                           \
        const auto aa_lhs_value = (lhs);                                                           \
        const auto aa_rhs_value = (rhs);                                                           \
        if (!(aa_lhs_value == aa_rhs_value)) {                                                     \
            std::ostringstream aa_details;                                                        \
            aa_details << "lhs=" << aa_lhs_value << ", rhs=" << aa_rhs_value;                    \
            ::test_framework::fail(#lhs " == " #rhs, __FILE__, __LINE__, aa_details.str());      \
        }                                                                                          \
    } while (false)

#define AA_REQUIRE_THROWS_AS(expression, exception_type)                                           \
    do {                                                                                           \
        bool aa_thrown = false;                                                                    \
        try {                                                                                      \
            static_cast<void>(expression);                                                        \
        } catch (const exception_type&) {                                                          \
            aa_thrown = true;                                                                      \
        }                                                                                          \
        if (!aa_thrown) {                                                                          \
            ::test_framework::fail(#expression " throws " #exception_type, __FILE__, __LINE__);  \
        }                                                                                          \
    } while (false)
