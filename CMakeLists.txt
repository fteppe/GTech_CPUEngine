cmake_minimum_required(VERSION 4.3)
project(CPURender)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

include_directories(${CMAKE_SOURCE_DIR}/Libs/glm)

# Create your game executable target as usual
add_subdirectory(CPURender)
set_property(TARGET CPURender PROPERTY CXX_STANDARD 20)


# Link to the actual SDL3 library.
target_link_libraries(CPURender SDL3::SDL3)