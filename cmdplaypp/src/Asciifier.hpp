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
	private:
		inline uint8_t MapByteToArray(uint8_t value);
		inline char ToChar(uint8_t index);
		void InitColors();
		inline std::string GetColor(uint8_t r, uint8_t g, uint8_t b);
		int m_frameWidth = 0;
		int m_frameHeight = 0;
		int m_frameSubpixelCount = 0;
		std::string m_brightnessLevels;
		uint8_t m_brightnessLevelCount = 0;
		int m_targetFramebufferSize = 0;
		std::vector<std::unique_ptr<ConsoleColor>> m_colors;
		bool m_useColors = false;
		int m_pixelStride = 1;

	public:
		Asciifier(const std::string& brightnessLevels, int frameWidth, int frameHeight, bool useColors = true);
		std::string BuildFrame(const uint8_t* rgbData);
	};
}