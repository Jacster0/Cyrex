#include "Logger.h"
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
    case OutputStream::crx_console_standard:
        SetOutputStream(Console::GetStandardStream());
        break;
    case OutputStream::crx_console_err:
        SetOutputStream(Console::GetErrorStream());
        break;
    case OutputStream::crx_console_wstandard:
        SetOutputStream(Console::GetWideStandardStream());
        break;
    case OutputStream::crx_console_werr:
        SetOutputStream(Console::GetWideErrorStream());
        break;
    default:
        break;
    }
}

void Cyrex::Logger::Reset() noexcept {
    prefix.clear();
    Console::SetTextColor(ConsoleColor::White);
}

void Cyrex::Logger::SetLevel(Level lvl) noexcept {
    switch (lvl) {
    case Level::crx_default: {
        SetLevel(OutputStream::crx_console_standard, ConsoleColor::White, "");
        break;
    }
    case Level::crx_error: {
        SetLevel(OutputStream::crx_console_err, ConsoleColor::Red, "<ERROR> ");
        break;
    }
    case Level::crx_warn: {
        SetLevel(OutputStream::crx_console_standard, ConsoleColor::Yellow, "<WARNING> ");
        break;
    } 
    case Level::crx_info: {
        SetLevel(OutputStream::crx_console_standard, ConsoleColor::Yellow, "<INFO> ");
        break;
    }
    case Level::crx_critical: {
        SetLevel(OutputStream::crx_console_err, ConsoleColor::Red, "<CRITICAL> ");
        break;
    }
    case Level::crx_wdefault: {
        SetLevel(OutputStream::crx_console_wstandard, ConsoleColor::White, "");
        break;
    }
    case Level::crx_werror: {
        SetLevel(OutputStream::crx_console_werr, ConsoleColor::Red, "<ERROR> ");
        break;
    }
    case Level::crx_wwarn: {
        SetLevel(OutputStream::crx_console_wstandard, ConsoleColor::Yellow, "<WARNING> ");
        break;
    }
    case Level::crx_winfo: {
        SetLevel(OutputStream::crx_console_wstandard, ConsoleColor::Yellow, "<INFO> ");
        break;
    }
    case Level::crx_wcritical: {
        SetLevel(OutputStream::crx_console_werr, ConsoleColor::Red, "<CRITICAL> ");
        break;
    }
    default:
        break;
    }
}

void Cyrex::Logger::SetLevel(OutputStream ostream, ConsoleColor clr, std::string_view attribute) noexcept {
    SetOutputStream(ostream);
    Console::SetTextColor(clr);
    prefix.clear();
    prefix.append("[").append(crxtime::GetCurrentTimeAsFormatedString()).append("] ").append(attribute);
}

void Cyrex::Logger::SetLevel(OutputStream ostream, ConsoleColor clr, std::wstring_view attribute) noexcept {
    SetOutputStream(ostream);
    Console::SetTextColor(clr);
    wprefix.clear();
    wprefix.append(L"[").append(crxtime::GetCurrentTimeAsWideFormatedString()).append(L"] ").append(attribute);
}
