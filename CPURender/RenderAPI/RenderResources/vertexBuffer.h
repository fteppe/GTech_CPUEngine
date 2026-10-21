#pragma once
#include <cstdint>
#include <vector>

#include "../../baseTypes/color.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace renderResource {
	enum class VertexAttribute {
		Vec3,
		Vec2,
		Int,
		Color,
	};


	class VertexBuffer {
	public:
		size_t getVertexCount() const {return vertexCount;}
		template<typename TVertexData>
		void addAttributeToBuffer(const std::vector<TVertexData>& data);
		template<typename TVertexData>
		void updateAttribute(const std::vector<TVertexData>& dataIn, size_t attributeIndex) {
			assert(getCorrespondingVertexAttribute<TVertexData>() == attributes[attributeIndex]);
			assert(dataIn.size() == vertexCount);
			uint8_t* startDataIn = (uint8_t*)&dataIn[0];
			uint8_t* endDataIn = (uint8_t*)startDataIn + dataIn.size() * sizeof(TVertexData);

			std::copy(startDataIn, endDataIn, data.begin() + attributesStartCache[attributeIndex]);
		}
		template<typename TVertexData>
		static VertexAttribute getCorrespondingVertexAttribute() {static_assert(0 && "This attribute cannot be used in a vertex buffer");return VertexAttribute::Int; }

		template<typename TVertexData>
TVertexData getVertexAttribute(size_t vertexIndex, size_t attributeIndex) const {
			assert(getCorrespondingVertexAttribute<TVertexData>() == attributes[attributeIndex]);
			return *reinterpret_cast<const TVertexData*>((&data[0] + attributesStartCache[attributeIndex] + sizeof(TVertexData) * vertexIndex));
		}
	protected:
		size_t getSizeOfAttribute(VertexAttribute attribute) const {
			switch (attribute) {
				case VertexAttribute::Vec3:
					return sizeof(glm::vec3);
				case VertexAttribute::Vec2:
					return sizeof(glm::vec2);
				case VertexAttribute::Int:
					return sizeof(int);
				case VertexAttribute::Color:
					return sizeof(Color);
			}
		}



		std::vector<uint8_t> data;
		std::vector<VertexAttribute> attributes;
		size_t vertexCount = -1;
		std::vector<size_t> attributesStartCache;
	};

	template<>
	VertexAttribute VertexBuffer::getCorrespondingVertexAttribute<glm::vec3>() {return VertexAttribute::Vec3;}
	template<>
	VertexAttribute VertexBuffer::getCorrespondingVertexAttribute<glm::vec2>() {return VertexAttribute::Vec2;}
	template<>
	VertexAttribute VertexBuffer::getCorrespondingVertexAttribute<int>() {return VertexAttribute::Int;}
	template<>
	VertexAttribute VertexBuffer::getCorrespondingVertexAttribute<Color>() {return VertexAttribute::Color;}

	template<typename TVertexData>
	void VertexBuffer::addAttributeToBuffer(const std::vector<TVertexData>& dataIn) {
		if (vertexCount == -1) {
			assert(attributes.size() == 0);
		}
		else {
			assert(dataIn.size() == vertexCount);
		}
		vertexCount = dataIn.size();
		VertexAttribute attribute = getCorrespondingVertexAttribute<TVertexData>();
		attributes.push_back(attribute);
		attributesStartCache.push_back(data.size());
		uint8_t* startDataIn = (uint8_t*)&dataIn[0];
		uint8_t* endDataIn = (uint8_t*)startDataIn + dataIn.size() * sizeof(TVertexData);
		data.insert(data.end(), startDataIn, endDataIn);
	}
}