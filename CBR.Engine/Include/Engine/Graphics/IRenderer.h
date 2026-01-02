#pragma once

namespace CBR::Engine::Graphics
{
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;

		virtual bool Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
	};
};

