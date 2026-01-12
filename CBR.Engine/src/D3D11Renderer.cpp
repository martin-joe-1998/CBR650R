#include "pch.h"
#include "Engine/Graphics/D3D11Renderer.h"
#include "Engine/Debug/Logger.h"
#include "Engine/WindowsMain.h"

namespace CBR::Engine::Graphics
{
	bool D3D11Renderer::Initialize()
	{
		assert(!initialized_);

		HRESULT hr = InitDevice();
		if (FAILED(hr))
		{
			LOG_ERROR("DX11 device failed initialization! [HRESULT = 0x" + std::to_string(hr) + "]");
			return false;
		}

		hr = InitSwapchain();
		if (FAILED(hr))
		{
			LOG_ERROR("DX11 swapchain failed initialization! [HRESULT = 0x" + std::to_string(hr) + "]");
			return false;
		}

		// Create RTV
		hr = CreateRenderTargetView();
		if (FAILED(hr))
		{
			LOG_ERROR("DX11 failed to create RTV! [HRESULT = 0x" + std::to_string(hr) + "]");
			return false;
		}

		// Create DepthStencilView
		hr = CreateDepthStencilView();
		if (FAILED(hr))
		{
			LOG_ERROR("DX11 failed to create DepthStencilView! [HRESULT = 0x" + std::to_string(hr) + "]");
			return false;
		}

		/// TODO: 函数化
		// Set RTV and DSV
		context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());

		SetupViewport();

		initialized_ = true;
		return true;
	}

	void D3D11Renderer::Shutdown()
	{
		device_.Reset();
		device1_.Reset();
		context_.Reset();
		context1_.Reset();
		swapChain_.Reset();
		swapChain1_.Reset();
		renderTargetView_.Reset();
		depthStencil_.Reset();
		depthStencilView_.Reset();

		initialized_ = false;
	}

	void D3D11Renderer::BeginFrame()
	{
		const float clearColor[4] = { 0.2f, 0.3f, 0.4f, 1.0f };
		context_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);
	}

	void D3D11Renderer::Render()
	{
		
	}

	void D3D11Renderer::EndFrame()
	{
		swapChain_->Present(0, 0);
	}

	HRESULT D3D11Renderer::InitDevice()
	{
		HRESULT hr = S_OK;

		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // 在 Debug 构建时启用 D3D11 Debug Layer（调试层）
#endif

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);
		
		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			driverType_ = driverTypes[driverTypeIndex];
			hr = D3D11CreateDevice(
				nullptr, 			      /* pAdapter           */
				driverType_, 		      /* DriverType         */
				nullptr, 			      /* Software	        */
				createDeviceFlags, 	      /* Flags              */
				featureLevels, 		      /* pFeatureLevels     */
				numFeatureLevels,	      /* FeatureLevels      */
				D3D11_SDK_VERSION, 	      /* SDKVersion         */
				device_.GetAddressOf(),   /* ppDevice           */
				&featureLevel_, 	      /* pFeatureLevel      */
				context_.GetAddressOf()   /* ppImmediateContext */
			);

			if (hr == E_INVALIDARG) // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			{
				hr = D3D11CreateDevice(
					nullptr, 
					driverType_, 
					nullptr, 
					createDeviceFlags, 
					&featureLevels[1], 
					numFeatureLevels - 1,
					D3D11_SDK_VERSION, 
					device_.GetAddressOf(), 
					&featureLevel_, 
					context_.GetAddressOf()
				);
			}

			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
			return hr;

		return hr;
	}

	HRESULT D3D11Renderer::InitSwapchain() 
	{
		HRESULT hr = S_OK;

		/// TODO: 为了将在获取GPU和显示器信息等来做更高级的引擎，可以考虑把dxgiDevice和adapter给cache起来
		// 从device获取DXGI factory
		ComPtr<IDXGIFactory1> dxgiFactory1;
		{
			ComPtr<IDXGIDevice> dxgiDevice;
			//hr = device_.Get()->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(dxgiDevice.GetAddressOf())); // 把 D3D11 Device “看成” DXGI Device，以便走 DXGI 的链路。
			hr = device_.As(&dxgiDevice);
			
			if (SUCCEEDED(hr))
			{
				ComPtr<IDXGIAdapter> adapter;
				hr = dxgiDevice->GetAdapter(adapter.GetAddressOf()); // 从 DXGI Device 取出它对应的 显卡适配器（Adapter，或者说实际在用的GPU）
				if (SUCCEEDED(hr))
				{
					//hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory1.GetAddressOf()));
					hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory1));
					adapter.Reset();
					if (FAILED(hr)) LOG_ERROR("DX11 failed to accquire IDXGIFactory1 interface! [HRESULT = 0x" + std::to_string(hr) + "]");
				}
				else 
				{
					LOG_ERROR("DX11 failed to get adapter interface! [HRESULT = 0x" + std::to_string(hr) + "]");
				}
				dxgiDevice.Reset();
			}
			else
			{
				LOG_ERROR("DX11 failed to accquire IDXGIDevice interface! [HRESULT = 0x" + std::to_string(hr) + "]");
			}
		}
		if (FAILED(hr))
			return hr;
		
		// 创建 Swapchain
		ComPtr<IDXGIFactory2> dxgiFactory2;
		//hr = dxgiFactory1->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(dxgiFactory2.GetAddressOf()));
		hr = dxgiFactory1.As(&dxgiFactory2);

		//RECT rc;
		const HWND hWnd = WindowsMain::GetMainWindowHandle();
		//GetClientRect(hWnd, &rc);
		const UINT width = WindowsMain::GetDefaultScreenWidth();
		const UINT height = WindowsMain::GetDefaultScreenHeight();

		if (dxgiFactory2.Get())
		{
			// 升级为 dx 11.1
			//hr = device_->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(device1_.GetAddressOf()));
			hr = device_.As(&device1_);
			if (SUCCEEDED(hr))
			{
				//(void)context_->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(context1_.GetAddressOf()));
				context_.As(&context1_);
			}

			DXGI_SWAP_CHAIN_DESC1 sd = {};
			sd.Width = width;
			sd.Height = height;
			sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 1;

			hr = dxgiFactory2->CreateSwapChainForHwnd(
				device_.Get(),
				hWnd, 
				&sd, 
				nullptr,
				nullptr, 
				swapChain1_.GetAddressOf());

			if (SUCCEEDED(hr))
			{
				//hr = swapChain1_->QueryInterface( __uuidof(IDXGISwapChain), reinterpret_cast<void**>(swapChain_.GetAddressOf()) );
				hr = swapChain1_.As(&swapChain_);
			}

			dxgiFactory2.Reset();
		}
		else
		{
			// DirectX 11.0 systems
			DXGI_SWAP_CHAIN_DESC sd = {};
			sd.BufferCount = 1;
			sd.BufferDesc.Width = width;
			sd.BufferDesc.Height = height;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = hWnd;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;

			hr = dxgiFactory1->CreateSwapChain(device_.Get(), &sd, swapChain_.GetAddressOf());
		}

		// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
		dxgiFactory1->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
		dxgiFactory1.Reset();

		return hr;
	}

	HRESULT D3D11Renderer::CreateRenderTargetView()
	{
		HRESULT hr;

		ComPtr<ID3D11Texture2D> backBuffer;
		hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		if (FAILED(hr))
			return hr;

		if (renderTargetView_.Get())
		{
			renderTargetView_.Reset();
		}

		hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView_.GetAddressOf());
		backBuffer.Reset();
		if (FAILED(hr))
			return hr;

		return S_OK;
	}

	HRESULT D3D11Renderer::CreateDepthStencilView()
	{
		HRESULT hr;

		D3D11_TEXTURE2D_DESC depthDesc
		{
			.Width = static_cast<UINT>(WindowsMain::GetDefaultScreenWidth()),
			.Height = static_cast<UINT>(WindowsMain::GetDefaultScreenHeight()),
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
			.SampleDesc =
			{
				.Count = 1,
				.Quality = 0,
			},
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_DEPTH_STENCIL,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
		};

		if (depthStencil_.Get())
		{
			depthStencil_.Reset();
		}

		hr = device_->CreateTexture2D(
			&depthDesc, 
			nullptr, 
			depthStencil_.GetAddressOf());
		if (FAILED(hr))
		{
			LOG_ERROR("DX11 failed to create depthStencil texture2D! [HRESULT = 0x" + std::to_string(hr) + "]");
			return hr;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc
		{
			.Format = depthDesc.Format,
			.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
			.Texture2D = {
				.MipSlice = 0,
			},
		};

		if (depthStencilView_.Get())
		{
			depthStencilView_.Reset();
		}

		hr = device_->CreateDepthStencilView(
			depthStencil_.Get(),
			&dsvDesc,
			depthStencilView_.GetAddressOf());
		if (FAILED(hr))
		{
			LOG_ERROR("DX11 failed to create DepthStencilView! [HRESULT = 0x" + std::to_string(hr) + "]");
			return hr;
		}

		return S_OK;
	}

	void D3D11Renderer::SetupViewport()
	{
		const UINT width = WindowsMain::GetDefaultScreenWidth();
		const UINT height = WindowsMain::GetDefaultScreenHeight();

		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		context_->RSSetViewports(1, &vp);
	}
}