#include "renderAPI/renderAPI.h"
class RenderAPI;

class MeshGenerator {
public:
	static VertexBufferHandle generateCube(RenderAPI& renderAPI, bool inverted = false);

	static VertexBufferHandle generateQuad(RenderAPI& renderAPI, glm::vec2 size);

};

