#include "Asciifier.hpp"
#include <memory>
#include <iostream>
inline uint8_t cmdplay::Asciifier::MapByteToArray(uint8_t value)
{
	return value * (m_brightnessLevelCount - 1) / 255;
}

inline char cmdplay::Asciifier::ToChar(uint8_t index)
{
	return m_brightnessLevels[index];
}

cmdplay::Asciifier::Asciifier(const std::string& brightnessLevels, int frameWidth, int frameHeight) :
	m_brightnessLevels(brightnessLevels), m_frameWidth(frameWidth), m_frameHeight(frameHeight),
	m_brightnessLevelCount(brightnessLevels.length()), m_frameSubpixelCount(frameWidth * frameHeight * 3),
	m_targetFramebufferSize((m_frameWidth + 1) * m_frameHeight)
{
	
}

std::string cmdplay::Asciifier::BuildFrame(const uint8_t* rgbData)
{
	auto asciiData = std::make_unique<char[]>(m_targetFramebufferSize);
	char* asciiDataArr = asciiData.get();
	for (int i = 0, scanX = 0; i < m_targetFramebufferSize; ++i)
	{
		if (++scanX == m_frameWidth + 1)
		{
			scanX = 0;
			asciiDataArr[i] = '\n';
		}
		else
			asciiDataArr[i] = ' ';
	}
	
	int scanX = 0;
	int rowOffset = 0;
	for (int i = 0; i < m_frameSubpixelCount; i += 3)
	{
		// Max value = 255.000
		uint32_t pixelBrightness =
			rgbData[i]		* PERCEIVED_LUMINANCE_R_FACTOR +
			rgbData[i + 1]	* PERCEIVED_LUMINANCE_G_FACTOR +
			rgbData[i + 2]	* PERCEIVED_LUMINANCE_B_FACTOR;

		uint8_t brightnessIndex = MapByteToArray(pixelBrightness / 1000);
		
		asciiDataArr[rowOffset + scanX] = ToChar(brightnessIndex);
		
		if (++scanX == m_frameWidth)
		{
			scanX = 0;
			rowOffset += m_frameWidth + 1;
		}
	}
	return std::string(asciiDataArr);
}
