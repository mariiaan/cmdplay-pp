#pragma once
#include <stdint.h>
namespace cmdplay
{
	namespace video
	{
		struct DecodedFrame
		{
			float m_time;
			uint8_t* m_data;
			int m_dataLength;

			DecodedFrame(int dataLength, float time) 
			{
				m_data = new uint8_t[dataLength];
				m_dataLength = dataLength;
				m_time = time;
			}

			DecodedFrame(const DecodedFrame& f)
			{
				m_time = f.m_time;
				m_dataLength = f.m_dataLength;
				m_data = new uint8_t[m_dataLength];
				for (int i = 0; i < m_dataLength; ++i)
					m_data[i] = f.m_data[i];
			}

			~DecodedFrame()
			{
				delete m_data;
			}
		};
	}
}