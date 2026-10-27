#include "vertexBuffer.h"
#include "../../logger.h"

renderResource::VertexBuffer::~VertexBuffer()
{
}

size_t renderResource::VertexBuffer::overwrite(VertexIterator beginDest,
	ConstVertexIterator beginSrc, ConstVertexIterator endSrc)
{
	size_t dstOffset = (beginDest.start - &data[0]);
	size_t endPos = dstOffset + (endSrc.start - beginSrc.start);

	if ((endSrc.start - beginSrc.start) == 0)
		return endPos / fullVertexSize;

	size_t sizeCopied = (endSrc.start - beginSrc.start);
	assert(beginDest.vertexSize == beginSrc.vertexSize && beginDest.vertexSize == fullVertexSize);
	assert(sizeCopied % fullVertexSize == 0);


	Logger::Log("adding %d verts, old size is %d, new is %d \n", sizeCopied / fullVertexSize, vertexCount, endPos / fullVertexSize);
	if(endPos > data.size())
		data.resize(endPos);

	uint8_t* dst = &data[0] + dstOffset;
	std::copy(beginSrc.start, endSrc.start, dst);
	vertexCount = data.size() / fullVertexSize;
	return endPos / fullVertexSize;
}
