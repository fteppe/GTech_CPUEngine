#pragma once
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

template<class> 
inline constexpr bool dependent_false_v = false;

namespace renderResource {
	enum class VertexAttribute {
		Float,
		Vec3,
		Vec2,
		Vec4,
		Undefined,
	};

	inline size_t getSizeOfAttribute(VertexAttribute attribute) {
		switch (attribute) {
			case VertexAttribute::Float:
				return sizeof(float);
			case VertexAttribute::Vec3:
				return sizeof(glm::vec3);
			case VertexAttribute::Vec2:
				return sizeof(glm::vec2);
			case VertexAttribute::Vec4:
				return sizeof(glm::vec4);
		}
		return 0;
	}
	template<typename TVertexData>
	static VertexAttribute getCorrespondingVertexAttribute() {static_assert(dependent_false_v<TVertexData>, "the default implementation should not be called"); return VertexAttribute::Undefined; }
	template<>
	VertexAttribute getCorrespondingVertexAttribute<glm::vec4>() {return VertexAttribute::Vec4;}
	template<>
	VertexAttribute getCorrespondingVertexAttribute<glm::vec3>() {return VertexAttribute::Vec3;}
	template<>
	VertexAttribute getCorrespondingVertexAttribute<glm::vec2>() {return VertexAttribute::Vec2;}
	template<>
	VertexAttribute getCorrespondingVertexAttribute<float>() {return VertexAttribute::Float;}

	template<typename T>
	VertexAttribute getCorrespondingVertexAttribute(const T& arg) { return getCorrespondingVertexAttribute<T>(); }

	template<typename AttributesTuple>
	std::vector<VertexAttribute> createAttributesVector() {
		std::vector<VertexAttribute> out;
		AttributesTuple tuple;
		std::apply([&out](auto&... arg) {
		((out.push_back(getCorrespondingVertexAttribute(arg))), ...);
		}, tuple);
		return out;
	}
}