#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/Configuration.h"
#include "Engine/Application.h"

#if CBR_USE_DEBUG_MANAGER
#include "Engine/Debug/DebugManager.h"
#endif

// 参考Keycon的结构，Initializer和Finalizer分别是各个Manager的初始化和关闭函数。
using Initializer = bool(*)(void);
using Finalizer = void(*)(void);
#define AUTOLOAD(type) { type::Initialize, type::Shutdown }

static std::vector<Finalizer> finalizers;

static bool initialized = false;

namespace CBR::Engine
{
	bool GameEngine::Initialize()
	{
		// 用initializer和finalizer作为各个Manager的初始化和关闭函数的指针
		const struct
		{
			Initializer initializer;
			Finalizer finalizer;
		} singletons[] = {
			/// TODO: 把DebugManager是否启用用宏定义在Configuration.h里
#if CBR_USE_DEBUG_MANAGER
			AUTOLOAD(Debug::DebugManager), // Debug 模式下开启控制台，并输出测试的信息
#endif
		};

		for (const auto& singleton : singletons)
		{
			if (!singleton.initializer())
			{
				return false;
			}
			else 
			{
				finalizers.emplace_back(singleton.finalizer);
			}
		}

		// 最后初始化CBRGame的实例
		if (!Application::GetInstance()->Initialize())
		{
			return false;
		}

		LOG_INFO("CBR engine initialized Successfully!");

		initialized = true;
		return true;
	}

	bool GameEngine::Iteration()
	{
		return true;
	}

	void GameEngine::Shutdown()
	{
		/// TODO: 按照初始化顺序反向shutdown,先shutdown Application
		Application::GetInstance()->Shotdowm();
		Application::DestroyInstance();

		for (const auto& finalizer : finalizers | std::views::reverse)
		{
			finalizer();
		}
		finalizers.clear();

		LOG_INFO("CBR engine Shutdown!");

		LOG_INFO("Press Enter to exit...");
		std::cin.get();
	}

	bool GameEngine::IsInitialized()
	{
		return initialized;
	}
}
