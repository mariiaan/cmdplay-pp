#pragma once
#include <bass.h>
#include <bass_fx.h>
#include <string>
#include "SpectrumElementCount.cuh"

namespace cmdplay
{
	namespace audio
	{
		class AudioEngine
		{
			friend class AudioSource;

		public:
			/// <summary>
			/// Constructs a new Audio Engine.
			/// </summary>
			/// <param name="device">The playback device</param>
			/// <param name="samplerate">The sample rate of the audio output</param>
			/// <param name="windowHwnd">The associated window handle (win32)</param>
			AudioEngine(int32_t device, int32_t samplerate, HWND windowHwnd);
			~AudioEngine();

		private:
			/// <summary>
			/// Describes the sample rate of the audio output.
			/// </summary>
			int32_t m_sampleRate = 0;

		private:	// AudioSource interop
			/// <summary>
			/// Creates/Loads a BASS stream from a file.
			/// </summary>
			/// <param name="filename">The source path to an audio file</param>
			/// <param name="flags">Any legacy BASS flags</param>
			/// <returns>The BASS stream handle</returns>
			HSTREAM CreateBassStreamFromFile(const std::string& filename,
				DWORD flags = 0);

			/// <summary>
			/// Wraps a new BASS FX stream around an existing BASS stream.
			/// </summary>
			/// <param name="src">The source BASS handle</param>
			/// <returns>The handle to the new BASS FX stream</returns>
			HSTREAM CreateBassFxTempoStream(HSTREAM src, DWORD flags = BASS_FX_FREESOURCE);

			/// <summary>
			/// Frees an existing BASS stream from memory (releases all
			/// resources aquired by the stream).
			/// </summary>
			/// <param name="src">The source handle to the BASS stream</param>
			void DeleteBassStream(HSTREAM src);

			/// <summary> 
			/// Checks for BASS related errors and throws an appropriate exception 
			/// if it finds one. 
			/// </summary>
			/// <param name="source">
			/// Absolute code path. 
			/// For example: "neoclient::engine::audio::AudioEngine::CheckAndThrowError" 
			/// </param>
			void CheckAndThrowError(const std::string& source);

			/// <summary>
			/// Gets the FFT spectrum of a BASS stream at current
			/// playback position.
			/// </summary>
			/// <param name="channel">The handle to the BASS stream</param>
			/// <param name="target">The array to fill in the data</param>
			/// <param name="targetSize">The size of target</param>
			/// <param name="size">The requested FFT size (sample/detail count)</param>
			void GetBassChannelSpectrum(HCHANNEL channel, float* target,
				size_t targetSize, SpectrumElementCount size);

		private:	// Common interop
			/// <summary>
			/// Plays a BASS channel.
			/// </summary>
			/// <param name="src">The handle to the BASS channel</param>
			void PlayBassChannel(DWORD src);

			/// <summary>
			/// Pauses a BASS channel.
			/// </summary>
			/// <param name="src">The handle of the BASS channel</param>
			void PauseBassChannel(DWORD src);

			/// <summary>
			/// Stops the playback of a BASS channel.
			/// </summary>
			/// <param name="src">The handle of the BASS channel</param>
			void StopBassChannel(DWORD src);

			/// <summary>
			/// Sets the playback tempo (only works for BASS FX channels) of a channel.
			/// 0 = stop, 1 = normal speed, 2 = double the speed
			/// (works for every value in betweeen)
			/// </summary>
			/// <param name="src">The handle to the BASS channel</param>
			/// <param name="tempo">The desired tempo</param>
			void SetTempoChannel(DWORD src, float tempo);

			/// <summary>
			/// Gets the playback tempo of a channel.
			/// </summary>
			/// <param name="src">The handle to the BASS channel</param>
			/// <returns>The tempo (See SetTempoChannel)</returns>
			float GetTempoChannel(DWORD src);

			/// <summary>
			/// Gets the sample rate of a channel.
			/// </summary>
			/// <param name="src">The handle to the BASS channel</param>
			/// <returns>The sample rate</returns>
			int GetSampleRateStream(DWORD src);

			/// <summary>
			/// Sets the volume of a BASS channel.
			/// </summary>
			/// <param name="src">The handle to the BASS channel</param>
			/// <param name="volume">The desired volume (1 = normal volume)</param>
			void SetVolumeChannel(DWORD src, float volume);

			/// <summary>
			/// Gets the volume of a BASS channel.
			/// </summary>
			/// <param name="src">The handle to the BASS channel</param>
			/// <returns>The tempo of the channel (See SetVolumeChannel)</returns>
			float GetVolumeChannel(DWORD src);

			/// <summary>
			/// Sets the pan of a BASS channel.
			/// (-1 = all the way left, 0 = center, 1 = all the way right,
			/// can be any value in between)
			/// </summary>
			/// <param name="src">The handle of the BASS channel</param>
			/// <param name="pan">The desired pan</param>
			void SetPanChannel(DWORD src, float pan);

			/// <summary>
			/// Gets the pan of a BASS channel.
			/// </summary>
			/// <param name="src">The handle to the BASS channel</param>
			/// <returns>The pan (See SetPanChannel)</returns>
			float GetPanChannel(DWORD src);

			/// <summary>
			/// Sets the playback position of a BASS channel.
			/// </summary>
			/// <param name="src">The handle to the BASS channel</param>
			/// <param name="pos">The desired position in seconds</param>
			void SetPlaybackPositionChannel(DWORD src, float pos);

			/// <summary>
			/// Gets the current playback position of a BASS channel.
			/// </summary>
			/// <param name="src">The handle of the BASS channel</param>
			/// <returns>The current playback position in seconds</returns>
			float GetPlaybackPositionChannel(DWORD src);

			/// <summary>
			/// Gets the playback position of a BASS channel.
			/// </summary>
			/// <param name="src">The handle of the BASS channel</param>
			/// <returns>The current playback position in bytes</returns>
			QWORD GetBytePlaybackPositionChannel(DWORD src);

			/// <summary>
			/// Gets the length of a BASS channel.
			/// </summary>
			/// <param name="src">The handle of the BASS channel</param>
			/// <returns>The length of a BASS channel in seconds</returns>
			float GetLengthChannel(DWORD src);

			/// <summary>
			/// Gets the length of a BASS channel.
			/// </summary>
			/// <param name="src">The handle of the BASS channel</param>
			/// <returns>The length of a BASS channel in bytes</returns>
			QWORD GetByteLengthChannel(DWORD src);

			/// <summary>
			/// Gets whether the channel is playing at the given time.
			/// </summary>
			/// <param name="src">The handle of the BASS channel</param>
			/// <returns>
			/// True, if the channel is currently playing, otherwise false.
			/// </returns>
			bool IsPlayingChannel(DWORD src);

		private:	// SampleSource interop
			/// <summary>
			/// Creates/Loads a BASS sample from a file. Can be used 
			/// for playing back a sample multiple times at once with low
			/// latency.
			/// </summary>
			/// <param name="filename">The path to the audio file</param>
			/// <param name="maxPlaybacks">
			/// The maximum amount of simultaneous playbacks
			/// </param>
			/// <param name="flags">Any legacy BASS flags</param>
			/// <returns>The handle of the BASS sample</returns>
			HSAMPLE CreateBassSampleFromFile(const std::string& filename, int32_t maxPlaybacks = 8, DWORD flags = 0);

			/// <summary>
			/// Deletes a BASS sample
			/// </summary>
			/// <param name="src">The handle to the BASS sample</param>
			void DeleteBassSample(HSAMPLE src);

			/// <summary>
			/// Creates a BASS channel from a BASS sample.
			/// This is the thing to play when wanting to play back
			/// BASS samples.
			/// </summary>
			/// <param name="src">The handle to the BASS sample</param>
			/// <returns>The handle of a new sample based BASS channel</returns>
			HCHANNEL CreateBassSampleChannel(HSAMPLE src);

		public:
			/// <summary>
			/// Gets the current-set sample rate of the output device.
			/// </summary>
			/// <returns>The current-set sample rate</returns>
			int GetSampleRate();

			/// <summary>
			/// Updates the audio engine (freshing buffers, etc.)
			/// </summary>
			void Update();

			/// <summary>
			/// Sets the master output volume of the output device.
			/// </summary>
			/// <param name="value">The desired volume (0 = silent, 1 = max)</param>
			void SetVolume(float value);

			/// <summary>
			/// Gets the master output volume of the output device.
			/// </summary>
			/// <returns>The volume (See SetVolume)</returns>
			float GetVolume();
		};
	}
}
