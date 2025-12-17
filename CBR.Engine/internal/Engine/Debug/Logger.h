#pragma once

namespace CBR::Engine::Debug
{
    constexpr int kConsoleWindowWidth = 120;
    constexpr int kConsoleWindowHeight = 31;

    struct LogLevel
    {
        enum class Value {
            Info,
            Warn,
            Error,
            Debug
        } value;
    
        constexpr LogLevel(Value v) : value(v) {}
    
        constexpr std::string_view ToString() const noexcept {
            switch (value) {
            case Value::Info:  return "INFO";
            case Value::Warn:  return "WARN";
            case Value::Error: return "ERROR";
            case Value::Debug: return "DEBUG";
            }
        
            return "UNKNOWN";
        }
    
        WORD ToColor() const noexcept {
            switch (value) {
            case Value::Info:  return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            case Value::Warn:  return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            case Value::Error: return FOREGROUND_RED | FOREGROUND_INTENSITY;
            case Value::Debug: return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            }
        
            return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        }
    };
    
    class Logger
    {
    public:
        Logger();
        ~Logger(); // 由DebugManager管理生命周期

        Logger(const Logger&) = delete; // 禁止拷贝
        Logger& operator=(const Logger&) = delete;

        template<typename... Args>
        void InfoMsg(const std::source_location loc, Args&&... args)
        { Info(FormatLogMessage(std::forward<Args>(args)...), loc.file_name(), static_cast<int>(loc.line()), ShortFunctionName(loc.function_name())); }
    
        template<typename... Args>
        void WarnMsg(const std::source_location loc, Args&&... args) 
        { Warn(FormatLogMessage(std::forward<Args>(args)...), loc.file_name(), static_cast<int>(loc.line()), ShortFunctionName(loc.function_name())); }
    
        template<typename... Args>
        void ErrorMsg(const std::source_location loc, Args&&... args) 
        { Error(FormatLogMessage(std::forward<Args>(args)...), loc.file_name(), static_cast<int>(loc.line()), ShortFunctionName(loc.function_name())); }
    
        template<typename... Args>
        void DebugMsg(const std::source_location loc, Args&&... args) 
        { Debug(FormatLogMessage(std::forward<Args>(args)...), loc.file_name(), static_cast<int>(loc.line()), ShortFunctionName(loc.function_name())); }
    
        void Info(std::string_view message, std::string_view file, int line, std::string_view func);
        void Warn(std::string_view message, std::string_view file, int line, std::string_view func);
        void Error(std::string_view message, std::string_view file, int line, std::string_view func);
        void Debug(std::string_view message, std::string_view file, int line, std::string_view func);

    private:
        static std::atomic<uint16_t> s_logSequence;
        static std::wstring AnsiToUtf16(std::string_view s);
        static std::string_view ShortFunctionName(std::string_view sig);

        void OpenDebugConsole(); // 目前是Logger在管理console，因为目前只有它能在console上输出信息
        void Write(const LogLevel& level, std::string_view message, std::string_view file, int line, std::string_view func);
        std::string GetTimestamp() const;
        std::string ExtractFilename(std::string_view filepath) const;
        template<typename... Args>
        std::string FormatLogMessage(Args&&... args)
        {
            std::ostringstream oss;
            (oss << ... << args);
            return oss.str();
        }
    
#ifdef _WIN32
        HANDLE hConsole_ = nullptr;
        WORD   defaultAttr_ = 0;
        bool   ownsConsole_ = false;
#endif
        std::mutex mutex_;
    };

    // 在 DebugManager 中实现，用于宏访问当前全局 Logger
    Logger& GetLogger() noexcept;
    
#if defined(_DEBUG)
    #define LOG_INFO(...)  CBR::Engine::Debug::GetLogger().InfoMsg(std::source_location::current(), __VA_ARGS__)
    #define LOG_WARN(...)  CBR::Engine::Debug::GetLogger().WarnMsg(std::source_location::current(), __VA_ARGS__)
    #define LOG_ERROR(...) CBR::Engine::Debug::GetLogger().ErrorMsg(std::source_location::current(), __VA_ARGS__)
    #define LOG_DEBUG(...) CBR::Engine::Debug::GetLogger().DebugMsg(std::source_location::current(), __VA_ARGS__)
#else
    #define LOG_INFO(...) 
    #define LOG_WARN(...) 
    #define LOG_ERROR(...)
    #define LOG_DEBUG(...)
#endif
} //namespace CBR::Engine::Debug