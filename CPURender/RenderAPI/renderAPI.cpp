#include "renderAPI.h"

#include "drawCommand.h"
#include "drawContext.h"
#include "../main.h"
#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "glm/vec2.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "RenderResources/resolvedDrawCommand.h"
#include "RenderResources/ColorTexture.h"
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
static float areaOfTriangle(glm::vec<2, TComponent> v1, glm::vec<2, TComponent> v2, glm::vec<2, TComponent> v3) {
	glm::vec<3,TComponent> A(v2-v1, 0.);
	glm::vec<3,TComponent> B(v3-v1, 0.);

	const float area = .5f * glm::length(glm::cross(A, B));
	return area;
}

template<typename TComponent>
static glm::vec3 getBarycentricCoord(glm::vec<2, TComponent> v1, glm::vec<2, TComponent> v2, glm::vec<2, TComponent> v3, glm::vec<2, TComponent> point, float areaTriangleInv) {
	float lambda1 = areaOfTriangle(v3, point, v2) * areaTriangleInv;
	float lambda2 = areaOfTriangle(v3, point, v1) * areaTriangleInv;
	return {lambda1, lambda2, 1 - (lambda1 + lambda2)};
}


RenderAPI::RenderAPI() {
}

RenderAPI::~RenderAPI() {
}

void RenderAPI::drawTextureAt(ResourceHandle<renderResource::ColorTexture> textureHandle, glm::vec2 clipPos, FinalFrameBuffer &frameBuffer) {
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

void RenderAPI::clearRenderContext(Color defaultColor, Depth defaultDepth, DrawContext &drawContext) {
	renderResource::ResolvedDrawContext resolvedDrawContext;
	resolvedDrawContext.config = drawContext.config;
	if (drawContext.color.isValid()) {
		resolvedDrawContext.color = dynamic_cast<renderResource::ColorTexture*>(this->frameBuffers[drawContext.color].get());
		resolvedDrawContext.color->fillT(defaultColor);
	}
	if (drawContext.depth.isValid()) {
		resolvedDrawContext.depth = dynamic_cast<renderResource::DepthBuffer*>(this->frameBuffers[drawContext.color].get());
		resolvedDrawContext.depth->fillT(defaultDepth);
	}
}

template<typename T>
static T interpolate(T a, T b , T c, glm::vec3 lambda) {
	return a * lambda.x + b * lambda.y + c * lambda.z;
}

static float rotation = 0.f;

void RenderAPI::draw(const DrawCommand &drawCommand, DrawContext &drawContext) {
	rotation += 0.5f;
	renderResource::ResolvedDrawCommand resolvedCommand;
	if (drawCommand.bindedTexture.isValid())
	{
		resolvedCommand.texture = this->textures[drawCommand.bindedTexture].get();
	}
	resolvedCommand.vertexBuffer = this->vertexBuffers[drawCommand.vertexBuffer].get();
	renderResource::ResolvedDrawContext resolvedDrawContext;
	resolvedDrawContext.config = drawContext.config;
	resolvedDrawContext.color = dynamic_cast<renderResource::ColorTexture*>(this->frameBuffers[drawContext.color].get());
	resolvedDrawContext.depth = dynamic_cast<renderResource::DepthBuffer*>(this->frameBuffers[drawContext.depth].get());
	draw(resolvedCommand, resolvedDrawContext);
}

ResourceHandle<renderResource::ColorTexture> RenderAPI::createTexture(glm::ivec2 dims, const std::vector<Color>& textureData) {
	assert(dims.x * dims.y == textureData.size());
	auto texturePtr = std::make_unique<renderResource::ColorTexture>(dims, textureData);
	auto handle = this->createHandle<renderResource::ColorTexture>();
	this->textures.emplace(handle, std::move(texturePtr));
	return handle;
}

TextureHandle RenderAPI::createFrameBuffer(glm::ivec2 dims, FrameBufferType type) {

	std::unique_ptr<renderResource::Texture> frameBuffer;
	auto handle = this->createHandle<renderResource::Texture>();
	switch (type) {
		case FrameBufferType::Color:
			frameBuffer = std::make_unique<renderResource::ColorTexture>(dims);
			break;
		case FrameBufferType::Depth:
			frameBuffer = std::make_unique<renderResource::DepthBuffer>(dims);
			break;
	}
	this->frameBuffers.emplace(handle, std::move(frameBuffer));
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

void RenderAPI::transferToFinalFrameBuffer(TextureHandle colorHandle, FinalFrameBuffer &frameBuffer) {

	auto texture = dynamic_cast<renderResource::ColorTexture*>(this->frameBuffers[colorHandle].get());
	// auto texture = this->textures[colorHandle].get();
	for (int i = 0; i < WIDTH; i ++) {
		for (int j = 0; j < HEIGHT; j ++) {
			frameBuffer.put_pixel(i,j, texture->getData({i , j ,0}));
		}
	}
}

void RenderAPI::draw(const renderResource::ResolvedDrawCommand &drawCommand, renderResource::ResolvedDrawContext &drawContext) {
	for (int iVert = 0; iVert < drawCommand.vertexBuffer->getVertexCount(); iVert+=3) {
		drawTriangle(drawCommand, iVert, drawContext);
	}
}

void RenderAPI::drawTriangle(const renderResource::ResolvedDrawCommand &command, size_t startIndex,
	renderResource::ResolvedDrawContext &drawContext) {


	glm::vec3 v1 = command.vertexBuffer->getVertexAttribute<glm::vec3>(startIndex, 0);
	glm::vec3 v2 = command.vertexBuffer->getVertexAttribute<glm::vec3>(startIndex + 1, 0);
	glm::vec3 v3 = command.vertexBuffer->getVertexAttribute<glm::vec3>(startIndex + 2, 0);


	glm::mat4 view = glm::lookAt(glm::vec3(0., 0., -1.), glm::vec3(0.,0.,1.), {0.,1.,0.});
	glm::mat4 projection = glm::perspective<float>(glm::radians(glm::degrees(90.f)), 4./3, 0.01f, 5.f);
	glm::mat4 object(1.);
	object = rotate(object, rotation * 0.01f, {0.,1.,0.});
	glm::mat4 mvp = projection * view * object;
	glm::vec4 v1_4 = mvp * glm::vec4(v1, 1.);
	glm::vec4 v2_4 = mvp * glm::vec4(v2, 1.);
	glm::vec4 v3_4 = mvp * glm::vec4(v3, 1.);
	v1 = glm::vec3(v1_4.x, v1_4.y, v1_4.z) / v1_4.w;
	v2 = glm::vec3(v2_4.x, v2_4.y, v2_4.z) / v2_4.w;
	v3 = glm::vec3(v3_4.x, v3_4.y, v3_4.z) / v3_4.w;
	glm::vec3 invDepths = {1.f/v1_4.z, 1.f/v2_4.z, 1.f/v3_4.z};

	glm::vec2 v1_2d = {v1.x, v1.y};
	glm::vec2 v2_2d = {v2.x, v2.y};
	glm::vec2 v3_2d = {v3.x, v3.y};

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
	float triangleAreaInv = 1 / areaOfTriangle(v1_2d, v2_2d, v3_2d);
	for (int i = iMinBox.x; i < iMaxBox.x; i++) {
		for (int j = iMinBox.y; j < iMaxBox.y; j++) {
			glm::ivec3 point(i,j,0);
			glm::vec2 floatingPoint = fromScreenToClip(glm::vec3(i,j,0), screenDims);

			if (pointInTriangle(point, iv1, iv2, iv3))
			{
				glm::vec3 baryCentricCoord = getBarycentricCoord(v1_2d, v2_2d, v3_2d, floatingPoint, triangleAreaInv);
				float gamma = baryCentricCoord.x * invDepths.x + baryCentricCoord.y * invDepths.y + baryCentricCoord.z * invDepths.z;
				float gammaInv = 1 / gamma;
				glm::vec3 depthCorrectedCoord = {
					baryCentricCoord.x * invDepths.x * gammaInv,
					baryCentricCoord.y * invDepths.y * gammaInv,
					baryCentricCoord.z * invDepths.z * gammaInv
				};
				glm::vec2 uv1 = command.vertexBuffer->getVertexAttribute<glm::vec2>(startIndex, 1);
				glm::vec2 uv2 = command.vertexBuffer->getVertexAttribute<glm::vec2>(startIndex + 1, 1);
				glm::vec2 uv3 = command.vertexBuffer->getVertexAttribute<glm::vec2>(startIndex + 2, 1);

				glm::vec2 interpolatedUV = interpolate(uv1, uv2, uv3, depthCorrectedCoord);
				Color pixelColor = colors::PINK;
				if (command.texture) {
					pixelColor = command.texture->sample(glm::vec3(interpolatedUV, 0.));
				}
				//pixelColor = Color::fromUNorm(baryCentricCoord.x, baryCentricCoord.y, baryCentricCoord.z);
				//pixelColor = Color::fromUNorm(interpolatedUV.x, interpolatedUV.y, 0.);

				drawContext.color->writeData({i, j, 0}, pixelColor);

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