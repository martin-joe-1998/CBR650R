#pragma once

namespace CBR
{
	class Application
	{
	protected:
		virtual bool Initialize() = 0;
		virtual void Shotdowm() = 0;
		static Application* GetInstance();
		static void DestroyInstance();
	public:
		static void RegisterInstance(Application* pApplication);
	};
};

