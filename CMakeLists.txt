cmake_minimum_required(VERSION 3.16)
project(metrics_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(metrics STATIC
        src/metrics.cpp
)

target_include_directories(metrics PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(example
        examples/main.cpp
)
target_link_libraries(example PRIVATE metrics)

add_subdirectory(tests)
