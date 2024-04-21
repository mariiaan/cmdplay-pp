#pragma once
#include "ConsoleColor.hpp"
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <memory>
#include <unordered_map>

namespace cmdplay
{
	constexpr uint16_t PERCEIVED_LUMINANCE_R_FACTOR = 299;
	constexpr uint16_t PERCEIVED_LUMINANCE_G_FACTOR = 587;
	constexpr uint16_t PERCEIVED_LUMINANCE_B_FACTOR = 114;
	constexpr float DITHER_FACTOR = 0.0625f;
	constexpr int DITHER_NEIGHBOR_RIGHT_FACTOR = 7;
	constexpr int DITHER_NEIGHBOR_BOTTOM_LEFT_FACTOR = 3;
	constexpr int DITHER_NEIGHBOR_BOTTOM_FACTOR = 5;
	constexpr int DITHER_NEIGHBOR_BOTTOM_RIGHT_FACTOR = 1;
	constexpr uint8_t COLOR_BATCHING_TOLERANCE = 4;
	constexpr uint8_t COLOR_REDRAW_TOLERANCE = 104;

	struct Pixel
	{
		char c;
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};

	class Asciifier
	{
	public:
		Asciifier(const std::string& brightnessLevels, int frameWidth, int frameHeight, 
			bool useColors = true, bool useColorDithering = true, bool useTextDithering = true, bool useAccurateColors = true, bool useAccurateColorsFullPixel = true);

		~Asciifier();
	private:
		inline int16_t MapByteToArray(int16_t value);
		inline char ToChar(int16_t index);
		inline char ToCharUnchecked(uint16_t index);
		void InitColors();
		inline std::string GetColor(uint8_t r, uint8_t g, uint8_t b);
		inline std::string GetColorDithered(uint8_t r, uint8_t g, uint8_t b, int x, int y);
		std::string ByteAsPaddedString(uint8_t i);
		bool ColorComponentNearlyEquals(uint8_t value, uint8_t other, uint8_t tolerance = COLOR_BATCHING_TOLERANCE);
		float CalculateFrameBufferNoisiness();
		float CalulateFrameBufferToBackBufferDifference();
		std::unique_ptr<float[]> m_hDitherErrors;
		bool m_useColorDithering = false;
		std::unique_ptr<float[]> m_textDitherErrors;
		bool m_useTextDithering = false;
		void ClearDitherErrors(float* buffer);
		void WriteDitherError(int x, int y, float error, float* buffer);
		int m_frameWidthWithStride = 0;
		int m_frameWidth = 0;
		int m_frameHeight = 0;
		int m_frameSubpixelCount = 0;
		std::string m_brightnessLevels;
		uint8_t m_brightnessLevelCount = 0;
		int m_targetFramebufferSize = 0;
		std::vector<std::unique_ptr<ConsoleColor>> m_colors;
		bool m_useColors = false;
		int m_pixelStride = 1;
		int m_lastBrightnessError = 0;
		bool m_useAccurateColors = false;
		bool m_useAccurateColorsFullPixel = false;
		uint8_t m_lastSetColor[3] = { 255, 255, 255 };

		Pixel* m_frameBuffer;
		Pixel* m_backBuffer;

	public:
		std::string BuildFrame(const uint8_t* rgbData, bool fullRedraw);
	};
}