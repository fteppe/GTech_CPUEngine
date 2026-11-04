#pragma once
#include "shader.h"

class SkyVS : public VertexShader {
public:

	BIND_CB(scene, SceneConstantBuffer, 0);
	DEFINE_VS_INPUT_OUTPUT((glm::vec3, glm::vec3, glm::vec2), (glm::vec4, glm::vec3))
	void VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const override;
};


class SkyPS : public PixelShader {
public:

	DEFINE_PS_INPUT(glm::vec4, glm::vec3)
	Color PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const override;
};
