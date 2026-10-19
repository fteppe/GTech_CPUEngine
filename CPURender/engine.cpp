#include "engine.h"
#include <stdlib.h>
#include <SDL3/SDL_events.h>
//Students can change whever they want in this file

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <glm/mat4x4.hpp>
#include "RenderAPI/renderAPI.h"


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
	std::vector<uint32_t> imageData;
};

static PixelData doomFace;

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

void Engine::init() {
	printf("Init engine \n");
	static int nbrChannel;
	unsigned char *data = stbi_load("Assets/doomguy.png", &doomFace.width, &doomFace.height, &nbrChannel, 4);

	doomFace.imageData.resize(doomFace.width * doomFace.height);

	//We copy the texture data in a format we can use
	for (int i = 0; i < doomFace.width; ++i) {
		for (int j = 0; j < doomFace.height; ++j) {
			uint32_t colorValue = 0;
			int linearized2D = i + j * doomFace.width;
			colorValue = colorValue | (uint32_t(data[0 + linearized2D * nbrChannel]) << 16);
			//First channel is Red, highest weight bits
			colorValue = colorValue | (uint32_t(data[1 + linearized2D * nbrChannel]) << 8); //Then green
			colorValue = colorValue | (uint32_t(data[2 + linearized2D * nbrChannel]) << 0); //Finally blue
			doomFace.imageData[linearized2D] = colorValue;
		}
	}
	free(data);
	resetFire();
}

void Engine::handleEvent(const SDL_Event &event) {
	if (event.type == SDL_EVENT_KEY_DOWN) {
		if (event.key.key == SDLK_SPACE) {
			//We reset the fire on space bar down
			resetFire();
		}
	}
}

void Engine::renderFrame(FinalFrameBuffer &frameBufferOut) {
	frameBufferOut.clear(0x000000);

	/* Fire update */
	for (uint32_t x = 0; x < FIRE_W; x++) {
		for (uint32_t y = 1; y < FIRE_H; y++) {
			uint32_t src = FIRE_W * y + x;
			uint32_t rand_idx = rand() % 4;
			int32_t dst = (int32_t) src - FIRE_W;
			int32_t dst_x = (int32_t) (src % FIRE_W) - (int32_t) rand_idx + 1;
			if (dst < 0 || dst_x < 0 || dst_x >= FIRE_W) {
				continue;
			} else {
				dst = dst - (int32_t) (src % FIRE_W) + dst_x;
				uint8_t src_val = firePixels[src];
				uint8_t decay = rand_idx & 1;
				firePixels[dst] = (src_val > decay) ? (src_val - decay) : 0;
			}
		}
	}

	//draws the doomguy face texture at the center of the screen
	for (int x = 0; x < doomFace.width; x++) {
		for (int y = 0; y < doomFace.height; y++) {
			int posX = (WIDTH / 2 - doomFace.width / 2) + x;
			int posY = (HEIGHT / 2 - doomFace.height / 2) + y;
			frameBufferOut.put_pixel(posX, posY, doomFace.imageData[x + y * doomFace.width]);
		}
	}
	/* Fire render */
	for (int y = 0; y < FIRE_H; y++) {
		for (int x = 0; x < FIRE_W; x++) {
			uint8_t idx = firePixels[y * FIRE_W + x];
			if (idx != 0)
				frameBufferOut.put_pixel(x, y, fire_palette[idx]);
		}
	}

	RenderAPI::drawTriangle(glm::vec3(0., 0.,0.), glm::vec3(0.5, 0.5, 0.), glm::vec3(0.,1.,0.), frameBufferOut);

}

void Engine::destroy() {
	doomFace.imageData = std::vector<uint32_t>();
	printf("Killing engine \n");
}
