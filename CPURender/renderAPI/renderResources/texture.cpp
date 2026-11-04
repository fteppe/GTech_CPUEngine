#include "texture.h"

renderResource::ColorCubemap::ColorCubemap(glm::ivec2 sizeSlice, const std::vector<Color> &concatenatedColorData) :
	ColorTexture(glm::ivec3(sizeSlice, 6))
{
	this->data.clear();
	size_t pixelCount = size.x * size.y * size.z;
	size_t sizePixel = sizeof(Color);
	const uint8_t* dataIn = reinterpret_cast<const uint8_t*>(concatenatedColorData.data());
	this->data.clear();
	this->data.insert(this->data.begin(), dataIn, dataIn + (pixelCount * sizePixel));
}

glm::vec4 renderResource::ColorCubemap::sample(glm::vec3 uv) const {

	for (int iAxis = 0; iAxis < 3; iAxis++) {
		if (std::abs(uv[iAxis]) >= std::abs(uv[(iAxis+1)%3]) && std::abs(uv[iAxis]) >= std::abs(uv[(iAxis+2)%3])) {
			int maxAxis = iAxis;
			int positiveDir = uv[maxAxis] > 0 ? 0 : 1;


			int textureLayer = iAxis * 2 + positiveDir;
			glm::vec2 intermidiateDirInLayer = {-uv[(iAxis+1)%3], uv[(iAxis+2)%3]};
			glm::vec2 dirInLayer = intermidiateDirInLayer;

			switch (textureLayer) {
				case 0: { //left
					dirInLayer = {uv.z, -uv.y};
					break;
				}
				case 1: {//right
					dirInLayer = {uv.z, uv.y};
					break;
				}
				case 2: {//top
					dirInLayer = {-uv.x, uv.z};
					break;
				}
				case 3: {//bottom
					dirInLayer = {uv.x, uv.z};
					break;
				}
				case 4: {//front
					dirInLayer = {-uv.x, -uv.y};
					break;
				}
				case 5: {//back
					dirInLayer = {-uv.x, uv.y};
					break;
				}
			}
			dirInLayer /= uv[maxAxis];
			dirInLayer = dirInLayer * 0.5f + 0.5f;

			return samplePixel( glm::vec3(dirInLayer, float(textureLayer) / 5.f));
		}
	}

	return ColorTexture::sample(uv);
}
