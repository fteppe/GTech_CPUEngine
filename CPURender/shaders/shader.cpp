#include "../renderAPI/renderResources/texture.h"
#include "shader.h"

Color BasePixelShader::PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const {
	auto [clipPos, uv] = interpolatedVertex.getVertexAttributes<PS_Inputs>();
	return getTexture(0).sample(glm::vec3(uv, 1));
}

glm::vec4 shaderUtils::ACES_Tonemapping(glm::vec4 colorIn) {
		float a = 2.51f;
		float b = 0.03f;
		float c = 2.43f;
		float d = 0.59f;
		float e = 0.14f;
		return glm::clamp((colorIn * (a * colorIn + b)) / (colorIn * (c * colorIn + d) + e), 0.f, 1.f);
}

