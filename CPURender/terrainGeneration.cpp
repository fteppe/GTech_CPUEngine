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

constexpr glm::ivec2 quadVerts[6] = {{0,0}, {0,1}, {1,1},
									{0,0}, {1,1}, {1,0}};

static glm::ivec2 getCoordInQuad(int index) {
	return quadVerts[index%6];
}


class TerrainGenerator {

	std::vector<float> heightMap;
	std::vector<glm::vec3> normalsFaces;
	glm::ivec2 heightPointCount;
	const TerrainGenerationParams& params;
	const float edgeLength;

	void addToNormalIfFound(glm::ivec2 coord, glm::vec3& normalInOut, int& normalsCount) {
		if (coord.x >= 0 && coord.y >= 0 && coord.x < params.tileCount.x && coord.y < params.tileCount.y)
		{
			normalInOut += normalsFaces[linearizeCoord(coord, params.tileCount)];
			normalsCount++;
		}
	}

	glm::vec3 getPosFromCoord(glm::ivec2 coord) {
		float height = heightMap[linearizeCoord(coord, heightPointCount)];
		return glm::vec3(coord.x * edgeLength, height, coord.y * edgeLength);
	}


public:

	TerrainGenerator(const TerrainGenerationParams&  paramsIn) : params(paramsIn), edgeLength(paramsIn.terrainSize.x / paramsIn.tileCount.x) {}

	VertexBufferHandle generateTerrain(RenderAPI &renderAPI) {

		heightPointCount = params.tileCount + 1;
		heightMap.resize(heightPointCount.x * heightPointCount.y);

		for (int i = 0 ; i < heightMap.size(); i++) {
			glm::ivec2 coord = getCoord(i, heightPointCount);

			glm::vec2 pos = {float(coord.x) / float(params.tileCount.x), coord.y / float(params.tileCount.y)};

			float height = glm::perlin(pos * 5.f);
			height += glm::perlin((pos + 12.f) * 2.f) * 0.7f;
			height += glm::perlin(pos * 30.f) * 0.1f;
			height = (height + 1 * 0.5f) * params.terrainSize.y;
			heightMap[i] = height ;//std::cos(pos.x) * 0.02f + std::cos(pos.y) * 0.03;
		}

		std::vector<glm::vec3> positions; positions.resize(params.tileCount.x * params.tileCount.y * 6);
		std::vector<glm::vec3> normals; normals.resize(positions.size());

		for (int i = 0 ; i < positions.size(); i++) {
			glm::ivec2 coordInQuad = getCoordInQuad(i);
			int quadIndex = i / 6;
			glm::ivec2 quadCoord = getCoord(quadIndex, params.tileCount);

			glm::ivec2 pointCoord = quadCoord + coordInQuad;

			float height = heightMap[linearizeCoord(pointCoord, heightPointCount)];

			positions[i] = getPosFromCoord(pointCoord);
		}

		normalsFaces.resize(params.tileCount.x * params.tileCount.y);
		for (int i =0; i < normalsFaces.size(); i++) {
			glm::ivec2 quadCoord = getCoord(i, params.tileCount);

			glm::vec3 normalFace(0.f);

			for (int iTri = 0; iTri < 2; iTri++) {
				int index1 = linearizeCoord(quadCoord + quadVerts[iTri * 3], heightPointCount);
				int index2 = linearizeCoord(quadCoord + quadVerts[iTri * 3 + 1], heightPointCount);
				int index3 = linearizeCoord(quadCoord + quadVerts[iTri * 3 + 2], heightPointCount);

				glm::vec3 v1 = getPosFromCoord(quadCoord + quadVerts[iTri * 3]);
				glm::vec3 v2 = getPosFromCoord(quadCoord + quadVerts[iTri * 3 + 1]);
				glm::vec3 v3 = getPosFromCoord(quadCoord + quadVerts[iTri * 3 + 2]);

				normalFace += glm::normalize(glm::cross(v2 - v1, v3 - v2));
			}

			normalFace *=0.5f;
			normalsFaces[i] = normalFace;
		}


		for (int i = 0 ; i < normals.size(); i++) {
			glm::ivec2 coordInQuad = getCoordInQuad(i);
			int quadIndex = i / 6;
			glm::ivec2 quadCoord = getCoord(quadIndex, params.tileCount);

			glm::ivec2 pointCoord = quadCoord + coordInQuad;

			//x²+y²+z² = 1 => z² = 1 - x² - y ² => z = sqrt(1 - x² - y ²)
			glm::vec3 vertexNormal(0.f);
			int normalCount = 0;
			addToNormalIfFound({pointCoord.x, pointCoord.y}, vertexNormal,normalCount);
			addToNormalIfFound({pointCoord.x -1, pointCoord.y}, vertexNormal,normalCount);
			addToNormalIfFound({pointCoord.x, pointCoord.y -1}, vertexNormal,normalCount);
			addToNormalIfFound({pointCoord.x -1, pointCoord.y -1}, vertexNormal,normalCount);

			vertexNormal = vertexNormal / float(normalCount);
			vertexNormal = glm::normalize(vertexNormal);

			normals[i] = vertexNormal;// normalsFaces[linearizeCoord(quadCoord, params.tileCount)];
		}

		const std::vector<renderResource::VertexAttribute> attributes = {renderResource::VertexAttribute::Vec3, renderResource::VertexAttribute::Vec3, renderResource::VertexAttribute::Vec3};

		auto handle = renderAPI.createVertexBuffer(positions.size(), attributes);
		renderAPI.updateVertexBuffer(handle, positions, 0);
		renderAPI.updateVertexBuffer(handle, normals, 1);
		std::vector<glm::vec3> materialWeights; materialWeights.resize(positions.size());

		for (int i = 0 ; i < materialWeights.size(); i++) {
			glm::vec3 pos = positions[i];
			glm::vec3 weight(0.f);
			if (pos.y > 0.9 * params.terrainSize.y) {
				weight = glm::vec3(0.f);//All 0 is snow
			}
			else if (pos.y > 0.5 * params.terrainSize.y) {
				weight = {0.,1.,0.}; //Stone
			}
			else if (pos.y > 0.1 * params.terrainSize.y) {
				weight = {1.,0.,0.}; //grass
			}
			else {
				weight = {0.,0.,1.}; //sand
			}
			materialWeights[i] = weight;
		}
		renderAPI.updateVertexBuffer(handle, materialWeights, 2);
		return handle;
	}


};


VertexBufferHandle generateTerrain(RenderAPI &renderAPI, const TerrainGenerationParams &params) {

	TerrainGenerator generator(params);
	return generator.generateTerrain(renderAPI);

}
