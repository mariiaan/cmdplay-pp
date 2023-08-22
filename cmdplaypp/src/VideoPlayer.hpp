#pragma once
#include "Asciifier.hpp"
#include "video/FfmpegDecoder.hpp"
#include "audio/AudioSource.hpp"
#include <string>
#include <memory>

namespace cmdplay
{
	constexpr float PREBUFFER_TIME = 0.5f;

	class VideoPlayer
	{
	public:
		VideoPlayer(const std::string& filePath, const std::string& brightnessLevels);

	private:
		int m_windowWidth = 0;
		int m_windowHeight = 0;
		bool m_textDitheringEnabled = true;
		bool m_colorsEnabled = true;
		bool m_colorDitheringEnabled = true;
		std::string m_filePath;
		std::string m_brightnessLevels;
		std::unique_ptr<Asciifier> m_asciifier;
		std::unique_ptr<video::FfmpegDecoder> m_decoder;
		std::unique_ptr<audio::AudioSource> m_audioSource;

	public:
		void LoadVideo();
		void Enter();
	};
}