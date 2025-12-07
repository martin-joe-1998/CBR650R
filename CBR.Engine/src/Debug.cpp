#include "pch.h"
#ifdef _DEBUG

namespace CBR::Engine::Debug {


    static void OpenDebugConsole()
    {
        // 1) 创建控制台（若想复用父进程控制台可用 AttachConsole(ATTACH_PARENT_PROCESS)）
        AllocConsole();
    
        // 2) 让 C/CPP 标准流指向这块控制台
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);
    
        // 可选：UTF-8 输出
        SetConsoleOutputCP(CP_UTF8);

#ifdef _WIN32
        // 1) 获取标准输出句柄
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

        // 2) 设置缓冲区大小（列数、行数）
        // 200 列，1000 行（行数可以设大一点方便滚动）
        COORD bufferSize;
        bufferSize.X = 200;   // 想要多宽就改这里：常见 160/200/240
        bufferSize.Y = 1000;

        SetConsoleScreenBufferSize(hOut, bufferSize);

        // 3) 设置窗口可见区域的大小（必须不超过 bufferSize）
        SMALL_RECT windowRect;
        windowRect.Left = 0;
        windowRect.Top = 0;
        windowRect.Right = bufferSize.X - 1;  // 宽度 = Right - Left + 1
        windowRect.Bottom = 30;                // 可见行数（自己喜欢几行就写多少-1）

        SetConsoleWindowInfo(hOut, TRUE, &windowRect);
#endif
    }
} // namespace CBR::Engine::Debug

#endif
