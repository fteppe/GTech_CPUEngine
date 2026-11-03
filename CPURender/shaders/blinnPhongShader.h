#pragma once
#include "shader.h"


namespace shaderUtils {
	glm::vec3 blinnPhong(glm::vec3 normal, glm::vec3 viewDir, glm::vec3 lightDir, glm::vec3 lightColor, float specularIntensity);
}

class BlinnPhongVS : public VertexShader {

	DEFINE_VS_INPUT_OUTPUT((glm::vec3, glm::vec3, glm::vec2), (glm::vec4, glm::vec3, glm::vec3, glm::vec2));
	BIND_CB(scene, SceneConstantBuffer, 0);
	BIND_CB(object, ObjectConstantBuffer, 1);
	void VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const override;
};

class BlinnPhongPS : public PixelShader {
public:
	BIND_CB(scene, SceneConstantBuffer, 0);
	Color PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const override;
private:
	DEFINE_PS_INPUT(glm::vec4, glm::vec3, glm::vec3, glm::vec2)
};


