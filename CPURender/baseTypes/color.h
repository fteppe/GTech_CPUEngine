#pragma once
#include <cmath>
#include <cstdint>

struct Color {
public:

	union {
		//To respect the convention where writing a color is 0x __(red)__(green)__(blue)
		//The components are placed in the order b g r a in memory because we have a little endian architecture
		uint32_t rgba;
		struct {
			uint8_t b , g , r, a;
		};
	};
	constexpr Color() : rgba(0) {}
	//constexpr  Color(float r, float g, float b, float a = 1.0) : r(r * 255), g(g * 255), b(b * 255), a(a * 255) {}
	static constexpr Color fromUNorm(float r, float g, float b, float a = 1.0) {return Color(r * 255,g * 255,b * 255,a * 255);}
	constexpr Color(uint32_t rgba) : rgba(rgba) {}
	constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
	operator uint32_t() const { return rgba; }
};


struct Depth {
	uint32_t depth;
	static constexpr float depthUnitValue = 1. / std::numeric_limits<uint32_t>::max();
	constexpr Depth() : depth(0) {}
	constexpr Depth(uint32_t depthIn) : depth(depthIn) {}
	constexpr float to01() const {return depthUnitValue * depth;}
	static constexpr Depth from01(float value) {return {uint32_t(std::floor(std::numeric_limits<uint32_t>::max() * value))};}
};

namespace colors {
	static constexpr Color WHITE = Color(0xFFFFFF);
	static constexpr Color BLACK = Color(0);
	static constexpr Color BLUE = Color(0,0,255);
	static constexpr Color RED = Color(255,0,0);
	static constexpr Color GREEN = Color(0,255,0);
	static constexpr Color YELLOW = Color(255,255,0);
	static constexpr Color PINK = Color(255,0,255);
}
