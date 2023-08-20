#pragma once
#include <string>

namespace cmdplay
{
	namespace audio
	{
		class AudioException
		{
		private:
			std::string m_msg;

		public:
			AudioException(const std::string& msg);
			std::string& GetMessage();
		};
	}
}