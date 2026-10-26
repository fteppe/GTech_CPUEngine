add_executable(CPURender main.cpp engine.cpp
        terrainGeneration.cpp)
add_subdirectory(renderAPI)
add_subdirectory(shaders)