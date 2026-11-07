#include "engine.h"
#include <stdlib.h>
#include <SDL3/SDL_events.h>
//Students can change whever they want in this file

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <chrono>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "terrainGeneration.h"
#include "baseTypes/color.h"
#include "renderAPI/drawCommand.h"
#include "renderAPI/drawContext.h"
#include "renderAPI/renderAPI.h"
#include "shaders/shader.h"
#include "shaders/terrainShader.h"
#include "logger.h"
#include "meshGenerator.h"
#include "scene.h"
#include "shaders/blinnPhongShader.h"
#include "shaders/skyShader.h"

struct PixelData {
	int width, height;
	std::vector<Color> imageData;
};

static PixelData loadTextureData(const char * filePath) {

	PixelData pixelData;
	int nbrChannel;
	constexpr int desiredChannels = 4;
	unsigned char *data = stbi_load(filePath, &pixelData.width, &pixelData.height, &nbrChannel, desiredChannels);
	pixelData.imageData.resize(pixelData.width * pixelData.height);

	//We copy the texture data in a format we can use
	for (int i = 0; i < pixelData.width; ++i) {
		for (int j = 0; j < pixelData.height; ++j) {
			Color colorValue;
			int linearized2D = i + j * pixelData.width;
			colorValue.r = data[0 + linearized2D * desiredChannels];
			//First channel is Red, highest weight bits
			colorValue.g = data[1 + linearized2D * desiredChannels]; //Then green
			colorValue.b = data[2 + linearized2D * desiredChannels];
			pixelData.imageData[linearized2D] = colorValue;
		}
	}
	free(data);
	return pixelData;
}

static TextureHandle loadTexture(const char* filePath, RenderAPI& renderAPI) {
	PixelData pixelData = loadTextureData(filePath);
	return renderAPI.createTexture(glm::ivec2(pixelData.width, pixelData.height), pixelData.imageData);
}

static TextureHandle loadCubemap(RenderAPI& renderAPI, const char* upPath, const char* downPath, const char* leftPath, const char* rightPath, const char* frontPath, const char* backPath) {
	PixelData up = loadTextureData(upPath);
	PixelData down = loadTextureData(downPath);
	PixelData left = loadTextureData(leftPath);
	PixelData right = loadTextureData(rightPath);
	PixelData front = loadTextureData(frontPath);
	PixelData back = loadTextureData(backPath);

	return renderAPI.createCubeMap({up.width, up.height}, up.imageData, down.imageData, left.imageData, right.imageData, front.imageData, back.imageData);
}

template <typename VS, typename PS>
ShaderProgram createShaderProgram(RenderAPI& renderAPI) {
	VertexShaderHandle vs = renderAPI.createVertexShader<VS>();
	PixelShaderHandle ps = renderAPI.createPixelShader<PS>();
	return {vs, ps};
}

Engine::Engine() {
}

Engine::~Engine() {
}

void Engine::init(SDL_Window* windowIn) {
	printf("Init engine \n");

	scene = std::make_unique<Scene>();
	time = 0.f;
	window = windowIn;

	doomHead = loadTexture("Assets/doomguy.png", renderAPI);
	terrainTex = loadTexture("Assets/terrain.png", renderAPI);
	quadMesh = renderAPI.createVertexBuffer(
//positions
{glm::vec3(-0.5, -0.5,0.), glm::vec3(-0.5, 0.5, 0.), glm::vec3(0.5,0.5,0.),
	glm::vec3(-0.5, -0.5,0.), glm::vec3(0.5, 0.5, 0.), glm::vec3(0.5,-0.5,0.),
		
		glm::vec3(-0.5, 0.5, 0.), glm::vec3(-0.5, -0.5,0.), glm::vec3(0.5,0.5,0.),
		glm::vec3(0.5, 0.5, 0.), glm::vec3(-0.5, -0.5,0.), glm::vec3(0.5,-0.5,0.)
		},

			//UVs
{glm::vec2(0.,0.), {0.,1.}, {1.,1},
			glm::vec2(0.,0.), {1.,1.}, {1.,0},
		
		{0.,1.}, glm::vec2(0.,0.), {1.,1},
		{1.,1.}, glm::vec2(0.,0.), {1.,0}
		});


	skybox = loadCubemap(renderAPI, "Assets/top.jpg", "Assets/bottom.jpg",
		"Assets/left.jpg", "Assets/right.jpg",
		"Assets/front.jpg", "Assets/back.jpg");

	frameBuffer = renderAPI.createFrameBuffer({WIDTH, HEIGHT}, FrameBufferType::Color);
	depthBuffer = renderAPI.createFrameBuffer({WIDTH, HEIGHT}, FrameBufferType::Depth);
	vs = renderAPI.createVertexShader<BaseVertexShader>();
	ps = renderAPI.createPixelShader<BasePixelShader>();

	TerrainGenerationParams genParams;
	genParams.tileCount = {64,64};
	genParams.terrainSize = {1.f, 0.1f, 1.f};
	terrainData.mesh = generateTerrain(renderAPI, genParams);
	terrainData.terrainProgram = createShaderProgram<TerrainShaderVertex, TerrainPixelShader>(renderAPI);

	sceneCB = renderAPI.createConstantBuffer<SceneConstantBuffer>();
	objectCB = renderAPI.createConstantBuffer<ObjectConstantBuffer>();
	blinnPhong =  createShaderProgram<BlinnPhongVS, BlinnPhongPS>(renderAPI);
	cubeMesh = MeshGenerator::generateCube(renderAPI);

	Node terrainNode;
	terrainNode.textures.push_back(terrainTex);
	terrainNode.textures.push_back(skybox);
	terrainNode.modelMatrix = glm::scale(terrainNode.modelMatrix, glm::vec3(20.f));
	terrainNode.shaderProgram = terrainData.terrainProgram;
	terrainNode.mesh = terrainData.mesh;

	Node cubeNode;
	cubeNode.shaderProgram = blinnPhong;
	cubeNode.mesh = cubeMesh;
	cubeNode.textures.push_back(doomHead);
	cubeNode.textures.push_back(skybox);

	scene->nodes.push_back(cubeNode);
	scene->nodes.push_back(terrainNode);
	cubeNode.modelMatrix = translate(cubeNode.modelMatrix, glm::vec3(1.,0.,0.));
	// scene->nodes.push_back(cubeNode);


	Node skyNode;
	skyNode.shaderProgram = createShaderProgram<SkyVS, SkyPS>(renderAPI);
	skyNode.textures.push_back(skybox);
	skyNode.mesh = MeshGenerator::generateCube(renderAPI, true);
	scene->nodes.push_back(skyNode);

}

void Engine::handleEvent(const SDL_Event &event) {
	SDL_SetWindowRelativeMouseMode(window, wantMouseCapture);
	float moveSpeed = 0.2f;
	glm::vec3 leftVector = glm::normalize(glm::cross(glm::vec3(0., 1., 0.), mainCam.dir)) * moveSpeed;
	glm::vec3 dirVector = mainCam.dir * moveSpeed;
	glm::vec3 upVector = glm::vec3(0., 1., 0.) * moveSpeed;
	float mouseVel = 0.001f;
	if (event.type == SDL_EVENT_KEY_DOWN) {
		if (event.key.key == SDLK_SPACE) {
		}
		switch (event.key.key) {
			case SDLK_S:
				mainCam.pos -= dirVector;
				break;
			case SDLK_Z:
				mainCam.pos += dirVector;
				break;
			case SDLK_Q:
				mainCam.pos += leftVector;
				break;
			case SDLK_D:
				mainCam.pos -= leftVector;
				break;
			case SDLK_P:
				Logger::LogThisFrame();
				break;
			case SDLK_A:
				mainCam.pos += upVector;
				break;
			case SDLK_E:
				mainCam.pos -= upVector;
				break;
			case SDLK_C:
				wantMouseCapture = !wantMouseCapture;
		}
	}
	if (event.type == SDL_EVENT_MOUSE_MOTION) {
		if (wantMouseCapture) {
			float x,y;
			SDL_GetMouseState(&x, &y);
			int windowX, windowY;
			SDL_GetWindowSize(window, &windowX, &windowY);
			glm::mat4 rotation(1.);
			mainCam.yaw += -event.motion.xrel * mouseVel;
			mainCam.pitch += event.motion.yrel * mouseVel;
			rotation = glm::rotate(rotation, mainCam.yaw, { 0.,1.,0. });
			mainCam.pitch = glm::clamp(mainCam.pitch,  -1.5f, 1.5f);
			rotation = glm::rotate(rotation, mainCam.pitch, glm::vec3(1.0,0.,0.));
			mainCam.dir = rotation * glm::vec4(glm::vec3(0.,0.,1.), 1.f);
		}
	}
}

void Engine::renderFrame(FinalFrameBuffer &frameBufferOut) {

	time += 0.16f;
	auto start = std::chrono::high_resolution_clock::now();

	DrawContext context;
	context.color = frameBuffer;
	context.depth= depthBuffer;
	renderAPI.clearRenderContext(Color::fromUNorm(0.2,0.2,0.2), Depth(std::numeric_limits<uint32_t>::max()), context);


	glm::mat4 view = glm::lookAt(mainCam.pos, mainCam.pos + mainCam.dir, {0.,1.,0.});
	glm::mat4 projection = glm::perspective<float>(glm::radians(90.f), 4./3, 0.1f, 1000.f);
	glm::mat4 object(1.);

	object = glm::scale(object, glm::vec3(10.,10.,10.));

	SceneConstantBuffer scene;
	float lightRotationPos = time * 0.1f;

	scene.lightDir = glm::normalize(glm::vec3(std::cos(lightRotationPos), -1.f,std::sin(lightRotationPos)));
	scene.VP = projection * view;
	scene.worldCameraPos = mainCam.pos;
	renderAPI.updateBuffer(sceneCB, scene);



	for (const auto node : this->scene->nodes) {
		DrawCommand nodeCommand;
		nodeCommand.vertexShader = node.shaderProgram.VS;
		nodeCommand.pixelShader = node.shaderProgram.PS;
		for (int iTex = 0; iTex < node.textures.size(); iTex++) {
			nodeCommand.PS_resources.bindTexture(node.textures[iTex], iTex);
		}
		nodeCommand.vertexBuffer = node.mesh;
		ObjectConstantBuffer objectBuffer;
		objectBuffer.modelMat = node.modelMatrix;
		renderAPI.updateBuffer(objectCB, objectBuffer);

		nodeCommand.VS_Resources.bindBuffer(sceneCB, 0);
		nodeCommand.VS_Resources.bindBuffer(objectCB, 1);
		nodeCommand.PS_resources.bindBuffer(sceneCB, 0);

		renderAPI.draw(nodeCommand, context);

	}


	// renderAPI.transferToFinalFrameBuffer(depthBuffer, frameBufferOut);
	renderAPI.transferToFinalFrameBuffer(frameBuffer, frameBufferOut);

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = duration_cast<std::chrono::milliseconds>(end - start);
	Logger::Log("frametime is %d ms\n", duration.count());
	Logger::endFrame();
}

void Engine::destroy() {
	printf("Killing engine \n");
}

void Engine::renderShadow() {

	DrawContext context;
	if (!dirShadow.depthBuffer.isValid()) {
		dirShadow.depthBuffer = renderAPI.createFrameBuffer({200,200}, FrameBufferType::Depth);
	}

	context.depth = dirShadow.depthBuffer;



}
