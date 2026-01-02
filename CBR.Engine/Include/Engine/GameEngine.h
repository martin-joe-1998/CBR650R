#pragma once

namespace CBR::Engine::Utility {
    class Timer;
}

namespace CBR::Engine
{
	class GameEngine
	{
		friend class WindowsMain;
	public:
	private:
		static bool Initialize();
		static bool Iteration();
		static void Shutdown();
		static bool IsInitialized();

		static std::unique_ptr<Utility::Timer> timer_;
	};
};

