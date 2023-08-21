#include "Asciifier.hpp"
#include "ColorConverter.hpp"
#include <iostream>
#include <algorithm>

inline uint8_t cmdplay::Asciifier::MapByteToArray(uint8_t value)
{
	return value * (m_brightnessLevelCount - 1) / 255;
}

inline char cmdplay::Asciifier::ToChar(uint8_t index)
{
	return m_brightnessLevels[index];
}

void cmdplay::Asciifier::InitColors()
{
	m_colors.push_back(std::make_unique<ConsoleColor>("30", ColorConverter::GetHue({ 0, 0, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("31", ColorConverter::GetHue({ 255, 0, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("32", ColorConverter::GetHue({ 0, 255, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("33", ColorConverter::GetHue({ 255, 255, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("34", ColorConverter::GetHue({ 0, 0, 255 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("35", ColorConverter::GetHue({ 255, 0, 255 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("36", ColorConverter::GetHue({ 0, 255, 255 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("37", ColorConverter::GetHue({ 255, 255, 255 })));
}

inline std::string cmdplay::Asciifier::GetColor(uint8_t r, uint8_t g, uint8_t b)
{
	auto colorToReturn = m_colors[0].get();
	float closest = 100000.0f;
	RGB col = { r / 255.0f, g / 255.0f, b / 255.0f };
	HSV hsv = ColorConverter::RGBToHSV(col);
	if (hsv.s < 0.1f)
		return "37";
	if (hsv.v < 0.1f)
		return "30";
	for (int i = 0; i < m_colors.size(); ++i)
	{
		float conColHue = m_colors[i]->m_hue;
		float hueDistance = std::abs(conColHue - hsv.h);

		if (hueDistance < closest)
		{
			colorToReturn = m_colors[i].get();
			closest = hueDistance;
		}
	}

	return colorToReturn->m_consoleColor;
}

cmdplay::Asciifier::Asciifier(const std::string& brightnessLevels, int frameWidth, int frameHeight, bool useColors) :
	m_brightnessLevels(brightnessLevels), m_frameWidth(frameWidth), m_frameHeight(frameHeight),
	m_brightnessLevelCount(static_cast<uint8_t>(brightnessLevels.length())), m_useColors(useColors)
{
	if (m_useColors)
	{
		InitColors();
		m_pixelStride = 6;
	}
	else
		m_pixelStride = 1;

	m_frameSubpixelCount = frameWidth * frameHeight * 3;
	m_frameWidth *= m_pixelStride;
	m_targetFramebufferSize = (m_frameWidth + 1) * m_frameHeight;
}

std::string cmdplay::Asciifier::BuildFrame(const uint8_t* rgbData)
{
	auto asciiData = std::make_unique<char[]>(m_targetFramebufferSize + 1);
	char* asciiDataArr = asciiData.get();
	for (int i = 0, scanX = 0; i < m_targetFramebufferSize; ++i)
	{
		if (++scanX == m_frameWidth + 1)
		{
			scanX = 0;
			asciiDataArr[i] = '\n';
		}
	}
	// Set null-terminator
	asciiData[m_targetFramebufferSize] = 0;

	int scanX = 0;
	int rowOffset = 0;
	for (int i = 0; i < m_frameSubpixelCount; i += 3)
	{
		// Max value = 255.000
		uint32_t pixelBrightness =
			rgbData[i] * PERCEIVED_LUMINANCE_R_FACTOR +
			rgbData[i + 1] * PERCEIVED_LUMINANCE_G_FACTOR +
			rgbData[i + 2] * PERCEIVED_LUMINANCE_B_FACTOR;

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

		scanX += m_pixelStride;
		if (scanX == m_frameWidth)
		{
			scanX = 0;
			rowOffset += m_frameWidth + 1;
		}
	}

	return std::string(asciiDataArr);
}
