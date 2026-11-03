//
// Created by francois on 11/2/26.
//

#include "blinnPhongShader.h"
#include "blinnPhongShader.h"

#include "../renderAPI/renderResources/texture.h"

void BlinnPhongVS::VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const {

	auto [pos, normal, uvs] = vertexIn.getVertexAttributes<VS_Inputs>();

	glm::vec4 clipPos = scene().VP * object().modelMat * glm::vec4(pos, 1.0f);

	vertexOut.setVertexAttributes(VS_Outputs(clipPos, pos, normal, uvs));

}

Color BlinnPhongPS::PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const {

	auto [clipPos, worldPos, normal, uv] = interpolatedVertex.getVertexAttributes<PS_Inputs>();
	glm::vec3 lightDir = scene().lightDir;


	glm::vec3 viewDir = glm::normalize(scene().worldCameraPos - worldPos);
	glm::vec3 reflectDir = glm::reflect(lightDir, normal);

	float spec = pow(std::max(glm::dot(viewDir, reflectDir), 0.f), 32);
	float specular = 1.f * spec;
	float lambert = std::max(0.f,glm::dot(-normal, lightDir));
	float lightIntensity = lambert + specular + 0.1f;

	glm::vec4 color = getTexture(0).sample(glm::vec3(uv, 0.f)) * lightIntensity;

	Color col = shaderUtils::ACES_Tonemapping(color);
	return col;
}
