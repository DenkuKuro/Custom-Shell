cmake_minimum_required(VERSION 3.14)

# Set project-wide C++ standard defaults
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Custom_Shell VERSION 1.0 DESCRIPTION "My Custom Shell implementation" LANGUAGES CXX)

include_directories(include)

file(GLOB SOURCES "src/main.cpp")

add_executable(shelldx ${SOURCES})
