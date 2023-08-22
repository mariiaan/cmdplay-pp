#pragma once
#include "ConsoleColor.hpp"
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <memory>

namespace cmdplay
{
	constexpr uint16_t PERCEIVED_LUMINANCE_R_FACTOR = 299;
	constexpr uint16_t PERCEIVED_LUMINANCE_G_FACTOR = 587;
	constexpr uint16_t PERCEIVED_LUMINANCE_B_FACTOR = 114;

	class Asciifier
	{
	public:
		Asciifier(const std::string& brightnessLevels, int frameWidth, int frameHeight, 
			bool useColors = true, bool useColorDithering = true, bool useTextDithering = true);

	private:
		inline int16_t MapByteToArray(int16_t value);
		inline char ToChar(int16_t index);
		void InitColors();
		inline std::string GetColor(uint8_t r, uint8_t g, uint8_t b);
		inline std::string GetColorDithered(uint8_t r, uint8_t g, uint8_t b, int x, int y);
		void ClearColorDitherErrors();
		void WriteColorDitherErrors(int x, int y, float error);
		std::unique_ptr<float[]> m_colorDitherErrors;
		bool m_useColorDithering = false;
		void ClearTextDitherErrors();
		void WriteTextDitherError(int x, int y, float error);
		std::unique_ptr<float[]> m_textDitherErrors;
		bool m_useTextDithering = false;
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

	public:
		std::string BuildFrame(const uint8_t* rgbData);
	};
}