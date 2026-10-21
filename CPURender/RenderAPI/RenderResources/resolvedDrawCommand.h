#pragma once


namespace renderResource {
	class VertexBuffer;
	class Texture;

	struct ResolvedDrawCommand {
		const VertexBuffer* vertexBuffer;
		const Texture* texture;
	};
}

