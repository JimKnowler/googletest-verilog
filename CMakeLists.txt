cmake_minimum_required(VERSION 3.31.6)

project(googletest-verilog
    VERSION 0.2.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

#
# GoogleTest Library
#

# download google test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
)

# prevent googletest from overriding our compiler/linker options
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#
# Verilog library
#

# based on verilator documentation
# https://veripool.org/guide/latest/verilating.html#cmake

find_package(verilator REQUIRED HINTS $ENV{VERILATOR_ROOT})

message(STATUS "VERILATOR_ROOT = ${VERILATOR_ROOT}")
message(STATUS "VERILATOR_BIN  = ${VERILATOR_BIN}")

#
# googletest-verilog static library
#

include_directories(include)

add_library(googletest-verilog
    src/ConsoleColour.cpp
    src/MatchesTrace.cpp
    src/PortDescription.cpp
    src/Step.cpp
    src/Trace.cpp
    src/TraceBuilder.cpp
)

# add googletest/googlemock includes, without linking to gtest
get_target_property(GTEST_INCLUDES GTest::gtest INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(GMOCK_INCLUDES GTest::gmock INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(googletest-verilog PRIVATE ${GTEST_INCLUDES} ${GMOCK_INCLUDES})

#
# googletest-verilog internal Unit Tests
#

add_executable(googletest-verilog-test
    test/main.cpp
    test/MatchesTrace.test.cpp
    test/Step.test.cpp
    test/TestBench.test.cpp
    test/Trace.test.cpp
    test/TraceBuilder.test.cpp
)

target_link_libraries(googletest-verilog-test
    gtest
    gmock
    googletest-verilog
)

#
# googletest-verilog Example Usage
#

add_executable(googletest-verilog-example
    example/main.cpp
    example/Counter.test.cpp
)

verilate(googletest-verilog-example SOURCES example/Counter.v)

target_link_libraries(googletest-verilog-example PRIVATE
    googletest-verilog    
    gtest
    gmock
)

target_include_directories(googletest-verilog-example PRIVATE ${VERILATOR_INCLUDE_DIRS})
