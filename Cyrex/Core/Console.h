#pragma once
#include <memory>
#include <string>
#include <iostream>

namespace Cyrex {
    enum class ConsoleColor : std::uint16_t { 
        Red     = (0x0004 | 0x0008), 
        Green   = (0x0002 | 0x0008),
        Blue    = (0x0001 | 0x0008),
        Yellow  = (0x0004 | 0x0002 | 0x0008),
        Magenta = (0x0004 | 0x0001 | 0x0008),
        Cyan    = (0x0001 | 0x0002 | 0x0008),
        White   = (0x0004 | 0x0002 | 0x0001),
    };

    class Console { 
    public:
        ~Console() = default;

        static void Hide() noexcept;
        static void Show() noexcept;
        static bool IsVisible() noexcept;

        static void Destroy() noexcept;
        static auto Instance() noexcept { return Console::console; };
        static void SetOpacity(uint8_t alpha) noexcept;
        static void Reset() noexcept;
        static void ResetTextColor() noexcept;

        static void WriteLine(std::string_view message) noexcept;
        static void Write(std::string_view message) noexcept;
        static void Error(std::string_view message) noexcept;
        static void Log(std::string_view message) noexcept;
        static void Flush() noexcept;
        static void SetTextColor(ConsoleColor clr) noexcept;

        static std::ostream* GetStandardStream() noexcept { return Console::console->m_standardOutput; }
        static std::ostream* GetErrorStream()    noexcept { return Console::console->m_errorOutput; }
        static std::ostream* GetLogStream()      noexcept { return Console::console->m_logOutput; }

        static std::wostream* GetWideStandardStream() noexcept { return Console::console->m_wideStandardOutput; }
        static std::wostream* GetWideErrorStream()    noexcept { return Console::console->m_wideErrorOutput; }
        static std::wostream* GetWideLogStream()      noexcept { return Console::console->m_wideLogOutput; }
    private:
        Console() noexcept;
        Console(const Console& rhs) = delete;
        Console& operator = (const Console rhs) = delete;

        static void Create() noexcept;

        static Console* console;
        std::ostream* m_standardOutput = &std::cout;
        std::ostream* m_errorOutput    = &std::cerr;
        std::ostream* m_logOutput      = &std::clog;

        std::wostream* m_wideStandardOutput = &std::wcout;
        std::wostream* m_wideErrorOutput    = &std::wcerr;
        std::wostream* m_wideLogOutput      = &std::wclog;
    };
}
