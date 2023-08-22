#pragma once
namespace cmdplay
{
	class ConsoleUtils
	{
	public:
		static void SetCursorPosition(int x, int y);
		static void GetWindowSize(int* width, int* height);
		static void ShowConsoleCursor(bool show);
		static bool GetWindowFocused();
	};
}