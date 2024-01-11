#pragma once
namespace cmdplay
{
	struct RGB
	{
		float r;
		float g;
		float b;
	};

	struct HSV
	{
		float h;
		float s;
		float v;
	};

	class ColorConverter
	{
	public:
		static float GetHue(RGB in);
		static HSV RGBToHSV(RGB in);
	};
}