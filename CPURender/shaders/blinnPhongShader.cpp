//
// Created by francois on 11/2/26.
//

#include "blinnPhongShader.h"
#include "blinnPhongShader.h"

#include "../renderAPI/renderResources/texture.h"

void BlinnPhongVS::VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const {

	auto [pos, normal, uvs] = vertexIn.getVertexAttributes<VS_Inputs>();

	glm::vec4 clipPos = scene().VP * object().modelMat * glm::vec4(pos, 1.0f);
	pos = object().modelMat * glm::vec4(pos, 1.0f);
	normal = object().modelMat * glm::vec4(normal, 1.0f);
	vertexOut.setVertexAttributes(VS_Outputs(clipPos, pos, normal, uvs));
}

Color BlinnPhongPS::PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const {

	auto [clipPos, worldPos, normal, uv] = interpolatedVertex.getVertexAttributes<PS_Inputs>();
	glm::vec3 lightDir = scene().lightDir;


	glm::vec3 viewDir = glm::normalize(scene().worldCameraPos - worldPos);
	glm::vec3 specularComp = shaderUtils::blinnPhong(normal, viewDir, scene().lightDir, glm::vec3(1.0), 1.f);
	float lambert = std::max(0.f,glm::dot(-normal, lightDir));
	glm::vec3 lightIntensity = lambert + specularComp + 0.1f;

	glm::vec4 color = getTexture(0).sample(glm::vec3(uv, 0.f)) * glm::vec4(lightIntensity, 1.0f);

	Color col = shaderUtils::ACES_Tonemapping(color);
	return col;
}


namespace shaderUtils {
	glm::vec3 blinnPhong(glm::vec3 normal, glm::vec3 viewDir, glm::vec3 lightDir, glm::vec3 lightColor, float specularIntensity) {
		glm::vec3 reflectDir = glm::reflect(lightDir, normal);
		float spec = pow(std::max(glm::dot(viewDir, reflectDir), 0.f), 32);
		float specular = specularIntensity * spec;
		return specular * lightColor;
	}
}