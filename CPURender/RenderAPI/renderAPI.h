#pragma once
#include <memory>
#include <span>
#include <glm/vec3.hpp>

#include "resourceHandle.h"
#include "glm/vec2.hpp"
#include "RenderResources/texture.h"

struct FinalFrameBuffer;

class RenderAPI {
	public:
	static void drawTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, uint32_t color, FinalFrameBuffer& frameBuffer);
	void drawTextureAt(ResourceHandle<renderResource::Texture> textureHandle, glm::vec2 clipPos, FinalFrameBuffer &frameBuffer);

	ResourceHandle<renderResource::Texture> createTexture(glm::ivec2 dims, const std::vector<Color>& textureData);

protected:

	size_t resourceCount = 0;
	template <typename TResource>
	ResourceHandle<TResource> createHandle();

	std::unordered_map<ResourceHandle<renderResource::Texture>, std::unique_ptr<renderResource::Texture>> textures;
};

template<typename TResource>
ResourceHandle<TResource> RenderAPI::createHandle() {
	assert(resourceCount < std::numeric_limits<size_t>::max());
	ResourceHandle<TResource> res(this->resourceCount);
	resourceCount++;
	return res;
}
