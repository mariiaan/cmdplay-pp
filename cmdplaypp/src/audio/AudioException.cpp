#include "AudioException.hpp"

cmdplay::audio::AudioException::AudioException(const std::string& msg)
{
	m_msg = msg;
}

std::string& cmdplay::audio::AudioException::GetMessage()
{
	return m_msg;
}
