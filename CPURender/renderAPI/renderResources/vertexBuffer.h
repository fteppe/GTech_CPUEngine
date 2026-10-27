#pragma once
#include <cstdint>
#include <vector>

#include "vertexAttribute.h"
#include "../vertexIterator.h"
#include "../../baseTypes/color.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace renderResource {


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
		const std::vector<VertexAttribute>& getAttributes() const {return attributes;}

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

		ConstVertexIterator begin() const {
			return ConstVertexIterator(&this->data[0], this->fullVertexSize);
		}

		VertexIterator begin() {
			return VertexIterator(&this->data[0], this->fullVertexSize);
		}

		void recycleVertexBuffer(size_t vertexCountIn, const std::vector<VertexAttribute>& attributesIn) {
			vertexCount = vertexCountIn;
			attributes = attributesIn;
			fullVertexSize = 0;
			attributesStartCache.clear();
			for (auto attribute : attributes) {
				attributesStartCache.push_back(fullVertexSize);
				fullVertexSize += getSizeOfAttribute(attribute);
			}
			data.resize(fullVertexSize * vertexCount);
		}

		void pushBack(ConstVertexReference newVertex);

	protected:

		std::vector<uint8_t> data;
		std::vector<VertexAttribute> attributes;
		size_t vertexCount = -1;
		size_t fullVertexSize = 0;
		std::vector<size_t> attributesStartCache;
	};

	inline void VertexBuffer::pushBack(ConstVertexReference newVertex) {
		assert(fullVertexSize == newVertex.vertexSize);
		data.insert(data.end(), newVertex.start, newVertex.start + newVertex.vertexSize);
	}
}
