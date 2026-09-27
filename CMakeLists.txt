cmake_minimum_required(VERSION 3.14)

project(udp_network_tester LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(udp_statistics
    src/Statistics.cpp
)

target_include_directories(udp_statistics PUBLIC include)

add_executable(udp_server
    src/server.cpp
)

target_include_directories(udp_server PRIVATE include)

add_executable(udp_client
    src/client.cpp
)

target_link_libraries(udp_client PRIVATE udp_statistics)
target_include_directories(udp_client PRIVATE include)

option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(udp_tests
        tests/test_statistics.cpp
    )

    target_link_libraries(udp_tests PRIVATE udp_statistics GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(udp_tests)
endif()
