#pragma once

namespace CBR::Engine::Debug
{
	class DebugManager
	{
	public:
		static void Initialize();
	private:
		static void OpenDebugConsole();
	};
};