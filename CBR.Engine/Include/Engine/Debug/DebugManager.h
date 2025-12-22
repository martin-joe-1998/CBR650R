#pragma once
#include "Engine/Debug/Logger.h"

namespace CBR::Engine::Debug
{	
	class DebugManager
	{
	public:
		static DebugManager& Instance()
		{
			static DebugManager instance;
			return instance;
		}
		
		static bool Initialize();
		static void Shutdown();

		Logger& GetLogger() noexcept { return logger_; }
		const Logger& GetLogger() const noexcept { return logger_; }

	private:
		DebugManager();
		~DebugManager();

		DebugManager(const DebugManager&) = delete;
		DebugManager& operator=(const DebugManager&) = delete;

		void InitializeImpl();
		void ShutdownImpl();

	private:
		Logger logger_;
		bool memoryTrackingEnabled_ = false;
	};

	// 给 Logger.h 里声明的那个函数提供定义
	inline Logger& GetLogger() noexcept;
};