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

char* cmdplay::gpuAsciiFier::BuildFrame(uint8_t * rgbData) {
	char* framechars;
	uint8_t* rgb;
	char* brightnesslevel;

	cudaDeviceProp p;

	int rgbsize, framecharssize, brightnesslevelsize;
	rgbsize = sizeof(unsigned char) * m_framepixelbytescount;
	framecharssize = sizeof(char) * m_framebuffersize;
	brightnesslevelsize = sizeof(char) * m_brightnessLevelCount;

	char* d_framechars;
	uint8_t* d_rgb;
	char * d_brightnessLevels;

	cudaSetDeviceFlags(cudaDeviceMapHost);

	cudaGetDeviceProperties(&p, 0);

	if (!p.canMapHostMemory)
		exit(0);

	cudaHostAlloc((void**)&framechars, framecharssize, cudaHostAllocWriteCombined | cudaHostAllocMapped);
	cudaHostAlloc((void**)&rgb, rgbsize, cudaHostAllocWriteCombined | cudaHostAllocMapped);
	cudaHostAlloc((void**)&brightnesslevel,brightnesslevelsize, cudaHostAllocWriteCombined | cudaHostAllocMapped);


	memcpy(rgb, rgbData, rgbsize);

	char* h_bri = (char*)m_brightnessLevels.c_str();

	memcpy(brightnesslevel, h_bri, brightnesslevelsize);

	for (int i = 1; i < m_framebuffersize / (m_frameWidthWithStride + 1) + 1; ++i) {
		memset(framechars + i * (m_frameWidthWithStride + 1) - 1,'\n', 1);

	}
	memset(framechars + m_framebuffersize, '\0', 1);

	cudaHostGetDevicePointer((void**)&d_framechars, framechars, 0);
	cudaHostGetDevicePointer((void**)&d_rgb, rgb, 0);
	cudaHostGetDevicePointer((void**)&d_brightnessLevels, brightnesslevel, 0);

	asciifier<< <m_frameHeight*m_frameWidth/256 + 1, 256>> > (d_rgb, d_framechars,d_brightnessLevels, m_brightnessLevelCount, m_frameWidth);

	cudaDeviceSynchronize();

	cudaFreeHost(rgb);
	cudaFreeHost(brightnesslevel);

	return framechars;
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