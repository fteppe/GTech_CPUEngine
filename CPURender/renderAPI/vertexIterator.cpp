#include "vertexIterator.h"

VertexIterator& VertexIterator::operator=(const ConstVertexReference& right)
{
	VertexReference ref(*this);
	ref = right;
	return *this;
}
