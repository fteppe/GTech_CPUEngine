#pragma once
#include <cstdint>
#include <vector>

#include "vertexIterator.h"
#include "../../baseTypes/color.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace renderResource {
	enum class VertexAttribute {
		Float,
		Vec3,
		Vec2,
	};

	class VertexBuffer {
	public:

		VertexBuffer(size_t vertexCount, const std::vector<VertexAttribute>& attributes) : vertexCount(vertexCount), attributes(attributes) {
			fullVertexSize = 0;
			for (auto attribute : attributes) {
				attributesStartCache.push_back(fullVertexSize);
				fullVertexSize += getSizeOfAttribute(attribute);
			}
			data.resize(fullVertexSize * vertexCount);
		};

		size_t getVertexCount() const {return vertexCount;}
		template<typename TVertexData>
		void updateAttribute(const std::vector<TVertexData>& dataIn, size_t attributeIndex) {
			assert(getCorrespondingVertexAttribute<TVertexData>() == attributes[attributeIndex]);
			assert(dataIn.size() == vertexCount);
			size_t interlacedOffset = attributesStartCache[attributeIndex];
			for (int iVert = 0; iVert < vertexCount; iVert++) {
				const TVertexData& vertexData = dataIn[iVert];
				uint8_t* startDataIn = (uint8_t*)&vertexData;
				uint8_t* endDataIn = (uint8_t*)startDataIn + sizeof(TVertexData);
				std::copy(startDataIn, endDataIn, data.begin() + (fullVertexSize  * iVert)+ interlacedOffset);
			}
		}

		template<typename TVertexData>
		static VertexAttribute getCorrespondingVertexAttribute() {static_assert(0 && "This attribute cannot be used in a vertex buffer");return VertexAttribute::Float; }

		ConstVertexIterator begin() const {
			return ConstVertexIterator(&this->data[0], this->fullVertexSize);
		}

		VertexIterator begin() {
			return VertexIterator(&this->data[0], this->fullVertexSize);
		}

	protected:
		size_t getSizeOfAttribute(VertexAttribute attribute) const {
			switch (attribute) {
				case VertexAttribute::Float:
					return sizeof(float);
				case VertexAttribute::Vec3:
					return sizeof(glm::vec3);
				case VertexAttribute::Vec2:
					return sizeof(glm::vec2);
			}
		}

		std::vector<uint8_t> data;
		std::vector<VertexAttribute> attributes;
		size_t vertexCount = -1;
		size_t fullVertexSize = 0;
		std::vector<size_t> attributesStartCache;
	};

	template<>
	VertexAttribute VertexBuffer::getCorrespondingVertexAttribute<glm::vec3>() {return VertexAttribute::Vec3;}
	template<>
	VertexAttribute VertexBuffer::getCorrespondingVertexAttribute<glm::vec2>() {return VertexAttribute::Vec2;}
	template<>
	VertexAttribute VertexBuffer::getCorrespondingVertexAttribute<float>() {return VertexAttribute::Float;}
}