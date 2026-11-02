//
// Created by francois on 11/2/26.
//

#include "blinnPhongShader.h"

void BlinnPhongVS::VS_MAIN(size_t vertexId, ConstVertexReference vertexIn, VertexReference vertexOut) const {

	auto [pos, normal, uvs] = vertexIn.getVertexAttributes<VS_Inputs>();

	glm::vec4 clipPos = scene().VP * object().modelMat * glm::vec4(pos, 1.0f);

	vertexOut.setVertexAttributes(VS_Outputs(clipPos, pos, normal, uvs));

}

Color BlinnPhongPS::PS_MAIN(glm::ivec2 screenSpaceCoord, ConstVertexReference interpolatedVertex) const {

	auto [clipPos, worldPos, normal, uvs] = interpolatedVertex.getVertexAttributes<PS_Inputs>();

	return colors::GREEN;
}
