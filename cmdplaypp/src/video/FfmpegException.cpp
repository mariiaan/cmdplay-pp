#include "FfmpegException.hpp"

cmdplay::video::FfmpegException::FfmpegException(const std::string& msg)
{
	m_msg = msg;
}

std::string& cmdplay::video::FfmpegException::GetMessage()
{
	return m_msg;
}
