#pragma once
#include <cstdint>
#include <string>

namespace cmdplay
{
	struct ConsoleColor
	{
		const char* m_consoleColor;
		float m_hue;
		
		ConsoleColor(const char* consoleColor, float hue);
	};
}