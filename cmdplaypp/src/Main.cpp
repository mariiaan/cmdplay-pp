#include <iostream>
#include "Version.hpp"
#include "VideoPlayer.hpp"
#include "VideoEncoder.hpp"
#include "ConsoleUtils.hpp"
#include <string>

constexpr const char* BRIGHTNESS_LEVELS = " .-+*wGHM#&%";

int main(int argc, char* argv[])
{
	std::cout << "cmdplay++ version " << cmdplay::VERSION << std::endl;

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

	std::cout << "1. Play video\n2. Export asciified video\nSelect Option: ";
	int option;
	std::cin >> option;
	switch (option)
	{
	case 1:
	{
		cmdplay::VideoPlayer player(filenameInput, BRIGHTNESS_LEVELS);
		player.LoadVideo();
		player.Enter();
		break;
	}
	case 2:
	{
		std::string outputFilename;
		int resX;
		int resY;
		if (!cmdplay::VideoEncoder::OpenSettings(outputFilename, &resX, &resY))
			break;
		cmdplay::VideoEncoder encoder(filenameInput, BRIGHTNESS_LEVELS);
		encoder.BeginEncoding(outputFilename, resX, resY);
	}
	default:
		std::cout << "Invalid option!\n";
		break;
	}
	
	std::cout << "EXIT_SUCCESS\n";
	return EXIT_SUCCESS;
}