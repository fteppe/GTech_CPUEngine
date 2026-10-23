#include "renderAPI.h"

#include "drawCommand.h"
#include "drawContext.h"
#include "../main.h"
#include "../shaders/shader.h"
#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "glm/vec2.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_projection.hpp"
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

static glm::vec2 fromScreenToClip(glm::ivec2 screenSpace, glm::ivec2 screenDims) {
	return glm::vec2(((float(screenSpace.x) / screenDims.x)-0.5) * 2.0,  ((float(screenSpace.y) / screenDims.y)-0.5) * 2.0);
}

int sign (glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

float det2d(glm::vec2 v1, glm::vec2 v2) {
	return v1.x * v2.y - v2.x * v1.y;
}

bool pointInTriangle (glm::vec2 pt, glm::vec2  v1, glm::vec2  v2, glm::vec2  v3)
{
	float d1, d2, d3;
	bool has_neg, has_pos;

	d1 = det2d(v2 - v1, pt - v1);
	d2 = det2d(v3 - v2, pt - v2);
	d3 = det2d(v1 - v3, pt - v3);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !has_neg;
}

static bool attributeListsEqual(const std::vector<renderResource::VertexAttribute>& v1, const std::vector<renderResource::VertexAttribute>& v2) {
	if (v1.size() == v2.size()) {
		for (int i = 0 ; i < v1.size(); i++) {
			if (v1[i] != v2[i]) {
				return false;
			}
		}
		return true;
	}
	else
		return false;
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
	recycledVertexBuffer = std::make_unique<renderResource::VertexBuffer>(0, std::vector<renderResource::VertexAttribute>());
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
		resolvedDrawContext.depth = dynamic_cast<renderResource::DepthBuffer*>(this->frameBuffers[drawContext.depth].get());
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
	auto vs = vertexShaders[drawCommand.vertexShader].get();
	auto ps = pixelShaders[drawCommand.pixelShader].get();
	this->resolveShaderResources(*vs, drawCommand.VS_Resources);
	this->resolveShaderResources(*ps,  drawCommand.PS_resources);
	resolvedCommand.vertexShader = vs;
	resolvedCommand.pixelShader = ps;


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
	std::vector<renderResource::VertexAttribute> vertexAttributes = {renderResource::VertexAttribute::Vec3, renderResource::VertexAttribute::Vec2};
	auto vertexBuffer = std::make_unique<renderResource::VertexBuffer>(positions.size(), vertexAttributes);
	auto handle = this->createHandle<renderResource::VertexBuffer>();
	vertexBuffer->updateAttribute(positions, 0);
	vertexBuffer->updateAttribute(texCoords, 1);
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

	auto vsInOut = drawCommand.vertexShader->getInputOutputs();
	auto [vsIn, vsOut] = vsInOut;
	auto psIn = drawCommand.pixelShader->getInputs();
	assert(attributeListsEqual(vsIn, drawCommand.vertexBuffer->getAttributes()));
	assert(attributeListsEqual(vsOut, psIn));
	recycledVertexBuffer->recycleVertexBuffer(drawCommand.vertexBuffer->getVertexCount(), vsOut);
	ConstVertexIterator verticesInBegin = drawCommand.vertexBuffer->begin();
	VertexIterator verticesOutBegin = recycledVertexBuffer->begin();

	for (int iVert = 0; iVert < drawCommand.vertexBuffer->getVertexCount(); ++iVert) {
		ConstVertexReference vertex1 = verticesInBegin + iVert;
		VertexReference vertexOut = verticesOutBegin + iVert;
		drawCommand.vertexShader->VS_MAIN(iVert, vertex1, vertexOut);
	}

	InPlaceVertex InPlaceVertex(vsOut);
	VertexReference interpolatedVertexRef = InPlaceVertex.getVertexReference();
	drawContext.interpolatedVertexRef = interpolatedVertexRef;
	//We should have a vec4 as output of the vertex shader !
	assert(vsOut[0] == renderResource::VertexAttribute::Vec4);
	for (int iVert = 0; iVert < drawCommand.vertexBuffer->getVertexCount(); iVert+=3) {
		drawTriangle(drawCommand, iVert, drawContext);
	}
}

static bool earlyDiscardTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
	if (v1.x < -1. && v2.x < -1. && v3.x < -1.) {
		return true;
	}
	if (v1.y < -1. && v2.y < -1. && v3.y < -1.) {
		return true;
	}
	if (v1.z < -1. && v2.z < -1. && v3.z < -1.) {
		return true;
	}
	
	if (v1.x > 1. && v2.x > 1. && v3.x > 1.) {
		return true;
	}
	if (v1.y > 1. && v2.y > 1. && v3.y > 1.) {
		return true;
	}
	if (v1.z > 1. && v2.z > 1. && v3.z > 1.) {
		return true;
	}
	return false;
}

void RenderAPI::drawTriangle(const renderResource::ResolvedDrawCommand &command, size_t startIndex,
	renderResource::ResolvedDrawContext &drawContext) {

	VertexIterator VerticesBegin = recycledVertexBuffer->begin();
	VertexReference vertex1 = VerticesBegin + startIndex;
	VertexReference vertex2 = VerticesBegin + startIndex + 1;
	VertexReference vertex3 = VerticesBegin + startIndex + 2;

	glm::vec4& v1 = vertex1.getFirst();
	glm::vec4& v2 = vertex2.getFirst();
	glm::vec4& v3 = vertex3.getFirst();

	glm::vec3 invDepths = {1.f/v1.z, 1.f/v2.z, 1.f/v3.z};
	v1 = v1 * (1.f/ v1.w);
	v2 = v2 * (1.f/ v2.w);
	v3 = v3 * (1.f/ v3.w);

	if (earlyDiscardTriangle(v1, v2, v3)) //out of the clipspace
		return;
	if (det2d(v2 - v1, v3 - v2) <= 0.) { //backface culling
		return;
	}

	glm::vec2 v1_2d = {v1.x, v1.y};
	glm::vec2 v2_2d = {v2.x, v2.y};
	glm::vec2 v3_2d = {v3.x, v3.y};

	glm::vec3 minBox = glm::min(glm::min(v1, v2), v3);
	glm::vec3 maxBox = glm::max(glm::max(v1, v2), v3);

	minBox = glm::clamp(minBox, glm::vec3(-1.), glm::vec3(1.));
	maxBox = glm::clamp(maxBox, glm::vec3(-1.), glm::vec3(1.));

	glm::ivec2 screenDims(WIDTH, HEIGHT);

	glm::ivec3 iMinBox = setToScreenSpace(minBox, screenDims);
	glm::ivec3 iMaxBox = setToScreenSpace(maxBox, screenDims);

	float triangleAreaInv = 1 / areaOfTriangle(v1_2d, v2_2d, v3_2d);
	for (int i = iMinBox.x; i < iMaxBox.x; i++) {
		for (int j = iMinBox.y; j < iMaxBox.y; j++) {
			glm::ivec3 point(i,j,0);
			glm::vec2 floatingPoint = fromScreenToClip({i,j}, screenDims);

			if (pointInTriangle(floatingPoint, v1, v2, v3))
			{
				glm::vec3 baryCentricCoord = getBarycentricCoord(v1_2d, v2_2d, v3_2d, floatingPoint, triangleAreaInv);
				float gamma = baryCentricCoord.x * invDepths.x + baryCentricCoord.y * invDepths.y + baryCentricCoord.z * invDepths.z;
				float gammaInv = 1 / gamma;
				glm::vec3 depthCorrectedCoord = {
					baryCentricCoord.x * invDepths.x * gammaInv,
					baryCentricCoord.y * invDepths.y * gammaInv,
					baryCentricCoord.z * invDepths.z * gammaInv
				};
				drawContext.interpolatedVertexRef.setFromInterpolation(vertex1, vertex2, vertex3, depthCorrectedCoord);
				float depth = drawContext.interpolatedVertexRef.getFirst().z;

				if (depth > 0. && depth < 1.)
				{
					Depth currentDepth = drawContext.depth->getData({i , j, 0});
					Depth newPixelDepth = Depth::from01(depth);
					if (newPixelDepth < currentDepth)
					{
						Color pixelColor = command.pixelShader->PS_MAIN(point, drawContext.interpolatedVertexRef);
						drawContext.color->writeData({i, j, 0}, pixelColor);
						drawContext.depth->writeData({i, j, 0}, newPixelDepth);
					}
				}



			}
		}
	}
}

void RenderAPI::resolveShaderResources(Shader& shaderIn, const CommandBindedResources& bindedResources) {

	shaderIn.colorTextures.reserve(bindedResources.textures.size() + 1);
	shaderIn.colorTextures.clear();
	for (auto handle : bindedResources.textures) {
		renderResource::ColorTexture* tex = nullptr;
		if (handle.isValid()) {
			tex = this->textures[handle].get();
		}
		shaderIn.colorTextures.push_back(tex);
	}
	shaderIn.constantBuffers.reserve(bindedResources.textures.size() + 1);
	shaderIn.constantBuffers.clear();
	for (auto handle : bindedResources.constantBuffers) {
		ConstantBuffer* buffer = nullptr;
		if (handle.isValid()) {
			buffer = this->constantBuffers[handle].get();
		}
		shaderIn.constantBuffers.push_back(buffer);
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