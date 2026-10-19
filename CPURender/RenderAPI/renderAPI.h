#pragma once
#include <glm/vec3.hpp>

struct FinalFrameBuffer;

class RenderAPI {
	public:
	static void drawTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, uint32_t color, FinalFrameBuffer& frameBuffer);


protected:


};
