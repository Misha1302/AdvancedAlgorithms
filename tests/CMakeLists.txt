add_executable(
    advanced_algorithms_tests
    test_main.cpp
    test_data_structures.cpp
    test_graphs.cpp
    test_optimization.cpp
    test_regressions.cpp
    test_strings.cpp
    test_trees.cpp)
target_link_libraries(advanced_algorithms_tests PRIVATE AdvancedAlgorithms::advanced_algorithms)
add_test(NAME advanced_algorithms.correctness COMMAND advanced_algorithms_tests)
set_tests_properties(advanced_algorithms.correctness PROPERTIES LABELS correctness TIMEOUT 60)

add_executable(advanced_algorithms_performance_tests performance_tests.cpp)
target_link_libraries(advanced_algorithms_performance_tests PRIVATE AdvancedAlgorithms::advanced_algorithms)
add_test(NAME advanced_algorithms.performance COMMAND advanced_algorithms_performance_tests)
set_tests_properties(advanced_algorithms.performance PROPERTIES LABELS performance TIMEOUT 45)

add_executable(advanced_algorithms_performance_regressions performance_regressions.cpp)
target_link_libraries(advanced_algorithms_performance_regressions PRIVATE AdvancedAlgorithms::advanced_algorithms)
add_test(NAME advanced_algorithms.performance_regressions COMMAND advanced_algorithms_performance_regressions)
set_tests_properties(advanced_algorithms.performance_regressions PROPERTIES LABELS performance TIMEOUT 10)
