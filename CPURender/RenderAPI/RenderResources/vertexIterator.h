#pragma once
#include <cassert>
#include <cstdint>
#include <tuple>


struct VertexIterator {
	VertexIterator(uint8_t* start, size_t vertexSize) : start(start), vertexSize(vertexSize) {}
	VertexIterator operator+(size_t offset) {
		return VertexIterator(start + offset*vertexSize, vertexSize);
	}
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

	template<typename ...Args>
	std::tuple<Args...> getVertexAttributes() const {
		using TReturn = std::tuple<Args...>;
		assert(sizeof(TReturn) == vertexSize);

		TReturn returnTuple;
		const uint8_t* posInVertex = start;
		std::apply([&posInVertex](auto&... arg) {
			((posInVertex = copyToDest(posInVertex, arg)), ...);
		}, returnTuple);
		return returnTuple;
	}

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

protected:
	uint8_t* start = nullptr;
	size_t vertexSize = 0;
};