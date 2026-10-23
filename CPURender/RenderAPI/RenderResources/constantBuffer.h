#pragma once
#include <cassert>
#include <cstdint>
#include <typeinfo>

class ConstantBuffer {
public:
	ConstantBuffer();
	template<typename T>
	const T& getData() const {
		assert(typeid(T) == typeId);
		return *reinterpret_cast<const T*>(constantBufferData);
	}

	template<typename T>
	void updateBuffer(const T& data) {
		assert(typeid(T) == typeId);
		*reinterpret_cast<T*>(constantBufferData) = data;
	}
private:
	const std::type_info typeId;
	const uint8_t* constantBufferData;
};

template<typename T>
class ConstantBufferT : public ConstantBuffer {
	T data;
	ConstantBufferT(const T& data) : data(data), typeId(typeid(T))
	{
		constantBufferData = &data;
	}
};