#pragma once
#include <cassert>
#include <cstdint>
#include <tuple>
#include <array>
#include <cstring>

#include "renderResources/vertexAttribute.h"


namespace renderResource {
	class VertexBuffer;
}

struct VertexIterator {
	VertexIterator(uint8_t* start, size_t vertexSize) : start(start), vertexSize(vertexSize) {}
	VertexIterator operator+(size_t offset) {
		return VertexIterator(start + offset*vertexSize, vertexSize);
	}

	bool operator!=(VertexIterator it) const {return it.start != start;}

	friend struct VertexReference;
	friend struct ConstVertexIterator;

protected:
	uint8_t* start = nullptr;
	size_t vertexSize = 0;
};

struct ConstVertexIterator {
	ConstVertexIterator(const uint8_t* start, size_t vertexSize) : start(start), vertexSize(vertexSize) {}
	ConstVertexIterator(VertexIterator it) : start(it.start), vertexSize(it.vertexSize) {}

	ConstVertexIterator operator+(size_t offset) const {
		return ConstVertexIterator(start+offset*vertexSize, vertexSize);
	}
	friend struct ConstVertexReference;

protected:
	const uint8_t* start = nullptr;
	size_t vertexSize = 0;
};

struct ConstVertexReference {
	ConstVertexReference() = default;
	ConstVertexReference(ConstVertexIterator it) : start(it.start), vertexSize(it.vertexSize) {}


	template<typename TVertexAttrib>
	static const uint8_t* copyToDest(const uint8_t* origin, TVertexAttrib& dest) {
		dest = *reinterpret_cast<const TVertexAttrib*>(origin);
		return origin + sizeof(TVertexAttrib);
	}

	template<typename TTuple>
	TTuple getVertexAttributes() const {
		using TReturn = TTuple;
		assert(sizeof(TReturn) == vertexSize);

		TReturn returnTuple;
		const uint8_t* posInVertex = start;
		std::apply([&posInVertex](auto&... arg) {
			((posInVertex = copyToDest(posInVertex, arg)), ...);
		}, returnTuple);
		return returnTuple;
	}

	friend struct VertexReference;
	friend class renderResource::VertexBuffer;

protected:
	const uint8_t* start = nullptr;
	size_t vertexSize = 0;
};

struct VertexReference {
	VertexReference() = default;
	VertexReference(VertexIterator it) : start(it.start), vertexSize(it.vertexSize) {}

	template<typename TVertexAttrib>
	static uint8_t* copyToOrigin(uint8_t* origin, const TVertexAttrib& dest) {
		*reinterpret_cast<TVertexAttrib*>(origin) = dest;
		return origin + sizeof(TVertexAttrib);
	}

	template<typename ...Args>
	void setVertexAttributes(const std::tuple<Args...>& tupleIn) {
		using TReturn = std::tuple<Args...>;
		assert(sizeof(TReturn) == vertexSize);
		uint8_t* posInVertex = start;
		std::apply([&posInVertex](const auto&... arg) {
			((posInVertex = copyToOrigin(posInVertex, arg)), ...);
		}, tupleIn);
	}

	VertexReference& operator=(ConstVertexReference right) {
		assert(vertexSize == right.vertexSize);
		memcpy(start, right.start, right.vertexSize);
		return *this;
	}

	//The first one should always be vec4 !
	glm::vec4& getFirst() { return *reinterpret_cast<glm::vec4*>(start);}

	friend struct InPlaceVertex;

	 operator ConstVertexReference() const {
		 ConstVertexReference ref;
	 	ref.vertexSize = vertexSize;
	 	ref.start = start;
	 	return ref;
	 }

	void setFromInterpolation(ConstVertexReference v1,ConstVertexReference v2, ConstVertexReference v3, glm::vec3 lambda) {
		assert(vertexSize == v1.vertexSize && vertexSize == v2.vertexSize && vertexSize == v3.vertexSize);
		assert(vertexSize % sizeof(float) == 0);
		//Because all vertex attributes are float, we can interpolate them this way
		//doesn't matter if it is 2 floats or a glmvec2
	 	const float* startV1 = reinterpret_cast<const float*>(v1.start);
	 	const float* startV2 = reinterpret_cast<const float*>(v2.start);
	 	const float* startV3 = reinterpret_cast<const float*>(v3.start);
		for (size_t i = 0 ; i < vertexSize/sizeof(float) ; ++i) {
			reinterpret_cast<float*>(start)[i] = startV1[i] * lambda.x +
				startV2[i] * lambda.y +
				startV3[i] * lambda.z;
		}
	}

	void setFrom1DInterpolation(ConstVertexReference v1,ConstVertexReference v2, float alpha) {
	 	assert(vertexSize == v1.vertexSize && vertexSize == v2.vertexSize);
	 	assert(vertexSize % sizeof(float) == 0);
	 	const float* startV1 = reinterpret_cast<const float*>(v1.start);
	 	const float* startV2 = reinterpret_cast<const float*>(v2.start);
	 	//Because all vertex attributes are float, we can interpolate them this way
	 	//doesn't matter if it is 2 floats or a glmvec2
	 	for (size_t i = 0 ; i < vertexSize/sizeof(float) ; ++i) {
	 		reinterpret_cast<float*>(start)[i] = startV1[i] * alpha +
	 			startV2[i] * (1-alpha);
	 	}
	 }

protected:
	uint8_t* start = nullptr;
	size_t vertexSize = 0;
};


//A structure to have vertices attributes, without allocating memory
struct InPlaceVertex {
	InPlaceVertex() = default;

	InPlaceVertex(const::std::vector<renderResource::VertexAttribute>& vertexAttributes ) {
		for (auto attribute : vertexAttributes) {
			usedSize += getSizeOfAttribute(attribute);
		}
		assert(usedSize < data.size());
	}

	VertexReference getVertexReference() {
		VertexReference ref;
		ref.vertexSize = usedSize;
		ref.start = &data[0];
		return ref;
	}

	std::size_t usedSize = 0;
	std::array<uint8_t, 128> data;
};