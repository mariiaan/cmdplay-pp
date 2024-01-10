#include <iostream>
#include <vector>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

namespace cmdplay {
		class gpuAsciiFier
		{
		public:
			gpuAsciiFier(const std::string& brightnessLevels, int frameWidth, int frameHeight,
				bool useColors, bool useColorDithering, bool useTextDithering, bool useAccurateColors, bool useAccurateColorsFullPixel);

			std::string BuildFrame(uint8_t * rgb_data);
		private:
			std::vector<std::string> frames;
			std::string m_brightnessLevels;
			int m_frameWidth;
			int m_frameHeight;
			int m_framebuffersize;
			int m_frameWidthWithStride;
			uint8_t m_brightnessLevelCount;
			int m_framepixelbytescount;
			bool m_useColors;
			bool m_useColorDithering;
			bool m_useTextDithering;
			bool m_useAccurateColors;
			bool m_useAccurateColorsFullPixel;
		};
}

__global__ void asciifier(uint8_t* rgbData, char* framechars, char* brightnesslevels, int brightnesslevelcount, int width);