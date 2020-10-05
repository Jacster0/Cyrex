#pragma once
#include <optional>
#include <queue>

namespace Cyrex {
    class Cursor;

    class Mouse {
    public:
        Mouse();
    public:
        struct RawDelta {
            int X;
            int Y;
        };
    public:
        void EnableRawInput() noexcept;
        void DisableRawInput() noexcept;
        void OnRawDelta(int dx, int dy) noexcept;
        std::optional<RawDelta> ReadRawDelta() noexcept;
    public:
        Cursor* cursor;
    private:
        void TrimRawInputBuffer() noexcept;
    private:
        static constexpr unsigned int m_bufferSize = 16u;
        bool m_rawEnabled = false;
        std::queue<RawDelta> m_rawDeltaBuffer;
        friend class Window;
    };
}