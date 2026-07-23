cmake_minimum_required(VERSION 3.20)

project(AdvancedAlgorithms VERSION 0.1.0 LANGUAGES CXX)

option(ADVANCED_ALGORITHMS_BUILD_TESTS "Build correctness and performance tests" ON)
option(ADVANCED_ALGORITHMS_BUILD_EXAMPLES "Build examples" ON)
option(ADVANCED_ALGORITHMS_ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)

add_library(advanced_algorithms INTERFACE)
add_library(AdvancedAlgorithms::advanced_algorithms ALIAS advanced_algorithms)

target_include_directories(
    advanced_algorithms
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_features(advanced_algorithms INTERFACE cxx_std_23)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        advanced_algorithms
        INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
            -Werror)
endif()

if(ADVANCED_ALGORITHMS_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(advanced_algorithms INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(advanced_algorithms INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

if(ADVANCED_ALGORITHMS_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

if(ADVANCED_ALGORITHMS_BUILD_EXAMPLES)
    add_executable(centroid_nearest_marked examples/centroid_nearest_marked.cpp)
    target_link_libraries(centroid_nearest_marked PRIVATE AdvancedAlgorithms::advanced_algorithms)
endif()
