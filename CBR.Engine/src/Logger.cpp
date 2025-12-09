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

    /// <summary>
    /// 假定控制台已经存在，只负责写字。
    /// </summary>
    Logger::Logger()
    {
#ifdef _WIN32
        // 拿到控制台句柄
        hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO info{};
        if (hConsole_ && GetConsoleScreenBufferInfo(hConsole_, &info)) {
            defaultAttr_ = info.wAttributes;
        }
#endif
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
#endif

        std::ostringstream oss;
        oss << '[' << GetTimestamp() << "] "
            << '[' << level.ToString() << "] ";
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
#endif
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
} // namespace CBR::Engine::Debug