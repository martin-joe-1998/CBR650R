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
	};
};

