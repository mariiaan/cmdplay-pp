#include "ConsoleUtils.cuh"
#include <iostream>
#ifdef mac
#include <unistd.h>
#include <sys/ioctl.h>
#else
#include <Windows.h>
#endif

void cmdplay::ConsoleUtils::SetCursorPosition(int x, int y)
{
	#ifdef mac
    // The ANSI escape code to move the cursor is "\033[y;xH"
    // So we can print this string with x and y replaced by the function's parameters
    std::cout << "\033[" << y << ";" << x << "H";
	#else
	COORD pos = { static_cast<short>(x), static_cast<short>(y) };
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(output, pos);
	#endif
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
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

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

bool cmdplay::ConsoleUtils::GetWindowFocused()
{
	#ifdef mac
	return true;
	#else
	return GetConsoleWindow() == GetForegroundWindow();
	#endif
}
