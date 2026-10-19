#pragma once
#include <cstdint>

struct Color {
public:

	union {
		uint32_t rgba;
		struct {
			uint8_t r, g, b, a;
		};
	};

	//constexpr  Color(float r, float g, float b, float a = 1.0) : r(r * 255), g(g * 255), b(b * 255), a(a * 255) {}
	static constexpr Color fromUNorm(float r, float g, float b, float a = 1.0) {return Color(r * 255,g * 255,b * 255,a * 255);}
	constexpr Color(uint32_t rgba) : rgba(rgba) {}
	constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
	operator uint32_t() const { return rgba; }
};
namespace colors {
	static constexpr Color WHITE = Color(0xFFFFFF);
	static constexpr Color BLACK = Color(0);
	static constexpr Color BLUE = Color(0,0,255);
	static constexpr Color RED = Color(255,0,0);
	static constexpr Color GREEN = Color(0,255,0);
}
