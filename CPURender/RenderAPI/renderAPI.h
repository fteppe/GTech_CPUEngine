#pragma once
#include <memory>
#include <span>
#include <glm/vec3.hpp>

#include "resourceHandle.h"
#include "glm/vec2.hpp"

namespace renderResource {
	struct ResolvedDrawCommand;
	class VertexBuffer;
	class Texture;
}
struct Color;
class DrawCommand;
struct FinalFrameBuffer;

class RenderAPI {
	public:
	RenderAPI();
	~RenderAPI();
	static void drawTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, uint32_t color, FinalFrameBuffer& frameBuffer);
	void drawTextureAt(ResourceHandle<renderResource::Texture> textureHandle, glm::vec2 clipPos, FinalFrameBuffer &frameBuffer);
	void draw(const DrawCommand& drawCommand, FinalFrameBuffer& frameBuffer);
	ResourceHandle<renderResource::Texture> createTexture(glm::ivec2 dims, const std::vector<Color>& textureData);
	ResourceHandle<renderResource::VertexBuffer> createVertexBuffer(const std::vector<glm::vec3>& positions, const std::vector<glm::vec2>& texCoords);
	template<typename TVertex>
	void updateVertexBuffer(ResourceHandle<renderResource::VertexBuffer> handle, const std::vector<TVertex>& vertexData, size_t attributeIndex);

protected:

	size_t resourceCount = 0;
	template <typename TResource>
	ResourceHandle<TResource> createHandle();

	void draw(const renderResource::ResolvedDrawCommand& drawCommand, FinalFrameBuffer& frameBuffer);
	void drawTriangle(const renderResource::ResolvedDrawCommand& command, size_t startIndex, FinalFrameBuffer& frameBuffer);

	std::unordered_map<ResourceHandle<renderResource::Texture>, std::unique_ptr<renderResource::Texture>> textures;
	std::unordered_map<ResourceHandle<renderResource::VertexBuffer>, std::unique_ptr<renderResource::VertexBuffer>> vertexBuffers;
};



template<typename TResource>
ResourceHandle<TResource> RenderAPI::createHandle() {
	assert(resourceCount < std::numeric_limits<size_t>::max());
	ResourceHandle<TResource> res(this->resourceCount);
	resourceCount++;
	return res;
}
