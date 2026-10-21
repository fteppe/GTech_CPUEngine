#pragma once
#include <vector>

#include "../../baseTypes/color.h"
#include "glm/common.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace renderResource {
	template<typename TPixelType>
	class Texture3D {
	public:
		glm::ivec3 size;
		std::vector<TPixelType> data;
		Texture3D(glm::ivec2 sizeIn, const std::vector<TPixelType>& dataIn) : size(sizeIn, 1) {
			assert(size.x * size.y * size.z == dataIn.size());
			this->data = dataIn;
		}

		TPixelType sample(glm::vec3 uv) const{
			uv = glm::clamp(uv, 0.0f, 1.0f);
			return getData(glm::ivec3(uv.x * (size.x-1) , uv.y * (size.y - 1), uv.z * (size.z-1)));
		}

		TPixelType getData(glm::ivec3 coord) const{
			assert(coord.x < size.x && coord.y < size.y && coord.z < size.z);
			return data[(coord.x + (size.x * coord.y) + coord.z * size.x * size.y)];
		}
	};

	class Texture : public Texture3D<Color> {
		public:
		Texture(glm::ivec2 sizeIn, const std::vector<Color>& dataIn) : Texture3D<Color>(sizeIn, dataIn) {}
	};
}



