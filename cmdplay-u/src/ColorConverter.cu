#include "ColorConverter.cuh"
#include <cmath>

float cmdplay::ColorConverter::GetHue(RGB in)
{
    return RGBToHSV(in).h;
}

cmdplay::HSV cmdplay::ColorConverter::RGBToHSV(RGB in)
{
    HSV out;
    float min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min < in.b ? min : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max > in.b ? max : in.b;

    out.v = max;                             
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; 
        return out;
    }
    if (max > 0.0) 
    { 
        out.s = (delta / max);                 
    }
    else
    {
        out.s = 0.0;
        out.h = NAN;                            
        return out;
    }
    if (in.r >= max)                           
        out.h = (in.g - in.b) / delta;      
    else
        if (in.g >= max)
            out.h = 2.0f + (in.b - in.r) / delta;
        else
            out.h = 4.0f + (in.r - in.g) / delta; 

    out.h *= 60.0f;                         

    if (out.h < 0.0)
        out.h += 360.0f;

    return out;
}
