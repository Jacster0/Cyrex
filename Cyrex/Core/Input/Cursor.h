#pragma once
#include "Platform/Windows/CrxWindow.h"

namespace Cyrex {
    class Cursor {
    public:
        void Enable() noexcept;
        void Disable(HWND hWnd) noexcept;
        bool IsEnabled() const noexcept;
    public:
        bool IsInWindow() const noexcept { return m_isInWindow; }
    private:
        void Confine(HWND hWnd) noexcept;
        void Free() noexcept;
        void Show() noexcept;
        void Hide() noexcept;
    private:
        bool m_cursorEnabled = true;
        bool m_isInWindow = false;
        friend class Window;
    };
}

