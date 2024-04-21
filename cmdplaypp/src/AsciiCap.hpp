#pragma once
#include <string>

class AsciiCap
{
public:
	AsciiCap(const std::string& brightnessLevels);

private:
	std::string m_brightnessLevels;

public:
	void Run();
};