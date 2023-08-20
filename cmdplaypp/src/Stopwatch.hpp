#pragma once
#include <chrono>
namespace cmdplay
{
	class Stopwatch
	{
	public:
		/// <summary>
		/// Constructs a new stopwatch.
		/// </summary>
		Stopwatch();

		/// <summary>
		/// Gets the elapsed time since reset or creation.
		/// </summary>
		/// <returns>The elapsed time in seconds</returns>
		double GetElapsed();

		/// <summary>
		/// Resets the elapsed time
		/// </summary>
		void Reset();

	private:
		/// <summary>
		/// Timing point used for setting the start position
		/// </summary>
		std::chrono::steady_clock::time_point start;
	};
}