#pragma once
#include "renderAPI/renderAPI.h"

struct TerrainGenerationParams {
	glm::vec3 terrainSize;
	glm::ivec2 tileCount;
};

VertexBufferHandle generateTerrain(RenderAPI& renderAPI, const TerrainGenerationParams& params);

