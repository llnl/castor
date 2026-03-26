cmake_minimum_required(VERSION 3.23)

project(peg-and-tests
        DESCRIPTION "a parsing expression grammar generator built on cmake and includes tests"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-O3 -Wall -Wextra -Wpedantic)

add_subdirectory(src/PEG)

enable_testing()
add_subdirectory(test)
