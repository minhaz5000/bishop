cmake_minimum_required(VERSION 3.16)
project(bishop VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BISHOP_BUILD_TESTS  "Build unit tests"   ON)
option(BISHOP_BUILD_BENCH  "Build benchmarks"   ON)
option(BISHOP_ASAN         "Enable AddressSanitizer" OFF)
option(BISHOP_UBSAN        "Enable UBSanitizer"      OFF)

if(BISHOP_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

if(BISHOP_UBSAN)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

include_directories(include)

set(BISHOP_SOURCES
    src/tokenizer.cpp
    src/nfa.cpp
    src/utils.cpp
    src/dfa.cpp
    src/debug.cpp
    src/serializer.cpp
    src/unicode.cpp
    src/analyzer.cpp
    src/cache.cpp
    src/lexer.cpp
    src/optimizer.cpp
    src/rewriter.cpp
    src/splitter.cpp
    src/token_stream.cpp
)

add_library(bishop_core STATIC ${BISHOP_SOURCES})
target_include_directories(bishop_core PUBLIC include)

if(BISHOP_BUILD_TESTS)
    enable_testing()
    add_executable(bishop_tests
        tests/test_main.cpp
        tests/test_tokenizer.cpp
        tests/test_nfa.cpp
        tests/test_dfa.cpp
        tests/test_errors.cpp
        tests/test_groups.cpp
        tests/test_flags.cpp
        tests/test_posix.cpp
        tests/test_boundaries.cpp
        tests/test_lazy.cpp
        tests/test_lookahead.cpp
        tests/test_debug.cpp
        tests/test_serial.cpp
        tests/test_unicode.cpp
        tests/test_match_iterator.cpp
        tests/test_exhaustive.cpp
        tests/test_splitter.cpp
        tests/test_cache.cpp
        tests/test_analyzer.cpp
        tests/test_integration.cpp
        tests/test_optimizer.cpp
        tests/test_regex_suite.cpp
        tests/test_rewriter.cpp
        tests/test_token_stream.cpp
    )
    target_link_libraries(bishop_tests PRIVATE bishop_core)
    add_test(NAME bishop_tests COMMAND bishop_tests)
endif()

if(BISHOP_BUILD_BENCH)
    find_package(benchmark QUIET)
    if(benchmark_FOUND)
        add_executable(bishop_bench bench/benchmark.cpp bench/benchmark_extended.cpp)
        target_link_libraries(bishop_bench PRIVATE bishop_core benchmark::benchmark)
    endif()
endif()

install(TARGETS bishop_core ARCHIVE DESTINATION lib)
install(DIRECTORY include/bishop DESTINATION include)
