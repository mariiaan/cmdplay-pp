#include "VideoEncoder.hpp"
#include "video/FfmpegDecoder.hpp"
#include "video/FfmpegEncoder.hpp"
#include <iostream>

cmdplay::VideoEncoder::VideoEncoder(const std::string& filepath, const std::string& brightnessLevels) :
	m_inputFilename(filepath), m_brightnessLevels(brightnessLevels)
{
}

bool cmdplay::VideoEncoder::OpenSettings(std::string& outputFilename, EncodeParams& params)
{
	std::cout << "VideoEncoder Settings:\n";
	std::cout << "Output filename (folder path + filename e.g. C:\\example\\video.mp4): ";
	std::getline(std::cin, outputFilename);
	if (outputFilename.size() == 0)
		return false;
	std::cout << "Output resolution:\nWidth:";
	std::cin >> params.width;
	std::cout << "Height:";
	std::cin >> params.height;

	return true;
}

void cmdplay::VideoEncoder::BeginEncoding(const std::string& outputFilename, const EncodeParams& params, bool output)
{
	std::cout << "Initializing decoder...\n";
	// open input video
	cmdplay::video::FfmpegDecoder decoder;
	std::cout << "Loading video...\n";
	decoder.LoadVideo(outputFilename, params.width, params.height);
	// open output video
	cmdplay::video::FfmpegEncoder encoder;
	encoder.Open(outputFilename, params);

	/* while
			new Frames

			RetrieveFrame()

			AsciifyFrame()

			RenderFrameToBitmap()

			Encode / Write to output
	*/
}
