#include "Keyboard.h"

bool Cyrex::Keyboard::KeyIsPressed(uint8_t keycode) const noexcept {
    return m_keystates[keycode];
}

std::optional<Cyrex::Keyboard::Event> Cyrex::Keyboard::ReadKey() noexcept {
    if (m_keybuffer.size() > 0u) {
        Keyboard::Event e = m_keybuffer.front();
        m_keybuffer.pop();
        return e;
    }
    return std::nullopt;
}

bool Cyrex::Keyboard::KeyIsEmpty() const noexcept {
    return m_keybuffer.empty();
}

void Cyrex::Keyboard::FlushKey() noexcept {
    m_keybuffer = std::queue<Event>();
}

std::optional<char> Cyrex::Keyboard::ReadChar() noexcept {
    if (m_charbuffer.size() > 0u) {
        auto charcode = m_charbuffer.front();
        m_charbuffer.pop();
        return charcode;
    }
    return std::nullopt;;
}

bool Cyrex::Keyboard::CharIsEmpty() const noexcept {
    return m_charbuffer.empty();
}

void Cyrex::Keyboard::FlushChar() noexcept {
    m_charbuffer = std::queue<char>();
}

void Cyrex::Keyboard::Flush() noexcept {
    FlushKey();
    FlushChar();
}

void Cyrex::Keyboard::EnableAutorepeat() noexcept {
    m_autorepeatEnabled = true;
}

void Cyrex::Keyboard::DisableAutorepeat() noexcept {
    m_autorepeatEnabled = false;
}

bool Cyrex::Keyboard::AutorepeatIsEnabled() const noexcept {
    return m_autorepeatEnabled;;
}

void Cyrex::Keyboard::OnKeyPressed(uint8_t keycode) noexcept {
    m_keystates[keycode] = true;
    m_keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, keycode));
    TrimBuffer(m_keybuffer);
}

void Cyrex::Keyboard::OnKeyReleased(uint8_t keycode) noexcept {
    m_keystates[keycode] = false;
    m_keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, keycode));
    TrimBuffer(m_keybuffer);
}

void Cyrex::Keyboard::OnChar(char character) noexcept {
    m_charbuffer.push(character);
    TrimBuffer(m_charbuffer);
}

void Cyrex::Keyboard::ClearState() noexcept {
    m_keystates.reset();
}

template<typename T>
void Cyrex::Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept {
    while (buffer.size() > m_bufferSize) {
        buffer.pop();
    }
}