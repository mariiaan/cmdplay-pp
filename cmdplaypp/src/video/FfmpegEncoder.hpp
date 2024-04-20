#pragma once

// heavily influenced by https://github.com/denesik/ffmpeg_video_encoder

extern "C"
{
#include <libavutil/pixfmt.h>
}
#include <string>
#include "EncodeParams.hpp"
#include "EncodeContext.hpp"

namespace cmdplay
{
	namespace video
	{
		class FfmpegEncoder
		{
		public:
			FfmpegEncoder() = default;
			~FfmpegEncoder();

		public:
			bool Open(const std::string& outputFilename, const EncodeParams& params);
			void Close();
			bool Write(const unsigned char* data);
			bool Flush();

		private:
			bool m_isOpen = false;
			EncodeContext m_context = { };
		};
	}
}