#include "../RenderAPI/RenderResources/ColorTexture.h"
#include "shader.h"



void Shader::bindTexture(ColorTextureHandle textureHandle, size_t index) {
	this->textureHandles.resize(index + 1);
	this->textureHandles[index] = textureHandle;
}

Color BasePixelShader::PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const {
	auto [clipPos, uv] = interpolatedVertex.getVertexAttributes<glm::vec4, glm::vec2>();
	return getColorTexture(0).sample(glm::vec3(uv, 1));
}
