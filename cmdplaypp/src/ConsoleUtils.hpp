#pragma once
namespace cmdplay
{
	class ConsoleUtils
	{
	public:
		static void SetCursorPosition(int x, int y);
		static void GetWindowSize(int* width, int* height);
	};
}