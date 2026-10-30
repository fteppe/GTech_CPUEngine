#pragma once
#include "shader.h"

class TerrainShaderVertex : public VertexShader {
public:
	DEFINE_VS_INPUT_OUTPUT((glm::vec3, glm::vec3, glm::vec3), (glm::vec4, glm::vec3 ,glm::vec3, glm::vec3))
	BIND_CB(global, GlobalConstantBuffer, 0)

	void VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const override;

};


class TerrainPixelShader : public PixelShader {
public:
	DEFINE_PS_INPUT(glm::vec4, glm::vec3 ,glm::vec3, glm::vec3)

	Color PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const override;
};

