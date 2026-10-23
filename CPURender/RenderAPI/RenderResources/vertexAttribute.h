#pragma once
namespace renderResource {
	enum class VertexAttribute {
		Float,
		Vec3,
		Vec2,
		Vec4,
	};

	size_t getSizeOfAttribute(VertexAttribute attribute) {
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
	}
}