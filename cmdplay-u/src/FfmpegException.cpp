#include "FfmpegException.hpp"

cmdplay::FfmpegException::FfmpegException(const std::string& msg)
{
	m_msg = msg;
}

std::string& cmdplay::FfmpegException::GetMessage()
{
	return m_msg;
}
