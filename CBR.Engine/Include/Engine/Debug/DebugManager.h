#pragma once

namespace CBR::Engine::Debug
{
	class Logger;
	
	class DebugManager
	{
	public:
		static DebugManager& Instance()
		{
			static DebugManager instance;
			return instance;
		}
		
		// Main调用的
		static void Initialize();
		static void Shutdown();

		Logger& GetLogger() noexcept { return *logger_; }
		const Logger& GetLogger() const noexcept { return *logger_; }

	private:
		DebugManager();
		~DebugManager();

		DebugManager(const DebugManager&) = delete;
		DebugManager& operator=(const DebugManager&) = delete;

		void InitializeImpl();
		void ShutdownImpl();

		static void OpenDebugConsole();

	private:
		Logger* logger_ = nullptr;
		bool memoryTrackingEnabled_ = false;
	};

	// 给 Logger.h 里声明的那个函数提供定义
	inline Logger& GetLogger() noexcept;
};