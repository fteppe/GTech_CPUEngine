#pragma once
#include "renderAPI/renderAPI.h"

struct TerrainGenerationParams {
	glm::vec2 terrainSize;
};

VertexBufferHandle generateTerrain(RenderAPI& renderAPI, const TerrainGenerationParams& params);