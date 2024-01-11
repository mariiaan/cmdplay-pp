#include "AudioException.hpp"

cmdplay::audio::AudioException::AudioException(const std::string& msg)
{
	m_msg = msg;
}

std::string& cmdplay::audio::AudioException::GetExMessage()
{
	return m_msg;
}
