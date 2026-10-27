#pragma once

#include "main.h"
#include "renderAPI/renderAPI.h"
#include "renderAPI/resourceHandle.h"

namespace renderResource {
	class DepthBuffer;
	class ColorTexture;
}
//Students can change whetever they want in this file

union SDL_Event;
struct SDL_Window;

struct Engine {
public:
	Engine() = default;

	void init(SDL_Window *window);

	void handleEvent(const SDL_Event& event);

	void renderFrame(FinalFrameBuffer& frameBufferOut);

	void destroy();
protected:

	float time = 0.f;
	RenderAPI renderAPI;
	ResourceHandle<renderResource::ColorTexture> doomHead;
	ResourceHandle<renderResource::VertexBuffer> quadMesh;
	ResourceHandle<renderResource::Texture> frameBuffer;
	ResourceHandle<renderResource::Texture> depthBuffer;

	ResourceHandle<VertexShader> vs;
	ResourceHandle<PixelShader> ps;
	ResourceHandle<ConstantBuffer> cb;

	struct TerrainData {
		ResourceHandle<VertexShader> vs;
		ResourceHandle<PixelShader> ps;
		VertexBufferHandle mesh;
	};

	TerrainData terrainData;

	struct Viewpoint {
		glm::vec3 worldPos = glm::vec3(0.,0.,-1.);
		glm::vec3 eyeDir = glm::vec3(0.,0.,1.);
	};
	Viewpoint mainViewpoint;
	SDL_Window* window = nullptr;
};