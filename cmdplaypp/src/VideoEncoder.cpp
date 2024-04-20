#include "VideoEncoder.hpp"
#include "video/FfmpegDecoder.hpp"
#include "video/FfmpegEncoder.hpp"
#include "Asciifier.hpp"
#include <iostream>
#include <queue>
#include <conio.h>
#include <cassert>

cmdplay::VideoEncoder::VideoEncoder(const std::string& filepath, const std::string& brightnessLevels) :
	m_inputFilename(filepath), m_brightnessLevels(brightnessLevels)
{
}

void cmdplay::VideoEncoder::Render(unsigned char* targetBuf, int32_t targetWidth, int32_t targetHeight, 
	unsigned char* toRender, int32_t toRenderWidth, int32_t toRenderHeight, int32_t x, int32_t y)
{
}

void cmdplay::VideoEncoder::Scale(unsigned char* what, int32_t inWidth, int32_t inHeight, int32_t targetWidth, int32_t targetHeight,
	unsigned char* destination)
{
	assert(inWidth != 0 && inHeight != 0);
	double xScaleFactor = static_cast<double>(targetWidth) / static_cast<double>(inWidth);
	double yScaleFactor = static_cast<double>(targetHeight) / static_cast<double>(inHeight);

	for (int32_t y = 0; y < targetHeight; ++y)
		for (int32_t x = 0; x < targetWidth; ++x)
		{
			int32_t srcX = static_cast<int32_t>(x / xScaleFactor);
			int32_t srcY = static_cast<int32_t>(y / yScaleFactor);

			int32_t srcIndex = (srcY * inWidth + srcX) * 3;
			int32_t dstIndex = (y * targetWidth + x) * 3;

			destination[dstIndex] = what[srcIndex];
			destination[dstIndex + 1] = what[srcIndex + 1];
			destination[dstIndex + 2] = what[srcIndex + 2];
		}
}

bool cmdplay::VideoEncoder::OpenSettings(std::string& outputFilename, EncodeParams& params)
{
	outputFilename = "C:\\dev\\hallo.mp4";
	params.width = 1920;
	params.height = 1080;
	return true;

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

void cmdplay::VideoEncoder::BeginEncoding(const std::string& outputFilename, EncodeParams params, bool output)
{
	std::cout << "Initializing decoder...\n";
	// open input video
	video::FfmpegDecoder decoder;
	std::cout << "Loading video...\n";
	decoder.LoadVideo(m_inputFilename, params.width, params.height);
	if (decoder.ContainsAudioStream())
		std::cout << "Video contains audio streams\n";
	else
		std::cout << "Video does not contain audio streams\n";
	// open output video
	std::cout << "Initializing encoder...\n";
	video::FfmpegEncoder encoder;
	std::cout << "Writing to \"" << outputFilename << "\"\n";
	params.constantRateFactor = 23;
	params.bitrate = 5000 * 1000;
	params.fps = decoder.GetFPS();
	params.src_format = AV_PIX_FMT_RGB24;
	params.dst_format = AV_PIX_FMT_YUV420P;
	params.preset = "medium";
	
	encoder.Open(outputFilename, params);
	float decodeTime = 0.0f;
	const float preBufferTime = 2.5f;
	int32_t decodedFrameCount = 0;
	std::cout << "Prebuffer time is " << preBufferTime << " seconds.\n";

	std::cout << "Initializing asciifier...\n";
	Asciifier asciifier(m_brightnessLevels, 120, 60);


	while (true)
	{
		decoder.SetPlaybackPosition(decodeTime + preBufferTime);
		auto nextFrame = decoder.GetNextFrame();
		if (nextFrame != nullptr)
		{
			decodeTime = ++decodedFrameCount / decoder.GetFPS();
			auto asciified = asciifier.BuildFrame(nextFrame->m_data);
			delete nextFrame;

			// render frame to bitmap


			std::cout << decodedFrameCount << "\n";
			
			
			if (decodedFrameCount % 60)
				encoder.Flush();
		}
		else if (decoder.GetEOF())
			break;


		if (_kbhit())
			break;
	}
	
	encoder.Close();
}
