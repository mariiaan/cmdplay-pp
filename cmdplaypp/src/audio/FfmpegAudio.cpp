#include "FfmpegAudio.hpp"
#include <iostream>
#include <thread>
#include "../FfmpegException.hpp"
#include "../MiniAudioException.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

int frameCountComplete = 0;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	frameCountComplete += frameCount;
	cmdplay::audio::FfmpegAudio* instance = reinterpret_cast<cmdplay::audio::FfmpegAudio*>(pDevice->pUserData);
	av_audio_fifo_read(instance->m_fifo, &pOutput, frameCount);
}

cmdplay::audio::FfmpegAudio::FfmpegAudio(const std::string& path)
{
	int ret = avformat_open_input(&format_ctx, path.c_str(), nullptr, nullptr);
	if (ret < 0)
		throw FfmpegException("[Audio] Unable to open media");

	ret = avformat_find_stream_info(format_ctx, nullptr);
	if (ret < 0)
		throw FfmpegException("[Audio] Unable to find stream info");

	std::cout << "[Audio] Found streams: " << format_ctx->nb_streams << std::endl;

	streamIndex = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	if (streamIndex < 0)
		throw FfmpegException("No audio stream found in media");

	stream = format_ctx->streams[streamIndex];

	const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
	if (!decoder)
		throw FfmpegException("No decoder found");

	codec_ctx = avcodec_alloc_context3(decoder);

	avcodec_parameters_to_context(codec_ctx, stream->codecpar);

	ret = avcodec_open2(codec_ctx, decoder, nullptr);
	if (ret < 0)
		throw FfmpegException("Unable to open decoder");
}

cmdplay::audio::FfmpegAudio::~FfmpegAudio()
{
	av_audio_fifo_free(m_fifo);
	ma_device_uninit(m_device);
	delete m_deviceConfig;
	delete m_device;
}

// todo get rid of this; replace it with background thread which deocdes it within a certain buffer (around current playback time)
void cmdplay::audio::FfmpegAudio::DecodeAll()
{
	m_decodePacket = av_packet_alloc();
	m_decodeFrame = av_frame_alloc();
	m_resampler = swr_alloc();
	int ret = swr_alloc_set_opts2(&m_resampler, &stream->codecpar->ch_layout,
		AV_SAMPLE_FMT_FLT, stream->codecpar->sample_rate, &stream->codecpar->ch_layout,
		(AVSampleFormat)stream->codecpar->format, stream->codecpar->sample_rate, 0, nullptr);

	m_fifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLT, stream->codecpar->ch_layout.nb_channels, 1);

	while (0 == av_read_frame(format_ctx, m_decodePacket))
	{
		if (m_decodePacket->stream_index != streamIndex) 
			continue;
		ret = avcodec_send_packet(codec_ctx, m_decodePacket);
		if (ret < 0)
		{
			if (ret != AVERROR(EAGAIN))
			{
				throw FfmpegAudio("EAGAIN fired in unexpected spot");
			}
		}
		while ((ret = avcodec_receive_frame(codec_ctx, m_decodeFrame)) == 0)
		{
			AVFrame* resampled_frame = av_frame_alloc();
			resampled_frame->sample_rate = m_decodeFrame->sample_rate;
			resampled_frame->ch_layout = m_decodeFrame->ch_layout;

			resampled_frame->format = AV_SAMPLE_FMT_FLT;
			ret = swr_convert_frame(m_resampler, resampled_frame, m_decodeFrame);
			av_frame_unref(m_decodeFrame);
			av_audio_fifo_write(m_fifo, (void**)resampled_frame->data, resampled_frame->nb_samples);
			av_frame_free(&resampled_frame);
		}
	}
}

void cmdplay::audio::FfmpegAudio::PlayASync()
{
	ma_device_config srcConfig = ma_device_config_init(ma_device_type_playback);
	m_deviceConfig = new ma_device_config();
	std::memcpy(m_deviceConfig, &srcConfig, sizeof(ma_device_config));

	//m_deviceConfig = ma_device_config_init(ma_device_type_playback);
	m_deviceConfig->playback.format = ma_format_f32;
	m_deviceConfig->playback.channels = stream->codecpar->ch_layout.nb_channels;
	m_deviceConfig->sampleRate = stream->codecpar->sample_rate;
	m_deviceConfig->dataCallback = data_callback;
	m_deviceConfig->pUserData = this;

	avformat_close_input(&format_ctx);
	av_frame_free(&m_decodeFrame);
	av_packet_free(&m_decodePacket);
	avcodec_free_context(&codec_ctx);
	swr_free(&m_resampler);
	m_device = new ma_device();

	if (ma_device_init(NULL, m_deviceConfig, m_device) != MA_SUCCESS)
		throw MiniAudioException("Could not initialize device");
	
	if (ma_device_start(m_device) != MA_SUCCESS)
	{
		ma_device_uninit(m_device);
		throw MiniAudioException("Could not start playback device");
	}
}

void cmdplay::audio::FfmpegAudio::Pause()
{
	if (m_isPaused)
		return;
	
	ma_device_stop(m_device);
	m_isPaused = true;
}

void cmdplay::audio::FfmpegAudio::Resume()
{
	if (!m_isPaused)
		return;

	ma_device_start(m_device);
	m_isPaused = false;
}

void cmdplay::audio::FfmpegAudio::PlayPause()
{
	if (m_isPaused)
		Resume();
	else
		Pause();
}

bool cmdplay::audio::FfmpegAudio::IsPlayingBack()
{
	return av_audio_fifo_size(m_fifo) > 0;
}

float cmdplay::audio::FfmpegAudio::GetPlaybackTime()
{
	return static_cast<float>(frameCountComplete) / static_cast<float>(m_deviceConfig->sampleRate);
}
