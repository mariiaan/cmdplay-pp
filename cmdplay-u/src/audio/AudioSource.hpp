#pragma once
#include <string>
#include <bass.h>
#include "SpectrumElementCount.hpp"
namespace cmdplay
{
	namespace audio
	{
		class AudioEngine;

		class AudioSource
		{
		public:
			AudioSource(bool fxEnabled);
			/// <summary>
			/// Creates a new audio source.
			/// </summary>
			/// <param name="filename">The path to the audio file</param>
			/// <param name="fxEnabled">Whether BASS FX should be used</param>
			AudioSource(const std::string& filename, bool fxEnabled = false);
			~AudioSource();

		private:
			/// <summary>
			/// Describes whether BASS FX is being used.
			/// </summary>
			bool m_fxEnabled = false;

			/// <summary>
			/// The handle to the BASS stream.
			/// </summary>
			HSTREAM m_streamHandle = -1;

			/// <summary>
			/// The handle to the BASS helper stream (used for loading audio
			/// into the BASS FX stream, when BASS FX is used).
			/// </summary>
			HSTREAM m_streamHelperHandle = -1;

			/// <summary>
			/// A reference to the audio engine.
			/// </summary>
			AudioEngine* m_audioEngine = nullptr;

			/// <summary>
			/// The base tempo.
			/// </summary>
			float m_baseTempo = 1.0f;

		public:
			void Load(const std::string& filename);
			/// <summary>
			/// Gets whether BASS FX is being used.
			/// </summary>
			/// <returns>True, if BASS FX is being used, otherwise false</returns>
			bool IsFxEnabled();

			/// <summary>
			/// Starts playback.
			/// </summary>
			void Play();

			/// <summary>
			/// Pauses playback.
			/// </summary>
			void Pause();

			/// <summary>
			/// Stops playback (pausing + position reset).
			/// </summary>
			void Stop();

			/// <summary>
			/// Toggles playback.
			/// </summary>
			void PlayPause();

			/// <summary>
			/// Sets the current playback position.
			/// </summary>
			/// <param name="value">The playback position in seconds</param>
			void SetPlaybackPosition(float value);

			/// <summary>
			/// Gets the current playback position.
			/// </summary>
			/// <returns>The playback position in seconds</returns>
			float GetPlaybackPosition();

			/// <summary>
			/// Gets the total length.
			/// </summary>
			/// <returns>The total length in seconds</returns>
			float GetTotalLength();

			/// <summary>
			/// Sets the pan.
			/// </summary>
			/// <param name="value">
			/// The desired pan while -1 is all the way left, 0 is center and
			/// 1 is all the way right
			/// </param>
			void SetPan(float value);

			/// <summary>
			/// Gets the pan. 
			/// </summary>
			/// <returns>The pan (See SetPan)</returns>
			float GetPan();

			/// <summary>
			/// Sets the playback volume of this specific source.
			/// </summary>
			/// <param name="value">The volume (0-1)</param>
			void SetVolume(float value);

			/// <summary>
			/// Gets the current playback volume of this specific source.
			/// </summary>
			/// <returns>The current playback volume (0-1)</returns>
			float GetVolume();

			/// <summary>
			/// Sets the playback state.
			/// </summary>
			/// <param name="value">True for playing, False for pausing</param>
			void SetPlaying(bool value);

			/// <summary>
			/// Gets the playback state.
			/// </summary>
			/// <returns>True for playing, False for pausing</returns>
			bool GetPlaying();

			/// <summary>
			/// Gets the playback tempo.
			/// </summary>
			/// <param name="value">
			/// 0 for stop, 1 for playing, 2 for twice the speed. Can be
			/// everything in between.
			/// </param>
			void SetTempo(float value);

			/// <summary>
			/// Gets the playback tempo.
			/// </summary>
			/// <returns>The playback tempo (See SetTempo)</returns>
			float GetTempo();

			/// <summary>
			/// Gets the handle to the BASS stream.
			/// </summary>
			/// <returns>The BASS handle</returns>
			HSTREAM GetStreamHandle();

			/// <summary>
			/// Gets the helper handle to the BASS stream.
			/// </summary>
			/// <returns>The BASS handle</returns>
			HSTREAM GetStreamHelperHandle();

			/// <summary>
			/// Gets the audio spectrum at the given playback time.
			/// </summary>
			/// <param name="targetData">The target array to fill the FFT data</param>
			/// <param name="targetDataSize">The size of the target array</param>
			/// <param name="fftSize">The desired fft size / sample count</param>
			void GetSpectrum(float* targetData, size_t targetDataSize,
				SpectrumElementCount fftSize);
		};
	}
}