#include "terrainShader.h"

void TerrainShaderVertex::VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const {

	auto [v1, normal] = vertexIn.getVertexAttributes<VS_Inputs>();

	GlobalConstantBuffer buffer = global();
	glm::vec3 worldPos = buffer.model * glm::vec4(v1, 1.f);
	normal = glm::mat3(buffer.model) * normal;
	glm::vec4 clipPos = buffer.MVP * glm::vec4(v1, 1.);

	vertexOut.setVertexAttributes(VS_Outputs(clipPos, worldPos, normal));
}

Color TerrainPixelShader::PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const {
	auto [clipPos, worldPos, normal] = interpolatedVertex.getVertexAttributes<PS_Inputs>();


	glm::vec3 lightDir = glm::normalize(glm::vec3(-1., 0., -1.));
	float lightIntensity = 0.2f + std::min(0.f, glm::dot(lightDir, normal));
	return colors::BLUE;// Color(lightIntensity, lightIntensity, lightIntensity);
}
