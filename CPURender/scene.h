#pragma once
#include "engine.h"

struct Node {
	ShaderProgram shaderProgram;
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	VertexBufferHandle mesh;
	std::vector<TextureHandle> textures;
};


class Scene {

public:
	std::vector<Node> nodes;
};

