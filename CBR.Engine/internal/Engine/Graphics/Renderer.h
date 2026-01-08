#pragma once
#include "Engine/Graphics/IRenderer.h"

namespace CBR::Engine::Graphics
{	
	/// <summary>
	///  相当于RendererManager，提供AUTOLOAD使用的静态入口，管理IRenderer的生命周期
	/// </summary>
	class Renderer
	{
	public:
		static bool Initialize();
		static void Shutdown();
		static void BeginFrame();
		static void EndFrame();
		static void Render();

	private:
		static inline std::unique_ptr<IRenderer> instance_;
	};
}