#pragma once
#include <cstdint>

struct EncodeParams
{
	uint32_t width;
	uint32_t height;
	double fps;
	uint32_t bitrate;
	const char* preset;

	uint32_t crf; //0–51

	enum AVPixelFormat src_format;
	enum AVPixelFormat dst_format;
};