#include "renderAPI.h"

#include "../main.h"
#include "glm/common.hpp"
#include "glm/vec2.hpp"

static glm::ivec3 setToScreenSpace(glm::vec3 clipSpace, glm::ivec2 ScreenDims) {
	glm::ivec3 screenSpace;
	screenSpace.x = ScreenDims.x * clipSpace.x;
	screenSpace.y = ScreenDims.y * clipSpace.y;
	return screenSpace;
}

int sign (glm::ivec3 p1, glm::ivec3 p2, glm::ivec3 p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool pointInTriangle (glm::ivec3 pt, glm::ivec3  v1, glm::ivec3  v2, glm::ivec3  v3)
{
	int d1, d2, d3;
	bool has_neg, has_pos;

	d1 = sign(pt, v1, v2);
	d2 = sign(pt, v2, v3);
	d3 = sign(pt, v3, v1);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

void RenderAPI::drawTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, uint32_t color, FinalFrameBuffer &frameBuffer) {

	glm::vec3 minBox = glm::min(glm::min(v1, v2), v3);
	glm::vec3 maxBox = glm::max(glm::max(v1, v2), v3);

	minBox = glm::clamp(minBox, glm::vec3(0.), glm::vec3(1.));
	maxBox = glm::clamp(maxBox, glm::vec3(0.), glm::vec3(1.));

	glm::ivec2 screenDims(WIDTH, HEIGHT);

	glm::ivec3 iv1 = setToScreenSpace(v1, screenDims);
	glm::ivec3 iv2 = setToScreenSpace(v2, screenDims);
	glm::ivec3 iv3 = setToScreenSpace(v3, screenDims);
	glm::ivec3 iMinBox = setToScreenSpace(minBox, screenDims);
	glm::ivec3 iMaxBox = setToScreenSpace(maxBox, screenDims);

	for (int i = iMinBox.x; i < iMaxBox.x; i++) {
		for (int j = iMinBox.y; j < iMaxBox.y; j++) {
			glm::ivec3 point(i,j,0);

			if (pointInTriangle(point, iv1, iv2, iv3))
			{
				frameBuffer.put_pixel(i,j,color);
			}
		}
	}

}
