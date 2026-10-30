#pragma once
#include "glm/ext/matrix_float4x4.hpp"

#include "../renderAPI/vertexIterator.h"
#include "../renderAPI/renderAPI.h"
#include "../baseTypes/color.h"

namespace renderResource {
	class ColorTexture;
}

using AttributesList = std::vector<renderResource::VertexAttribute>;

#define UNWRAP(...) __VA_ARGS__ 

#define DEFINE_VS_INPUT_OUTPUT(Inputs, Outputs) using VS_Inputs = std::tuple<UNWRAP Inputs>;\
	using VS_Outputs = std::tuple<UNWRAP Outputs>;\
	AttributesList VS_inputAttributes = renderResource::createAttributesVector<VS_Inputs>();\
	AttributesList VS_outputAttributes = renderResource::createAttributesVector<VS_Outputs>();\
	virtual std::pair<const AttributesList&, const AttributesList&> getInputOutputs() const override {return {VS_inputAttributes, VS_outputAttributes}; }


#define DEFINE_PS_INPUT(...) using PS_Inputs = std::tuple<__VA_ARGS__>;\
	AttributesList PS_inputAttributes = renderResource::createAttributesVector<PS_Inputs>();\
	virtual const AttributesList& getInputs() const override {return PS_inputAttributes;};

#define BIND_CB(bufferName, BufferType, bindingPoint) BufferType bufferName() const {return getConstantBuffer<BufferType>(bindingPoint);}

class Shader {
public:
	template<typename TConstantBuffer>
	const TConstantBuffer& getConstantBuffer(size_t index) const {
		return constantBuffers[index]->getData<TConstantBuffer>();
	}
	virtual ~Shader(){}
	const renderResource::Texture& getTexture(size_t index) const {
		return *textures[index];
	}

private:
	std::vector<renderResource::Texture*> textures;
	std::vector<ConstantBuffer*> constantBuffers;
	friend class RenderAPI;
};


class VertexShader : public Shader {
public:
	virtual void VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const = 0;

	virtual std::pair<const AttributesList&, const AttributesList&> getInputOutputs() const = 0;
};

class PixelShader : public Shader {
public:

	virtual const AttributesList& getInputs() const = 0;
	virtual Color PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const = 0;
};

struct SceneConstantBuffer {
	glm::vec3 worldCameraPos;
	glm::mat4 VP;
};

struct GlobalConstantBuffer {
	glm::mat4 model;
	glm::mat4 MVP;
};

class BaseVertexShader : public VertexShader {
	public:


	DEFINE_VS_INPUT_OUTPUT((glm::vec3, glm::vec2), (glm::vec4, glm::vec2));
	BIND_CB(global, GlobalConstantBuffer, 0)

	void VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const  override {
		auto [v1, uv1] = vertexIn.getVertexAttributes<VS_Inputs>();
		GlobalConstantBuffer buffer = global();

		glm::vec4 v1_4 = buffer.MVP * glm::vec4(v1, 1.);
		vertexOut.setVertexAttributes(VS_Outputs(v1_4, uv1));
	};
};

class BasePixelShader : public PixelShader {

	DEFINE_PS_INPUT(glm::vec4, glm::vec2);
	Color PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const override;
};