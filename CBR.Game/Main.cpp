#include "pch.h"

#include "CBR.Engine/Debug.cpp"
#include "CBR.Engine/WindowsMain.h"

using namespace CBR;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Debug 模式下开启控制台
#ifdef _DEBUG
	Engine::Debug::OpenDebugConsole();
#endif

    // 通过constructor创建窗口
	std::cout << "Create Window\n";
	WindowsMain* pWindow = new WindowsMain(hInstance, nCmdShow);

    bool running = true;
    while (running)
    {
        if (!pWindow->ProcessMessages())
        {
            std::cout << "Close Window\n";
            running = false;
        }

        // Render

        Sleep(10);
    }

    delete pWindow;

	return 0;
}