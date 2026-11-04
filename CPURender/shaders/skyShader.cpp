#include "skyShader.h"
#include "../renderAPI/renderResources/texture.h"

void SkyVS::VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const {

	auto [pos, normal, uv] = vertexIn.getVertexAttributes<VS_Inputs>();

	glm::vec4 clipSpacePos = scene().VP * glm::vec4(pos * 20.f, 1.f);

	vertexOut.setVertexAttributes(VS_Outputs(clipSpacePos, pos));

}

Color SkyPS::PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const {

	auto [clip, dir] = interpolatedVertex.getVertexAttributes<PS_Inputs>();

	// dir = dir - scene().worldCameraPos;

	glm::vec3 visibleDir = dir * 0.5f + 0.5f;
	// return glm::vec4(visibleDir, 1.f);

	return getTexture(0).sample(glm::normalize(dir));
}
