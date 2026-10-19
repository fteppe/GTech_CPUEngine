cmake_minimum_required(VERSION 3.5)
project(CPURender)
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)


# Create your game executable target as usual
add_executable(CPURender WIN32 CPURender/main.cpp CPURender/engine.cpp)

include_directories(${CMAKE_SOURCE_DIR}/Libs/glm)

# Link to the actual SDL3 library.
target_link_libraries(CPURender PRIVATE SDL3::SDL3)