#include "Mouse.h"
#include "Cursor.h"

Cyrex::Mouse::Mouse()
{
    Cursor cursor;
    this->cursor = &cursor;
}

void Cyrex::Mouse::EnableRawInput() noexcept { m_rawEnabled = true; }

void Cyrex::Mouse::DisableRawInput() noexcept { m_rawEnabled = false; }

void Cyrex::Mouse::OnRawDelta(int dx, int dy) noexcept {
    m_rawDeltaBuffer.push({ dx,dy });
    TrimRawInputBuffer();
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
