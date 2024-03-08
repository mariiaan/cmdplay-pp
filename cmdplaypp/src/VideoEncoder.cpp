#include "VideoEncoder.hpp"
#include <iostream>

cmdplay::VideoEncoder::VideoEncoder(const std::string& filepath, const std::string& brightnessLevels)
{
}

bool cmdplay::VideoEncoder::OpenSettings(std::string& outputFilename, int* resX, int* resY)
{
	std::cout << "VideoEncoder Settings:\n";
	std::cout << "Output filename (folder path + filename e.g. C:\\example\\video.mp4): ";
	std::getline(std::cin, outputFilename);
	if (outputFilename.size() == 0)
		return false;
	std::cout << "Output resolution:\nWidth:";
	std::cin >> *resX;
	std::cout << "Height:";
	std::cin >> *resY;

	return true;
}

void cmdplay::VideoEncoder::BeginEncoding(const std::string& outputFilename, int resX, int resY, bool output)
{
	// open input video
	// open output video

	/* while
			new Frames

			RetrieveFrame()

			AsciifyFrame()

			RenderFrameToBitmap()

			Encode / Write to output
	*/
}
