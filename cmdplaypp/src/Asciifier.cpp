#include "Asciifier.hpp"
#include "ColorConverter.hpp"
#include <iostream>
#include <algorithm>

cmdplay::Asciifier::Asciifier(const std::string& brightnessLevels, int frameWidth, int frameHeight, 
	bool useColors, bool useColorDithering, bool useTextDithering) :
	m_brightnessLevels(brightnessLevels), m_frameWidth(frameWidth), m_frameHeight(frameHeight),
	m_useColorDithering(useColorDithering), m_useTextDithering(useTextDithering),
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
	if (m_useColorDithering)
	{
		m_hDitherErrors = std::make_unique<float[]>(frameWidth * frameHeight);
	}
	if (m_useTextDithering)
		m_textDitherErrors = std::make_unique<float[]>(frameWidth * frameHeight);
	
	m_frameWidthWithStride = m_frameWidth * m_pixelStride;
	m_targetFramebufferSize = (m_frameWidthWithStride + 1) * m_frameHeight;
}

inline int16_t cmdplay::Asciifier::MapByteToArray(int16_t value)
{
	return value * (m_brightnessLevelCount - 1) / 255;
}

inline char cmdplay::Asciifier::ToChar(int16_t index)
{
	if (index < 0)
		index = 0;
	else if (index > m_brightnessLevelCount - 1)
		index = m_brightnessLevelCount - 1;
	return m_brightnessLevels[index];
}

inline char cmdplay::Asciifier::ToCharUnchecked(uint16_t index)
{
	return m_brightnessLevels[index];
}

void cmdplay::Asciifier::InitColors()
{
//	m_colors.push_back(std::make_unique<ConsoleColor>("30", ColorConverter::GetHue({ 0, 0, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("31", ColorConverter::GetHue({ 255, 0, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("31", ColorConverter::GetHue({ 255, 0, 0 }) + 360.0f));
	m_colors.push_back(std::make_unique<ConsoleColor>("32", ColorConverter::GetHue({ 0, 255, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("33", ColorConverter::GetHue({ 255, 255, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("34", ColorConverter::GetHue({ 0, 0, 255 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("35", ColorConverter::GetHue({ 255, 0, 255 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("36", ColorConverter::GetHue({ 0, 255, 255 })));
	//m_colors.push_back(std::make_unique<ConsoleColor>("37", ColorConverter::GetHue({ 255, 255, 255 })));
}

inline std::string cmdplay::Asciifier::GetColor(uint8_t r, uint8_t g, uint8_t b)
{
	auto colorToReturn = m_colors[0].get();
	float closest = 100000.0f;
	RGB col = { r / 255.0f, g / 255.0f, b / 255.0f };
	HSV hsv = ColorConverter::RGBToHSV(col);
	
	if (hsv.s < 0.05f && hsv.v > 0.01f)
		return "37";

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

inline std::string cmdplay::Asciifier::GetColorDithered(uint8_t r, uint8_t g, uint8_t b, int x, int y)
{
	auto colorToReturn = m_colors[0].get();
	float closestColorDistance = 100000.0f;
	RGB col = { r / 255.0f, g / 255.0f, b / 255.0f };
	HSV hsv = ColorConverter::RGBToHSV(col);
	int ditherAddesss = x + y * m_frameWidth;

	if (hsv.s < 0.05f && hsv.v > 0.01f)
		return "37";

	hsv.h += m_hDitherErrors[ditherAddesss];
	for (int i = 0; i < m_colors.size(); ++i)
	{
		float hueDistance = std::abs(m_colors[i]->m_hue - hsv.h);

		if (hueDistance < closestColorDistance)
		{
			colorToReturn = m_colors[i].get();
			closestColorDistance = hueDistance;
		}
	}

	float ditherError = (hsv.h - colorToReturn->m_hue) * DITHER_FACTOR;
	WriteDitherError(x, y, ditherError, m_hDitherErrors.get());

	return colorToReturn->m_consoleColor;
}

void cmdplay::Asciifier::ClearDitherErrors(float* buffer)
{
	for (int i = 0; i < m_frameWidth * m_frameHeight; ++i)
		buffer[i] = 0.0f;
}

void cmdplay::Asciifier::WriteDitherError(int x, int y, float error, float* buffer)
{
	if (x < m_frameWidth - 1)
		buffer[x + (y * m_frameWidth) + 1] +=
		error * DITHER_NEIGHBOR_RIGHT_FACTOR;

	if (y < m_frameHeight - 1)
	{
		if (x > 0)
			buffer[x + (y + 1) * m_frameWidth - 1] +=
			error * DITHER_NEIGHBOR_BOTTOM_LEFT_FACTOR;

		buffer[x + (y + 1) * m_frameWidth] +=
			error * DITHER_NEIGHBOR_BOTTOM_FACTOR;

		if (x < m_frameWidth - 1)
			buffer[x + (y + 1) * m_frameWidth + 1] +=
			error * DITHER_NEIGHBOR_BOTTOM_RIGHT_FACTOR;
	}
}

std::string cmdplay::Asciifier::BuildFrame(const uint8_t* rgbData)
{
	if (m_useColorDithering)
		ClearDitherErrors(m_hDitherErrors.get());

	if (m_useTextDithering)
		ClearDitherErrors(m_textDitherErrors.get());

	auto asciiData = std::make_unique<char[]>(m_targetFramebufferSize + 1);
	char* asciiDataArr = asciiData.get();
	for (int i = 0, scanX = 0; i < m_targetFramebufferSize; ++i)
	{
		if (++scanX == m_frameWidthWithStride + 1)
		{
			scanX = 0;
			asciiDataArr[i] = '\n';
		}
	}
	// Set null-terminator
	asciiData[m_targetFramebufferSize] = 0;

	int scanX = 0;
	int rowOffset = 0;
	int col = 0;
	int row = 0;
	for (int i = 0; i < m_frameSubpixelCount; i += 3)
	{
		// Max value = 255.000
		int32_t pixelBrightness =
			rgbData[i] * PERCEIVED_LUMINANCE_R_FACTOR +
			rgbData[i + 1] * PERCEIVED_LUMINANCE_G_FACTOR +
			rgbData[i + 2] * PERCEIVED_LUMINANCE_B_FACTOR;

		int16_t brightnessIndex;
		if (m_useTextDithering)
		{
			pixelBrightness += static_cast<int>(1000 * m_textDitherErrors[col + row * m_frameWidth]);
			int16_t trueBrightnessByte = pixelBrightness / 1000;
			brightnessIndex = MapByteToArray(trueBrightnessByte);
			int actualBrightnessByte = brightnessIndex * 255 / (m_brightnessLevelCount - 1);
			float brightnessError = (static_cast<int>(trueBrightnessByte) - actualBrightnessByte) * DITHER_FACTOR;
			WriteDitherError(col, row, brightnessError, m_textDitherErrors.get());
		}
		else
			brightnessIndex = MapByteToArray(pixelBrightness / 1000);

		if (m_useColors)
		{
			auto color =
				m_useColorDithering ?
				GetColorDithered(rgbData[i], rgbData[i + 1], rgbData[i + 2], col, row) :
				GetColor(rgbData[i], rgbData[i + 1], rgbData[i + 2]);
			asciiDataArr[rowOffset + scanX] = '\x1B';
			asciiDataArr[rowOffset + scanX + 1] = '[';
			asciiDataArr[rowOffset + scanX + 2] = color[0];
			asciiDataArr[rowOffset + scanX + 3] = color[1];
			asciiDataArr[rowOffset + scanX + 4] = 'm';
			asciiDataArr[rowOffset + scanX + 5] = ToChar(brightnessIndex);
		}
		else
		{
			asciiDataArr[rowOffset + scanX] = m_useTextDithering ? 
				ToChar(brightnessIndex) : ToCharUnchecked(brightnessIndex);
		}

		scanX += m_pixelStride;
		++col;
		if (scanX == m_frameWidthWithStride)
		{
			scanX = 0;
			rowOffset += m_frameWidthWithStride + 1;
			++row;
			col = 0;
			m_lastBrightnessError = 0;
		}
	}

	return std::string(asciiDataArr);
}
