#pragma once

#include "main.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "renderAPI/renderAPI.h"
#include "renderAPI/resourceHandle.h"

class Scene;

namespace renderResource {
	class DepthBuffer;
	class ColorTexture;
}
//Students can change whetever they want in this file

union SDL_Event;
struct SDL_Window;

struct ShaderProgram {
	VertexShaderHandle VS;
	PixelShaderHandle PS;
};

struct DirectionalShadowData {
	TextureHandle depthBuffer;
	glm::vec3 dir;

	glm::mat4 view;
	glm::mat4 projection;
};

struct Engine {
public:
	Engine();
	~Engine();
	void init(SDL_Window *window);

	void handleEvent(const SDL_Event& event);

	void renderFrame(FinalFrameBuffer& frameBufferOut);

	void destroy();

	void renderShadow();

protected:

	struct Camera {
		glm::vec3 pos = glm::vec3(0.f, 1.f, -5.f);
		glm::vec3 dir = glm::vec3(0.f, 0.f, 1.f);
		float pitch = 0.f;
		float yaw = 0.f;
	};

	Camera mainCam;

	float time = 0.f;
	RenderAPI renderAPI;
	std::unique_ptr<Scene> scene;
	ResourceHandle<renderResource::Texture> doomHead;
	TextureHandle terrainTex;
	ResourceHandle<renderResource::VertexBuffer> quadMesh;
	ResourceHandle<renderResource::Texture> frameBuffer;
	ResourceHandle<renderResource::Texture> depthBuffer;

	ResourceHandle<VertexShader> vs;
	ResourceHandle<PixelShader> ps;

	struct TerrainData {
		ShaderProgram terrainProgram;
		VertexBufferHandle mesh;
	};

	TerrainData terrainData;

	ConstantBufferHandle sceneCB;
	ConstantBufferHandle objectCB;
	ShaderProgram blinnPhong;
	VertexBufferHandle cubeMesh;

	TextureHandle skybox;

	SDL_Window* window = nullptr;
	bool wantMouseCapture = true;

	DirectionalShadowData dirShadow;
};