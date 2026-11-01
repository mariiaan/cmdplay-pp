#include "ConsoleUtils.hpp"
#include <iostream>
#ifdef mac
#include <unistd.h>
#include <sys/ioctl.h>
#else
#include <Windows.h>
#endif

void cmdplay::ConsoleUtils::SetCursorPosition(int x, int y)
{
    std::cout << "\033[" << y << ";" << x << "H";
}

void cmdplay::ConsoleUtils::GetWindowSize(int* width, int* height)
{
	#ifdef mac
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	
	if (width != nullptr)
		*width = w.ws_col - 1;
	if (height != nullptr)
		*height = w.ws_row - 1;
	#else
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
	{
		// No console window (output redirected): fall back to a sane default instead of
		// scaling the video to uninitialized numbers.
		if (width != nullptr)
			*width = 100;
		if (height != nullptr)
			*height = 30;
		return;
	}

	if (width != nullptr)
		*width = csbi.srWindow.Right - csbi.srWindow.Left - 1;
	if (height != nullptr)
		*height = csbi.srWindow.Bottom - csbi.srWindow.Top - 1;
	#endif
}

void cmdplay::ConsoleUtils::ShowConsoleCursor(bool show)
{
	#ifdef mac
	if (show)
		std::cout << "\e[?25h]";
	else
		std::cout << "\e[?25l]";
	#else
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO     cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = show;
	SetConsoleCursorInfo(out, &cursorInfo);
	#endif
}