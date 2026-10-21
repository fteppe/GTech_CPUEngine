#include "renderAPI.h"

#include "drawCommand.h"
#include "../main.h"
#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "glm/vec2.hpp"
#include "RenderResources/resolvedDrawCommand.h"
#include "RenderResources/texture.h"
#include "RenderResources/vertexBuffer.h"

static glm::ivec3 setToScreenSpace(glm::vec3 clipSpace, glm::ivec2 screenDims) {
	glm::ivec3 screenSpace;
	screenSpace.x = screenDims.x * (clipSpace.x * .5 + 0.5);
	screenSpace.y = screenDims.y * (clipSpace.y * .5 + 0.5);
	return screenSpace;
}

static glm::vec3 fromScreenToClip(glm::ivec3 screenSpace, glm::ivec2 screenDims) {
	return glm::vec3(((float(screenSpace.x) / screenDims.x)-0.5) * 2.0,  ((float(screenSpace.y) / screenDims.y)-0.5) * 2.0, screenSpace.z);
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


RenderAPI::RenderAPI() {
}

RenderAPI::~RenderAPI() {
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
template<typename T>
static T interpolate(T a, T b , T c, glm::vec3 lambda) {
	return a * lambda.x + b * lambda.y + c * lambda.z;
}

void RenderAPI::draw(const DrawCommand &drawCommand, FinalFrameBuffer &frameBuffer) {

	renderResource::ResolvedDrawCommand resolvedCommand;
	if (drawCommand.bindedTexture.isValid())
	{
		resolvedCommand.texture = this->textures[drawCommand.bindedTexture].get();
	}
	resolvedCommand.vertexBuffer = this->vertexBuffers[drawCommand.vertexBuffer].get();

	draw(resolvedCommand, frameBuffer);


}

ResourceHandle<renderResource::Texture> RenderAPI::createTexture(glm::ivec2 dims, const std::vector<Color>& textureData) {
	assert(dims.x * dims.y == textureData.size());
	auto texturePtr = std::make_unique<renderResource::Texture>(dims, textureData);
	auto handle = this->createHandle<renderResource::Texture>();
	this->textures.emplace(handle, std::move(texturePtr));
	return handle;
}

ResourceHandle<renderResource::VertexBuffer> RenderAPI::createVertexBuffer(const std::vector<glm::vec3>& positions,
	const std::vector<glm::vec2>& texCoords)
{
	assert(positions.size() == texCoords.size());
	auto vertexBuffer = std::make_unique<renderResource::VertexBuffer>();
	auto handle = this->createHandle<renderResource::VertexBuffer>();
	vertexBuffer->addAttributeToBuffer(positions);
	vertexBuffer->addAttributeToBuffer(texCoords);
	this->vertexBuffers.emplace(handle, std::move(vertexBuffer));
	return handle;
}

void RenderAPI::draw(const renderResource::ResolvedDrawCommand &drawCommand, FinalFrameBuffer &frameBuffer) {
	for (int iVert = 0; iVert < drawCommand.vertexBuffer->getVertexCount(); iVert+=3) {
		drawTriangle(drawCommand, iVert, frameBuffer);
	}
}

void RenderAPI::drawTriangle(const renderResource::ResolvedDrawCommand &command, size_t startIndex,
	FinalFrameBuffer &frameBuffer) {


	glm::vec3 v1 = command.vertexBuffer->getVertexAttribute<glm::vec3>(startIndex, 0);
	glm::vec3 v2 = command.vertexBuffer->getVertexAttribute<glm::vec3>(startIndex + 1, 0);
	glm::vec3 v3 = command.vertexBuffer->getVertexAttribute<glm::vec3>(startIndex + 2, 0);

	glm::vec3 minBox = glm::min(glm::min(v1, v2), v3);
	glm::vec3 maxBox = glm::max(glm::max(v1, v2), v3);

	minBox = glm::clamp(minBox, glm::vec3(-1.), glm::vec3(1.));
	maxBox = glm::clamp(maxBox, glm::vec3(-1.), glm::vec3(1.));

	glm::ivec2 screenDims(WIDTH, HEIGHT);

	glm::ivec3 iv1 = setToScreenSpace(v1, screenDims);
	glm::ivec3 iv2 = setToScreenSpace(v2, screenDims);
	glm::ivec3 iv3 = setToScreenSpace(v3, screenDims);
	glm::ivec3 iMinBox = setToScreenSpace(minBox, screenDims);
	glm::ivec3 iMaxBox = setToScreenSpace(maxBox, screenDims);

	for (int i = iMinBox.x; i < iMaxBox.x; i++) {
		for (int j = iMinBox.y; j < iMaxBox.y; j++) {
			glm::ivec3 point(i,j,0);
			glm::vec3 floatingPoint = fromScreenToClip(glm::vec3(i,j,0), screenDims);

			if (pointInTriangle(point, iv1, iv2, iv3))
			{
				glm::vec3 baryCentricCoord = getBarycentricCoord(v1, v2, v3, floatingPoint);
				glm::vec2 uv1 = command.vertexBuffer->getVertexAttribute<glm::vec2>(startIndex, 1);
				glm::vec2 uv2 = command.vertexBuffer->getVertexAttribute<glm::vec2>(startIndex + 1, 1);
				glm::vec2 uv3 = command.vertexBuffer->getVertexAttribute<glm::vec2>(startIndex + 2, 1);

				glm::vec2 interpolatedUV = interpolate(uv1, uv2, uv3, baryCentricCoord);
				Color pixelColor = colors::PINK;
				if (command.texture) {
					pixelColor = command.texture->sample(glm::vec3(interpolatedUV, 0.));
				}
				frameBuffer.put_pixel(i,HEIGHT - j,pixelColor);
				//frameBuffer.put_pixel(i,HEIGHT - j,Color::fromUNorm(interpolatedUV.x, interpolatedUV.y, 0.));
				//frameBuffer.put_pixel(i,HEIGHT - j,Color::fromUNorm(baryCentricCoord.x, baryCentricCoord.y, baryCentricCoord.z));

			}
		}
	}
}

template<typename TVertex>
void RenderAPI::updateVertexBuffer(ResourceHandle<renderResource::VertexBuffer> handle,
	const std::vector<TVertex> &vertexData, size_t attributeIndex)
{
	auto& vertexBuffer = this->vertexBuffers[handle];
	vertexBuffer->updateAttribute(vertexData, attributeIndex);
}

template
void RenderAPI::updateVertexBuffer(ResourceHandle<renderResource::VertexBuffer> handle,const std::vector<glm::vec3> &vertexData, size_t attributeIndex);

template
void RenderAPI::updateVertexBuffer(ResourceHandle<renderResource::VertexBuffer> handle,const std::vector<glm::vec2> &vertexData, size_t attributeIndex);