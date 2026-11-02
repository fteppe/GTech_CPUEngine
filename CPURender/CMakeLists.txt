add_executable(CPURender main.cpp engine.cpp
        terrainGeneration.cpp
        logger.cpp
        meshGenerator.cpp)
add_subdirectory(renderAPI)
add_subdirectory(shaders)