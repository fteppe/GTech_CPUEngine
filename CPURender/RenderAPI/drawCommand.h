#pragma once
#include "resourceHandle.h"
#include "../engine.h"

class PixelShader;
class VertexShader;

struct Triangle {
	std::array<glm::vec3 , 3> pos;
	std::array<glm::vec2,3> uv;
};


struct DrawCommand {
	ResourceHandle<renderResource::VertexBuffer> vertexBuffer;
	ResourceHandle<renderResource::ColorTexture> bindedTexture;

	VertexShader* vertexShader;
	PixelShader* pixelShader;
};

