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

	struct Camera {
		glm::vec3 pos = glm::vec3(0.f, 0.f, -5.f);
		glm::vec3 dir = glm::vec3(0.f, 0.f, 1.f);
		float pitch = 0.f;
		float yaw = 0.f;
	};

	Camera mainCam;

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

	SDL_Window* window = nullptr;
	bool wantMouseCapture = true;
};