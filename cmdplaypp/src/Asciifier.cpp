#include "Asciifier.hpp"
#include <iostream>

inline uint8_t cmdplay::Asciifier::MapByteToArray(uint8_t value)
{
	return value * (m_brightnessLevelCount - 1) / 255;
}

inline char cmdplay::Asciifier::ToChar(uint8_t index)
{
	return m_brightnessLevels[index];
}

inline std::string cmdplay::Asciifier::GetColor(uint8_t r, uint8_t g, uint8_t b)
{
	int lessDistance = 100000000;
	auto colorToReturn = m_colors[0].get();
	for (int i = 0; i < m_colors.size(); ++i)
	{
		auto color = m_colors[i].get();
		int rDistance = std::abs(color->m_r - r);
		int gDistance = std::abs(color->m_g - g);
		int bDistance = std::abs(color->m_b - b);
		int totalDistance = rDistance + gDistance + bDistance;

		if (totalDistance < lessDistance)
		{
			lessDistance = totalDistance;
			colorToReturn = color;
		}
	}
	return colorToReturn->m_consoleColor;
}

cmdplay::Asciifier::Asciifier(const std::string& brightnessLevels, int frameWidth, int frameHeight, bool useColors) :
	m_brightnessLevels(brightnessLevels), m_frameWidth(frameWidth), m_frameHeight(frameHeight),
	m_brightnessLevelCount(static_cast<uint8_t>(brightnessLevels.length())), m_frameSubpixelCount(frameWidth * frameHeight * 3),
	m_targetFramebufferSize((m_frameWidth + 1) * m_frameHeight), m_useColors(useColors)
{
	m_colors.push_back(std::make_unique<ConsoleColor>("30", 0, 0, 0));
	m_colors.push_back(std::make_unique < ConsoleColor>("31", 128, 0, 0));
	m_colors.push_back(std::make_unique < ConsoleColor>("32", 0, 128, 0));
	m_colors.push_back(std::make_unique < ConsoleColor>("33", 128, 128, 0));
	m_colors.push_back(std::make_unique < ConsoleColor>("34", 0, 0, 128));
	m_colors.push_back(std::make_unique < ConsoleColor>("35", 128, 0, 128));
	m_colors.push_back(std::make_unique < ConsoleColor>("36", 0, 128, 128));
	m_colors.push_back(std::make_unique < ConsoleColor>("37", 128, 128, 128));
	m_colors.push_back(std::make_unique < ConsoleColor>("91", 255, 0, 0));
	m_colors.push_back(std::make_unique < ConsoleColor>("91", 0, 255, 0));
	m_colors.push_back(std::make_unique < ConsoleColor>("93", 255, 255, 0));
	m_colors.push_back(std::make_unique < ConsoleColor>("94", 0, 0, 255));
	m_colors.push_back(std::make_unique < ConsoleColor>("95", 255, 0, 255));
	m_colors.push_back(std::make_unique < ConsoleColor>("96", 0, 255, 255));
	m_colors.push_back(std::make_unique < ConsoleColor>("97", 255, 255, 255));
}

std::string cmdplay::Asciifier::BuildFrame(const uint8_t* rgbData)
{
	int charsPerPixel = m_useColors ? 6 : 1;
	auto asciiData = std::make_unique<char[]>(m_targetFramebufferSize * charsPerPixel);
	char* asciiDataArr = asciiData.get();
	for (int i = 0, scanX = 0; i < m_targetFramebufferSize * charsPerPixel; ++i)
	{
		if (++scanX == (m_frameWidth * charsPerPixel) + 1)
		{
			scanX = 0;
			asciiDataArr[i] = '\n';
		}
		else
			asciiDataArr[i] = 'U';
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
		
		if (m_useColors)
		{
			auto color = GetColor(rgbData[i], rgbData[i + 1], rgbData[i + 2]);
			asciiDataArr[rowOffset + scanX] = '\x1B';
			asciiDataArr[rowOffset + scanX + 1] = '[';
			asciiDataArr[rowOffset + scanX + 2] = color[0];
			asciiDataArr[rowOffset + scanX + 3] = color[1];
			asciiDataArr[rowOffset + scanX + 4] = 'm';
			asciiDataArr[rowOffset + scanX + 5] = ToChar(brightnessIndex);
		}
		else
		{
			asciiDataArr[rowOffset + scanX] = ToChar(brightnessIndex);
		}
	
		scanX += charsPerPixel;
		if (scanX == (m_frameWidth * charsPerPixel))
		{
			scanX = 0;
			rowOffset += (m_frameWidth * charsPerPixel) + 1;
		}
	}
	return std::string(asciiDataArr);
}
