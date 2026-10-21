#pragma once
#include <memory>
#include <span>
#include <glm/vec3.hpp>

#include "resourceHandle.h"
#include "glm/vec2.hpp"
#include "RenderResources/ColorTexture.h"

struct DrawContext;

namespace renderResource {
	struct ResolvedDrawContext;
	struct ResolvedDrawCommand;
	class VertexBuffer;
	class ColorTexture;
}
struct Color;
class DrawCommand;
struct FinalFrameBuffer;

using ColorTextureHandle = ResourceHandle<renderResource::ColorTexture>;
using VertexBufferHandle = ResourceHandle<renderResource::VertexBuffer>;
using TextureHandle = ResourceHandle<renderResource::Texture>;

enum class FrameBufferType : uint8_t {
	Color,
	Depth,
};

class RenderAPI {
	public:
	RenderAPI();
	~RenderAPI();
	void drawTextureAt(ResourceHandle<renderResource::ColorTexture> textureHandle, glm::vec2 clipPos, FinalFrameBuffer &frameBuffer);
	void clearRenderContext(Color defaultColor, Depth defaultDepth, DrawContext& drawContext);
	void draw(const DrawCommand& drawCommand, DrawContext& drawContext);
	ResourceHandle<renderResource::ColorTexture> createTexture(glm::ivec2 dims, const std::vector<Color>& textureData);
	TextureHandle createFrameBuffer(glm::ivec2 dims, FrameBufferType type);
	ResourceHandle<renderResource::VertexBuffer> createVertexBuffer(const std::vector<glm::vec3>& positions, const std::vector<glm::vec2>& texCoords);
	template<typename TVertex>
	void updateVertexBuffer(ResourceHandle<renderResource::VertexBuffer> handle, const std::vector<TVertex>& vertexData, size_t attributeIndex);

	void transferToFinalFrameBuffer(TextureHandle colorHandle, FinalFrameBuffer& frameBuffer);
protected:

	size_t resourceCount = 0;
	template <typename TResource>
	ResourceHandle<TResource> createHandle();

	void draw(const renderResource::ResolvedDrawCommand& drawCommand, renderResource::ResolvedDrawContext& drawContext);
	void drawTriangle(const renderResource::ResolvedDrawCommand& command, size_t startIndex, renderResource::ResolvedDrawContext& drawContext);

	std::unordered_map<ColorTextureHandle, std::unique_ptr<renderResource::ColorTexture>> textures;
	std::unordered_map<VertexBufferHandle, std::unique_ptr<renderResource::VertexBuffer>> vertexBuffers;
	std::unordered_map<TextureHandle, std::unique_ptr<renderResource::Texture>> frameBuffers;
};



template<typename TResource>
ResourceHandle<TResource> RenderAPI::createHandle() {
	assert(resourceCount < std::numeric_limits<size_t>::max());
	ResourceHandle<TResource> res(this->resourceCount);
	resourceCount++;
	return res;
}
