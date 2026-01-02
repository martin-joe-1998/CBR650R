#include "pch.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/D3D11Renderer.h"

namespace CBR::Engine::Graphics
{
	bool Renderer::Initialize()
	{
		/// TODO: 未来根据RenderSetting可以选择11和12
		instance_ = std::make_unique<D3D11Renderer>();
		return instance_->Initialize();
	}

	void Renderer::Shutdown()
	{
		instance_.reset();
	}

	void Renderer::BeginFrame()
	{
		instance_->BeginFrame();
	}

	void Renderer::EndFrame()
	{
		instance_->EndFrame();
	}
}