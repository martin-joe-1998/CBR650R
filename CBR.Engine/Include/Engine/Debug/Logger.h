#pragma once

namespace CBR::Engine::Debug
{
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
        static Logger& Instance() {
            static Logger instance;
            return instance;
        }

        template<typename... Args>
        void InfoMsg(const std::source_location loc, Args&&... args)
        { Info(FormatLogMessage(std::forward<Args>(args)...), loc.file_name(), static_cast<int>(loc.line()), loc.function_name()); }
    
        template<typename... Args>
        void WarnMsg(const std::source_location loc, Args&&... args) 
        { Warn(FormatLogMessage(std::forward<Args>(args)...), loc.file_name(), static_cast<int>(loc.line()), loc.function_name()); }
    
        template<typename... Args>
        void ErrorMsg(const std::source_location loc, Args&&... args) 
        { Error(FormatLogMessage(std::forward<Args>(args)...), loc.file_name(), static_cast<int>(loc.line()), loc.function_name()); }
    
        template<typename... Args>
        void DebugMsg(const std::source_location loc, Args&&... args) 
        { Debug(FormatLogMessage(std::forward<Args>(args)...), loc.file_name(), static_cast<int>(loc.line()), loc.function_name()); }
    
        void Info(std::string_view message, std::string_view file, int line, std::string_view func);
        void Warn(std::string_view message, std::string_view file, int line, std::string_view func);
        void Error(std::string_view message, std::string_view file, int line, std::string_view func);
        void Debug(std::string_view message, std::string_view file, int line, std::string_view func);
    private:
        Logger();
        ~Logger();
        /// ½ûÖ¹¿½±´
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

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
        HANDLE m_hConsole = nullptr;
        WORD   m_defaultAttr = 0;
        bool   m_ownsConsole = false;
#endif
        std::mutex m_mutex;
    };
    
    
#if defined(_DEBUG)
    #define LOG_INFO(...)  CBR::Engine::Debug::Logger::Instance().InfoMsg(std::source_location::current(), __VA_ARGS__)
    #define LOG_WARN(...)  CBR::Engine::Debug::Logger::Instance().WarnMsg(std::source_location::current(), __VA_ARGS__)
    #define LOG_ERROR(...) CBR::Engine::Debug::Logger::Instance().ErrorMsg(std::source_location::current(), __VA_ARGS__)
    #define LOG_DEBUG(...) CBR::Engine::Debug::Logger::Instance().DebugMsg(std::source_location::current(), __VA_ARGS__)
#else
    #define LOG_INFO(...) 
    #define LOG_WARN(...) 
    #define LOG_ERROR(...)
    #define LOG_DEBUG(...)
#endif
} //namespace CBR::Engine::Debug