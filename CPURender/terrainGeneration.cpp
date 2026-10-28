#include "terrainGeneration.h"
#include <utility>

#include "glm/trigonometric.hpp"
#include "glm/gtc/noise.hpp"
#include "renderAPI/renderResources/vertexAttribute.h"


static int linearizeCoord(glm::ivec2 coord, glm::ivec2 dims)
{
	if (coord.x < 0 || coord.x >= dims.x || coord.y < 0 || coord.y >= dims.y)
		return -1;
	return coord.x + coord.y * dims.x;
}

static glm::ivec2 getCoord(int index, glm::ivec2 dims) {
	return {index % dims.x, index / dims.x};
}

static glm::ivec2 getCoordInQuad(int index) {
	constexpr glm::ivec2 quadVerts[6] = {{0,0}, {0,1}, {1,1},
										{0,0}, {1,1}, {1,0}};
	return quadVerts[index%6];
}

struct AnglesToUp {
	float xBefore = 0.f;
	float xAfter = 0.f;
	float yBefore = 0.f;
	float yAfter = 0.f;
};


class TerrainGenerator {

	std::vector<float> heightMap;
	std::vector<glm::vec2> angleToUpXY;
	glm::ivec2 heightPointCount;
	const TerrainGenerationParams& params;
	AnglesToUp getAngleNeighbours(glm::ivec2 coord) {
		AnglesToUp angles;
		glm::vec2 anglesNext = angleToUpXY[linearizeCoord(coord, heightPointCount)];
		angles.xAfter = anglesNext.x;
		angles.xAfter = anglesNext.y;
		if (coord.x > 0) {
			angles.xBefore = angleToUpXY[linearizeCoord({coord.x - 1, coord.y}, heightPointCount)].x;
		}
		if (coord.y > 0) {
			angles.yBefore = angleToUpXY[linearizeCoord({coord.x, coord.y - 1}, heightPointCount)].y;
		}
		return angles;
	}

	float getAngleOnAxisForPoint(float angleBefore, float angleAfter, int posInEdge) {
		float angleDelta = abs(angleAfter - angleBefore);
		if (angleDelta > 3.14f /4.f) {
			return posInEdge ? angleAfter : angleBefore;
		}
		return (angleBefore + angleAfter) / 2.f;
	}

public:

	TerrainGenerator(const TerrainGenerationParams&  paramsIn) : params(paramsIn) {}

	VertexBufferHandle generateTerrain(RenderAPI &renderAPI) {

		heightPointCount = params.tileCount + 1;
		heightMap.resize(heightPointCount.x * heightPointCount.y);

		for (int i = 0 ; i < heightMap.size(); i++) {
			glm::ivec2 coord = getCoord(i, heightPointCount);

			glm::vec2 pos = {coord.x, coord.y};

			float height = glm::perlin(pos);

			heightMap[i] = std::cos(pos.x) * 0.2f;
		}

		angleToUpXY.resize(heightMap.size());
		float edgeLength = params.terrainSize.x / params.tileCount.x;

		for (int i = 0 ; i < angleToUpXY.size(); i++) {
			glm::ivec2 coord = getCoord(i, heightPointCount);
			float currentHeight = heightMap[linearizeCoord(coord, heightPointCount)];
			float nextHeight_X = currentHeight;
			if (coord.x < heightPointCount.x - 1) {
				nextHeight_X = heightMap[linearizeCoord({coord.x + 1, coord.y}, heightPointCount)];
			}
			float nextHeight_Y = currentHeight;
			if (coord.y < heightPointCount.y - 1) {
				nextHeight_Y = heightMap[linearizeCoord({coord.x, coord.y + 1}, heightPointCount)];
			}
			glm::vec2 dz_xy = glm::vec2(nextHeight_X - currentHeight, nextHeight_Y - currentHeight);

			float angleToUpX = glm::atan(dz_xy.x /  edgeLength);
			float angleToUpY = glm::atan(dz_xy.y /  edgeLength);
			angleToUpXY[i] = glm::vec2(angleToUpX, angleToUpY);
		}

		std::vector<glm::vec3> positions; positions.resize(params.tileCount.x * params.tileCount.y * 6);
		std::vector<glm::vec3> normals; normals.resize(positions.size());

		for (int i = 0 ; i < positions.size(); i++) {
			glm::ivec2 coordInQuad = getCoordInQuad(i);
			int quadIndex = i / 6;
			glm::ivec2 quadCoord = getCoord(quadIndex, params.tileCount);

			glm::ivec2 pointCoord = quadCoord + coordInQuad;

			float height = heightMap[linearizeCoord(pointCoord, heightPointCount)];

			positions[i] = glm::vec3(pointCoord.x * edgeLength, height, params.terrainSize.y - pointCoord.y * edgeLength);
		}


		for (int i = 0 ; i < normals.size(); i++) {
			glm::ivec2 coordInQuad = getCoordInQuad(i);
			int quadIndex = i / 6;
			glm::ivec2 quadCoord = getCoord(quadIndex, params.tileCount);

			glm::ivec2 pointCoord = quadCoord + coordInQuad;

			AnglesToUp angles = getAngleNeighbours(pointCoord);

			float xAngle = getAngleOnAxisForPoint(angles.xBefore, angles.xAfter, coordInQuad.x);
			float yAngle = getAngleOnAxisForPoint(angles.yBefore, angles.yAfter, coordInQuad.y);
			glm::vec2 normalXY(std::sin(xAngle), std::sin(yAngle));
			//x²+y²+z² = 1 => z² = 1 - x² - y ² => z = sqrt(1 - x² - y ²)
			glm::vec3 vertexNormal = glm::vec3(normalXY.x, std::sqrt(1 - glm::dot(normalXY, normalXY)), normalXY.y);
			normals[i] = vertexNormal;
		}

		const std::vector<renderResource::VertexAttribute> attributes = {renderResource::VertexAttribute::Vec3, renderResource::VertexAttribute::Vec3};

		auto handle = renderAPI.createVertexBuffer(positions.size(), attributes);
		renderAPI.updateVertexBuffer(handle, positions, 0);
		renderAPI.updateVertexBuffer(handle, normals, 1);

		return handle;
	}


};


VertexBufferHandle generateTerrain(RenderAPI &renderAPI, const TerrainGenerationParams &params) {

	TerrainGenerator generator(params);
	return generator.generateTerrain(renderAPI);

}
