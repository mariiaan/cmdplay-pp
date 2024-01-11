#pragma once

namespace cmdplay
{
	namespace audio
	{
		class AudioEngine;
	}

	/// <summary>
	/// Class used for holding current instances
	/// </summary>
	class Instance
	{
	public:
		static audio::AudioEngine* AudioEngine;
	};
}