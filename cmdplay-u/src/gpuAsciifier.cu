#include "gpuAsciifier.cuh"
#include <math.h>


cmdplay::gpuAsciiFier::gpuAsciiFier(const std::string& brightnessLevels, int frameWidth, int frameHeight,
	bool useColors, bool useColorDithering, bool useTextDithering, bool useAccurateColors, bool useAccurateColorsFullPixel):
	m_brightnessLevels(brightnessLevels), m_frameWidth(frameWidth), m_frameHeight(frameHeight),
	m_useColorDithering(useColorDithering), m_useTextDithering(useTextDithering),
	m_brightnessLevelCount(static_cast<uint8_t>(brightnessLevels.length())), m_useColors(useColors),
	m_useAccurateColors(useAccurateColors), m_useAccurateColorsFullPixel(useAccurateColorsFullPixel)
{
	m_framepixelbytescount = m_frameWidth * m_frameHeight * 4;
	m_framebuffersize = (m_frameWidth+1) * m_frameHeight;
	m_frameWidthWithStride = m_frameWidth;
}

std::string cmdplay::gpuAsciiFier::BuildFrame(uint8_t * rgbData) {
	char* framechars;
	uint8_t* rgb;
	char* brightnesslevel;
	char* d_framechars;

	int rgbsize, framecharssize, brightnesslevelsize;
	rgbsize = sizeof(unsigned char) * m_framepixelbytescount;
	framecharssize = sizeof(char) * m_framebuffersize;
	brightnesslevelsize = sizeof(char) * m_brightnessLevelCount;

	uint8_t* d_rgb;
	char * d_brightnessLevels;

	

	cudaMallocHost((void**)&framechars, framecharssize, cudaHostAllocDefault);
	cudaMallocHost((void**)&rgb, rgbsize, cudaHostAllocDefault);
	cudaMallocHost((void**)&brightnesslevel,brightnesslevelsize, cudaHostAllocDefault);

	cudaMalloc((void**)&d_framechars, framecharssize);
	cudaMalloc((void**)&d_rgb, rgbsize);
	cudaMalloc(&d_brightnessLevels, brightnesslevelsize);

	rgb = rgbData;

	brightnesslevel = (char*)m_brightnessLevels.c_str();

	for (int i = 1; i < m_framebuffersize / (m_frameWidthWithStride + 1) + 1; ++i) {
		framechars[i * (m_frameWidthWithStride + 1) - 1] = '\n';

	}
	framechars[m_framebuffersize] = '\0';

	cudaMemcpy(d_framechars, framechars, framecharssize, cudaMemcpyHostToDevice);
	cudaMemcpy(d_rgb, rgb, rgbsize, cudaMemcpyHostToDevice);
	cudaMemcpy(d_brightnessLevels, brightnesslevel, brightnesslevelsize, cudaMemcpyHostToDevice);

	asciifier<< <m_frameHeight*m_frameWidth/256 + 1, 256>> > (d_rgb, d_framechars,d_brightnessLevels, m_brightnessLevelCount, m_frameWidth);

	cudaMemcpy(framechars, d_framechars, framecharssize, cudaMemcpyHostToDevice);

	cudaFree(d_framechars);
	cudaFree(d_rgb);
	cudaFree(d_brightnessLevels);

	cudaFreeHost(framechars);
	cudaFreeHost(rgb);
	cudaFreeHost(brightnesslevel);

	return std::string(framechars);
}

__global__ void asciifier(uint8_t * rgbData, char * framechars, char * brightnesslevel, int brightnesslevelcount, int width) {
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	int byteindex = index * 4;
	float check = static_cast<float>(index) / width;
	int frameindex = index + (int)floor(check);

	float r, g, b;
	r = (float)(int)rgbData[byteindex] / 255;
	g = (float)(int)rgbData[byteindex + 1] / 255;
	b = (float)(int)rgbData[byteindex + 2] / 255;

	int brightnessindex = (0.299 * r + 0.587 * g + 0.114 * b) * brightnesslevelcount;

	if (brightnessindex < 0) {
		brightnessindex = 0;
	}
	else if (brightnessindex >= brightnesslevelcount) {
		brightnessindex = brightnesslevelcount - 1;
	}

	framechars[frameindex] = brightnesslevel[brightnessindex];
}