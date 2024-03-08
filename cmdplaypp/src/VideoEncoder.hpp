#pragma once
#include <string>
#include "video/EncodeParams.hpp"

namespace cmdplay
{
	class VideoEncoder
	{
	public:
		VideoEncoder(const std::string& filepath, const std::string& brightnessLevels);
		
	private:
		std::string m_inputFilename;
		std::string m_brightnessLevels;

	public:
		static bool OpenSettings(std::string& outputFilename, EncodeParams& params);
		void BeginEncoding(const std::string& outputFilename, const EncodeParams& params, bool output = true);
	};
}