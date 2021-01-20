#include "Cursor.h"

void Cyrex::Cursor::Enable() noexcept {
    m_cursorEnabled = true;
    Show();
    Free();
}

void Cyrex::Cursor::Disable(HWND hWnd) noexcept {
    m_cursorEnabled = false;
    Hide();
    Confine(hWnd);
}

bool Cyrex::Cursor::IsEnabled() const noexcept { return m_cursorEnabled; }

void Cyrex::Cursor::Confine(HWND hWnd) noexcept {
    RECT rect;
    GetClientRect(hWnd, &rect);
    MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
    ClipCursor(&rect);
}

void Cyrex::Cursor::Free() noexcept {
    ClipCursor(nullptr);
}

void Cyrex::Cursor::Show() noexcept {
    while (::ShowCursor(TRUE) < 0);
}

void Cyrex::Cursor::Hide() noexcept {
    while (::ShowCursor(FALSE) >= 0);
}
