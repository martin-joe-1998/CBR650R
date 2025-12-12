#pragma once

namespace CBR::Engine
{
	class Application
	{
		//friend class GameEngine;
		//friend class Renderer;
	protected:
		virtual bool Initialize() = 0;
		virtual void Shotdowm() = 0;
		static Application* GetInstance();
		static void DestroyInstance(); // 在GameEngine的Shutdown里先Shutdown再摧毁实例
	public:
		static void RegisterInstance(Application* pApplication);
	};
};

