#include "Asciifier.hpp"
#include "ColorConverter.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <unordered_map>

cmdplay::Asciifier::Asciifier(const std::string& brightnessLevels, int frameWidth, int frameHeight, 
	bool useColors, bool useColorDithering, bool useTextDithering, bool useAccurateColors, bool useAccurateColorsFullPixel) :
	m_brightnessLevels(brightnessLevels), m_frameWidth(frameWidth), m_frameHeight(frameHeight),
	m_useColorDithering(useColorDithering), m_useTextDithering(useTextDithering),
	m_brightnessLevelCount(static_cast<uint8_t>(brightnessLevels.length())), m_useColors(useColors), 
	m_useAccurateColors(useAccurateColors), m_useAccurateColorsFullPixel(useAccurateColorsFullPixel)
{
	if (m_useColors)
	{
		if (m_useAccurateColors)
			m_pixelStride = 20;
		else
		{
			InitColors();
			m_pixelStride = 6;
		}
	}
	else
		m_pixelStride = 1;

	m_frameSubpixelCount = frameWidth * frameHeight * 3;
	if (m_useColorDithering)
		m_hDitherErrors = std::make_unique<float[]>(frameWidth * frameHeight);
	
	if (m_useTextDithering)
		m_textDitherErrors = std::make_unique<float[]>(frameWidth * frameHeight);
	
	m_frameWidthWithStride = m_frameWidth * m_pixelStride;
	m_targetFramebufferSize = (m_frameWidthWithStride + 1) * m_frameHeight;

	m_frameBuffer = (Pixel*)malloc(m_frameWidth * m_frameHeight * sizeof(Pixel));
	m_backBuffer = (Pixel*)malloc(m_frameWidth * m_frameHeight * sizeof(Pixel));
}

cmdplay::Asciifier::~Asciifier()
{
	free(m_frameBuffer);
	free(m_backBuffer);
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
	m_colors.push_back(std::make_unique<ConsoleColor>("31", ColorConverter::GetHue({ 255, 0, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("31", ColorConverter::GetHue({ 255, 0, 0 }) + 360.0f));
	m_colors.push_back(std::make_unique<ConsoleColor>("32", ColorConverter::GetHue({ 0, 255, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("33", ColorConverter::GetHue({ 255, 255, 0 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("34", ColorConverter::GetHue({ 0, 0, 255 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("35", ColorConverter::GetHue({ 255, 0, 255 })));
	m_colors.push_back(std::make_unique<ConsoleColor>("36", ColorConverter::GetHue({ 0, 255, 255 })));
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

inline std::string cmdplay::Asciifier::ByteAsPaddedString(uint8_t i)
{
	std::string out = std::to_string(i);

	// We want this number to be always 3 characters wide (e.g. 4 => 004, 27 => 027; 174 => 174)
	while (out.size() < 3)
		out.insert(0, "0");

	return out;
}

inline bool cmdplay::Asciifier::ColorComponentNearlyEquals(uint8_t value, uint8_t other, uint8_t tolerance)
{
	return std::abs(value - other) <= tolerance;
}

float cmdplay::Asciifier::CalculateFrameBufferNoisiness()
{
	std::unordered_map<uint32_t, uint32_t> colorToOccurences;
	for (int i = 0; i < m_frameWidth * m_frameHeight; i++)
	{
		const uint8_t colorBatchedR = (m_frameBuffer[i].r / COLOR_BATCHING_TOLERANCE) * COLOR_BATCHING_TOLERANCE;
		const uint8_t colorBatchedG = (m_frameBuffer[i].g / COLOR_BATCHING_TOLERANCE) * COLOR_BATCHING_TOLERANCE;
		const uint8_t colorBatchedB = (m_frameBuffer[i].b / COLOR_BATCHING_TOLERANCE) * COLOR_BATCHING_TOLERANCE;
		uint32_t colorInt = colorBatchedR << 24 + colorBatchedG << 16 + colorBatchedB << 8;
		colorToOccurences[colorInt]++;
	}

	double entropy = 0.f;
	for (auto& kvp : colorToOccurences)
	{
		double p = (double)(kvp.second) / (m_frameWidth * m_frameHeight);
		if (p > 0)
		{
			entropy -= p * log2(p);
		}
	}

	const double linearAlpha = entropy / log2(2 ^ 24); // divide through the maximum amount of colors;
	const double expoAlpha = 1 - pow(1 - linearAlpha, 1.5f);
	return expoAlpha;
}

float cmdplay::Asciifier::CalulateFrameBufferToBackBufferDifference()
{
	double differenceR = 0.f;
	double differenceG = 0.f;
	double differenceB = 0.f;
	for (int i = 0; i < m_frameWidth * m_frameHeight; i++)
	{
		differenceR += std::abs(m_frameBuffer[i].r - m_backBuffer[i].r);
		differenceG += std::abs(m_frameBuffer[i].g - m_backBuffer[i].g);
		differenceB += std::abs(m_frameBuffer[i].b - m_backBuffer[i].b);
	}

	differenceR /= m_frameWidth * m_frameHeight * 255;
	differenceG /= m_frameWidth * m_frameHeight * 255;
	differenceB /= m_frameWidth * m_frameHeight * 255;

	double totalDifference = differenceR + differenceG + differenceB;
	totalDifference /= 3;
	return totalDifference;
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

std::string cmdplay::Asciifier::BuildFrame(const uint8_t* rgbData, bool fullRedraw)
{
	if (m_useColors && m_useAccurateColors)
	{
		std::stringstream ss;

		{
			Pixel* temp = m_frameBuffer;
			m_frameBuffer = m_backBuffer;
			m_backBuffer = temp;
		}

		bool repositionCursor = false;
		int lastRow = 0;
		for (int i = 0; i < m_frameWidth * m_frameHeight; i++)
		{
			int32_t pixelBrightness =
				rgbData[i] * PERCEIVED_LUMINANCE_R_FACTOR +
				rgbData[i + 1] * PERCEIVED_LUMINANCE_G_FACTOR +
				rgbData[i + 2] * PERCEIVED_LUMINANCE_B_FACTOR;

			int16_t brightnessIndex = MapByteToArray(pixelBrightness / 1000);

			if (m_useAccurateColorsFullPixel)
				m_frameBuffer[i].c = ' ';
			else
				m_frameBuffer[i].c = ToCharUnchecked(brightnessIndex);

			m_frameBuffer[i].r = rgbData[i * 3];
			m_frameBuffer[i].g = rgbData[i * 3 + 1];
			m_frameBuffer[i].b = rgbData[i * 3 + 2];
		}

		const double frameNoisinessAlpha = CalculateFrameBufferNoisiness();
		const double frameDifferenceAlpha = CalulateFrameBufferToBackBufferDifference();
		const double totalAlpha = frameNoisinessAlpha * frameDifferenceAlpha;

		for (int i = 0; i < m_frameWidth * m_frameHeight; i++)
		{
			if (ColorComponentNearlyEquals(m_frameBuffer[i].r, m_backBuffer[i].r, COLOR_REDRAW_TOLERANCE * totalAlpha) &&
				ColorComponentNearlyEquals(m_frameBuffer[i].g, m_backBuffer[i].g, COLOR_REDRAW_TOLERANCE * totalAlpha) &&
				ColorComponentNearlyEquals(m_frameBuffer[i].b, m_backBuffer[i].b, COLOR_REDRAW_TOLERANCE * totalAlpha) &&
				!fullRedraw)
			{
				// we skip this pixel as the color on the screen is close enough
				repositionCursor = true;

				// set the color of the framebuffer back to the color that's on the screen
				m_frameBuffer[i].c = m_frameBuffer[i].c;
				m_frameBuffer[i].r = m_frameBuffer[i].r;
				m_frameBuffer[i].g = m_frameBuffer[i].g;
				m_frameBuffer[i].b = m_frameBuffer[i].b;

				//ss << "B";
				continue;
			}

			if (repositionCursor)
			{
				ss << "\x1B[" << std::to_string(i / m_frameWidth + 1) << ";" << std::to_string(i % m_frameWidth + 1) << "H";
				repositionCursor = false;
			}

			if (!(ColorComponentNearlyEquals(m_frameBuffer[i].r, m_lastSetColor[0], COLOR_BATCHING_TOLERANCE) &&
				ColorComponentNearlyEquals(m_frameBuffer[i].g, m_lastSetColor[1], COLOR_BATCHING_TOLERANCE) &&
				ColorComponentNearlyEquals(m_frameBuffer[i].b, m_lastSetColor[2], COLOR_BATCHING_TOLERANCE)))
			{
				ss << "\x1B[48;2;" << std::to_string(m_frameBuffer[i].r) << ";" << std::to_string(m_frameBuffer[i].g) << ";" << std::to_string(m_frameBuffer[i].b) << "m";
				m_lastSetColor[0] = m_frameBuffer[i].r;
				m_lastSetColor[1] = m_frameBuffer[i].g;
				m_lastSetColor[2] = m_frameBuffer[i].b;
			}

			ss << m_frameBuffer[i].c;

			if (i / m_frameWidth != lastRow)
			{
				repositionCursor = true;
				lastRow = i / m_frameWidth;
			}
		}

		return ss.str();
	}
	else // I cant port that shit over man
	{
		if (m_useColorDithering)
			ClearDitherErrors(m_hDitherErrors.get());

		if (m_useTextDithering)
			ClearDitherErrors(m_textDitherErrors.get());

		auto asciiData = std::make_unique<char[]>(m_targetFramebufferSize + 1);
		char* asciiDataArr = asciiData.get();
		for (int i = 0, scanX = 0; i < m_targetFramebufferSize; ++i)
			if (++scanX == m_frameWidthWithStride + 1)
			{
				scanX = 0;
				asciiDataArr[i] = '\n';
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
				asciiDataArr[rowOffset + scanX] = m_useTextDithering ?
				ToChar(brightnessIndex) : ToCharUnchecked(brightnessIndex);

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
}
