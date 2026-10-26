#pragma once
#include <cassert>
#include <cstdint>
#include <typeinfo>

class ConstantBuffer {
public:
	ConstantBuffer();
	ConstantBuffer(const std::type_info& typeId, uint8_t* constantBufferData) : constantBufferData(constantBufferData), typeId(typeId) {}

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
protected:
	const std::type_info& typeId;
	uint8_t* constantBufferData;
};

template<typename T>
class ConstantBufferT : public ConstantBuffer {
	T data;
public:
	ConstantBufferT(const T& data) : ConstantBuffer(typeid(T), reinterpret_cast<uint8_t*>(&this->data)), data(data)
	{
	}
};