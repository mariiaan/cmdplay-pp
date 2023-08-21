#include <iostream>
#include "Version.hpp"
#include "audio/AudioEngine.hpp"
#include "Instance.hpp"
#include "VideoPlayer.hpp"
#include "ConsoleUtils.hpp"
#include <string>

constexpr const char* BRIGHTNESS_LEVELS = " .-+*wGHM#&%";

int main(int argc, char* argv)
{
	std::cout << "cmdplay++ version " << cmdplay::VERSION << std::endl;
	cmdplay::Instance::AudioEngine = new cmdplay::audio::AudioEngine(-1, 44100, 0);

	std::cout << "Enter filename: ";
	std::string filenameInput;
	std::getline(std::cin, filenameInput);
	if (filenameInput.length() == 0)
	{
		std::cout << "Invalid filename" << std::endl;
		return EXIT_FAILURE;
	}

	if (filenameInput.length() > 2) // Remove quotation marks if present
	{
		if (filenameInput[0] == '"')
			filenameInput = filenameInput.substr(1);
		if (filenameInput[filenameInput.length() - 1] == '"')
			filenameInput = filenameInput.substr(0, filenameInput.length() - 1);
	}

	int cwidth = 0;
	int cheight = 0;
	cmdplay::ConsoleUtils::GetWindowSize(&cwidth, &cheight);

	cmdplay::VideoPlayer player(filenameInput, BRIGHTNESS_LEVELS, cwidth, cheight);
	player.LoadVideo();
	player.Enter();

	return EXIT_SUCCESS;
}