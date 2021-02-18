#pragma once
#include <string_view>
#include "Console.h"

namespace Cyrex {
    enum class OutputStream { crx_console_standard, crx_console_err, crx_console_wstandard, crx_console_werr};
    enum class Level {crx_default, crx_info, crx_warn, crx_error, crx_critical, crx_wdefault, crx_winfo, crx_wwarn, crx_werror, crx_wcritical};

    class Logger {
    public:
        static Logger& Get() noexcept;
        void StartUp() const noexcept;
        void ShutDown() noexcept;
        void SetOutputStream(OutputStream ostream) noexcept;
        void Reset() noexcept;

        static constexpr auto NewLine()  noexcept { return std::endl<char, std::char_traits<char>>; }
        static constexpr auto WNewLine() noexcept { return std::endl<wchar_t, std::char_traits<wchar_t>>; }

        template<typename... Args>
        void Log(Level lvl, Args&& ...args);
        template<typename... Args>
        void Log(Args&& ...args);

        template<typename... Args>
        void WLog(Level lvl, Args&& ...args);
        template<typename... Args>
        void WLog(Args&& ...args);

        template<typename... Args>
        void DebugLog(Level lvl, Args&& ...args);
        template<typename... Args>
        void DebugLog(Args&& ...args);

        template<typename... Args>
        void WDebugLog(Level lvl, Args&& ...args);
        template<typename... Args>
        void WDebugLog(Args&& ...args);
    private:
        Logger() = default;

        void SetOutputStream(std::ios_base* stream) noexcept { this->stream = stream; }
        void SetLevel(Level lvl) noexcept;
        void SetLevel(OutputStream ostream, ConsoleColor clr, std::string_view attribute) noexcept;
        void SetLevel(OutputStream ostream, ConsoleColor clr, std::wstring_view attribute) noexcept;

        std::ios_base* stream = Console::GetStandardStream();
        std::string prefix{};
        std::wstring wprefix{};
    };

    template<typename ...Args>
    inline void Logger::Log(Level lvl, Args&& ...args) {
        SetLevel(lvl);
        Log(args...);
        SetLevel(Level::crx_default);
    }

    template<typename ...Args>
    inline void Logger::Log(Args&& ...args) {
        std::ostream& ostream = dynamic_cast<std::ostream&>(*stream);
        ostream << prefix;
        (ostream << ... << args);
    }

    template<typename ...Args>
    inline void Logger::WLog(Level lvl, Args&& ...args) {
        SetLevel(lvl);
        WLog(args...);
        SetLevel(Level::crx_wdefault);
    }

    template<typename ...Args>
    inline void Logger::WLog(Args&& ...args) {
        std::wostream& ostream = dynamic_cast<std::wostream&>(*stream);
        ostream << wprefix;
        (ostream << ... << args);
    }

    template<typename ...Args>
    inline void Logger::DebugLog(Level lvl, Args&& ...args) {
#ifdef _DEBUG
        SetLevel(lvl);
        DebugLog(args...);
        SetLevel(Level::crx_default);
#endif
    }

    template<typename ...Args>
    inline void Logger::DebugLog(Args&& ...args) {
#ifdef _DEBUG
        std::ostream& ostream = dynamic_cast<std::ostream&>(*stream);
        ostream << prefix;
        (ostream << ... << args);
#endif
    }

    template<typename ...Args>
    inline void Logger::WDebugLog(Level lvl, Args&& ...args) {
#ifdef _DEBUG
        SetLevel(lvl);
        WDebugLog(args...);
        SetLevel(Level::crx_wdefault);
#endif
    }

    template<typename ...Args>
    inline void Logger::WDebugLog(Args&& ...args) {
#ifdef _DEBUG
        std::wostream& ostream = dynamic_cast<std::wostream&>(*stream);
        ostream << wprefix;
        (ostream << ... << args);
#endif
    }
}

namespace Cyrex::crxlog {
    template<typename ...Args>
    inline void info(Args&& ...args) noexcept {
        Logger::Get().Log(Level::crx_info, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void err(Args&& ...args) noexcept {
        Logger::Get().Log(Level::crx_error, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void warn(Args&& ...args) noexcept {
        Logger::Get().Log(Level::crx_warn, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void critical(Args&& ...args) noexcept {
        Logger::Get().Log(Level::crx_critical, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void log(Args&& ...args) noexcept {
        Logger::Get().Log(Level::crx_default, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void winfo(Args&& ...args) noexcept {
        Logger::Get().WLog(Level::crx_winfo, args..., Logger::WNewLine());
    }

    template<typename ...Args>
    inline void werr(Args&& ...args) noexcept {
        Logger::Get().WLog(Level::crx_werror, args..., Logger::WNewLine());
    }

    template<typename ...Args>
    inline void wwarn(Args&& ...args) noexcept {
        Logger::Get().WLog(Level::crx_wwarn, args..., Logger::WNewLine());
    }

    template<typename ...Args>
    inline void wcritical(Args&& ...args) noexcept {
        Logger::Get().WLog(Level::crx_wcritical, args..., Logger::WNewLine());
    }

    template<typename ...Args>
    inline void wlog(Args&& ...args) noexcept {
        Logger::Get().WLog(Level::crx_wdefault, args..., Logger::WNewLine());
    }
}

namespace Cyrex::crxdebuglog {
    template<typename ...Args>
    inline void info(Args&& ...args) noexcept {
        Logger::Get().DebugLog(Level::crx_info, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void err(Args&& ...args) noexcept {
        Logger::Get().DebugLog(Level::crx_error, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void warn(Args&& ...args) noexcept {
        Logger::Get().DebugLog(Level::crx_warn, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void critical(Args&& ...args) noexcept {
        Logger::Get().DebugLog(Level::crx_critical, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void log(Args&& ...args) noexcept {
        Logger::Get().DebugLog(Level::crx_default, args..., Logger::NewLine());
    }

    template<typename ...Args>
    inline void winfo(Args&& ...args) noexcept {
        Logger::Get().WDebugLog(Level::crx_winfo, args..., Logger::WNewLine());
    }

    template<typename ...Args>
    inline void werr(Args&& ...args) noexcept {
        Logger::Get().WDebugLog(Level::crx_werror, args..., Logger::WNewLine());
    }

    template<typename ...Args>
    inline void wwarn(Args&& ...args) noexcept {
        Logger::Get().WDebugLog(Level::crx_wwarn, args..., Logger::WNewLine());
    }

    template<typename ...Args>
    inline void wcritical(Args&& ...args) noexcept {
        Logger::Get().WDebugLog(Level::crx_wcritical, args..., Logger::WNewLine());
    }

    template<typename ...Args>
    inline void wlog(Args&& ...args) noexcept {
        Logger::Get().WDebugLog(Level::crx_wdefault, args..., Logger::WNewLine());
    }
}