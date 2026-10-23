#pragma once
#include "glm/ext/matrix_float4x4.hpp"

#include "../RenderAPI/vertexIterator.h"
#include "../RenderAPI/renderAPI.h"
#include "../baseTypes/color.h"

namespace renderResource {
	class ColorTexture;
}

class Shader {
public:
	template<typename TConstantBuffer>
	const TConstantBuffer& getConstantBuffer() {

	}
	virtual ~Shader(){}
	const renderResource::ColorTexture& getColorTexture(size_t index) const {
		return *colorTextures[index];
	}

	void bindTexture(ColorTextureHandle, size_t index);

private:
	std::vector<renderResource::ColorTexture*>	colorTextures;
	std::vector<ColorTextureHandle> textureHandles;

	friend class RenderAPI;
};


class VertexShader : public Shader {
public:
	virtual void VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const = 0;
};

class PixelShader : public Shader {
public:
	virtual Color PS_MAIN(ConstVertexReference interpolatedVertex) const = 0;
};

struct GlobalConstantBuffer {
	glm::mat4 model;
	glm::mat4 MVP;
};

class BaseVertexShader : public VertexShader {
	public:

	GlobalConstantBuffer buffer;

	void VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const  override {
		auto [v1, uv1] = vertexIn.getVertexAttributes<glm::vec3, glm::vec2>();
		glm::vec4 v1_4 = buffer.MVP * glm::vec4(v1, 1.);
		vertexOut.setVertexAttributes(std::make_tuple(v1_4, uv1));
	};
};

class BasePixelShader : public PixelShader {

	Color PS_MAIN(ConstVertexReference interpolatedVertex) const override;
};