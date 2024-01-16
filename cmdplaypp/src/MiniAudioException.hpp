#pragma once
#include <string>

namespace cmdplay
{
	class MiniAudioException
	{
	private:
		std::string m_msg;

	public:
		MiniAudioException(const std::string& msg);
		std::string& GetMessage();
	};
}