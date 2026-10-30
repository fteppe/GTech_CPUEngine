#include "../renderAPI/renderResources/texture.h"
#include "shader.h"

Color BasePixelShader::PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const {
	auto [clipPos, uv] = interpolatedVertex.getVertexAttributes<PS_Inputs>();
	return getTexture(0).sample(glm::vec3(uv, 1));
}
