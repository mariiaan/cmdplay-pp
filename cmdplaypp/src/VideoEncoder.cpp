#include "VideoEncoder.hpp"
#include "video/FfmpegDecoder.hpp"
#include "video/FfmpegEncoder.hpp"
#include <iostream>
#include <queue>
#include <conio.h>

cmdplay::VideoEncoder::VideoEncoder(const std::string& filepath, const std::string& brightnessLevels) :
	m_inputFilename(filepath), m_brightnessLevels(brightnessLevels)
{
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
	int decodedFrameCount = 0;
	std::cout << "Prebuffer time is " << preBufferTime << " seconds.\n";

	std::queue<video::DecodedFrame*> frameQueue;

	while (true)
	{
		decoder.SetPlaybackPosition(decodeTime + preBufferTime);
		auto nextFrame = decoder.GetNextFrame();
		if (nextFrame != nullptr)
		{
			++decodedFrameCount;
			std::cout << decodedFrameCount << "\n";
			frameQueue.push(nextFrame);

			//if (decodedFrameCount % 60)
			//	encoder.Flush();
			//delete nextFrame;
		}
		else if (decoder.GetEOF() && frameQueue.empty())
			break;


		if (_kbhit())
			break;
	}

	encoder.Close();

	/* while
			new Frames

			RetrieveFrame()

			AsciifyFrame()

			RenderFrameToBitmap()

			Encode / Write to output
	*/
}
