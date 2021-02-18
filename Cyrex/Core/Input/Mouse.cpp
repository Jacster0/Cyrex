#include "Mouse.h"
#include "Cursor.h"

void Cyrex::Mouse::EnableRawInput() noexcept { m_rawEnabled = true; }

void Cyrex::Mouse::DisableRawInput() noexcept { m_rawEnabled = false; }

void Cyrex::Mouse::OnMouseMove(int x, int y) noexcept {
    m_x = x;
    m_y = y;

    m_buffer.push(Mouse::Event(Mouse::Event::Type::Move, *this));
    TrimBuffer();
}

void Cyrex::Mouse::OnMouseLeave() noexcept {
    m_isInWindow = false;

    m_buffer.push(Mouse::Event(Mouse::Event::Type::Leave, *this));
    TrimBuffer();
}

void Cyrex::Mouse::OnMouseEnter() noexcept {
    m_isInWindow = true;

    m_buffer.push(Mouse::Event(Mouse::Event::Type::Enter, *this));
    TrimBuffer();
}

void Cyrex::Mouse::OnRawDelta(int dx, int dy) noexcept {
    m_rawDeltaBuffer.push({ dx,dy });
    TrimRawInputBuffer();
}

void Cyrex::Mouse::OnLeftPressed(int x, int y) noexcept {
    m_leftIsPressed = true;

    m_buffer.push(Mouse::Event(Mouse::Event::Type::LPress, *this));
    TrimBuffer();
}

void Cyrex::Mouse::OnLeftReleased(int x, int y) noexcept {
    m_leftIsPressed = false;

    m_buffer.push(Mouse::Event(Mouse::Event::Type::LRelease, *this));
    TrimBuffer();
}

void Cyrex::Mouse::OnRightPressed(int x, int y) noexcept {
    m_rightIsPressed = true;

    m_buffer.push(Mouse::Event(Mouse::Event::Type::RPress, *this));
    TrimBuffer();
}

void Cyrex::Mouse::OnRightReleased(int x, int y) noexcept {
    m_rightIsPressed = false;

    m_buffer.push(Mouse::Event(Mouse::Event::Type::RRelease, *this));
    TrimBuffer();
}

void Cyrex::Mouse::OnWheelUp(int x, int y) noexcept {
    m_buffer.push(Mouse::Event(Mouse::Event::Type::WheelUp, *this));
    TrimBuffer();
}

void Cyrex::Mouse::OnWheelDown(int x, int y) noexcept {
    m_buffer.push(Mouse::Event(Mouse::Event::Type::WheelDown, *this));
    TrimBuffer();
}

void Cyrex::Mouse::TrimBuffer() noexcept {
    while (m_buffer.size() > m_bufferSize) {
        m_buffer.pop();
    }
}

std::optional<Cyrex::Mouse::RawDelta> Cyrex::Mouse::ReadRawDelta() noexcept {
    if (m_rawDeltaBuffer.empty()) {
        return std::nullopt;
    }

    const RawDelta d = m_rawDeltaBuffer.front();
    m_rawDeltaBuffer.pop();
    return d;
}

void Cyrex::Mouse::TrimRawInputBuffer() noexcept {
    while (m_rawDeltaBuffer.size() > m_bufferSize) {
        m_rawDeltaBuffer.pop();
    }
}

void Cyrex::Mouse::OnWheelDelta(int x, int y, int delta) noexcept {
    m_wheelDelta = delta / static_cast<int>(WHEEL_DELTA);
   
    (m_wheelDelta < 0) ? OnWheelUp(x, y) : OnWheelDown(x, y);
}

std::pair<int, int> Cyrex::Mouse::GetPos() const noexcept {
    return { m_x,m_y };
}

int Cyrex::Mouse::GetPosX() const noexcept {
    return m_x;
}

int Cyrex::Mouse::GetPosY() const noexcept {
    return m_y;
}

int Cyrex::Mouse::GetDeltaX() const noexcept {
    return m_deltaX;
}

int Cyrex::Mouse::GetDeltaY() const noexcept {
    return m_deltaY;
}

bool Cyrex::Mouse::LeftIsPressed() const noexcept {
    return m_leftIsPressed;
}

bool Cyrex::Mouse::RightIsPressed() const noexcept {
    return m_rightIsPressed;
}

bool Cyrex::Mouse::IsInWindow() const noexcept {
    return m_isInWindow;
}

std::optional<Cyrex::Mouse::Event> Cyrex::Mouse::Read() noexcept {
    if (m_buffer.size() > 0u) {
        Mouse::Event e = m_buffer.front();
        m_buffer.pop();

        return e;
   }
    return {};
}

void Cyrex::Mouse::Flush() noexcept {
    m_buffer = std::queue<Event>();
}
