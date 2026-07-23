#include "test_framework.hpp"

int main() {
    std::size_t passed = 0;
    for (const auto& test : test_framework::registry()) {
        try {
            test.function();
            ++passed;
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
            return 1;
        } catch (...) {
            std::cerr << "[FAIL] " << test.name << ": unknown exception\n";
            return 1;
        }
    }
    std::cout << "Passed " << passed << " tests\n";
    return 0;
}
