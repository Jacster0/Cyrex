#include <stdio.h>
#include "Console.h"
#include "Platform/Windows/CrxWindow.h"

Cyrex::Console* Cyrex::Console::console(new Console);

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
    ResetTextColor();
}

void Cyrex::Console::ResetTextColor() noexcept {
    SetTextColor(Color::White);
}

void Cyrex::Console::WriteLine(std::string_view message) noexcept {
    Write(message);
    Write("\n");
}

void Cyrex::Console::Write(std::string_view message) noexcept {
    *Console::GetStandardStream() << message;
}

void Cyrex::Console::Error(std::string_view message) noexcept {
    *Console::GetErrorStream() << message;
}

void Cyrex::Console::Log(std::string_view message) noexcept {
    *Console::GetLogStream() << message;
}

void Cyrex::Console::Flush() noexcept {
    *Console::GetStandardStream() << std::flush;
}

void Cyrex::Console::SetTextColor(Color clr) noexcept {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(clr));
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