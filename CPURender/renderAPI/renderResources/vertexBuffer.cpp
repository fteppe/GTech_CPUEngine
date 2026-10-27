#include "vertexBuffer.h"
#include "../../logger.h"

renderResource::VertexBuffer::~VertexBuffer()
{
}

void renderResource::VertexBuffer::overwrite(VertexIterator beginDest,
	ConstVertexIterator beginSrc, ConstVertexIterator endSrc)
{
	if ((endSrc.start - beginSrc.start) == 0)
		return;
	size_t sizeCopied = (endSrc.start - beginSrc.start);
	assert(beginDest.vertexSize == beginSrc.vertexSize && beginDest.vertexSize == fullVertexSize);
	assert(sizeCopied % fullVertexSize == 0);

	size_t dstOffset = (beginDest.start - &data[0]);
	size_t newSize = dstOffset + (endSrc.start - beginSrc.start);
	Logger::Log("adding %d verts, old size is %d, new is %d \n", sizeCopied / fullVertexSize, vertexCount, newSize / fullVertexSize);
	data.resize(newSize);
	uint8_t* dst = &data[0] + dstOffset;
	std::copy(beginSrc.start, endSrc.start, dst);
	vertexCount = newSize / fullVertexSize;
}
