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


/* Doom fire variables and color palette */
#define FIRE_W WIDTH
#define FIRE_H HEIGHT

#define FIRE_PALETTE_SIZE 37

static uint8_t firePixels[FIRE_W * FIRE_H];

static const uint32_t fire_palette[FIRE_PALETTE_SIZE] = {
	0x070707, 0x1F0707, 0x2F0F07, 0x470F07, 0x571707, 0x671F07, 0x771F07,
	0x8F2707, 0x9F2F07, 0xAF3F07, 0xBF4707, 0xC74707, 0xDF4F07, 0xDF5707,
	0xDF5707, 0xD75F07, 0xD75F07, 0xD7670F, 0xCF6F0F, 0xCF770F, 0xCF7F0F,
	0xCF8717, 0xC78717, 0xC78F17, 0xC7971F, 0xBF9F1F, 0xBF9F1F, 0xBFA727,
	0xBFA727, 0xBFAF2F, 0xB7AF2F, 0xB7B72F, 0xB7B737, 0xCFCF6F, 0xDFDF9F,
	0xEFEFC7, 0xFFFFFF
};


struct PixelData {
	int width, height;
	std::vector<Color> imageData;
};


void resetFire() {
	/* Fire initialization */
	for (int i = 0; i < FIRE_W * FIRE_H; i++) {
		firePixels[i] = 0;
	}

	/* Seed the fire bottom row with maximum intensity (this is the fire source) */
	for (int i = 0; i < FIRE_W; i++) {
		firePixels[(FIRE_H - 1) * FIRE_W + i] = FIRE_PALETTE_SIZE - 1;
	}
}

void Engine::init(SDL_Window* windowIn) {
	printf("Init engine \n");
	time = 0.f;
	window = windowIn;
	PixelData doomFace;
	int nbrChannel;
	unsigned char *data = stbi_load("Assets/doomguy.png", &doomFace.width, &doomFace.height, &nbrChannel, 4);
	doomFace.imageData.resize(doomFace.width * doomFace.height);

	//We copy the texture data in a format we can use
	for (int i = 0; i < doomFace.width; ++i) {
		for (int j = 0; j < doomFace.height; ++j) {
			Color colorValue;
			int linearized2D = i + j * doomFace.width;
			colorValue.r = data[0 + linearized2D * nbrChannel];
			//First channel is Red, highest weight bits
			colorValue.g = data[1 + linearized2D * nbrChannel]; //Then green
			colorValue.b =data[2 + linearized2D * nbrChannel];
			doomFace.imageData[linearized2D] = colorValue;
		}
	}

	doomHead = renderAPI.createTexture(glm::ivec2(doomFace.width, doomFace.height), doomFace.imageData);
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

	frameBuffer = renderAPI.createFrameBuffer({WIDTH, HEIGHT}, FrameBufferType::Color);
	depthBuffer = renderAPI.createFrameBuffer({WIDTH, HEIGHT}, FrameBufferType::Depth);
	vs = renderAPI.createVertexShader<BaseVertexShader>();
	ps = renderAPI.createPixelShader<BasePixelShader>();
	cb = renderAPI.createConstantBuffer<GlobalConstantBuffer>();

	TerrainGenerationParams genParams;
	genParams.tileCount = {32,32};
	genParams.terrainSize = {1.f, 1.f};
	terrainData.mesh = generateTerrain(renderAPI, genParams);
	terrainData.vs = renderAPI.createVertexShader<TerrainShaderVertex>();
	terrainData.ps = renderAPI.createPixelShader<TerrainPixelShader>();
	free(data);
	resetFire();
}

void Engine::handleEvent(const SDL_Event &event) {
	SDL_SetWindowRelativeMouseMode(window, wantMouseCapture);
	float moveSpeed = 0.2f;
	glm::vec3 leftVector = glm::cross(glm::vec3(0., 1., 0.), mainCam.dir) * moveSpeed;
	glm::vec3 dirVector = mainCam.dir * moveSpeed;
	glm::vec3 upVector = glm::vec3(0., 1., 0.) * moveSpeed;
	float mouseVel = 0.01f;
	if (event.type == SDL_EVENT_KEY_DOWN) {
		if (event.key.key == SDLK_SPACE) {
			//We reset the fire on space bar down
			resetFire();
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
		float x,y;
		SDL_GetMouseState(&x, &y);
		int windowX, windowY;
		SDL_GetWindowSize(window, &windowX, &windowY);
		glm::mat4 rotation(1.); 
		rotation = glm::rotate(rotation, -event.motion.xrel * mouseVel, { 0.,1.,0. });
		rotation = glm::rotate(rotation, -event.motion.yrel * mouseVel, leftVector);
		if (wantMouseCapture) {
			mainCam.dir = rotation * glm::vec4(mainCam.dir, 1.f);
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

	/* Fire render */
	for (int y = 0; y < FIRE_H; y++) {
		for (int x = 0; x < FIRE_W; x++) {
			uint8_t idx = firePixels[y * FIRE_W + x];
			if (idx != 0)
				frameBufferOut.put_pixel(x, y, fire_palette[idx]);
		}
	}


	glm::mat4 view = glm::lookAt(mainCam.pos, mainCam.pos + mainCam.dir, {0.,1.,0.});
	glm::mat4 projection = glm::perspective<float>(glm::radians(90.f), 4./3, 0.1f, 100.f);
	glm::mat4 object(1.);

	object = rotate(object, 3.14f*.0f, {1.,0.,0.});
	GlobalConstantBuffer buffer;
	buffer.MVP = projection * view * object;
	buffer.model = object;
	renderAPI.updateBuffer(cb, buffer);

	DrawCommand command1;
	//command1.vertexBuffer = quadMesh;
	//command1.vertexShader = vs;
	//command1.pixelShader = ps;

	command1.vertexBuffer = terrainData.mesh;
	command1.vertexShader = terrainData.vs;
	command1.pixelShader = terrainData.ps;

	command1.PS_resources.textures.push_back(doomHead);
	command1.VS_Resources.constantBuffers.push_back(cb);
	renderAPI.draw(command1, context);

	renderAPI.transferToFinalFrameBuffer(frameBuffer, frameBufferOut);

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = duration_cast<std::chrono::milliseconds>(end - start);
	Logger::Log("frametime is %d ms\n", duration.count());
	Logger::endFrame();
}

void Engine::destroy() {
	printf("Killing engine \n");
}
