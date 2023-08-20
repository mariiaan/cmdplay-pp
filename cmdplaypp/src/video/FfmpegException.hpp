#pragma once
#include <string>

namespace cmdplay
{
	namespace video
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
}