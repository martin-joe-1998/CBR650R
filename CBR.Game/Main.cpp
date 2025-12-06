#include "pch.h"
#ifdef _DEBUG
#include "CBR.Engine/Debug.cpp"
#endif
#include "CBR.Engine/Logger.h"
#include "CBR.Engine/MemoryLT.h"
#include "CBR.Engine/WindowsMain.h"

using namespace CBR;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
#ifdef _DEBUG
    // Debug 模式下开启控制台，并输出测试的信息
	Engine::Debug::OpenDebugConsole();
    LOG_INFO("INFO.");
    LOG_WARN("WARN.");
    LOG_ERROR("ERROR.");
    LOG_DEBUG("DEBUG");

    // 开启内存泄漏检测
    Engine::Debug::Init(true, 256);
#endif

    // 通过constructor创建窗口
    LOG_INFO("Create Window.");
	WindowsMain* pWindow = new WindowsMain(hInstance, nCmdShow);

    bool running = true;
    while (running)
    {
        if (!pWindow->ProcessMessages())
        {
            LOG_INFO("Close Window.");
            running = false;
        }

        // Render

        Sleep(10);
    }
    
    delete pWindow;

#ifdef _DEBUG
    // 关闭内存泄漏检测
    Engine::Debug::Close();

    LOG_INFO("Press Enter to exit...");
    std::cin.get();
#endif
    
	
    return 0;
}