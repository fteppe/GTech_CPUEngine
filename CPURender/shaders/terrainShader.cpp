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

	normal = glm::normalize(normal);
	glm::vec3 lightDir = glm::normalize(glm::vec3(-1., -1., -1.));
	float lightIntensity = 0.1f + std::max(0.f, glm::dot(lightDir, -normal));
	lightIntensity = glm::clamp(lightIntensity, 0.f, 1.f);
	Color lightColor = Color::fromUNorm(lightIntensity, lightIntensity, lightIntensity);
	//return colors::BLUE;
	glm::vec3 normalColor = normal * 0.5f + 0.5f;
	Color colorNormal = Color::fromUNorm(normalColor.x, normalColor.y, normalColor.z);
	return lightColor;
}
