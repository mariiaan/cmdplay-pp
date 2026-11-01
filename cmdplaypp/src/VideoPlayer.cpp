#include "VideoPlayer.hpp"
#include "ConsoleUtils.hpp"
#include "Stopwatch.hpp"
#include "MiniAudioException.hpp"
#include <conio.h>
#include <iostream>

#define COUT_BUFFER_SIZE 1'048'576

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
		m_colorDitheringEnabled, m_textDitheringEnabled, m_accurateColorsEnabled, m_accurateColorsFullPixelEnabled);
}

void cmdplay::VideoPlayer::LoadVideo()
{
	m_decoder->LoadVideo(m_filePath, m_windowWidth, m_windowHeight);
	
		try
		{
			if (m_decoder->ContainsAudioStream())
			{
				m_audioSource = std::make_unique<audio::FfmpegAudio>(m_filePath);
				m_audioSource->DecodeAll();
			}
			else
				throw MiniAudioException("");
		}
		catch (...) { m_audioSource = nullptr; }
}

void cmdplay::VideoPlayer::Enter()
{
	if (m_audioSource != nullptr)
		m_audioSource->PlayASync();
	float syncTime = 0.0f;
	bool playing = true;
	Stopwatch syncWatch;
	bool fullRedraw = true;

	char* coutBuffer = (char*)malloc(COUT_BUFFER_SIZE * sizeof(char));
	std::cout.rdbuf()->pubsetbuf(coutBuffer, COUT_BUFFER_SIZE);
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

			fullRedraw = true;
			switch (c)
			{
			case ' ':
			{
				playing = !playing;
				if (playing)
				{
					syncWatch.Resume();
					if (m_audioSource != nullptr)
						m_audioSource->Resume();
				}
				else
				{			
					syncWatch.Pause();
					if (m_audioSource != nullptr)
						m_audioSource->Pause();
				}
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
			case 'a':
			{
				m_accurateColorsEnabled = !m_accurateColorsEnabled;
				InitAsciifier();
				fullRedraw = true;
				if (!m_accurateColorsEnabled)
				{
					// Reset colors
					std::cout << "\x1B[0m";
				}
				break;
			}
			case 'b':
			{
				m_accurateColorsFullPixelEnabled = !m_accurateColorsFullPixelEnabled;
				InitAsciifier();
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
			case 'r':
			{
				m_restartRequested = true;
				break;
			}
			default: 
				break;
			}

			if (m_restartRequested)
				break; // start over from the first frame

			DrawHud(playing);
			std::cout.flush();

		}

		if (m_audioSource != nullptr)
			syncTime = m_audioSource->GetPlaybackTime();
		else
			syncTime = syncWatch.GetElapsed();
		
		if (!playing)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
			fullRedraw = true;
		}

		m_decoder->SetPlaybackPosition(syncTime + PREBUFFER_TIME);
		// skip to the newest already decoded frame which can be shown
		m_decoder->SkipTo(syncTime);
		auto nextFrame = m_decoder->GetNextFrame();
		if (nextFrame == nullptr)
		{
			if (IsVideoOver(syncTime))
				break; // the video played to its end

			continue;
		}

		// wait until our frame can be played back
		while (nextFrame->m_time > syncTime)
		{
			if (m_audioSource == nullptr)
				syncTime = syncWatch.GetElapsed();
			else
				syncTime = m_audioSource->GetPlaybackTime();
		}
		cmdplay::ConsoleUtils::SetCursorPosition(0, 0);
		std::cout << m_asciifier->BuildFrame(nextFrame->m_data, fullRedraw);
		DrawHud(playing);
		std::cout.flush();
		fullRedraw = false;

		delete nextFrame;
	}

	std::cout.rdbuf()->pubsetbuf(nullptr, 0);
	free(coutBuffer);
	coutBuffer = nullptr;
	ConsoleUtils::ShowConsoleCursor(true);
	
}

bool cmdplay::VideoPlayer::IsVideoOver(float playbackTime)
{
	if (!m_decoder->IsFinished())
		return false;

	// The decoder reaches the end of the file as soon as it has read it, usually before the
	// last frame was shown. Only stop once the video is really over.
	// Without a known duration the video plays until the user quits.
	float duration = m_decoder->GetDuration();
	return duration > 0.0f && playbackTime >= duration - 0.25f;
}

std::string cmdplay::VideoPlayer::BuildHud(bool playing)
{
	constexpr const char* TOGGLE_ON = "[92m";  // green
	constexpr const char* TOGGLE_OFF = "[90m"; // grey
	constexpr const char* PLAIN = "[97m";
	constexpr const char* RESET = "[0m";

	struct Entry
	{
		const char* key;
		const char* label;
		bool isToggle;
		bool enabled;
	};

	Entry entries[] = {
		{ "SPACE", playing ? "pause" : "play", false, false },
		{ "C", "colour", true, m_colorsEnabled },
		{ "D", "col dith", true, m_colorDitheringEnabled },
		{ "T", "txt dith", true, m_textDitheringEnabled },
		{ "A", "true col", true, m_accurateColorsEnabled },
		{ "B", "full px", true, m_accurateColorsFullPixelEnabled },
		{ "R", "restart", false, false },
		{ "Q", "quit", false, false },
	};

	std::string hud;
	int visibleLength = 0;
	const int maxLength = m_windowWidth - 1; // never touch the last column

	for (const Entry& entry : entries)
	{
		std::string text = std::string(entry.key) + " " + entry.label;
		if (!hud.empty())
			text = "   " + text;

		if (visibleLength + static_cast<int>(text.length()) > maxLength)
			break;

		hud += entry.isToggle ? (entry.enabled ? TOGGLE_ON : TOGGLE_OFF) : PLAIN;
		hud += text;
		visibleLength += static_cast<int>(text.length());
	}

	hud += RESET;
	hud += "[K"; // wipe anything left over from a wider bar
	return hud;
}

void cmdplay::VideoPlayer::DrawHud(bool playing)
{
	cmdplay::ConsoleUtils::SetCursorPosition(0, m_windowHeight);
	std::cout << BuildHud(playing);
}

bool cmdplay::VideoPlayer::WantsRestart() const
{
	return m_restartRequested;
}
