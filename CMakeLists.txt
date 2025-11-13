cmake_minimum_required(VERSION 3.10)

project(ExchangeRates VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(rates
    src/main.cpp
    src/cpp/exchange_rates.cpp
)

add_executable(tests_simple
    src/tests/tests_simple.cpp
    src/cpp/exchange_rates.cpp
)

target_include_directories(rates PRIVATE hpp)
target_include_directories(tests_simple PRIVATE hpp)

enable_testing()
add_test(NAME exchange_rates_tests COMMAND tests_simple)