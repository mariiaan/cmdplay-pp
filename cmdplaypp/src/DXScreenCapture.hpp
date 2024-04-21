#pragma once
#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>
#include <functional>
#include <wincodec.h>

constexpr int32_t DX_SRC_SCREEN_WIDTH = 1280;
constexpr int32_t DX_SRC_SCREEN_HEIGHT = 720;

class DXScreenCapture
{
public:
	DXScreenCapture();
	~DXScreenCapture();
	void BeginLoop(const std::function<bool(bool success, uint64_t frameCounter, const void* data)>& callback);
	int32_t GetScreenWidth();
	int32_t GetScreenHeight();

private:
	int32_t m_screenWidth = 1;
	int32_t m_screenHeight = 1;
	int32_t m_outputResX = 1280;
	int32_t m_outputResY = 720;

private: // d3d setuo
	D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	IDXGIDevice1* m_dxgiDevice = nullptr;
	IDXGIAdapter* m_dxgiAdapter = nullptr;
	IDXGIOutput* m_dxgiOutput = nullptr;
	IDXGIOutput1* m_dxgiOutput1 = nullptr;
	std::unique_ptr<DXGI_OUTPUT_DESC> m_outputDesc;
	IDXGIOutputDuplication* m_desktopDuplication = nullptr;
	std::unique_ptr<D3D11_TEXTURE2D_DESC> m_textureDesc;
	ID3D11Texture2D* m_copyTexture = nullptr;

private: // wic
	IWICImagingFactory* m_imagingFactory = nullptr;

public:
	void WICResizeFrame(const void* srcData, int32_t sourceWidth, int32_t sourceHeight, void* destData,
		int32_t destWidth, int32_t destHeight);
	void SetOutputResolution(int32_t x, int32_t y);
};