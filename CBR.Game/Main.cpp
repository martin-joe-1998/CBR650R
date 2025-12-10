#include "pch.h"
#ifdef _DEBUG
#include "Engine/Debug/DebugManager.h"
#endif
#include "Engine/WindowsMain.h"

using namespace CBR;
using namespace CBR::Engine;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // TODO: 移动到未来GameEngine类的Initialize里
#ifdef _DEBUG
    // Debug 模式下开启控制台，并输出测试的信息
    Engine::Debug::DebugManager::Initialize();
#endif

    // TODO: 移动到WindowsMain::Run里
    // 通过constructor创建窗口
    //LOG_INFO("Create Window.");
	WindowsMain* pWindow = new WindowsMain(hInstance, nCmdShow);

    bool running = true;
    while (running)
    {
        if (!pWindow->ProcessMessages())
        {
            //LOG_INFO("Close Window.");
            running = false;
        }

        // Render

        Sleep(10);
    }
    
    delete pWindow;

    // TODO: 移动到WindowsMain::Run里
#ifdef _DEBUG
    Engine::Debug::DebugManager::Shutdown();
#endif
    
    return 0;
}