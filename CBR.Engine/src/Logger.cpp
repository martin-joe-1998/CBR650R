#include "pch.h"
#include "Engine/Debug/Logger.h"

namespace CBR::Engine::Debug
{
    struct ColorGuard {
#ifdef _WIN32
        HANDLE h{};
        WORD   restore{};
        ColorGuard(HANDLE hConsole, WORD color, WORD restoreAttr) : h(hConsole), restore(restoreAttr) {
            if (h) SetConsoleTextAttribute(h, color);
        }
        ~ColorGuard() { if (h) SetConsoleTextAttribute(h, restore); }
#else
        std::string reset = "\x1b[0m";
        ColorGuard(const char* colorSeq) { std::fputs(colorSeq, stdout); }
        ~ColorGuard() { std::fputs(reset.c_str(), stdout); }
#endif
    };

    Logger::Logger()
    {
#ifdef _DEBUG
        // 创建控制台
        OpenDebugConsole();
#endif // _DEBUG

#ifdef _WIN32
        // 拿到控制台句柄
        hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO info{};
        if (hConsole_ && GetConsoleScreenBufferInfo(hConsole_, &info)) {
            defaultAttr_ = info.wAttributes;
        }
#endif // _WIN32
    }

    Logger::~Logger()
    {
#ifdef _WIN32
        // 关闭控制台
        FreeConsole();
#endif // _WIN32
    }


    void Logger::Info(std::string_view message, std::string_view file, int line, std::string_view func)
    {
        Write(LogLevel(LogLevel::Value::Info), message, file, line, func);
    }

    void Logger::Warn(std::string_view message, std::string_view file, int line, std::string_view func)
    {
        Write(LogLevel(LogLevel::Value::Warn), message, file, line, func);
    }

    void Logger::Error(std::string_view message, std::string_view file, int line, std::string_view func)
    {
        Write(LogLevel(LogLevel::Value::Error), message, file, line, func);
    }

    void Logger::Debug(std::string_view message, std::string_view file, int line, std::string_view func)
    {
        Write(LogLevel(LogLevel::Value::Debug), message, file, line, func);
    }

    void Logger::Write(const LogLevel& level, std::string_view message, std::string_view file, int line, std::string_view func)
    {
        std::scoped_lock lock(mutex_);

#ifdef _WIN32
        ColorGuard cg(hConsole_, level.ToColor(), defaultAttr_);
#else
        // 简易 ANSI 映射（示意）
        const char* seq = "\x1b[37m";
        switch (level.value) {
        case LogLevel::Value::Info:  seq = "\x1b[92m"; break; // 亮绿
        case LogLevel::Value::Warn:  seq = "\x1b[93m"; break; // 亮黄
        case LogLevel::Value::Error: seq = "\x1b[91m"; break; // 亮红
        case LogLevel::Value::Debug: seq = "\x1b[96m"; break; // 亮青
        }
        ColorGuard cg(seq);
#endif // _WIN32

        std::ostringstream oss;
        oss << '[' << GetTimestamp() << "] "
            << '[' << level.ToString() << "] ";
        // 对齐各级log的message的首字符
        if (level.value == LogLevel::Value::Info || level.value == LogLevel::Value::Warn) {
            oss << " ";
        }
        // 输出消息
        oss << message << " ";

        if (!file.empty()) {
            // 直接视图切片，避免额外分配
            size_t pos = file.find_last_of("/\\");
            std::string_view fname = (pos == std::string_view::npos) ? file : file.substr(pos + 1);
            oss << '[' << fname << ':' << line << ' ' << func << "] ";
        }
        oss << '\n';

        std::cout << oss.str();

#ifdef _WIN32
        // （可选）同步给调试器
        OutputDebugStringA(oss.str().c_str());
#endif // _WIN32
    }

    std::string Logger::GetTimestamp() const
    {
        using namespace std::chrono;
        const auto now = system_clock::now();
        const std::time_t t = system_clock::to_time_t(now);
        std::tm tm{};
        localtime_s(&tm, &t);
        const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S")
            << '.' << std::setw(3) << std::setfill('0') << ms.count();
        return oss.str();
    }

    std::string Logger::ExtractFilename(std::string_view filepath) const
    {
        size_t pos = filepath.find_last_of("/\\");
        return (pos == std::string_view::npos) ? std::string(filepath) : std::string(filepath.substr(pos + 1));
    }

    void Logger::OpenDebugConsole()
    {
        // 创建控制台（若想复用父进程控制台可用 AttachConsole(ATTACH_PARENT_PROCESS)）
        AllocConsole();

        // 让 C/CPP 标准流指向这块控制台
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);

        // 可选：UTF-8 输出
        SetConsoleOutputCP(CP_UTF8);

#ifdef _WIN32
        // 获取标准输出句柄
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

        // 设置缓冲区大小（列数、行数）
        // 200 列，1000 行（行数可以设大一点方便滚动）
        COORD bufferSize;
        bufferSize.X = 200;   // 想要多宽就改这里：常见 160/200/240
        bufferSize.Y = 1000;

        SetConsoleScreenBufferSize(hOut, bufferSize);

        // 设置窗口可见区域的大小（必须不超过 bufferSize）
        SMALL_RECT windowRect;
        windowRect.Left = 0;
        windowRect.Top = 0;
        windowRect.Right = bufferSize.X - 1;  // 宽度 = Right - Left + 1
        windowRect.Bottom = 30;               // 可见行数（自己喜欢几行就写多少-1）

        SetConsoleWindowInfo(hOut, TRUE, &windowRect);
#endif // _WIN32
    }
} // namespace CBR::Engine::Debug