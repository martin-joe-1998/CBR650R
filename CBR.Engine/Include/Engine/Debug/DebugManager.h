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
		
		static void Initialize();

		Logger& GetLogger() noexcept { return m_logger; }
		const Logger& GetLogger() const noexcept { return m_logger; }

	private:
		DebugManager() {}
		~DebugManager() = default;

		DebugManager(const DebugManager&) = delete;
		DebugManager& operator=(const DebugManager&) = delete;

		static void OpenDebugConsole();

	private:
		Logger m_logger;
	};

	// 给 Logger.h 里声明的那个函数提供定义
	inline Logger& GetLogger() noexcept
	{
		return DebugManager::Instance().GetLogger();
	}
};