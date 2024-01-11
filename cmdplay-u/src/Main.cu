#include <iostream>
#include <stdio.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "Version.hpp"
#include "audio/AudioEngine.hpp"
#include "audio/AudioException.hpp"
#include "Instance.hpp"
#include "VideoPlayer.hpp"
#include "ConsoleUtils.hpp"
#include <string>

constexpr const char* BRIGHTNESS_LEVELS = " .-+*wGHM#&%";


int main(int argc, char* argv[])
{
	std::cout << "cmdplay++ version " << cmdplay::VERSION << std::endl;
	try
	{
		cmdplay::Instance::AudioEngine = new cmdplay::audio::AudioEngine(-1, 44100, 0);
	}
	catch (cmdplay::audio::AudioException& ex)
	{
		std::cout << "Failed to initialize audio! Error message: " << ex.GetExMessage() << std::endl;
	}
	std::string filenameInput;
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

	if (filenameInput.length() > 2) // Remove quotation marks if present
	{
		if (filenameInput[0] == '"')
			filenameInput = filenameInput.substr(1);
		if (filenameInput[filenameInput.length() - 1] == '"')
			filenameInput = filenameInput.substr(0, filenameInput.length() - 1);
	}

	cmdplay::VideoPlayer player(filenameInput, BRIGHTNESS_LEVELS);
	player.LoadVideo();
	player.Enter();

	return EXIT_SUCCESS;
}