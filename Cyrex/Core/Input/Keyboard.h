#pragma once
#include <cstdint>
#include <optional>
#include <queue>
#include <bitset>

namespace Cyrex {
    class Keyboard {
        class Event {
        public:
            enum class Type {Press, Release};
        public:
            Event(Type type, uint8_t code) noexcept
                :
                type(type),
                code(code)
            {}

            bool IsPress() const noexcept { return type == Type::Press; }
            bool IsRelease() const noexcept { return type == Type::Release; }

            auto GetCode() const noexcept { return code; }
        private:
            Type type;
            uint8_t code;
        };
    public:
        Keyboard() = default;
        Keyboard(const Keyboard&) = delete;
        Keyboard& operator=(const Keyboard&) = delete;
        ~Keyboard() = default;
    public:
        bool KeyIsPressed(uint8_t keycode) const noexcept;
        std::optional<Event> ReadKey() noexcept;
        bool KeyIsEmpty() const noexcept;
        void FlushKey() noexcept;

        std::optional<char> ReadChar() noexcept;
        bool CharIsEmpty() const noexcept;
        void FlushChar() noexcept;
        void Flush() noexcept;

        void EnableAutorepeat() noexcept;
        void DisableAutorepeat() noexcept;
        bool AutorepeatIsEnabled() const noexcept;
    public:
        void OnKeyPressed(uint8_t keycode) noexcept;
        void OnKeyReleased(uint8_t keycode) noexcept;
        void OnChar(char character) noexcept;
        void ClearState() noexcept;

        template<typename T>
        static void TrimBuffer(std::queue<T>& buffer) noexcept;
    private:
        static constexpr unsigned int m_nKeys = 256u;
        static constexpr unsigned int m_bufferSize = 16u;
        bool m_autorepeatEnabled = false;
        std::bitset<m_nKeys> m_keystates;
        std::queue<Event> m_keybuffer;
        std::queue<char> m_charbuffer;
    };
}
