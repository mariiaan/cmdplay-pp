#include "AsciiCap.hpp"
#include "DXScreenCapture.hpp"
#include "Asciifier.hpp"
#include "ConsoleUtils.hpp"
#include <cstdint>
#include <iostream>

AsciiCap::AsciiCap(const std::string& brightnessLevels) : m_brightnessLevels(brightnessLevels)
{
}

void AsciiCap::Run()
{
	using namespace cmdplay;

	int w;
	int h;
	ConsoleUtils::GetWindowSize(&w, &h);

	DXScreenCapture dxs;
	dxs.SetOutputResolution(w, h);
	Asciifier ascii(m_brightnessLevels, w, h, false, false, true, false, false);

	dxs.BeginLoop([&](bool success, uint64_t frameCounter, const void* data)
		{
			if (success)
			{
				auto asciified = ascii.BuildFrame(static_cast<const uint8_t*>(data));
				ConsoleUtils::SetCursorPosition(0, 0);
				std::cout << asciified;
			}
			int w2;
			int h2;
			ConsoleUtils::GetWindowSize(&w2, &h2);
			if (w2 != w || h2 != h)
			{
				w = w2;
				h = h2;
				ascii = Asciifier(m_brightnessLevels, w, h, true, true, true, false, false);
				dxs.SetOutputResolution(w, h);
			}
			return true;
		});
}
