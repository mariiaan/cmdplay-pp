#include "Asciifier.hpp"
#include "ColorConverter.hpp"
#include <iostream>
#include <algorithm>

cmdplay::Asciifier::Asciifier(const std::string& brightnessLevels, int frameWidth, int frameHeight, bool useColors, bool useColorDithering, bool useTextDithering) :
	m_brightnessLevels(brightnessLevels), m_frameWidthWithStride(frameWidth), m_frameWidth(frameWidth), m_frameHeight(frameHeight),
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
		m_colorDitherErrors = std::make_unique<float[]>(frameWidth * frameHeight);
		ClearColorDitherErrors();
	}
	m_textDitherErrors = std::make_unique<int[]>(frameWidth * frameHeight);
	m_frameWidthWithStride *= m_pixelStride;
	m_targetFramebufferSize = (m_frameWidthWithStride + 1) * m_frameHeight;
}

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

inline std::string cmdplay::Asciifier::GetColorDithered(uint8_t r, uint8_t g, uint8_t b, int x, int y)
{
	auto colorToReturn = m_colors[0].get();
	float closest = 100000.0f;
	RGB col = { r / 255.0f, g / 255.0f, b / 255.0f };
	HSV hsv = ColorConverter::RGBToHSV(col);
	if (hsv.s < 0.05f)
		return "37";
	if (hsv.v < 0.05f)
		return "30";
	hsv.h += m_colorDitherErrors[x + y * m_frameWidth];
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

	int tAddr = x + y * m_frameWidthWithStride;
	if (tAddr == m_frameWidthWithStride * m_frameHeight - 1)
		return colorToReturn->m_consoleColor;
	float ditherError = (hsv.h - colorToReturn->m_hue) * 0.0625f;
	WriteColorDitherErrors(x, y, ditherError);

	return colorToReturn->m_consoleColor;
}

void cmdplay::Asciifier::ClearColorDitherErrors()
{
	for (int i = 0; i < m_frameWidth * m_frameHeight; ++i)
		m_colorDitherErrors[i] = 0.0f;
}

void cmdplay::Asciifier::WriteColorDitherErrors(int x, int y, float error)
{
	if (x < m_frameWidth - 1)
		m_colorDitherErrors[x + (y * m_frameWidth) + 1] += error * 7;
	if (y < m_frameHeight - 1)
	{
		if (x > 0)
			m_colorDitherErrors[x + (y + 1) * m_frameWidth - 1] += error * 3;
		m_colorDitherErrors[x + (y + 1) * m_frameWidth] += error * 5;
		if (x < m_frameWidth - 1)
			m_colorDitherErrors[x + (y + 1) * m_frameWidth + 1] += error;
	}
}

void cmdplay::Asciifier::ClearTextDitherErrors()
{
	for (int i = 0; i < m_frameWidth * m_frameHeight; ++i)
		m_textDitherErrors[i] = 0;
}

void cmdplay::Asciifier::WriteTextDitherError(int x, int y, int error)
{
	if (x < m_frameWidth - 1)
		m_textDitherErrors[x + (y * m_frameWidth) + 1] += error * 7;
	if (y < m_frameHeight - 1)
	{
		if (x > 0)
			m_textDitherErrors[x + (y + 1) * m_frameWidth - 1] += error * 3;
		m_textDitherErrors[x + (y + 1) * m_frameWidth] += error * 5;
		if (x < m_frameWidth - 1)
			m_textDitherErrors[x + (y + 1) * m_frameWidth + 1] += error;
	}
}

std::string cmdplay::Asciifier::BuildFrame(const uint8_t* rgbData)
{
	if (m_useColorDithering)
		ClearColorDitherErrors();
	if (m_useTextDithering)
		ClearTextDitherErrors();
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
		if (m_useTextDithering)
		{
			pixelBrightness += 1000 * m_textDitherErrors[col + row * m_frameWidth];
			if (pixelBrightness < 0)
				pixelBrightness = 0;
			else if (pixelBrightness > 255000)
				pixelBrightness = 255000;
		}

		uint8_t trueBrightnessByte = pixelBrightness / 1000;
		uint8_t brightnessIndex = MapByteToArray(trueBrightnessByte);
		if (m_useTextDithering)
		{
			int actualBrightnessByte = brightnessIndex * 255 / (m_brightnessLevelCount - 1);
			int brightnessError = static_cast<int>((actualBrightnessByte - static_cast<int>(trueBrightnessByte)) * 0.0625f);
			WriteTextDitherError(col, row, brightnessError);
		}

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
			asciiDataArr[rowOffset + scanX] = ToChar(brightnessIndex);
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
