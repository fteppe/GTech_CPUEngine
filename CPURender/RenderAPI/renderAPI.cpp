#include "renderAPI.h"

#include "../main.h"
#include "glm/common.hpp"
#include "glm/geometric.hpp"
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

template<typename TComponent>
static float areaOfTriangle(glm::vec<3, TComponent> v1, glm::vec<3, TComponent> v2, glm::vec<3, TComponent> v3) {
	glm::vec<3,TComponent> A = v2-v1;
	glm::vec<3,TComponent> B = v3-v1;

	const float area = .5f * glm::length(glm::cross(A, B));
	return area;
}

template<typename TComponent>
static glm::vec3 getBarycentricCoord(glm::vec<3, TComponent> v1, glm::vec<3, TComponent> v2, glm::vec<3, TComponent> v3, glm::vec<3, TComponent> point) {
	float totalTriangleArea = areaOfTriangle(v1, v2, v3);
	float lambda1 = areaOfTriangle(v3, point, v2) / totalTriangleArea;
	float lambda2 = areaOfTriangle(v3, point, v1) / totalTriangleArea;
	return {lambda1, lambda2, 1 - (lambda1 + lambda2)};
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
			glm::vec3 floatingPoint(float(i)/WIDTH, float(j)/HEIGHT, 0);

			if (pointInTriangle(point, iv1, iv2, iv3))
			{
				glm::vec3 baryCentricCoord = getBarycentricCoord(v1, v2, v3, floatingPoint);
				frameBuffer.put_pixel(i,j,Color::fromUNorm(baryCentricCoord.x, baryCentricCoord.y, baryCentricCoord.z));
			}
		}
	}
}

void RenderAPI::drawTextureAt(ResourceHandle<renderResource::Texture> textureHandle, glm::vec2 clipPos, FinalFrameBuffer &frameBuffer) {
	glm::ivec2 screenDims(WIDTH, HEIGHT);

	glm::ivec3 screenSpaceStart = setToScreenSpace(glm::vec3(clipPos,1.), screenDims);

	auto& texturePtr = this->textures[textureHandle];

	for (int x = 0; x < texturePtr->size.x; x++) {
		for (int y = 0; y < texturePtr->size.y; y++) {
			int posX = x + screenSpaceStart.x;
			int posY = y + screenSpaceStart.y;
			frameBuffer.put_pixel(posX, posY, texturePtr->getData(glm::ivec3(x,y,0)));
		}
	}
}

ResourceHandle<renderResource::Texture> RenderAPI::createTexture(glm::ivec2 dims, const std::vector<Color>& textureData) {
	assert(dims.x * dims.y == textureData.size());
	auto texturePtr = std::make_unique<renderResource::Texture>(dims, textureData);
	auto handle = this->createHandle<renderResource::Texture>();
	this->textures.emplace(handle, std::move(texturePtr));
	return handle;
}
