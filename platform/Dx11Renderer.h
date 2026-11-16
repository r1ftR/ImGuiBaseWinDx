#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace platform
{
class Dx11Renderer
{
public:
	bool Initialize(HWND window, UINT width, UINT height);
	void BeginFrame();
	void RenderFrame();
	void Resize(UINT width, UINT height);
	void Shutdown();

private:
	bool CreateRenderTarget();
	void CleanupRenderTarget();

	Microsoft::WRL::ComPtr<ID3D11Device> device_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget_;
	bool imguiInitialized_ = false;
};
}

