#include "Cursor.h"

void Cyrex::Cursor::Enable() noexcept {
    m_cursorEnabled = true;
    ShowCursor();
    FreeCursor();
}

void Cyrex::Cursor::Disable(HWND hWnd) noexcept {
    m_cursorEnabled = false;
    HideCursor();
    ConfineCursor(hWnd);
}

bool Cyrex::Cursor::IsEnabled() const noexcept { return m_cursorEnabled; }

void Cyrex::Cursor::ConfineCursor(HWND hWnd) noexcept {
    RECT rect;
    GetClientRect(hWnd, &rect);
    MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
    ClipCursor(&rect);
}

void Cyrex::Cursor::FreeCursor() noexcept {
    ClipCursor(nullptr);
}

void Cyrex::Cursor::ShowCursor() noexcept {
    while (::ShowCursor(TRUE) < 0);
}

void Cyrex::Cursor::HideCursor() noexcept {
    while (::ShowCursor(FALSE) >= 0);
}
