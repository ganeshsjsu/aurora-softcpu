cmake_minimum_required(VERSION 3.24)
# Define the project name and language
project(softcpu LANGUAGES CXX)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build unit tests
option(SOFTCPU_BUILD_TESTS "Build optional unit tests" OFF)

# Define the core library with source files
add_library(softcpu_core
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

# Include directories for the core library
target_include_directories(softcpu_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Define the main executable
add_executable(softcpu src/main.cpp)
# Link the core library to the executable
target_link_libraries(softcpu PRIVATE softcpu_core)

# Set compiler options for warnings
target_compile_options(softcpu_core PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(softcpu PRIVATE -Wall -Wextra -Wpedantic)

# Enable testing if the option is set
if (SOFTCPU_BUILD_TESTS)
    enable_testing()
    # Placeholder for future tests
endif()
