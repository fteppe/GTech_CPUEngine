#include "vertexBuffer.h"

renderResource::VertexBuffer::~VertexBuffer()
{
}

void renderResource::VertexBuffer::overwrite(VertexIterator beginDest,
	ConstVertexIterator beginSrc, ConstVertexIterator endSrc)
{
	if ((endSrc.start - beginSrc.start) == 0)
		return;

	assert(beginDest.vertexSize == beginSrc.vertexSize && beginDest.vertexSize == fullVertexSize);
	assert((endSrc.start - beginSrc.start) % fullVertexSize == 0);

	size_t dstOffset = (beginDest.start - &data[0]);
	size_t newSize = dstOffset + (endSrc.start - beginSrc.start);
	data.resize(newSize);
	uint8_t* dst = &data[0] + dstOffset;
	std::copy(beginSrc.start, endSrc.start, dst);
	vertexCount = newSize / fullVertexSize;
}
