#pragma once
#include "Engine/Graphics/IRenderer.h"

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace CBR::Engine::Graphics
{
	class D3D11Renderer : public IRenderer
	{
	public:
		bool Initialize() override;
		void Shutdown() override;
		void BeginFrame() override;
		void EndFrame() override;

	private:
		HRESULT InitDevice();
		HRESULT InitSwapchain();
		HRESULT CreateRenderTargetView();
		HRESULT CreateDepthStencilView();
		void SetupViewport();
		
		bool initialized_ = false;

		D3D_DRIVER_TYPE driverType_;
		D3D_FEATURE_LEVEL featureLevel_;
		ComPtr<ID3D11Device> device_ {};
		ComPtr<ID3D11Device1> device1_ {};
		ComPtr<ID3D11DeviceContext> context_ {};
		ComPtr<ID3D11DeviceContext1> context1_ {};
		ComPtr<IDXGISwapChain> swapChain_ {};
		ComPtr<IDXGISwapChain1> swapChain1_ {};
		ComPtr<ID3D11RenderTargetView> renderTargetView_ {};
		ComPtr<ID3D11Texture2D> depthStencil_ {};
		ComPtr<ID3D11DepthStencilView> depthStencilView_ {};
	};
};

