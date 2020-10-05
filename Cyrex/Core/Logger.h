#pragma once
#include <ostream>
#include <iostream>
#include <string_view>
#include "Console.h"

namespace Cyrex {
    enum class OutputStream { crx_console_default, crx_console_err};
    enum class Level {crx_default, crx_info, crx_warn, crx_error, crx_critical};

    class Logger {
    private:
        Logger() = default;
    public:
        static Logger& Get() noexcept;
        void StartUp() const noexcept;
        void ShutDown() noexcept;
    public:
        void SetOutputStream(OutputStream ostream) noexcept;
        std::ostream* GetOutputStream() noexcept { return stream; }
        void Reset() noexcept;
        constexpr auto NewLine() const noexcept { return std::endl<char, std::char_traits<char>>; }
    public:
        template<typename... Args>
        void Log(Level lvl, Args&& ...args);
        template<typename... Args>
        void Log(Args&& ...args);
    private:
        void SetOutputStream(std::ostream* stream) noexcept { this->stream = stream; }
        void SetLevel(Level lvl) noexcept;
        void SetLevel(OutputStream ostream, Color clr, std::string_view attribute) noexcept;
    private:
        std::ostream* stream = &std::cout;
    private:
        std::string prefix{};
    };

    template<typename ...Args>
    inline void Logger::Log(Level lvl, Args&& ...args) {
        SetLevel(lvl);
        Log(args...);
    }

    template<typename ...Args>
    inline void Logger::Log(Args&& ...args) {
        *stream << prefix;
        (*stream << ... << args);  
    }
}

namespace Cyrex::crxlog {
    template<typename ...Args>
    inline void info(Args&& ...args) {
        auto logger = Logger::Get();
        logger.Log(Level::crx_info, args..., logger.NewLine());
    }

    template<typename ...Args>
    inline void err(Args&& ...args) {
        auto logger = Logger::Get();
        logger.Log(Level::crx_error, args..., logger.NewLine());
    }

    template<typename ...Args>
    inline void warn(Args&& ...args) {
        auto logger = Logger::Get();
        logger.Log(Level::crx_warn, args..., logger.NewLine());
    }

    template<typename ...Args>
    inline void critical(Args&& ...args) {
        auto logger = Logger::Get();
        logger.Log(Level::crx_critical, args..., logger.NewLine());
    }

    template<typename ...Args>
    inline void normal(Args&& ...args) {
        auto logger = Logger::Get();
        logger.Log(Level::crx_default, args..., logger.NewLine());
    }
}