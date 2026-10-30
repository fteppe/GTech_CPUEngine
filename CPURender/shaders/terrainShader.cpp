#include "terrainShader.h"

#include "../renderAPI/renderResources/texture.h"

const glm::vec3 lightDir = glm::normalize(glm::vec3(-1., -1., -1.));


void TerrainShaderVertex::VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const {

	auto [v1, normal] = vertexIn.getVertexAttributes<VS_Inputs>();

	GlobalConstantBuffer buffer = global();
	glm::vec3 worldPos = buffer.model * glm::vec4(v1, 1.f);
	normal = glm::mat3(buffer.model) * normal;
	normal = glm::normalize(normal);
	glm::vec4 clipPos = buffer.MVP * glm::vec4(v1, 1.);
	float lightIntensity = 0.1f + std::max(0.f, glm::dot(lightDir, -normal));

	// worldPos.x = glm::clamp(lightIntensity, 0.f, 1.f);
	vertexOut.setVertexAttributes(VS_Outputs(clipPos, worldPos, normal));
}

Color TerrainPixelShader::PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const {
	auto [clipPos, worldPos, normal] = interpolatedVertex.getVertexAttributes<PS_Inputs>();

	normal = glm::normalize(normal);
	// float lightIntensity = normal.z;
	float lightIntensity = 0.1f + std::max(0.f, glm::dot(lightDir, -normal));
	lightIntensity = glm::clamp(lightIntensity, 0.f, 1.f);

	glm::vec2 uv = {worldPos.x, worldPos.z};
	uv = glm::mod(uv, glm::vec2(1.f));

	glm::vec4 colorTex = getTexture(0).sample(glm::vec3(uv, 0.f));
	//lightIntensity = std::abs(lightIntensity - worldPos.x);
	colorTex *= lightIntensity;
	Color lightColor = Color::fromUNorm(colorTex.x, colorTex.y, colorTex.y);
	//return colors::BLUE;
	glm::vec3 normalColor = normal * 0.5f + 0.5f;
	Color colorNormal = Color::fromUNorm(normal.y, normal.y, normal.y);
	//return colorNormal;
	return lightColor;

}
