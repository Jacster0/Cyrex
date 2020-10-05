#include "Logger.h"
#include <fstream>
#include "Time/Time.h"

static Cyrex::Logger* gpSingleton = nullptr;

Cyrex::Logger& Cyrex::Logger::Get() noexcept {
    if (!gpSingleton) {
        gpSingleton = new Logger();
    }
    return *gpSingleton;
}

void Cyrex::Logger::StartUp() const noexcept {
}

void Cyrex::Logger::ShutDown() noexcept {
    if (gpSingleton != nullptr)
        delete gpSingleton;
}

void Cyrex::Logger::SetOutputStream(OutputStream ostream) noexcept {
    switch (ostream) {
    case OutputStream::crx_console_default:
        SetOutputStream(&std::cout);
        break;
    case OutputStream::crx_console_err:
        SetOutputStream(&std::cerr);
        break;
    default:
        break;
    }
}

void Cyrex::Logger::Reset() noexcept {
    prefix.clear();
    Console::SetTextColor(Color::White);
}

void Cyrex::Logger::SetLevel(Level lvl) noexcept {
    switch (lvl) {
    case Level::crx_default: {
        SetLevel(OutputStream::crx_console_default, Color::White, "");
        break;
    }
    case Level::crx_error: {
        SetLevel(OutputStream::crx_console_err, Color::Red, "<ERROR> ");
        break;
    }
    case Level::crx_warn: {
        SetLevel(OutputStream::crx_console_default, Color::Yellow, "<WARNING> ");
        break;
    } 
    case Level::crx_info: {
        SetLevel(OutputStream::crx_console_default, Color::Yellow, "<INFO> ");
        break;
    }
    case Level::crx_critical: {
        SetLevel(OutputStream::crx_console_err, Color::Red, "<CRITICAL> ");
        break;
    }
    default:
        break;
    }
}

void Cyrex::Logger::SetLevel(
    OutputStream ostream, 
    Color clr, 
    std::string_view attribute) noexcept
{
    SetOutputStream(ostream);
    Console::SetTextColor(clr);
    prefix.clear();
    prefix.append("[").append(crxtime::GetCurrentTimeAsFormatedString()).append("] ").append(attribute);
}
