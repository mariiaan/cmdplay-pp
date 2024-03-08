#pragma once
#include <string>

namespace cmdplay
{
	class VideoEncoder
	{
	public:
		VideoEncoder(const std::string& filepath, const std::string& brightnessLevels);
		
	public:
		static bool OpenSettings(std::string& outputFilename, int* resX, int* resY);
		void BeginEncoding(const std::string& outputFilename, int resX, int resY, bool output = true);
	};
}