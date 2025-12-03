#include "pch.h"

#include "CBR.Engine/Debug.cpp"
#include "CBR.Engine/Logger.h"
#include "CBR.Engine/WindowsMain.h"

using namespace CBR;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Debug 模式下开启控制台，并输出测试的信息
#ifdef _DEBUG
	Engine::Debug::OpenDebugConsole();
    LOG_INFO("INFO.");
    LOG_WARN("WARN.");
    LOG_ERROR("ERROR.");
    LOG_DEBUG("DEBUG");
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

	return 0;
}