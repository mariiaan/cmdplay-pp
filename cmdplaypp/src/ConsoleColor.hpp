#pragma once
#include <cstdint>
#include <string>

namespace cmdplay
{
	struct ConsoleColor
	{
		const char* m_consoleColor;
		uint8_t m_r;
		uint8_t m_g;
		uint8_t m_b;
		
		ConsoleColor(const char* consoleColor, uint8_t r, uint8_t g, uint8_t b);
	};
}