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
# Verilator library
#

# based on verilator documentation
# https://veripool.org/guide/latest/verilating.html#cmake

find_package(verilator REQUIRED HINTS $ENV{VERILATOR_ROOT})

message(STATUS "VERILATOR_ROOT = ${VERILATOR_ROOT}")
message(STATUS "VERILATOR_BIN  = ${VERILATOR_BIN}")

#
# googletest-verilog test bench generation
#

find_package(Python3 COMPONENTS Interpreter REQUIRED)

function(googletest_verilog_testbench TB_TARGET)
    cmake_parse_arguments(
        TB                          # prefix
        ""                          # list of options (true / false based on whether option is present or not)
        "SOURCE"             # single value keywords
        "INCLUDE_DIRS"              # multi-value keywords
        ${ARGN}                     # arguments provided to the function
    )

    message(STATUS "SOURCE = ${TB_SOURCE}")
    message(STATUS "TARGET = ${TB_TARGET}")
    message(STATUS "INCLUDE_DIRS = ${TB_INCLUDE_DIRS}")

    set(TB_VERILATED_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/verilated/${TB_TESTBENCH})
    message(STATUS "TB_VERILATED_OUTPUT_DIR = ${TB_VERILATED_OUTPUT_DIR}")

    get_filename_component(TB_SOURCE_BASENAME "${TB_SOURCE}" NAME_WE)
    set(TB_TESTBENCH "${TB_SOURCE_BASENAME}TestBench")
    set(TB_TESTBENCH_HEADER "${TB_VERILATED_OUTPUT_DIR}/${TB_TESTBENCH}.h")
    set(TB_TESTBENCH_SOURCE "${TB_VERILATED_OUTPUT_DIR}/${TB_TESTBENCH}.cpp")

    message(STATUS "TB_TESTBENCH = ${TB_TESTBENCH}")
    message(STATUS "TB_TESTBENCH_HEADER = ${TB_TESTBENCH_HEADER}")
    message(STATUS "TB_TESTBENCH_SOURCE = ${TB_TESTBENCH_SOURCE}")
    message(STATUS "CMAKE_BINARY_DIR = " ${CMAKE_BINARY_DIR})
    message(STATUS "CMAKE_CURRENT_BINARY_DIR = " ${CMAKE_CURRENT_BINARY_DIR})

    # 1. verilate the verilog module    

    verilate(${TB_TARGET} SOURCES ${TB_SOURCE} INCLUDE_DIRS ${TB_INCLUDE_DIRS} DIRECTORY ${TB_VERILATED_OUTPUT_DIR})
    
    # 2. generate test bench
    set(TOOL_GENERATE_TESTBENCH ${CMAKE_SOURCE_DIR}/tools/generate_testbench.py)
    
    add_custom_command(
        OUTPUT ${TB_TESTBENCH_HEADER} ${TB_TESTBENCH_SOURCE}
        COMMAND ${Python3_EXECUTABLE} ${TOOL_GENERATE_TESTBENCH} --name ${TB_TESTBENCH} --verilated-header-dir ${TB_VERILATED_OUTPUT_DIR} --output-header ${TB_TESTBENCH_HEADER} --output-source ${TB_TESTBENCH_SOURCE}
        DEPENDS ${TOOL_GENERATE_TESTBENCH} ${TB_SOURCE}
    )

    target_sources(${TB_TARGET} PRIVATE ${TB_TESTBENCH_HEADER} ${TB_TESTBENCH_SOURCE})

    set(TB_TESTBENCH_TARGET "googletest_verilog_testbench-${TB_TESTBENCH}")

    # 3. use a custom target to force cmake to create testbench first, before compiling TARGET

    add_custom_target(${TB_TESTBENCH_TARGET}
        DEPENDS ${TB_TESTBENCH_HEADER} ${TB_TESTBENCH_SOURCE}
    )

    add_dependencies(${TB_TARGET} ${TB_TESTBENCH_TARGET})
    
endfunction()



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

googletest_verilog_testbench(googletest-verilog-example SOURCE example/Counter.v)

target_link_libraries(googletest-verilog-example PRIVATE
    googletest-verilog    
    gtest
    gmock
)

target_include_directories(googletest-verilog-example PRIVATE ${VERILATOR_INCLUDE_DIRS})
