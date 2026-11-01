#pragma once
#include "Asciifier.hpp"
#include "video/FfmpegDecoder.hpp"
#include "audio/FfmpegAudio.hpp"
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
		bool m_colorsEnabled = false;
		bool m_accurateColorsEnabled = false;
		bool m_accurateColorsFullPixelEnabled = false;
		bool m_colorDitheringEnabled = true;
		std::string m_filePath;
		std::string m_brightnessLevels;
		std::unique_ptr<Asciifier> m_asciifier;
		std::unique_ptr<video::FfmpegDecoder> m_decoder;
		std::unique_ptr<audio::FfmpegAudio> m_audioSource;

		void InitAsciifier();

		/// <summary>
		/// Whether playback reached the end. The decoder finishes reading the file long before the
		/// last frame is shown, so the playback position decides, and containers that don't report
		/// a duration keep playing until the user quits.
		/// </summary>
		bool IsVideoOver(float playbackTime);

		/// <summary>
		/// Whether the player should start the video over from the beginning.
		/// </summary>
		bool m_restartRequested = false;

		/// <summary>
		/// The hotkey bar that sits in the row below the video. Toggles are green when they
		/// are on and grey when they are off.
		/// </summary>
		std::string BuildHud(bool playing);
		void DrawHud(bool playing);

	public:
		void LoadVideo();
		void Enter();

		/// <summary>
		/// True when the user asked for a restart, so the video has to be played again.
		/// </summary>
		bool WantsRestart() const;
	};
}