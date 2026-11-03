#include "terrainShader.h"
#include "blinnPhongShader.h"
#include "../renderAPI/renderResources/texture.h"



void TerrainShaderVertex::VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const {

	auto [v1, normal, matWeights] = vertexIn.getVertexAttributes<VS_Inputs>();

	glm::vec3 worldPos = object().modelMat * glm::vec4(v1, 1.f);
	normal = glm::mat3(object().modelMat) * normal;
	normal = glm::normalize(normal);
	glm::vec4 clipPos = scene().VP * object().modelMat * glm::vec4(v1, 1.);
	vertexOut.setVertexAttributes(VS_Outputs(clipPos, worldPos, normal, matWeights));
}


static glm::vec4 getTerrainColor(glm::vec3 worldPos, glm::vec4 matWeights, const renderResource::Texture& tex) {
	glm::vec2 uv = {worldPos.x, worldPos.z};
	uv = glm::mod(uv * 2.f, glm::vec2(1.f));

	glm::vec2 localUV = uv * 0.49f;

	glm::vec4 grass = tex.sample(glm::vec3(localUV ,0.));
	glm::vec4 stone = tex.sample(glm::vec3(localUV + glm::vec2(0.5f,0.f) ,0.));
	glm::vec4 sand = tex.sample(glm::vec3(localUV + glm::vec2(0.0f,0.5f),0.));
	glm::vec4 snow = tex.sample(glm::vec3(localUV + glm::vec2(0.5f,0.5f),0.));

	float weightSum = matWeights.x + matWeights.y + matWeights.z + matWeights.w;
	assert(weightSum > 0.99f && weightSum < 1.01f);

	glm::vec4 finalColor = grass * matWeights.x + stone * matWeights.y + sand * matWeights.z + snow * matWeights.w;
	finalColor = glm::clamp(finalColor, 0.f, 1.f);

	return finalColor;
}

Color TerrainPixelShader::PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const {
	auto [clipPos, worldPos, normal, matWeights] = interpolatedVertex.getVertexAttributes<PS_Inputs>();

	normal = glm::normalize(normal);

	glm::vec4 completeWeight = glm::vec4(matWeights, std::max(1 - (matWeights.x + matWeights.y + matWeights.z), 0.f));

	float matWeightSum = completeWeight.x + completeWeight.y + completeWeight.z + completeWeight.w;
	float matWeightSumInv = 1. / matWeightSum;
	completeWeight *= matWeightSumInv;
	glm::vec3 lightDir = scene().lightDir;
	glm::vec3 viewDir = glm::normalize(scene().worldCameraPos - worldPos);
	glm::vec3 specularComp = shaderUtils::blinnPhong(normal, viewDir, scene().lightDir, glm::vec3(1.0), 1.f);

	glm::vec3 lightIntensity = 0.1f + std::max(0.f, glm::dot(lightDir, -normal)) + specularComp;


	glm::vec2 uv = {worldPos.x, worldPos.z};
	uv = glm::mod(uv, glm::vec2(1.f));

	glm::vec4 colorTex = getTerrainColor(worldPos, completeWeight, getTexture(0));
	//lightIntensity = std::abs(lightIntensity - worldPos.x);
	colorTex *= glm::vec4(lightIntensity, 1.f);
	Color lightColor = shaderUtils::ACES_Tonemapping(colorTex);
	return lightColor;

}
