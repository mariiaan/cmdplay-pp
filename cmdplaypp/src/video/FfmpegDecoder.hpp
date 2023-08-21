#pragma once
#include "DecodedFrame.hpp"
#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <future>

extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}


namespace cmdplay
{
	namespace video
	{
		class FfmpegDecoder
		{
		public:
			/// <summary>
			/// Initializes a new video playback handler. 
			/// </summary>
			FfmpegDecoder();
			~FfmpegDecoder();

			/// <summary>
			/// Loads a video (also unloads the previous video, if still loaded).
			/// </summary>
			/// <param name="src">The path to the video file</param>
			void LoadVideo(const std::string& src, int width, int height);

			/// <summary>
			/// Unloads a video from memory.
			/// </summary>
			void UnloadVideo();

			/// <summary>
			/// Loads a video asynchronously (also unloads the previous video, if still loaded).
			/// </summary>
			/// <param name="src">The path to the video file</param>
			void LoadVideoAsync(const std::string& src, int width, int height);

			/// <summary>
			/// Asynchronously unloads a video from memory.
			/// </summary>
			void UnloadVideoAsync();

			/// <summary>
			/// Sets the playback position for the playback handler.
			/// </summary>
			/// <param name="currentTime">The desired playback position</param>
			void SetPlaybackPosition(float currentTime);

			/// <summary>
			/// Gets whether the video is ready for playback.
			/// </summary>
			/// <returns>True, if yes, otherwise false</returns>
			bool IsVideoReadyToPlay();

			/// <summary>
			/// Resets the playback position to the beginning of the video.
			/// </summary>
			void ResetPlaybackPosition();

			DecodedFrame* GetNextFrame();
			void DeleteUnnecessaryFrames(float playbackTime);

		private:
			std::mutex m_avLock;
			AVFormatContext* m_formatCtx = nullptr;
			AVCodecContext* m_codecCtxOrig = nullptr;
			AVCodecContext* m_codecCtx = nullptr;
			const AVCodec* m_codec = nullptr;
			AVFrame* m_frame = nullptr;
			AVFrame* m_frameRGB = nullptr;
			uint8_t* m_buffer = nullptr;
			SwsContext* m_swsCtx = nullptr;
			AVPacket* m_packet = nullptr;
			int m_videoStreamIndex = -1;
			int64_t m_skippedFrames = 0;

			unsigned char* m_mainThreadFramebuffer = nullptr;
			std::mutex m_mainThreadFramebufferLock;
			int m_mainThreadFramebufferSize = 0;
			int m_mainThreadFrameBufferVideoWidth = 0;
			int m_mainThreadFrameBufferVideoHeight = 0;

			std::atomic<float> m_currentTime = 0.0f;

			std::atomic<bool> m_videoLoaded = false;

			std::thread m_workerThread;
			std::atomic<bool> m_workerThreadShouldExit = false;
			std::atomic<bool> m_workerThreadRunning = false;
			std::future<void> m_asyncLoadTask;

			std::vector<DecodedFrame*> m_decodedFrames;

			std::atomic<int> m_width = 0;
			std::atomic<int> m_height = 0;

		private:
			/// <summary>
			/// The main worker thread for decoding the video.
			/// </summary>
			/// <param name="instance">The controller instance of the video playback handler</param>
			static void WorkerThread(FfmpegDecoder* instance);
		};
	}
}