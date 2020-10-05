#include <stdio.h>
#include "Console.h"
#include "Platform/Windows/CrxWindow.h"
#include "Logger.h"


Cyrex::Console* Cyrex::Console::console(new Console);

#define COLOR_RED     FOREGROUND_RED   |  FOREGROUND_INTENSITY							
#define COLOR_GREEN   FOREGROUND_GREEN |  FOREGROUND_INTENSITY							
#define COLOR_YELLOW  FOREGROUND_RED   |  FOREGROUND_GREEN		| FOREGROUND_INTENSITY 
#define COLOR_BLUE    FOREGROUND_BLUE  |  FOREGROUND_INTENSITY							
#define COLOR_MAGENTA FOREGROUND_RED   |  FOREGROUND_BLUE		| FOREGROUND_INTENSITY	 
#define COLOR_CYAN    FOREGROUND_BLUE  |  FOREGROUND_GREEN		| FOREGROUND_INTENSITY  
#define COLOR_WHITE   FOREGROUND_BLUE  |  FOREGROUND_GREEN		| FOREGROUND_RED	


Cyrex::Console::Console() noexcept {
    Create();
    Hide();
}

Cyrex::Console::~Console() {
    Hide();
    FreeConsole();

    if (!Cyrex::Console::console) {
        delete Cyrex::Console::console;
        Cyrex::Console::console = nullptr;
    }
}

void Cyrex::Console::Hide() noexcept {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
}

void Cyrex::Console::Show() noexcept {
    ShowWindow(GetConsoleWindow(), SW_SHOW);
}

bool Cyrex::Console::IsVisible() noexcept {
    return IsWindowVisible(GetConsoleWindow()) != false;
}

void Cyrex::Console::Destroy() noexcept {
    Cyrex::Console::console->~Console();
}

void Cyrex::Console::SetOpacity(uint8_t alpha) noexcept {
    SetLayeredWindowAttributes(GetConsoleWindow(), NULL, alpha, LWA_ALPHA);
}

void Cyrex::Console::Reset() noexcept {
    SetOpacity(std::numeric_limits<uint8_t>::max());
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_WHITE);
}

void Cyrex::Console::WriteLine(std::string_view message) noexcept {
    Write(message);
    Write("\n");
}

void Cyrex::Console::Write(std::string_view message) noexcept {
    auto logger = Logger::Get();
    logger.Reset();
    logger.SetOutputStream(OutputStream::crx_console_default);
    logger.Log(message);
}

void Cyrex::Console::Flush() noexcept {
    auto stream = Logger::Get().GetOutputStream();

    if(stream->rdbuf() == std::cout.rdbuf())
        *stream << std::flush;
}

void Cyrex::Console::SetTextColor(Color clr) noexcept {
    switch (clr) {
    case Color::Red:
        SetTextColor(COLOR_RED);
        break;
    case Color::Green:
        SetTextColor(COLOR_GREEN);
        break;
    case Color::Yellow:
        SetTextColor(COLOR_YELLOW);
        break;
    case Color::Blue:
        SetTextColor(COLOR_BLUE);
        break;
    case Color::Magenta:
        SetTextColor(COLOR_MAGENTA);
        break;
    case Color::Cyan:
        SetTextColor(COLOR_CYAN);
        break;
    case Color::White:
        SetTextColor(COLOR_WHITE);
        break;
    default:
        break;
    }
}

void Cyrex::Console::Create() noexcept {
    if (AllocConsole()) {
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    SMALL_RECT size = { 0,0,96,19 };
    auto handle = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleWindowInfo(handle, true, &size);
    SetConsoleTitle(L"Cyrex Console");
}

void Cyrex::Console::SetTextColor(int color) noexcept {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
