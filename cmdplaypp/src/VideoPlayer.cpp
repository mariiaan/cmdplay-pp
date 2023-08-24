#include "VideoPlayer.hpp"
#include "audio/AudioException.hpp"
#include "ConsoleUtils.hpp"
#include "Instance.hpp"
#include "Stopwatch.hpp"
#include <conio.h>
#include <iostream>

cmdplay::VideoPlayer::VideoPlayer(const std::string& filePath, const std::string& brightnessLevels) :
	m_filePath(filePath), m_brightnessLevels(brightnessLevels)
{
	ConsoleUtils::GetWindowSize(&m_windowWidth, &m_windowHeight);
	InitAsciifier();
	m_decoder = std::make_unique<video::FfmpegDecoder>();
}

void cmdplay::VideoPlayer::InitAsciifier()
{
	m_asciifier = std::make_unique<Asciifier>(m_brightnessLevels,
		m_windowWidth, m_windowHeight, m_colorsEnabled,
		m_colorDitheringEnabled, m_textDitheringEnabled);
}

void cmdplay::VideoPlayer::LoadVideo()
{
	m_decoder->LoadVideo(m_filePath, m_windowWidth, m_windowHeight);
	if (Instance::AudioEngine != nullptr)
	{
		try
		{
			if (m_decoder->ContainsAudioStream())
				m_audioSource = std::make_unique<audio::AudioSource>(m_filePath);
			else
				throw audio::AudioException("");
		}
		catch (audio::AudioException&) { m_audioSource = nullptr; }
	}
}

void cmdplay::VideoPlayer::Enter()
{
	if (m_audioSource != nullptr)
		m_audioSource->Play();
	float syncTime = 0.0f;
	bool playing = true;
	Stopwatch syncWatch;

	while (true)
	{
		// for some reason, windows enables the cursor every time we resize, so we should set it each time
		ConsoleUtils::ShowConsoleCursor(false);

		if (_kbhit())
		{
			char c = static_cast<char>(_getch());
			if (c >= 'A' && c <= 'Z')
			{
				c += 32;
			}
			if (c == 'q')
			{
				break;
			}

			switch (c)
			{
			case ' ':
			{
				playing = !playing;
				if (m_audioSource != nullptr)
					m_audioSource->SetPlaying(playing);
				if (playing)
					syncWatch.Resume();
				else
					syncWatch.Pause();
				break;
			}
			case 'c':
			{
				m_colorsEnabled = !m_colorsEnabled;
				InitAsciifier();
				if (!m_colorsEnabled)
				{
					// Reset colors
					std::cout << "\x1B[0m";
				}
				break;
			}
			case 't':
			{
				m_textDitheringEnabled = !m_textDitheringEnabled;
				InitAsciifier();
				break;
			}
			case 'd':
			{
				m_colorDitheringEnabled = !m_colorDitheringEnabled;
				InitAsciifier();
				break;
			}
			default: 
				break;
			}

		}

		if (m_audioSource != nullptr)
			syncTime = m_audioSource->GetPlaybackPosition();
		else
			syncTime = syncWatch.GetElapsed();
		
		if (!playing)
		{
			Sleep(10);
			continue;
		}

		int newWidth = 0, newHeight = 0;
		ConsoleUtils::GetWindowSize(&newWidth, &newHeight);

		// resize everything if window size has been changed
		if (newWidth != m_windowWidth || newHeight != m_windowHeight)
		{
			m_windowWidth = newWidth;
			m_windowHeight = newHeight;
			m_decoder->Resize(m_windowWidth, m_windowHeight);
			InitAsciifier();
		}

		m_decoder->SetPlaybackPosition(syncTime + PREBUFFER_TIME);
		// skip all frames which have a playable frame already decoded
		m_decoder->SkipTo(syncTime);
		auto nextFrame = m_decoder->GetNextFrame();
		if (nextFrame == nullptr)
			continue;

		// wait until our frame can be played back
		while (nextFrame->m_time > syncTime)
		{
			if (m_audioSource == nullptr)
				syncTime = syncWatch.GetElapsed();
			else
				syncTime = m_audioSource->GetPlaybackPosition();
		}
		cmdplay::ConsoleUtils::SetCursorPosition(0, 0);
		std::cout << m_asciifier->BuildFrame(nextFrame->m_data);

		delete nextFrame;
	}
	ConsoleUtils::ShowConsoleCursor(true);
}
