#define _CRT_SECURE_NO_WARNINGS
#include "MediaSource.hpp"
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <iostream>
#include <string>

#ifdef mac
#include <sys/stat.h>
#else
#include <Windows.h>
#endif

cmdplay::MediaSourceException::MediaSourceException(const std::string& msg)
{
	m_msg = msg;
}

std::string& cmdplay::MediaSourceException::GetMessageText()
{
	return m_msg;
}

namespace
{
	constexpr const char* VIDEOS_DIRECTORY = "videos";
#ifdef mac
	constexpr char PATH_SEPARATOR = '/';
#else
	constexpr char PATH_SEPARATOR = '\\';
#endif

	std::string GetTempDirectory()
	{
#ifdef mac
		const char* tempDir = std::getenv("TMPDIR");
		return tempDir != nullptr ? std::string(tempDir) : std::string("/tmp/");
#else
		char buffer[MAX_PATH + 1] = { 0 };
		if (GetTempPathA(MAX_PATH + 1, buffer) == 0)
			return std::string(".\\");
		return std::string(buffer);
#endif
	}

	/// Directory of the executable, so downloaded videos always end up in the same place
	/// no matter which directory cmdplay was started from.
	std::string GetApplicationDirectory()
	{
#ifdef mac
		return std::string(".");
#else
		char buffer[MAX_PATH + 1] = { 0 };
		DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
		std::string executable(buffer, length);
		size_t separator = executable.find_last_of("\\/");
		return separator == std::string::npos ? std::string(".") : executable.substr(0, separator);
#endif
	}

	bool EnsureDirectory(const std::string& path)
	{
#ifdef mac
		return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#else
		return CreateDirectoryA(path.c_str(), nullptr) != 0 ||
			GetLastError() == ERROR_ALREADY_EXISTS;
#endif
	}

	/// Path of the "videos" folder next to the executable, without creating anything.
	std::string GetVideosPath()
	{
		return GetApplicationDirectory() + PATH_SEPARATOR + VIDEOS_DIRECTORY + PATH_SEPARATOR;
	}

	/// Same folder, but created when it doesn't exist yet.
	std::string GetVideosDirectory()
	{
		std::string directory = GetApplicationDirectory() + PATH_SEPARATOR + VIDEOS_DIRECTORY;
		if (!EnsureDirectory(directory))
			throw cmdplay::MediaSourceException("Could not create the videos directory: " + directory);
		return directory + PATH_SEPARATOR;
	}

	/// Files that could not be deleted yet, because the decoder still had them open.
	std::string GetPendingDeletesPath()
	{
		return GetVideosPath() + ".pending-delete";
	}

	std::string ReadFileTail(const std::string& path, long maxChars)
	{
		FILE* file = std::fopen(path.c_str(), "rb");
		if (file == nullptr)
			return "";

		std::fseek(file, 0, SEEK_END);
		long size = std::ftell(file);
		if (size <= 0)
		{
			std::fclose(file);
			return "";
		}

		long start = size > maxChars ? size - maxChars : 0;
		std::fseek(file, start, SEEK_SET);
		std::string tail(static_cast<size_t>(size - start), '\0');
		size_t read = std::fread(&tail[0], 1, tail.size(), file);
		tail.resize(read);
		std::fclose(file);
		return tail;
	}

	/// yt-dlp appends the final file path to a file while downloading, the last line is the wanted one.
	std::string ReadLastLine(const std::string& path)
	{
		std::string content = ReadFileTail(path, 8192);
		while (!content.empty() && (content.back() == '\r' || content.back() == '\n'))
			content.pop_back();

		size_t lastBreak = content.find_last_of("\r\n");
		std::string line = lastBreak == std::string::npos ? content : content.substr(lastBreak + 1);
		if (line.length() > 1 && line.front() == '"' && line.back() == '"')
			line = line.substr(1, line.length() - 2);
		return line;
	}

	/// Deletes a file given as an utf-8 path. The narrow std::remove uses the ANSI code page on
	/// windows and fails for titles with characters outside of it (☆, ｜, emoji, ...).
	bool DeleteFileUtf8(const std::string& path)
	{
#ifdef mac
		return std::remove(path.c_str()) == 0;
#else
		int length = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
		if (length <= 0)
			return false;

		std::wstring widePath(static_cast<size_t>(length), L'\0');
		if (MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &widePath[0], length) <= 0)
			return false;

		return DeleteFileW(widePath.c_str()) != 0;
#endif
	}

	bool FileExistsUtf8(const std::string& path)
	{
#ifdef mac
		return std::ifstream(path).good();
#else
		int length = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
		if (length <= 0)
			return false;

		std::wstring widePath(static_cast<size_t>(length), L'\0');
		if (MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &widePath[0], length) <= 0)
			return false;

		return GetFileAttributesW(widePath.c_str()) != INVALID_FILE_ATTRIBUTES;
#endif
	}

	int RunDownload(const std::string& url, const std::string& outputPattern, const std::string& pathFile,
		const std::string& logFile, const std::string& format, bool mergeStreams)
	{
		// Progress and status go to the console, only errors are captured for the failure message
		std::string command = "yt-dlp --no-playlist --continue --progress -f \"" + format + "\"";
		command += " -S \"res:1080,fps,codec:h264\"";
		if (mergeStreams)
			command += " --merge-output-format mp4";
		command += " -o \"" + outputPattern + "\"";
		command += " --print-to-file after_move:filepath \"" + pathFile + "\"";
		command += " \"" + url + "\" 2>> \"" + logFile + "\"";
		return std::system(command.c_str());
	}
}

bool cmdplay::MediaSource::IsUrl(const std::string& input)
{
	static const char* prefixes[] = { "http://", "https://" };
	for (const char* prefix : prefixes)
	{
		size_t prefixLength = std::string(prefix).length();
		if (input.length() <= prefixLength)
			continue;

		bool matches = true;
		for (size_t i = 0; i < prefixLength; ++i)
		{
			if (std::tolower(static_cast<unsigned char>(input[i])) != prefix[i])
			{
				matches = false;
				break;
			}
		}
		if (matches)
			return true;
	}
	return false;
}

std::string cmdplay::MediaSource::Resolve(const std::string& input)
{
	if (!IsUrl(input))
		return input;

	if (input.find('"') != std::string::npos)
		throw MediaSourceException("URLs must not contain quotation marks");

	std::string videosDir = GetVideosDirectory();
	std::string tempDir = GetTempDirectory();
	std::string stem = std::string("cmdplay-") + std::to_string(static_cast<long long>(std::time(nullptr)));
	std::string pathFile = tempDir + stem + ".path";
	std::string logFile = tempDir + stem + ".log";

	std::cout << "[INFO] Videos are saved to " << videosDir << std::endl;
	std::cout << "[INFO] Checking for yt-dlp..." << std::endl;
	std::string checkCommand = "yt-dlp --version > \"" + logFile + "\" 2>&1";
	if (std::system(checkCommand.c_str()) != 0)
	{
		std::remove(logFile.c_str());
		throw MediaSourceException(
			"yt-dlp is required to play videos from URLs, but it could not be run.\n"
			"Install it from https://github.com/yt-dlp/yt-dlp and make sure it is in PATH.");
	}

	// The version output is not part of the download log, which is only used for error messages
	std::remove(logFile.c_str());
	std::remove(pathFile.c_str());

	std::cout << "[INFO] Downloading video, this can take a while..." << std::endl;
	// Most sites only serve separate video/audio streams, so merge them first (needs ffmpeg).
	// 1080p is the target: it is the highest resolution H.264 is offered in on most sites, and
	// H.264 decodes fastest, so it wins ties at the same resolution (see the -S rule).
	bool downloadedOk = RunDownload(input, videosDir + "%(title).80B [%(id)s].%(ext)s", pathFile, logFile,
		"bv*[height<=1080]+ba[ext=m4a]/b[height<=1080]/b", true) == 0;

	if (!downloadedOk)
	{
		// Sites with a combined stream, or machines without ffmpeg, work with a single file instead
		std::cout << "[INFO] Retrying with a single-file format..." << std::endl;
		downloadedOk = RunDownload(input, videosDir + "%(title).80B [%(id)s].%(ext)s", pathFile, logFile, "b", false) == 0;
	}

	std::string downloaded = ReadLastLine(pathFile);
	if (!downloadedOk || downloaded.empty())
	{
		std::string details = ReadFileTail(logFile, 1500);
		std::remove(logFile.c_str());
		std::remove(pathFile.c_str());
		throw MediaSourceException("Downloading the video failed" +
			(details.empty() ? std::string() : (":\n" + details)));
	}

	std::remove(logFile.c_str());
	std::remove(pathFile.c_str());

	std::cout << "[INFO] Playing " << downloaded << std::endl;
	return downloaded;
}

void cmdplay::MediaSource::Release(const std::string& path, const std::string& originalInput)
{
	if (path.empty() || path == originalInput) // a file the user passed in is left alone
		return;

	if (DeleteFileUtf8(path) || !FileExistsUtf8(path))
		return;

	// The decoder still has the file open (Ctrl+C, closing the terminal), so it can only go
	// once this process is gone. Remember it and remove it on the next start.
	FILE* pending = std::fopen(GetPendingDeletesPath().c_str(), "a");
	if (pending == nullptr)
		return;

	std::fprintf(pending, "%s\n", path.c_str());
	std::fclose(pending);
}

void cmdplay::MediaSource::CleanupPending()
{
	std::string listPath = GetPendingDeletesPath();
	std::string content = ReadFileTail(listPath, 65536);
	if (content.empty())
		return;

	std::string stillLocked;
	size_t start = 0;
	while (start < content.length())
	{
		size_t end = content.find('\n', start);
		std::string line = content.substr(start, end == std::string::npos ? std::string::npos : end - start);
		start = end == std::string::npos ? content.length() : end + 1;

		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			continue;

		if (!DeleteFileUtf8(line) && FileExistsUtf8(line))
			stillLocked += line + "\n";
	}

	std::remove(listPath.c_str());

	if (stillLocked.empty())
		return;

	FILE* pending = std::fopen(listPath.c_str(), "w");
	if (pending == nullptr)
		return;

	std::fputs(stillLocked.c_str(), pending);
	std::fclose(pending);
}
