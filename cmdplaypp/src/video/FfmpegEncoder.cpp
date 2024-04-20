extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/rational.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
}

#include <iostream>
#include "FfmpegEncoder.hpp"

cmdplay::video::FfmpegEncoder::~FfmpegEncoder()
{
}

bool cmdplay::video::FfmpegEncoder::Open(const std::string& outputFilename, const EncodeParams& params)
{
	Close();

	do
	{
		avformat_alloc_output_context2(&m_context.format_context, nullptr, nullptr, outputFilename.c_str());
		if (!m_context.format_context)
		{
			std::cout << "Could not allocate output format" << std::endl;
			break;
		}

		m_context.codec = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_H264));
		if (!m_context.codec)
		{
			std::cout << "Could not find encoder" << std::endl;
			break;
		}

		m_context.stream = avformat_new_stream(m_context.format_context, nullptr);
		if (!m_context.stream)
		{
			std::cout << "Could not create stream" << std::endl;
			break;
		}
		m_context.stream->id = (int)(m_context.format_context->nb_streams - 1);

		m_context.codec_context = avcodec_alloc_context3(m_context.codec);
		if (!m_context.codec_context)
		{
			std::cout << "Could not allocate mContext codec context" << std::endl;
			break;
		}

		m_context.codec_context->codec_id = m_context.format_context->oformat->video_codec;
		m_context.codec_context->bit_rate = params.bitrate;
		m_context.codec_context->width = static_cast<int>(params.width);
		m_context.codec_context->height = static_cast<int>(params.height);
		m_context.stream->time_base = av_d2q(1.0 / params.fps, 120);
		m_context.codec_context->time_base = m_context.stream->time_base;
		m_context.codec_context->pix_fmt = params.dst_format;
		m_context.codec_context->gop_size = 12;
		m_context.codec_context->max_b_frames = 2;

		if (m_context.format_context->oformat->flags & AVFMT_GLOBALHEADER)
			m_context.codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		int ret = 0;
		if (params.preset)
		{
			ret = av_opt_set(m_context.codec_context->priv_data, "preset", params.preset, 0);
			if (ret != 0)
			{
				std::cout << "Could not set preset: " << params.preset << std::endl;
				break;
			}
		}

		{
			ret = av_opt_set_int(m_context.codec_context->priv_data, "crf", params.constantRateFactor, 0);
			if (ret != 0)
			{
				std::cout << "Could not set crf: " << params.constantRateFactor << std::endl;
				break;
			}
		}

		ret = avcodec_open2(m_context.codec_context, m_context.codec, nullptr);
		if (ret != 0)
		{
			std::cout << "Could not open codec: " << ret << std::endl;
			break;
		}

		m_context.frame = av_frame_alloc();
		if (!m_context.frame)
		{
			std::cout << "Could not allocate mContext frame" << std::endl;
			break;
		}
		m_context.frame->format = m_context.codec_context->pix_fmt;
		m_context.frame->width = m_context.codec_context->width;
		m_context.frame->height = m_context.codec_context->height;

		ret = av_frame_get_buffer(m_context.frame, 32);
		if (ret < 0)
		{
			std::cout << "Could not allocate the m_context frame data" << std::endl;
			break;
		}

		ret = avcodec_parameters_from_context(m_context.stream->codecpar, m_context.codec_context);
		if (ret < 0)
		{
			std::cout << "Could not copy the stream parameters" << std::endl;
			break;
		}

		m_context.sws_context = sws_getContext(
			m_context.codec_context->width, m_context.codec_context->height, params.src_format,   // src
			m_context.codec_context->width, m_context.codec_context->height, params.dst_format, // dst
			SWS_BICUBIC, nullptr, nullptr, nullptr
		);
		if (!m_context.sws_context)
		{
			std::cout << "Could not create SWS Context" << std::endl;
			break;
		}

		av_dump_format(m_context.format_context, 0, outputFilename.c_str(), 1);

		ret = avio_open(&m_context.format_context->pb, outputFilename.c_str(), AVIO_FLAG_WRITE);
		if (ret != 0)
		{
			std::cout << "Could not open " << outputFilename.c_str() << std::endl;
			break;
		}

		ret = avformat_write_header(m_context.format_context, nullptr);
		if (ret < 0)
		{
			std::cout << "Could not write header" << std::endl;
			ret = avio_close(m_context.format_context->pb);
			if (ret != 0)
				std::cout << "Failed to close file" << std::endl;
			break;
		}

		m_context.frame_index = 0;
		m_isOpen = true;
		return true;
	} while (false);

	Close();

	return false;
}

void cmdplay::video::FfmpegEncoder::Close()
{
	if (m_isOpen)
	{
		avcodec_send_frame(m_context.codec_context, nullptr);

		Flush();

		av_write_trailer(m_context.format_context);

		auto ret = avio_close(m_context.format_context->pb);
		if (ret != 0)
			std::cout << "Failed to close file" << std::endl;
	}

	if (m_context.sws_context)
		sws_freeContext(m_context.sws_context);

	if (m_context.frame)
		av_frame_free(&m_context.frame);

	if (m_context.codec_context)
		avcodec_free_context(&m_context.codec_context);

	if (m_context.codec_context)
		avcodec_close(m_context.codec_context);

	if (m_context.format_context)
		avformat_free_context(m_context.format_context);

	m_context = {};
	m_isOpen = false;
}

bool cmdplay::video::FfmpegEncoder::Write(const unsigned char* data)
{
	if (!m_isOpen)
		return false;

	auto ret = av_frame_make_writable(m_context.frame);
	if (ret < 0)
	{
		std::cout << "Frame unwritable" << std::endl;
		return false;
	}

	const int in_linesize[1] = { m_context.codec_context->width * 3 };

	sws_scale(
		m_context.sws_context,
		&data, in_linesize, 0, m_context.codec_context->height,  // src
		m_context.frame->data, m_context.frame->linesize // dst
	);
	m_context.frame->pts = m_context.frame_index++;

	ret = avcodec_send_frame(m_context.codec_context, m_context.frame);
	if (ret < 0)
	{
		std::cout << "Error sending frame for encoding" << std::endl;
		return false;
	}
	
	return true;
}

bool cmdplay::video::FfmpegEncoder::Flush()
{
	int ret;
	do
	{
		AVPacket packet = { 0 };

		ret = avcodec_receive_packet(m_context.codec_context, &packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;

		if (ret < 0)
		{
			std::cout << "Error encoding frame: " << ret << std::endl;
			return false;
		}

		av_packet_rescale_ts(&packet, m_context.codec_context->time_base, m_context.stream->time_base);
		packet.stream_index = m_context.stream->index;

		ret = av_interleaved_write_frame(m_context.format_context, &packet);
		av_packet_unref(&packet);
		if (ret < 0)
		{
			std::cout << "Error while writing output packet: " << ret << std::endl;
			return false;
		}
	} while (ret >= 0);

	return true;
}
