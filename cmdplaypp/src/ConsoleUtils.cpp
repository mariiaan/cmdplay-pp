#include "ConsoleUtils.hpp"
#include <Windows.h>

void cmdplay::ConsoleUtils::SetCursorPosition(int x, int y)
{
	COORD pos = { x, y };
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(output, pos);
}

void cmdplay::ConsoleUtils::GetWindowSize(int* width, int* height)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

	if (width != nullptr)
		*width = csbi.srWindow.Right - csbi.srWindow.Left - 1;
	if (height != nullptr)
		*height = csbi.srWindow.Bottom - csbi.srWindow.Top - 1;
}
