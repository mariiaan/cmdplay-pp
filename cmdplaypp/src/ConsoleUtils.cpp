#include "ConsoleUtils.hpp"
#include <Windows.h>

void cmdplay::ConsoleUtils::SetCursorPosition(int x, int y)
{
	COORD pos = { static_cast<short>(x), static_cast<short>(y) };
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

void cmdplay::ConsoleUtils::ShowConsoleCursor(bool show)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO     cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = show;
	SetConsoleCursorInfo(out, &cursorInfo);
}

bool cmdplay::ConsoleUtils::GetWindowFocused()
{
	return GetConsoleWindow() == GetForegroundWindow();
}
