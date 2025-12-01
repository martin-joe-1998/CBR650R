#ifdef _DEBUG
#include <cstdio>
#include <io.h>
#include <fcntl.h>
#include <windows.h>

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
    }
} // namespace CBR::Engine::Debug

#endif
