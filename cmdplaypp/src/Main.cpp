#include <iostream>
#include "Version.hpp"
#include "VideoPlayer.hpp"
#include "MediaSource.hpp"
#include "ConsoleUtils.hpp"
#include <string>
#include <cstdlib>

#ifdef mac
#include <csignal>
#else
#include <Windows.h>
#endif

constexpr const char* BRIGHTNESS_LEVELS = " .-+*wGHM#&%";

namespace
{
	// A downloaded video is removed again when playback ends, Ctrl+C is pressed or the
	// console is closed, so downloads don't pile up. Local files are never touched.
	std::string g_downloadedFile;
	std::string g_sourceInput;

	void RemoveDownloadedFile()
	{
		cmdplay::MediaSource::Release(g_downloadedFile, g_sourceInput);
	}

#ifdef mac
	void OnInterrupt(int)
	{
		RemoveDownloadedFile();
		std::_Exit(EXIT_FAILURE);
	}
#else
	BOOL WINAPI OnConsoleEvent(DWORD event)
	{
		if (event == CTRL_C_EVENT || event == CTRL_BREAK_EVENT || event == CTRL_CLOSE_EVENT)
			RemoveDownloadedFile();

		return FALSE; // let windows terminate the process afterwards
	}
#endif

	void PrintControls()
	{
		std::cout << "\n"
			<< "  SPACE  play/pause           C  colours\n"
			<< "  D      colour dithering     T  text dithering\n"
			<< "  A      accurate colours     B  full pixel\n"
			<< "  R      restart              Q  quit (the download is deleted)\n"
			<< "\n";
	}
}

int main(int argc, char* argv[])
{
	std::cout << "CmdPlay version " << cmdplay::VERSION << std::endl;

	// Removes videos that were still in use when the previous run was interrupted
	cmdplay::MediaSource::CleanupPending();

	std::string filenameInput;
	if (argc > 1)
	{
		filenameInput = std::string(argv[1]);
	}
	else
	{
		PrintControls();
		std::cout << "Enter filename or URL: ";
		std::getline(std::cin, filenameInput);
		if (filenameInput.length() == 0)
		{
			std::cout << "Invalid filename" << std::endl;
			return EXIT_FAILURE;
		}
	}

	if (filenameInput.length() > 2) // Remove quotation marks if present
	{
		if (filenameInput[0] == '"')
			filenameInput = filenameInput.substr(1);
		if (filenameInput[filenameInput.length() - 1] == '"')
			filenameInput = filenameInput.substr(0, filenameInput.length() - 1);
	}

	// URLs are downloaded into the videos folder first, local paths are used as they are
	std::string filePath;
	try
	{
		filePath = cmdplay::MediaSource::Resolve(filenameInput);
	}
	catch (cmdplay::MediaSourceException& e)
	{
		std::cout << "[ERROR] " << e.GetMessageText() << std::endl;
		return EXIT_FAILURE;
	}

	g_downloadedFile = filePath;
	g_sourceInput = filenameInput;
#ifdef mac
	std::signal(SIGINT, OnInterrupt);
	std::signal(SIGTERM, OnInterrupt);
#else
	SetConsoleCtrlHandler(OnConsoleEvent, TRUE);
#endif

	try
	{
		bool restartRequested = false;
		do
		{
			cmdplay::VideoPlayer player(filePath, BRIGHTNESS_LEVELS);
			player.LoadVideo();
			player.Enter();
			restartRequested = player.WantsRestart();
		} while (restartRequested);
	}
	catch (...)
	{
		RemoveDownloadedFile();
		throw;
	}

	RemoveDownloadedFile();
	return EXIT_SUCCESS;
}
