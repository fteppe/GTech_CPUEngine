#ifndef CPURENDER_DRAWCONTEXT_H
#define CPURENDER_DRAWCONTEXT_H
#include "renderAPI.h"


struct DrawContextConfig {
	bool depthTest = true;
	bool depthWrite = true;
};

struct DrawContext {

	DrawContextConfig config;
	TextureHandle color;
	TextureHandle depth;
};


#endif //CPURENDER_DRAWCONTEXT_H
