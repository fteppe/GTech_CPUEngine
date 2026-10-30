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
		std::vector<uint8_t> data;
		Texture(glm::ivec2 sizeIn, size_t pixelSize) : size(sizeIn,1) {data.resize(size.x * size.y * size.z * pixelSize);}

		Texture(glm::ivec2 sizeIn, const uint8_t* dataIn, size_t pixelCount, size_t sizePixel) : size(sizeIn, 1) {
			assert(size.x * size.y * size.z == pixelCount);
			this->data.clear();
			this->data.insert(this->data.begin(), dataIn, dataIn + (pixelCount * sizePixel));
		}

		template<typename TPixelType>
		Texture(glm::ivec2 sizeIn, TPixelType defaultVal) : Texture(sizeIn, sizeof(TPixelType)) {}

		template<typename TPixelType>
		Texture(glm::ivec2 sizeIn, const std::vector<TPixelType>& dataIn) : Texture(sizeIn, (const uint8_t*)&dataIn[0], dataIn.size(), sizeof(TPixelType)) {}

		virtual ~Texture() {}

		virtual glm::vec4 sample(glm::vec3 uv) const = 0;

		template<typename TPixelType>
		void fillT(TPixelType defaultVal) {
			TPixelType* begin = (TPixelType*)&data[0];
			TPixelType* end = (TPixelType*)(&data[0] + data.size());
			std::fill(begin, end, defaultVal);
		}

	protected:
		template<typename TPixelType>
		TPixelType sampleT(glm::vec3 uv) const{
			uv = glm::clamp(uv, 0.0f, 1.0f);
			return getDataT<TPixelType>(glm::ivec3 (std::nearbyint(uv.x * (size.x-1)) ,
				std::nearbyint(uv.y * (size.y - 1)), std::nearbyint(uv.z * (size.z-1))));
		}

		size_t getLinearizeCoord(glm::ivec3 coord) const {
			assert(coord.x < size.x && coord.y < size.y && coord.z < size.z);
			return (coord.x + (size.x * coord.y) + coord.z * size.x * size.y);
		}

		template<typename TPixelType>
		TPixelType getDataT(glm::ivec3 coord) const{
			return *(const TPixelType*)&data[getLinearizeCoord(coord) * sizeof(TPixelType)];
		}

		template<typename TPixelType>
		void writeDataT(glm::ivec3 coord, const TPixelType& pixelData) {
			TPixelType& pixelMemory = *(TPixelType*)(&data[getLinearizeCoord(coord) * sizeof(TPixelType)]);
			pixelMemory = pixelData;
		}
	};


	template<typename TPixelType>
	class TextureT : public Texture {
	public:
		TextureT(glm::ivec2 sizeIn) : Texture(sizeIn, TPixelType()) {}
		TextureT(glm::ivec2 sizeIn, const std::vector<TPixelType>& dataIn) : Texture(sizeIn, dataIn) {}
		void writeData(glm::ivec3 pos, TPixelType data) {writeDataT(pos, data);}
		TPixelType getData(glm::ivec3 pos) const {return getDataT<TPixelType>(pos);}
		TPixelType samplePixel(glm::vec3 uv) const {return sampleT<TPixelType>(uv);}
	};

	class ColorTexture : public TextureT<Color> {
		public:
		using TextureT<Color>::TextureT;
		glm::vec4 sample(glm::vec3 uv) const override {return samplePixel(uv);}
	};

	class DepthBuffer : public TextureT<Depth> {
		using TextureT<Depth>::TextureT;

		glm::vec4 sample(glm::vec3 uv) const override {return glm::vec4(samplePixel(uv).to01(), 0.,0.,0.);}
	};
}



