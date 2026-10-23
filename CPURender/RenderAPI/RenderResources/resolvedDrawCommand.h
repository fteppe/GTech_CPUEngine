#pragma once
#include "vertexIterator.h"


namespace renderResource {
	class DepthBuffer;
	class VertexBuffer;
	class ColorTexture;

	struct ResolvedDrawCommand {
		const VertexBuffer* vertexBuffer;
		const ColorTexture* texture;
	};

	struct ResolvedDrawContext {
		DrawContextConfig config;
		ColorTexture* color;
		DepthBuffer* depth;

		//a cache used for interpolation
		mutable VertexReference interpolatedVertexRef;
	};
}

