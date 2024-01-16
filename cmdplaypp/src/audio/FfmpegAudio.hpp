#pragma once
#include <string>
struct ma_device_config;
struct ma_device;

extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

namespace cmdplay
{
	namespace audio
	{
		class FfmpegAudio
		{
		public:
			FfmpegAudio(const std::string& path);
			~FfmpegAudio();

		private:
			AVCodecContext* codec_ctx = nullptr;
			AVFormatContext* format_ctx = nullptr;
			int streamIndex = 0;
			AVStream* stream = nullptr;
			int m_readFrames = 0;
			AVPacket* m_decodePacket = nullptr;
			AVFrame* m_decodeFrame = nullptr;
			SwrContext* m_resampler = nullptr;
			ma_device_config* m_deviceConfig = nullptr;
			ma_device* m_device = nullptr;
			bool m_isPaused = false;

		public:
			AVAudioFifo* m_fifo = nullptr;

		public:
			void DecodeAll();
			void PlayASync();
			void Pause();
			void Resume();
			void PlayPause();
			bool IsPlayingBack();
			float GetPlaybackTime();
		};
	}
}