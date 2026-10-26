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
	std::vector<ColorTextureHandle> textures;
	std::vector<ResourceHandle<ConstantBuffer>> constantBuffers;
};

struct DrawCommand {
	ResourceHandle<renderResource::VertexBuffer> vertexBuffer;
	ResourceHandle<renderResource::ColorTexture> bindedTexture;



	ResourceHandle<VertexShader> vertexShader;
	CommandBindedResources VS_Resources;
	ResourceHandle<PixelShader> pixelShader;
	CommandBindedResources PS_resources;
};

