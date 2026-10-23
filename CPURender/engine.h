#pragma once

#include "main.h"
#include "RenderAPI/renderAPI.h"
#include "RenderAPI/resourceHandle.h"

namespace renderResource {
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
	glm::ivec2 doomHeadPos = glm::ivec2(0);



	SDL_Window* window = nullptr;
};