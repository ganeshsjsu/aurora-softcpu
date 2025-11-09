cmake_minimum_required(VERSION 3.24)
project(aurora_softcpu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AURORA_BUILD_TESTS "Build optional unit tests" OFF)

add_library(aurora_core
    src/memory.cpp
    src/bus.cpp
    src/devices.cpp
    src/alu.cpp
    src/control_unit.cpp
    src/cpu.cpp
    src/emulator.cpp
    src/assembler.cpp
    src/utils.cpp
)

target_include_directories(aurora_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(softcpu src/main.cpp)
target_link_libraries(softcpu PRIVATE aurora_core)

target_compile_options(aurora_core PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(softcpu PRIVATE -Wall -Wextra -Wpedantic)

if (AURORA_BUILD_TESTS)
    enable_testing()
    # Placeholder for future tests
endif()
