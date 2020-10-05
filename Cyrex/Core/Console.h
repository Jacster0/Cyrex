#pragma once
#include <memory>
#include <string>
#include <iostream>

namespace Cyrex {
    enum class Color { Red, Green, Yellow, Blue, Magenta, Cyan, White };

    class Console {
    private:
        Console() noexcept;
        Console(const Console& rhs) = delete;
        Console& operator = (const Console rhs) = delete;
    public:
        ~Console() noexcept;
    public:
        static void Hide() noexcept;
        static void Show() noexcept;
        static bool IsVisible() noexcept;
    public:
        static void Destroy() noexcept;
        static auto Instance() noexcept { return Console::console; };
        static void SetOpacity(uint8_t alpha) noexcept;
        static void Reset() noexcept;
        static void ResetTextColor() noexcept;
    public:
        static void WriteLine(std::string_view message) noexcept;
        static void Write(std::string_view message) noexcept;
        static void Flush() noexcept;
        static void SetTextColor(Color clr) noexcept;
    public:
        static std::ostream* GetStandardStream() noexcept { return Console::console->m_standardOutput; }
        static std::ostream* GetErrorStream() noexcept { return Console::console->m_errorOutput; }
        static std::ostream* GetLogStream() noexcept { return Console::console->m_logOutput; }
    private:
        static void Create() noexcept;
        static void SetTextColor(int color) noexcept;
    private:
        static Console* console;
        std::ostream* m_standardOutput = &std::cout;
        std::ostream* m_errorOutput = &std::cerr;
        std::ostream* m_logOutput = &std::clog;
    };
}
