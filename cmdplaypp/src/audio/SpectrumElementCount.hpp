#pragma once
namespace cmdplay
{
	namespace audio
	{
		/// <summary>
		/// Defines the size of an FFT
		/// </summary>
		enum class SpectrumElementCount
		{
			FFT_128,
			FFT_256,
			FFT_512,
			FFT_1024,
			FFT_2048,
			FFT_4096
		};
	}
}