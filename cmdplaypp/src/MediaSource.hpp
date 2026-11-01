#pragma once
#include <string>

namespace cmdplay
{
	class MediaSourceException
	{
	private:
		std::string m_msg;

	public:
		MediaSourceException(const std::string& msg);
		std::string& GetMessageText();
	};

	class MediaSource
	{
	public:
		/// <summary>
		/// Whether the input looks like a web URL (http:// or https://).
		/// </summary>
		static bool IsUrl(const std::string& input);

		/// <summary>
		/// Turns user input into a local file path. Local paths are returned unchanged,
		/// URLs are downloaded into the videos folder using yt-dlp and that path is returned.
		/// </summary>
		/// <param name="input">A local file path or a video URL</param>
		/// <exception cref="MediaSourceException">When the URL could not be downloaded</exception>
		static std::string Resolve(const std::string& input);

		/// <summary>
		/// Deletes a file that Resolve downloaded. Local files given by the user are left alone.
		/// If the decoder still holds the file open (for example after Ctrl+C), the file is
		/// remembered and removed by the next call to CleanupPending.
		/// </summary>
		static void Release(const std::string& path, const std::string& originalInput);

		/// <summary>
		/// Deletes downloads that could not be removed at the end of the previous run.
		/// </summary>
		static void CleanupPending();
	};
}
