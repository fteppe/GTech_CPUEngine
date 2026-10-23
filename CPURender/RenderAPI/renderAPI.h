#pragma once
#include <memory>
#include <span>
#include <glm/vec3.hpp>
#include "RenderResources/constantBuffer.h"
#include "resourceHandle.h"
#include "glm/vec2.hpp"

struct CommandBindedResources;
class ConstantBuffer;
class PixelShader;
class VertexShader;
struct Depth;
class Shader;
struct DrawContext;

namespace renderResource {
	struct ResolvedDrawContext;
	struct ResolvedDrawCommand;
	class VertexBuffer;
	class ColorTexture;
	class Texture;
}
struct Color;
class DrawCommand;
struct FinalFrameBuffer;

using ColorTextureHandle = ResourceHandle<renderResource::ColorTexture>;
using VertexBufferHandle = ResourceHandle<renderResource::VertexBuffer>;
using TextureHandle = ResourceHandle<renderResource::Texture>;
using VertexShaderHandle = ResourceHandle<VertexShader>;
using PixelShaderHandle = ResourceHandle<PixelShader>;
using ConstantBufferHandle = ResourceHandle<ConstantBuffer>;

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

	template<typename TShader>
	VertexShaderHandle createVertexShader() {
		std::unique_ptr<TShader> shader = std::make_unique<TShader>();
		auto handle = this->createHandle<VertexShader>();
		this->vertexShaders.emplace(handle, std::move(shader));
		return handle;
	}

	template<typename TShader>
	PixelShaderHandle createPixelShader() {
		std::unique_ptr<TShader> shader = std::make_unique<TShader>();
		auto handle = this->createHandle<PixelShader>();
		this->pixelShaders.emplace(handle, std::move(shader));
		return handle;

	}

	template<typename TConstantBuffer>
	ConstantBufferHandle createConstantBuffer() {
		TConstantBuffer defaultBuffer;
		auto constantBuffer = std::make_unique<ConstantBufferT<TConstantBuffer>>(defaultBuffer);
		auto handle = this->createHandle<ConstantBuffer>();
		this->constantBuffers.emplace(handle, std::move(constantBuffer));
		return handle;

	}

	template<typename TBuffer>
	void updateBuffer(ConstantBufferHandle handle, const TBuffer& data) {
		this->constantBuffers[handle]->updateBuffer(data);
	}

protected:

	size_t resourceCount = 0;
	template <typename TResource>
	ResourceHandle<TResource> createHandle();

	void draw(const renderResource::ResolvedDrawCommand& drawCommand, renderResource::ResolvedDrawContext& drawContext);
	void drawTriangle(const renderResource::ResolvedDrawCommand& command, size_t startIndex, renderResource::ResolvedDrawContext& drawContext);
	void resolveShaderResources(Shader& shaderIn, const CommandBindedResources& bindedResources);

	std::unordered_map<ColorTextureHandle, std::unique_ptr<renderResource::ColorTexture>> textures;
	std::unordered_map<VertexBufferHandle, std::unique_ptr<renderResource::VertexBuffer>> vertexBuffers;
	std::unordered_map<TextureHandle, std::unique_ptr<renderResource::Texture>> frameBuffers;
	std::unordered_map<VertexShaderHandle, std::unique_ptr<VertexShader>> vertexShaders;
	std::unordered_map<PixelShaderHandle, std::unique_ptr<PixelShader>> pixelShaders;
	std::unordered_map<ConstantBufferHandle, std::unique_ptr<ConstantBuffer>> constantBuffers;

	//This one is used for the output of vertex Shader execution
	std::unique_ptr<renderResource::VertexBuffer> recycledVertexBuffer;
};



template<typename TResource>
ResourceHandle<TResource> RenderAPI::createHandle() {
	assert(resourceCount < std::numeric_limits<size_t>::max());
	ResourceHandle<TResource> res(this->resourceCount);
	resourceCount++;
	return res;
}
