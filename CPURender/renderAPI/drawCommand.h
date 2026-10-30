#pragma once
#include "resourceHandle.h"
#include <array>
#include "../engine.h"

class ConstantBuffer;
class PixelShader;
class VertexShader;

struct Triangle {
	std::array<glm::vec3 , 3> pos;
	std::array<glm::vec2,3> uv;
};

struct CommandBindedResources {
	void bindTexture(TextureHandle handle, size_t index) {
		if (textures.size() <= index) {textures.resize(index +1);}
		textures[index] = handle;
	}

	void bindBuffer(ConstantBufferHandle handle, size_t index) {
		if (constantBuffers.size() <= index) {constantBuffers.resize(index +1);}
		constantBuffers[index] = handle;
	}
	friend class RenderAPI;
protected:
	std::vector<TextureHandle> textures;
	std::vector<ResourceHandle<ConstantBuffer>> constantBuffers;
};

struct DrawCommand {
	ResourceHandle<renderResource::VertexBuffer> vertexBuffer;

	ResourceHandle<VertexShader> vertexShader;
	CommandBindedResources VS_Resources;
	ResourceHandle<PixelShader> pixelShader;
	CommandBindedResources PS_resources;
};

