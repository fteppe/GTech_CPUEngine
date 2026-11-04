#include "meshGenerator.h"

#include <complex>
#include <vector>
#include <glm/vec3.hpp>

#include "../Libs/glm/glm/geometric.hpp"
#include "renderAPI/renderAPI.h"
#include "renderAPI/renderResources/vertexAttribute.h"

VertexBufferHandle MeshGenerator::generateCube(RenderAPI &renderAPI, bool inverted) {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;

	constexpr int quadIndices[6] = {0,1,2,0,2,3};
	constexpr glm::vec2 uvs[4] = {{0.,0.}, {0.,1.}, {1.,1.}, {1.,0.f}};

	for (int iFace = 0; iFace < 6; iFace++) {

		int iAxis = iFace/2;
		int iDir = iFace%2 ? -1: 1;

		glm::vec3 normal(0.f);
		normal[iAxis] = iDir;

		int tanAxis = (iAxis + 1)%3;

		glm::vec3 tan(0.f); tan[tanAxis] = iDir;

		glm::vec3 coTan = glm::cross(tan, normal);

		const glm::vec3 facePos[4] = {normal - tan - coTan, normal - tan + coTan, normal + tan + coTan, normal  + tan - coTan};

		for (int iVertex = 0; iVertex < 6; iVertex++) {

			int vertexIndex = inverted ? 5 - iVertex : iVertex;

			positions.push_back(facePos[quadIndices[vertexIndex]]);
			normals.push_back(normal);
			texCoords.push_back(uvs[quadIndices[vertexIndex]]);
		}
	}
	assert(positions.size() == 6 * 6);

	std::vector<renderResource::VertexAttribute> attributes = {renderResource::VertexAttribute::Vec3,renderResource::VertexAttribute::Vec3,renderResource::VertexAttribute::Vec2};
	auto handle = renderAPI.createVertexBuffer(6*6, attributes);
	renderAPI.updateVertexBuffer(handle, positions, 0);
	renderAPI.updateVertexBuffer(handle, normals, 1);
	renderAPI.updateVertexBuffer(handle, texCoords, 2);
	return handle;
}

VertexBufferHandle MeshGenerator::generateQuad(RenderAPI &renderAPI, glm::vec2 size) {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;

	constexpr int quadIndices[6] = {0,1,2,0,2,3};
	constexpr glm::vec2 uvs[4] = {{0.,0.}, {0.,1.}, {1.,1.}, {1.,0.f}};
	constexpr glm::vec3 pos[4] = {{-1,0,-1.f}, {-1,0,1.f}, {1,0,1.f}, {1,0,-1.f}};

	for (int i = 0 ; i < 6; i++) {
		glm::vec3 poslocal = pos[quadIndices[i]];
		poslocal.x *= size.x;
		poslocal.z *= size.y;
		positions.push_back(poslocal);
		texCoords.push_back(uvs[quadIndices[i]]);
		normals.push_back(glm::vec3(0.,1.,0.));
	}

	std::vector<renderResource::VertexAttribute> attributes = {renderResource::VertexAttribute::Vec3,renderResource::VertexAttribute::Vec3,renderResource::VertexAttribute::Vec2};
	auto handle = renderAPI.createVertexBuffer(positions.size(), attributes);
	renderAPI.updateVertexBuffer(handle, positions, 0);
	renderAPI.updateVertexBuffer(handle, normals, 1);
	renderAPI.updateVertexBuffer(handle, texCoords, 2);
	return handle;
}
