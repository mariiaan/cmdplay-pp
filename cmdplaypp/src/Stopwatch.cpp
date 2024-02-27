#include "Stopwatch.hpp"

cmdplay::Stopwatch::Stopwatch()
{
	Reset();
}

float cmdplay::Stopwatch::GetElapsed()
{
	auto now = std::chrono::high_resolution_clock::now();
	if (m_paused)
		return m_pauseOffset;
	else
	{
		auto duration = now - start;
		return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / 1000000000.0f + m_pauseOffset;
	}
}

void cmdplay::Stopwatch::Reset()
{
	start = std::chrono::high_resolution_clock::now();
	m_pauseOffset = 0.0;
}

void cmdplay::Stopwatch::Pause()
{
	float elapsed = GetElapsed() - m_pauseOffset;
	m_pauseOffset += elapsed;
	m_paused = true;
}

void cmdplay::Stopwatch::Resume()
{
	start = std::chrono::high_resolution_clock::now();
	m_paused = false;
}
