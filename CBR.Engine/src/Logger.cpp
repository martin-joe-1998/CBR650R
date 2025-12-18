#include "pch.h"
#include "Engine/Debug/Logger.h"

namespace CBR::Engine::Debug
{
    std::atomic<uint16_t> Logger::s_logSequence{ 0 };

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

        std::size_t windowWidth = kConsoleWindowWidth;
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        if (hConsole_ && GetConsoleScreenBufferInfo(hConsole_, &csbi)) {
            windowWidth = static_cast<std::size_t>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        }
        if (windowWidth == 0) windowWidth = kConsoleWindowWidth;
#endif

        // header: [#logSequence] [timestamp] [LEVEL] [location]
        std::ostringstream headerOss;
        
        uint16_t seq = ++s_logSequence;
        headerOss << "[#" << seq << "] ";
        headerOss << '[' << GetTimestamp() << "] "
            << '[' << level.ToString() << "] ";
        if (level.value == LogLevel::Value::Info || level.value == LogLevel::Value::Warn) {
            headerOss << ' ';
        }

        // location
        if (!file.empty()) {
            std::size_t pos = file.find_last_of("/\\");
            std::string_view fname = (pos == std::string_view::npos) ? file : file.substr(pos + 1);
            headerOss << '[' << fname << ':' << line << " | " << func << ']';
        }

        std::string header = headerOss.str();

        // 输出 header（如果 header 超过窗口宽度，也按窗口宽度切行）
        std::ostringstream out;

        auto WriteWrapped = [&](std::string_view text, std::size_t indentSpaces)
            {
                const std::string indent(indentSpaces, ' ');

                // 第一行不缩进
                bool first = true;

                while (!text.empty())
                {
                    if (!first) out << indent;

                    const std::size_t usableWidth =
                        (windowWidth > indentSpaces) ? (windowWidth - indentSpaces) : 1;

                    const std::size_t len = std::min<std::size_t>(text.size(), usableWidth);
                    out << text.substr(0, len) << '\n';
                    text.remove_prefix(len);

                    first = false;
                }
            };

        // header：不缩进换行（indent=0）
        if (!header.empty())
            WriteWrapped(header, 0);
        else
            out << '\n';

        // 输出 message
        constexpr std::size_t kMessageIndent = 0; // 第二行缩进

        if (message.empty()) {
            out << std::string(kMessageIndent, ' ') << '\n';
        }
        else {
            // message 第一行开始就缩进 kMessageIndent
            // 这意味着可用宽度 = windowWidth - kMessageIndent
            WriteWrapped(message, kMessageIndent);
        }

        std::string finalText = out.str();
        finalText += '\n';

#ifdef _WIN32
        std::wstring wtext = AnsiToUtf16(finalText);
        DWORD written = 0;
        if (!WriteConsoleW(hConsole_, wtext.c_str(), (DWORD)wtext.size(), &written, nullptr))
        {
            std::cout << finalText;
        }

        // 同步给调试器
        OutputDebugStringW(wtext.c_str());
#else
        std::cout << finalText;
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

    std::wstring Logger::AnsiToUtf16(std::string_view s)
    {
        if (s.empty()) return {};
        constexpr UINT kCodePage = CP_ACP;

        int wlen = MultiByteToWideChar(kCodePage, 0, s.data(), (int)s.size(), nullptr, 0);
        if (wlen <= 0) return {};

        std::wstring w(wlen, L'\0');
        MultiByteToWideChar(kCodePage, 0, s.data(), (int)s.size(), w.data(), wlen);
        return w;
    }

    std::string_view Logger::ShortFunctionName(std::string_view sig)
    {
        // 找到参数列表起点
        const auto lp = sig.find('(');
        if (lp == std::string_view::npos)
            return sig;

        // 函数名前缀部分（含返回类型/调用约定）
        std::string_view prefix = sig.substr(0, lp);
        // 参数列表（含括号）
        std::string_view params = sig.substr(lp);

        // 在 prefix 中找到最后一个空格
        // 它后面通常就是“函数的限定名”
        if (auto sp = prefix.rfind(' '); sp != std::string_view::npos)
            prefix = prefix.substr(sp + 1);

        // 拼回 函数名 + (参数列表)
        return std::string_view{
            prefix.data(),
            prefix.size() + params.size()
        };
    }

    void Logger::OpenDebugConsole()
    {
        // 创建控制台（若想复用父进程控制台可用 AttachConsole(ATTACH_PARENT_PROCESS)）
        AllocConsole();
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        // 让 C/CPP 标准流指向这块控制台
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);

        // UTF-8 输出
        SetConsoleOutputCP(CP_UTF8);

#ifdef _WIN32
        // 获取标准输出句柄
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

        // 设置缓冲区大小（列数、行数）
        COORD bufferSize;
        bufferSize.X = 400;
        bufferSize.Y = 2000;
        SetConsoleScreenBufferSize(hOut, bufferSize);

        // 设置窗口可见区域的大小（必须不超过 bufferSize）
        SMALL_RECT windowRect;
        windowRect.Left = 0;
        windowRect.Top = 0;
        windowRect.Right = kConsoleWindowWidth - 1;          // 宽度 = Right - Left + 1
        windowRect.Bottom = kConsoleWindowHeight - 1; // 可见行数（自己喜欢几行就写多少-1）
        SetConsoleWindowInfo(hOut, TRUE, &windowRect);

        // 禁用“到行尾自动换行”
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode))
        {
            mode &= ~ENABLE_WRAP_AT_EOL_OUTPUT;  // 禁用 wrap
            SetConsoleMode(hOut, mode);
        }

        // 禁用改变窗口大小
        HWND hwndConsole = GetConsoleWindow();
        if (hwndConsole)
        {
            // 去掉可调整大小边框(拖拽)和最大化按钮
            LONG style = GetWindowLong(hwndConsole, GWL_STYLE);
            style &= ~WS_THICKFRAME;   // 禁止拖拽边框调整大小
            style &= ~WS_MAXIMIZEBOX;  // 禁止最大化
            SetWindowLong(hwndConsole, GWL_STYLE, style);

            // 灰掉系统菜单里的 Size/Maximize
            HMENU hMenu = GetSystemMenu(hwndConsole, FALSE);
            if (hMenu)
            {
                EnableMenuItem(hMenu, SC_SIZE, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
            }

            // 让样式立刻生效
            SetWindowPos(hwndConsole, nullptr, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }
#endif // _WIN32
    }
} // namespace CBR::Engine::Debug