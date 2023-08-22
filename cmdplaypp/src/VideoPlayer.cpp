#include "VideoPlayer.hpp"
#include "ConsoleUtils.hpp"
#include <conio.h>
#include <iostream>

cmdplay::VideoPlayer::VideoPlayer(const std::string& filePath, const std::string& brightnessLevels) :
	m_filePath(filePath), m_brightnessLevels(brightnessLevels)
{
	ConsoleUtils::GetWindowSize(&m_windowWidth, &m_windowHeight);
	m_asciifier = std::make_unique<Asciifier>(brightnessLevels, m_windowWidth, m_windowHeight, m_colorsEnabled);
	m_decoder = std::make_unique<video::FfmpegDecoder>();
}

void cmdplay::VideoPlayer::LoadVideo()
{
	m_decoder->LoadVideo(m_filePath, m_windowWidth, m_windowHeight);
	m_audioSource = std::make_unique<audio::AudioSource>(m_filePath);
}

void cmdplay::VideoPlayer::Enter()
{
	m_audioSource->Play();

	// TODO: aint working for window terminal
	while (true)
	{
		// for some reason, windows enables the cursor every time we resize, so we should set it each time
		ConsoleUtils::ShowConsoleCursor(false);

		if (_kbhit())
		{
			char c = static_cast<char>(_getch());
			if (c == 'q')
			{
				break;
			}

			switch (c)
			{
			case ' ':
			{
				m_audioSource->PlayPause();
				break;
			}
			case 'c':
			{
				m_colorsEnabled = !m_colorsEnabled;
				m_asciifier = std::make_unique<Asciifier>(m_brightnessLevels, m_windowWidth, m_windowHeight, 
					m_colorsEnabled, m_colorDitheringEnabled, m_textDitheringEnabled);
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
				m_asciifier = std::make_unique<Asciifier>(m_brightnessLevels, m_windowWidth, m_windowHeight, 
					m_colorsEnabled, m_colorDitheringEnabled, m_textDitheringEnabled);
				break;
			}
			case 'd':
			{
				m_colorDitheringEnabled = !m_colorDitheringEnabled;
				m_asciifier = std::make_unique<Asciifier>(m_brightnessLevels, m_windowWidth, m_windowHeight,
					m_colorsEnabled, m_colorDitheringEnabled, m_textDitheringEnabled);
				break;
			}
			default: break;
			}

		}
		if (!m_audioSource->GetPlaying())
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
			m_asciifier = std::make_unique<Asciifier>(m_brightnessLevels, m_windowWidth, m_windowHeight, m_colorsEnabled);
		}

		m_decoder->SetPlaybackPosition(m_audioSource->GetPlaybackPosition() + PREBUFFER_TIME);

		// skip all frames which have a playable frame already decoded
		m_decoder->DeleteUnnecessaryFrames(m_audioSource->GetPlaybackPosition());
		auto nextFrame = m_decoder->GetNextFrame();
		if (nextFrame == nullptr)
			continue;

		// wait until our frame can be played back
		while (nextFrame->m_time > m_audioSource->GetPlaybackPosition());

		cmdplay::ConsoleUtils::SetCursorPosition(0, 0);
		std::cout << m_asciifier->BuildFrame(nextFrame->m_data);

		delete nextFrame;
	}
	ConsoleUtils::ShowConsoleCursor(true);
}
