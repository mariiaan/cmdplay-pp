#include "DXScreenCapture.hpp"
#include <iostream>
#include <vector>

std::vector<HMONITOR> mons;

BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
{
	mons.push_back(hMon);
	return TRUE;
}


DXScreenCapture::DXScreenCapture()
{
	HRESULT hr;
	mons.clear();
	EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);

	hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &m_device, &m_featureLevel, &m_deviceContext);
	if (FAILED(hr))
		throw "Failed to create Direct3D device.";

	hr = m_device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&m_dxgiDevice);
	if (FAILED(hr))
		throw "Failed to obtain DXGI device.";

	hr = m_dxgiDevice->GetAdapter(&m_dxgiAdapter);
	if (FAILED(hr))
		throw "Failed to obtain DXGI adapter.";

	hr = m_dxgiAdapter->EnumOutputs(0, &m_dxgiOutput);
	if (FAILED(hr))
		throw "Failed to obtain DXGI output.";

	hr = m_dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&m_dxgiOutput1);
	if (FAILED(hr))
		throw "Failed to obtain DXGI Output1.";

	m_outputDesc = std::make_unique<DXGI_OUTPUT_DESC>();

	m_dxgiOutput->GetDesc(m_outputDesc.get());

	m_screenWidth = m_outputDesc->DesktopCoordinates.right - m_outputDesc->DesktopCoordinates.left;
	m_screenHeight = m_outputDesc->DesktopCoordinates.bottom - m_outputDesc->DesktopCoordinates.top;

	hr = m_dxgiOutput1->DuplicateOutput(m_device, &m_desktopDuplication);
	if (FAILED(hr))
		throw "Failed to initialize Desktop Duplication.";

	m_textureDesc = std::make_unique<D3D11_TEXTURE2D_DESC>();

	m_textureDesc->Width = m_screenWidth;
	m_textureDesc->Height = m_screenHeight;
	m_textureDesc->MipLevels = 1;
	m_textureDesc->ArraySize = 1;
	m_textureDesc->Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	m_textureDesc->SampleDesc.Count = 1;
	m_textureDesc->Usage = D3D11_USAGE_STAGING;
	m_textureDesc->CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	hr = m_device->CreateTexture2D(m_textureDesc.get(), nullptr, &m_copyTexture);
	if (FAILED(hr))
		throw "Failed to create the copy texture.";

	std::cout << "Initialized Direct3D screen capture!" << std::endl;

	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&m_imagingFactory);
	if (FAILED(hr))
	{
		std::cout << "ResizeFrame: Error CoCreateInstance" << std::endl;
		return;
	}

	std::cout << "Initialized WIC" << std::endl;
}

DXScreenCapture::~DXScreenCapture()
{
	if (m_device)
		m_device->Release();
	if (m_dxgiAdapter)
		m_dxgiAdapter->Release();
	if (m_dxgiOutput)
		m_dxgiOutput->Release();
	if (m_dxgiOutput1)
		m_dxgiOutput1->Release();

	m_imagingFactory->Release();
}

void DXScreenCapture::BeginLoop(const std::function<bool(bool success, uint64_t frameCounter, const void* data)>& callback)
{
	HRESULT hr = 0;
	uint64_t frameCounter = 0;
	bool run = true;
	void* destData = new unsigned char[DX_SRC_SCREEN_WIDTH * DX_SRC_SCREEN_HEIGHT * 4];
	while (run)
	{
		IDXGIResource* desktopResource = nullptr;
		DXGI_OUTDUPL_FRAME_INFO frameInfo;
		hr = m_desktopDuplication->AcquireNextFrame(500, &frameInfo, &desktopResource);
		if (FAILED(hr))
		{
			std::cout << "Failed to acquire next frame!" << std::endl;
			continue;
		}

		if (frameInfo.AccumulatedFrames > 0)
		{
			ID3D11Texture2D* texture = nullptr;
			hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);
			if (SUCCEEDED(hr))
			{
				m_deviceContext->CopyResource(m_copyTexture, texture);
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				hr = m_deviceContext->Map(m_copyTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
				if (SUCCEEDED(hr))
				{
					WICResizeFrame(mappedResource.pData, m_screenWidth, m_screenHeight, destData, m_outputResX, m_outputResY);
					uint8_t* newBuffer = (uint8_t*)malloc(3 * m_screenWidth * m_screenHeight);
					uint8_t* frame = (uint8_t*)destData;

					size_t nbi = 0;
					for (size_t i = 0; i < 4 * m_outputResX * m_outputResY; i += 4)
					{
						uint8_t blue = frame[i];
						uint8_t green = frame[i + 1];
						uint8_t red = frame[i + 2];
						newBuffer[nbi++] = red;
						newBuffer[nbi++] = green;
						newBuffer[nbi++] = blue;
					}

					run = callback(true, frameCounter, newBuffer);
					free(newBuffer);
					m_deviceContext->Unmap(m_copyTexture, 0);
					++frameCounter;
				}
				else
					run = callback(false, frameCounter, nullptr);

				texture->Release();
			}
			else
				run = callback(false, frameCounter, nullptr);

			desktopResource->Release();
		}

		m_desktopDuplication->ReleaseFrame();
	}
	delete[] destData;
}

int32_t DXScreenCapture::GetScreenWidth()
{
	return m_screenWidth;
}

int32_t DXScreenCapture::GetScreenHeight()
{
	return m_screenHeight;
}

void DXScreenCapture::WICResizeFrame(const void* srcData, int32_t sourceWidth, int32_t sourceHeight, void* destData, int32_t destWidth, int32_t destHeight)
{
	HRESULT hr;
	// Initialize WIC

	// Create the source bitmap
	IWICBitmap* pSrcBitmap = NULL;
	m_imagingFactory->CreateBitmapFromMemory(sourceWidth, sourceHeight, GUID_WICPixelFormat32bppBGRA,
		sourceWidth * 4, sourceWidth * sourceHeight * 4, (BYTE*)srcData, &pSrcBitmap);

	// Create the destination bitmap
	IWICBitmapScaler* pScaler = NULL;
	m_imagingFactory->CreateBitmapScaler(&pScaler);
	pScaler->Initialize(pSrcBitmap, destWidth, destHeight, WICBitmapInterpolationModeLinear);

	// Copy the resized data to the destination buffer
	pScaler->CopyPixels(NULL, destWidth * 4, destWidth * destHeight * 4, (BYTE*)destData);

	// Release WIC resources
	pSrcBitmap->Release();
	pScaler->Release();
}

void DXScreenCapture::SetOutputResolution(int32_t x, int32_t y)
{
	m_outputResX = x;
	m_outputResY = y;
}
