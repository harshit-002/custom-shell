cmake_minimum_required(VERSION 3.13)

project(shell-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cpr/include)  
include_directories(/usr/include/nlohmann)     
# Add the subdirectory for cpr
add_subdirectory(cpr)

add_executable(shell ${SOURCE_FILES})

# Link the cpr library with your shell
target_link_libraries(shell PRIVATE cpr::cpr)