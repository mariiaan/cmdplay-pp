#include "MiniAudioException.hpp"

cmdplay::MiniAudioException::MiniAudioException(const std::string& msg)
{
	m_msg = msg;
}

std::string& cmdplay::MiniAudioException::GetMessage()
{
	return m_msg;
}
