#pragma once
#include "renderAPI/renderAPI.h"

struct TerrainGenerationParams {
	glm::vec2 terrainSize;
	glm::ivec2 tileCount;
};

VertexBufferHandle generateTerrain(RenderAPI& renderAPI, const TerrainGenerationParams& params);

