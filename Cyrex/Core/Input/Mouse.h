#pragma once
#include <optional>
#include <queue>
#include "Cursor.h"

namespace Cyrex {
    class Mouse {
    public:
        struct RawDelta {
            int X;
            int Y;
        };
        class Event {
        public:
            enum class Type {
                LPress,
                LRelease,
                RPress,
                RRelease,
                WheelUp,
                WheelDown,
                Move,
                Enter,
                Leave
            };
        private:
            Type type;
            bool leftIsPressed;
            bool rightIsPressed;
            int x;
            int y;
        public:
            Event(Type type, Mouse& mouse)
                :
                type(type),
                leftIsPressed(mouse.m_leftIsPressed),
                rightIsPressed(mouse.m_rightIsPressed),
                x(mouse.m_x),
                y(mouse.m_y)
            {}

            Type GetType() const noexcept { return type; }
            std::pair<int, int> GetPos() const noexcept {  return{ x,y }; }

            int GetPosX() const noexcept { return x; }
            int GetPosY() const noexcept { return y; }

            bool LeftIsPressed() const noexcept { return leftIsPressed; }
            bool RightIsPressed() const noexcept { return rightIsPressed; }
        };
    public:
        Mouse() = default;
        Mouse(const Mouse&) = delete;
        Mouse& operator=(const Mouse&) = delete;

        int GetWheelDelta() const noexcept { return m_wheelDelta; }
        std::pair<int, int> GetPos() const noexcept;
      
        int GetPosX() const noexcept;
        int GetPosY() const noexcept;

        int GetDeltaX() const noexcept;
        int GetDeltaY() const noexcept;
 
        bool LeftIsPressed() const noexcept;
        bool RightIsPressed() const noexcept;
        bool IsInWindow() const noexcept;

        std::optional<Mouse::Event> Read() noexcept;
        bool IsEmpty() const noexcept { return m_buffer.empty(); }
        void Flush() noexcept;

        void EnableRawInput() noexcept;
        void DisableRawInput() noexcept;
        std::optional<RawDelta> ReadRawDelta() noexcept;

        Cursor cursor;
    private:
        void OnMouseMove(int x, int y) noexcept;
        void OnMouseLeave() noexcept;
        void OnMouseEnter() noexcept;

        void OnLeftPressed(int x, int y) noexcept;
        void OnLeftReleased(int x, int y) noexcept;

        void OnRightPressed(int x, int y) noexcept;
        void OnRightReleased(int x, int y) noexcept;

        void OnWheelUp(int x, int y) noexcept;
        void OnWheelDown(int x, int y) noexcept;

        void TrimBuffer() noexcept;
        void TrimRawInputBuffer() noexcept;

        void OnWheelDelta(int x, int y, int delta) noexcept;
        void OnRawDelta(int dx, int dy) noexcept;

        static constexpr unsigned int m_bufferSize = 16u;
        int m_x = 0;
        int m_y = 0;
        float m_deltaX = 0;
        float m_deltaY = 0;
        bool m_leftIsPressed = false;
        bool m_rightIsPressed = false;
        bool m_isInWindow = false;
        int m_wheelDelta = 0;
        bool m_rawEnabled = false;
        std::queue<RawDelta> m_rawDeltaBuffer;
        std::queue<Event> m_buffer;
        friend class Window;
    };
}