#include "Stopwatch.hpp"

cmdplay::Stopwatch::Stopwatch()
{
	Reset();
}

double cmdplay::Stopwatch::GetElapsed()
{
	auto now = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() / 1000.0;
}

void cmdplay::Stopwatch::Reset()
{
	start = std::chrono::steady_clock::now();
}
