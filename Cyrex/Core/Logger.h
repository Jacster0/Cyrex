#pragma once
#include <string_view>
#include <mutex>
#include <regex>

#include "Console.h"
#include "assimp/DefaultLogger.hpp"

namespace Cyrex {
    class Logger {
    public:
        enum class OutputStream { 
            crx_console_standard, 
            crx_console_err, 
            crx_console_wstandard, 
            crx_console_werr 
        };

        enum class Level { 
            crx_default, 
            crx_info, 
            crx_warn, 
            crx_error, 
            crx_critical, 
            crx_debug,
            crx_wdefault, 
            crx_winfo, 
            crx_wwarn, 
            crx_werror, 
            crx_wcritical,
            crx_wdebug
        };

        static Logger& Get() noexcept;

        void SetOutputStream(OutputStream ostream) noexcept;
        void Reset() noexcept;

        static constexpr auto NewLine()  noexcept { return std::endl<char, std::char_traits<char>>; }
        static constexpr auto WNewLine() noexcept { return std::endl<wchar_t, std::char_traits<wchar_t>>; }

        void Log(Level lvl, auto&& ...args) noexcept;
        template<typename... Args>
        void WLog(Level lvl, Args&& ...args) noexcept;

        template<typename... Args>
        void DebugLog(Level lvl, Args&& ...args) noexcept;
        template<typename... Args>
        void WDebugLog(Level lvl, Args&& ...args) noexcept;
    private:
        Logger() = default;

        void Log(auto&& ...args) const noexcept;
        void WLog(auto&& ...args) const noexcept;

        void DebugLog(auto&& ...args) const noexcept;
        void WDebugLog(auto&& ...args) const noexcept;

        void SetOutputStream(std::ios_base* stream) noexcept { m_stream = stream; }
        void SetLevel(Level lvl) noexcept;
        void SetLevel(OutputStream ostream, ConsoleColor clr, std::string_view attribute) noexcept;
        void SetLevel(OutputStream ostream, ConsoleColor clr, std::wstring_view attribute) noexcept;

        std::ios_base* m_stream = Console::GetStandardStream();
        std::string m_prefix{};
        std::wstring m_wprefix{};

        static inline std::mutex m_loggingMutex;
    };

    inline void Logger::Log(Level lvl, auto&& ...args) noexcept {
        std::lock_guard<std::mutex> lock(Cyrex::Logger::m_loggingMutex);
        SetLevel(lvl);
        Log(args...);
        SetLevel(Level::crx_default);
    }

    inline void Logger::Log(auto&& ...args) const noexcept {
        auto& ostream = dynamic_cast<std::ostream&>(*m_stream);
        ostream << m_prefix;
        (ostream << ... << args);
    }

    inline void Logger::WLog(Level lvl, auto&& ...args) noexcept {
        std::lock_guard<std::mutex> lock(Cyrex::Logger::m_loggingMutex);
        SetLevel(lvl);
        WLog(args...);
        SetLevel(Level::crx_wdefault);
    }

    inline void Logger::WLog(auto&& ...args) const noexcept {
        auto& ostream = dynamic_cast<std::wostream&>(*m_stream);
        ostream << m_wprefix;
        (ostream << ... << args);
    }

    inline void Logger::DebugLog(Level lvl, auto&& ...args) noexcept {
#ifdef _DEBUG
        std::lock_guard<std::mutex> lock(Cyrex::Logger::m_loggingMutex);
        SetLevel(lvl);
        DebugLog(args...);
        SetLevel(Level::crx_default);
#endif
    }

    inline void Logger::DebugLog(auto&& ...args) const noexcept {
#ifdef _DEBUG
        auto& ostream = dynamic_cast<std::ostream&>(*m_stream);
        ostream << m_prefix;
        (ostream << ... << args);
#endif
    }

    inline void Logger::WDebugLog(Level lvl, auto&& ...args) noexcept {
#ifdef _DEBUG
        std::lock_guard<std::mutex> lock(Cyrex::Logger::m_loggingMutex);
        SetLevel(lvl);
        WDebugLog(args...);
        SetLevel(Level::crx_wdefault);
#endif
    }

    inline void Logger::WDebugLog(auto&& ...args) const noexcept {
#ifdef _DEBUG
        auto& ostream = dynamic_cast<std::wostream&>(*m_stream);
        ostream << m_wprefix;
        (ostream << ... << args);
#endif
    }

    class AssimpLogger : public Assimp::LogStream {
    public:
        virtual void write(const char* message) noexcept override {
            static const std::regex m_assimpLogRegex(R"((?:Debug|Info|Warn|Error),\s*(.*)\n)");
            std::cmatch match;
            std::regex_match(message, match, m_assimpLogRegex);

            if (match.size() > 1) {
                Logger::Get().Log(m_level, match.str(1), Logger::NewLine());
            }
        }
        static void Attach() noexcept {
#if defined( _DEBUG )
            Assimp::Logger::LogSeverity logSeverity = Assimp::Logger::VERBOSE;
#else
            Assimp::Logger::LogSeverity logSeverity = Assimp::Logger::NORMAL;
#endif
            auto assimpLogger = Assimp::DefaultLogger::create("", logSeverity, 0);

            assimpLogger->attachStream(new AssimpLogger(Logger::Level::crx_debug), Assimp::Logger::Debugging);
            assimpLogger->attachStream(new AssimpLogger(Logger::Level::crx_info),  Assimp::Logger::Info);
            assimpLogger->attachStream(new AssimpLogger(Logger::Level::crx_warn),  Assimp::Logger::Warn);
            assimpLogger->attachStream(new AssimpLogger(Logger::Level::crx_error), Assimp::Logger::Err);
        }

        static void Detach() noexcept {
            Assimp::DefaultLogger::kill();
        }
    private:
        AssimpLogger(Logger::Level lvl = Logger::Level::crx_default)
            :
            m_level(lvl)
        {}

        Logger::Level m_level;
    };
}

namespace Cyrex::crxlog {
    inline void info(auto&& ...args) noexcept {
        Logger::Get().Log(Logger::Level::crx_info, args..., Logger::NewLine());
    }

    inline void err(auto&& ...args) noexcept {
        Logger::Get().Log(Logger::Level::crx_error, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void warn(auto&& ...args) noexcept {
        Logger::Get().Log(Logger::Level::crx_warn, args..., Logger::NewLine());
    }

    inline void critical(auto&& ...args) noexcept {
        Logger::Get().Log(Logger::Level::crx_critical, args..., Logger::NewLine());
    }

    inline void log(auto&& ...args) noexcept {
        Logger::Get().Log(Logger::Level::crx_default, args..., Logger::NewLine());
    }

    inline void winfo(auto&& ...args) noexcept {
        Logger::Get().WLog(Logger::Level::crx_winfo, args..., Logger::WNewLine());
    }

    inline void werr(auto&& ...args) noexcept {
        Logger::Get().WLog(Logger::Level::crx_werror, args..., Logger::WNewLine());
    }

    inline void wwarn(auto&& ...args) noexcept {
        Logger::Get().WLog(Logger::Level::crx_wwarn, args..., Logger::WNewLine());
    }

    inline void wcritical(auto&& ...args) noexcept {
        Logger::Get().WLog(Logger::Level::crx_wcritical, args..., Logger::WNewLine());
    }

    inline void wlog(auto&& ...args) noexcept {
        Logger::Get().WLog(Logger::Level::crx_wdefault, args..., Logger::WNewLine());
    }

    inline void debug(auto&& ...args) noexcept {
        Logger::Get().DebugLog(Logger::Level::crx_debug, args..., Logger::NewLine());
    }

    inline void wdebug(auto&& ...args) noexcept {
        Logger::Get().WDebugLog(Logger::Level::crx_wdebug, args..., Logger::WNewLine());
    }
}