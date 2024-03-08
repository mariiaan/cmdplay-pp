#pragma once
extern "C"
{
#include <libavutil/pixfmt.h>
}
#include <cstdint>

struct EncodeContext
{
	struct AVFormatContext* format_context = nullptr;
	struct AVStream* stream = nullptr;
	struct AVCodecContext* codec_context = nullptr;
	struct AVFrame* frame = nullptr;
	struct SwsContext* sws_context = nullptr;
	struct AVCodec* codec = nullptr;

	uint32_t frame_index = 0;
};