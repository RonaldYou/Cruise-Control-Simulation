cmake_minimum_required(VERSION 3.10)

# Project name
project(CruiseControlSimulator)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directory for headers
include_directories(include)

# Gather source files
set(SOURCES
    src/main.cpp
    src/Vehicle.cpp
    src/Car.cpp
    src/PIDController.cpp
    src/Simulator.cpp
)

# Build executable
add_executable(cruise_sim ${SOURCES})