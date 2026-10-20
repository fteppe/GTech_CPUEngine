#pragma once
#include <vector>

#include "../../baseTypes/color.h"
#include "glm/common.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace renderResource {
	class Texture {
	public:
		glm::ivec3 size;
		std::vector<Color> data;
		Texture(glm::ivec2 sizeIn, const std::vector<Color>& dataIn) : size(sizeIn, 1) {
			assert(size.x * size.y * size.z == dataIn.size());
			this->data = dataIn;
		}

		Color sample(glm::vec3 uv) {
			uv = glm::clamp(uv, 0.0f, 1.0f);
			return getData(glm::ivec3(uv.x * (size.x-1) , uv.y * (size.y - 1), uv.z * (size.z-1)));
		}

		Color getData(glm::ivec3 coord) {
			assert(coord.x < size.x && coord.y < size.y && coord.z < size.z);
			return data[(coord.x + (size.x * coord.y) + coord.z * size.x * size.y)];
		}
	};
}



