#include <iostream>
#include "Version.hpp"
#include "audio/AudioEngine.hpp"
#include "Instance.hpp"
#include "VideoPlayer.hpp"
#include "ConsoleUtils.hpp"
#include <string>

constexpr const char* BRIGHTNESS_LEVELS = " .-+*wGHM#&%";

int main(int argc, char* argv[])
{
	std::cout << "cmdplay++ version " << cmdplay::VERSION << std::endl;
	cmdplay::Instance::AudioEngine = new cmdplay::audio::AudioEngine(-1, 44100, 0);
	std::string filenameInput;
	std::string colorInput;
	bool enableColors = false;
	if (argc > 1)
	{
		filenameInput = std::string(argv[1]);
	}
	else
	{
		std::cout << "Enter filename: ";
		std::getline(std::cin, filenameInput);
		if (filenameInput.length() == 0)
		{
			std::cout << "Invalid filename" << std::endl;
			return EXIT_FAILURE;
		}
	}
	if (argc > 2)
	{
		colorInput = std::string(argv[2]);
	}
	else
	{
		std::cout << "Do you want colors? (can limit performance in some terminal emulators) y/n: ";
		std::getline(std::cin, colorInput);
	}
	enableColors = colorInput == "y";

	if (filenameInput.length() > 2) // Remove quotation marks if present
	{
		if (filenameInput[0] == '"')
			filenameInput = filenameInput.substr(1);
		if (filenameInput[filenameInput.length() - 1] == '"')
			filenameInput = filenameInput.substr(0, filenameInput.length() - 1);
	}

	cmdplay::VideoPlayer player(filenameInput, BRIGHTNESS_LEVELS, enableColors);
	player.LoadVideo();
	player.Enter();

	return EXIT_SUCCESS;
}