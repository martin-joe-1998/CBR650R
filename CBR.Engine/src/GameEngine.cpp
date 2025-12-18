#include "pch.h"
#include "Engine/GameEngine.h"

#ifdef _DEBUG
#include "Engine/Debug/DebugManager.h"
#endif

namespace CBR::Engine
{
	bool GameEngine::Initialize()
	{
#ifdef _DEBUG
		// Debug 模式下开启控制台，并输出测试的信息
		Debug::DebugManager::Initialize();
#endif
		return true;
	}

	bool GameEngine::Iteration()
	{
		return true;
	}

	bool GameEngine::Shutdown()
	{
#ifdef _DEBUG
		Engine::Debug::DebugManager::Shutdown();
#endif

		return true;
	}
}
