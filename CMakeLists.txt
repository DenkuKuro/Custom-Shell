cmake_minimum_required(VERSION 3.14)

# Set project-wide C++ standard defaults
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Custom_Shell VERSION 1.0 DESCRIPTION "My Custom Shell implementation" LANGUAGES CXX)

# Find required packages
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

link_directories(${JSONCPP_LIBRARY_DIRS})

include_directories(include)

file(GLOB SOURCES "src/shell.cpp")

add_executable(shelldx ${SOURCES})
add_executable(gemini_api "src/gemini-cmd.cpp")

# Link libraries
target_link_libraries(gemini_api 
    CURL::libcurl
    ${JSONCPP_LIBRARIES}
)

# Include directories
target_include_directories(gemini_api PRIVATE 
    ${JSONCPP_INCLUDE_DIRS}
)

