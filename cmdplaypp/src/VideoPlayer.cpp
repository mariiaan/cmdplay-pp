#include "VideoPlayer.hpp"
#include "ConsoleUtils.hpp"
#include <iostream>

cmdplay::VideoPlayer::VideoPlayer(const std::string& filePath, const std::string& brightnessLevels, bool enableColors) :
	m_filePath(filePath), m_brightnessLevels(brightnessLevels), 
	m_colorsEnabled(enableColors)
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
	SHORT lastSpaceKeyState = GetAsyncKeyState(VK_SPACE);
	SHORT lastEscapeKeyState = GetAsyncKeyState(VK_ESCAPE);
	SHORT lastReturnKeyState = GetAsyncKeyState(VK_RETURN);

	while (true)
	{
		// for some reason, windows enables the cursor every time we resize, so we should set it each time
		ConsoleUtils::ShowConsoleCursor(false);

		SHORT currentSpaceKeyState = GetAsyncKeyState(VK_SPACE);
		SHORT currentEscapeKeyState = GetAsyncKeyState(VK_ESCAPE);
		SHORT currentReturnKeyState = GetAsyncKeyState(VK_RETURN);

		// we need to check if the input is meant for the console window, 
		// GetAsyncKeyState catches every systemwide input
		if (ConsoleUtils::GetWindowFocused())
		{
			if (lastSpaceKeyState != currentSpaceKeyState)
			{
				lastSpaceKeyState = currentSpaceKeyState;
				if (currentSpaceKeyState & 0x01)
					m_audioSource->PlayPause();
			}
			if (lastEscapeKeyState != currentEscapeKeyState)
			{
				lastEscapeKeyState = currentEscapeKeyState;
				if (currentEscapeKeyState & 0x01)
					break;
			}
			if (lastReturnKeyState != currentReturnKeyState)
			{
				lastReturnKeyState = currentReturnKeyState;

				// do this to circumvent the problem, that the user probably pressed enter
				// right before playback
				if (currentReturnKeyState & 0x01 && m_audioSource->GetPlaybackPosition() > 0.5f)
				{
					m_colorsEnabled = !m_colorsEnabled;
					m_asciifier = std::make_unique<Asciifier>(m_brightnessLevels, m_windowWidth, m_windowHeight, m_colorsEnabled);
					if (!m_colorsEnabled)
					{
						// Reset colors
						std::cout << "\x1B[0m";
					}
				}
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
