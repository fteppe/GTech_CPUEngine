#pragma once
#include "resourceHandle.h"
#include "../engine.h"

struct Triangle {
	std::array<glm::vec3 , 3> pos;
	std::array<glm::vec2,3> uv;
};


struct DrawCommand {
	Triangle triToDraw;
	ResourceHandle<renderResource::Texture> bindedTexture;
};

