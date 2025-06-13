add_executable(test_format   test_format.cpp)
target_link_libraries(test_format PRIVATE metrics)

add_executable(bench_latency bench_latency.cpp)
target_link_libraries(bench_latency PRIVATE metrics)

add_custom_target(run-tests
        COMMAND $<TARGET_FILE:test_format>
        COMMAND $<TARGET_FILE:bench_latency>
        DEPENDS test_format bench_latency
)
