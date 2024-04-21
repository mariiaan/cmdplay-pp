#include "COMRAII.hpp"
#include <objbase.h>

COMRAII::COMRAII()
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
		throw "Failed to initialize COM";

}

COMRAII::~COMRAII()
{
	CoUninitialize();
}
