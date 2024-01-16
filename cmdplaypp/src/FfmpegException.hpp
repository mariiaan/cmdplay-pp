#pragma once
#include <string>

namespace cmdplay
{
	class FfmpegException
	{
	private:
		std::string m_msg;

	public:
		FfmpegException(const std::string& msg);
		std::string& GetMessage();
	};
}