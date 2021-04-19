#pragma once
#include <cstdint>
#include <optional>
#include <queue>
#include <bitset>

namespace Cyrex {
    enum class KeyCode : uint8_t {
        LButton            = 0x01, 
        RButton            = 0x02,  
        Cancel             = 0x03,  
        MButton            = 0x04,  
        XButton1           = 0x05,  
        XButton2           = 0x06, 
        Back               = 0x08,
        Tab                = 0x09,
        Clear              = 0x0c, 
        Enter              = 0x0d,  
        Shift              = 0x10, 
        Control            = 0x11,  
        Alt                = 0x12,  
        Pause              = 0x13,  
        Capital            = 0x14,  
        CapsLock           = 0x14,  
        KanaMode           = 0x15,  
        HanguelMode        = 0x15,  
        HangulMode         = 0x15,  
        JunjaMode          = 0x17,  
        FinalMode          = 0x18, 
        HanjaMode          = 0x19,  
        KanjiMode          = 0x19,  
        Escape             = 0x1b,  
        IMEConvert         = 0x1c, 
        IMINoConvert       = 0x1d,  
        IMEAccept          = 0x1e,
        IMIModeChange      = 0x1f,  
        Space              = 0x20,  
        Prior              = 0x21,  
        PageUp             = 0x21,  
        Next               = 0x22,  
        PageDown           = 0x22,  
        End                = 0x23,  
        Home               = 0x24,  
        Left               = 0x25,  
        Up                 = 0x26,  
        Right              = 0x27, 
        Down               = 0x28, 
        Select             = 0x29, 
        Print              = 0x2a,  
        Execute            = 0x2b,  
        PrintScreen        = 0x2c,  
        Snapshot           = 0x2c, 
        Insert             = 0x2d,  
        Delete             = 0x2e,  
        Help               = 0x2f,  
        D0                 = 0x30,  
        D1                 = 0x31,  
        D2                 = 0x32, 
        D3                 = 0x33,  
        D4                 = 0x34,  
        D5                 = 0x35,  
        D6                 = 0x36, 
        D7                 = 0x37,  
        D8                 = 0x38,  
        D9                 = 0x39,  
        A                  = 0x41,  
        B                  = 0x42,  
        C                  = 0x43,  
        D                  = 0x44, 
        E                  = 0x45,  
        F                  = 0x46,  
        G                  = 0x47,  
        H                  = 0x48,  
        I                  = 0x49,  
        J                  = 0x4a, 
        K                  = 0x4b,  
        L                  = 0x4c, 
        M                  = 0x4d,  
        N                  = 0x4e,  
        O                  = 0x4f,  
        P                  = 0x50,  
        Q                  = 0x51,  
        R                  = 0x52,  
        S                  = 0x53,  
        T                  = 0x54,  
        U                  = 0x55,  
        V                  = 0x56,  
        W                  = 0x57,  
        X                  = 0x58,  
        Y                  = 0x59,  
        Z                  = 0x5a,  
        LWin               = 0x5b,  
        RWin               = 0x5c,  
        Apps               = 0x5d,  
        Sleep              = 0x5f,  
        NumPad0            = 0x60,  
        NumPad1            = 0x61,  
        NumPad2            = 0x62, 
        NumPad3            = 0x63,  
        NumPad4            = 0x64,  
        NumPad5            = 0x65, 
        NumPad6            = 0x66,  
        NumPad7            = 0x67,  
        NumPad8            = 0x68,  
        NumPad9            = 0x69,  
        Multiply           = 0x6a, 
        Add                = 0x6b,  
        Separator          = 0x6c, 
        Subtract           = 0x6d,  
        Decimal            = 0x6e,  
        Divide             = 0x6f, 
        F1                 = 0x70,  
        F2                 = 0x71,  
        F3                 = 0x72,  
        F4                 = 0x73,  
        F5                 = 0x74,  
        F6                 = 0x75,  
        F7                 = 0x76,  
        F8                 = 0x77,  
        F9                 = 0x78,  
        F10                = 0x79,  
        F11                = 0x7a,  
        F12                = 0x7b,  
        F13                = 0x7c,  
        F14                = 0x7d,  
        F15                = 0x7e,  
        F16                = 0x7f,  
        F17                = 0x80,  
        F18                =  0x81,  
        F19                = 0x82, 
        F20                = 0x83,  
        F21                = 0x84,  
        F22                = 0x85,  
        F23                = 0x86,  
        F24                = 0x87,  
        NumLock            = 0x90,  
        Scroll             = 0x91,  
        LShift             = 0xa0,  
        RShift             = 0xa1,  
        LControl           = 0xa2,  
        RControl           = 0xa3,  
        LMenu              = 0xa4, 
        RMenu              = 0xa5,  
        BrowserBack        = 0xa6, 
        BrowserForward     = 0xa7, 
        BrowserRefresh     = 0xa8, 
        BrowserStop        = 0xa9,  
        BrowserSearch      = 0xaa, 
        BrowserFavorites   = 0xab, 
        BrowserHome        = 0xac, 
        VolumeMute         = 0xad,  
        VolumeDown         = 0xae,  
        VolumeUp           = 0xaf,  
        MediaNextTrack     = 0xb0,  
        MediaPreviousTrack = 0xb1,  
        MediaStop          = 0xb2, 
        MediaPlayPause     = 0xb3,  
        LaunchMail         = 0xb4,  
        SelectMedia        = 0xb5,  
        LaunchApplication1 = 0xb6,  
        LaunchApplication2 = 0xb7,  
        OemSemicolon       = 0xba,    
        Oem1               = 0xba,             
        OemPlus            = 0xbb,  
        OemComma           = 0xbc,  
        OemMinus           = 0xbd, 
        OemPeriod          = 0xbe, 
        OemQuestion        = 0xbf,  
        Oem2               = 0xbf, 
        OemTilde           = 0xc0,  
        Oem3               = 0xc0,  
        OemOpenBrackets    = 0xdb,     
        Oem4               = 0xdb,  
        OemPipe            = 0xdc, 
        Oem5               = 0xdc,  
        OemCloseBrackets   = 0xdd,     
        Oem6               = 0xdd, 
        OemQuotes          = 0xde,  
        Oem7               = 0xde,  
        Oem8               = 0xdf,  
        OemBackslash       = 0xe2, 
        Oem102             = 0xe2,  
        ProcessKey         = 0xe5,  
        Packet             = 0xe7,  
        Attn               = 0xf6,  
        CrSel              = 0xf7, 
        ExSel              = 0xf8, 
        EraseEof           = 0xf9,  
        Play               = 0xfa, 
        Zoom               = 0xfb,  
        NoName             = 0xfc,  
        Pa1                = 0xfd, 
        OemClear           = 0xfe,  
    };

    class Keyboard {
    private:
        class Event {
        public:
            enum class Type {Press, Release};
        public:
            Event(Type type, uint8_t code) noexcept
                :
                type(type),
                code(code)
            {}

            [[nodiscard]] bool IsPress() const noexcept { return type == Type::Press; }
            [[nodiscard]] bool IsRelease() const noexcept { return type == Type::Release; }

            [[nodiscard]] const auto GetCode() const noexcept { return static_cast<KeyCode>(code); }
        private:
            Type type;
            uint8_t code;
        };
    public:
        Keyboard() = default;
        Keyboard(const Keyboard&) = delete;
        Keyboard& operator=(const Keyboard&) = delete;
        ~Keyboard() = default;

        [[nodiscard]] bool KeyIsPressed(KeyCode keycode) const noexcept;
        [[nodiscard]] bool KeyIsPressedOnce(KeyCode keycode) noexcept;
        [[nodiscard]] bool KeyIsPressed(uint8_t keycode) const noexcept;
        [[nodiscard]] bool KeyIsPressedOnce(uint8_t keycode) noexcept;
        [[nodiscard]] std::optional<Event> ReadKey() noexcept;
        [[nodiscard]] bool KeyIsEmpty() const noexcept;
        void FlushKey() noexcept;

        [[nodiscard]] std::optional<char> ReadChar() noexcept;
        [[nodiscard]] bool CharIsEmpty() const noexcept;
        void FlushChar() noexcept;
        void Flush() noexcept;

        void EnableAutorepeat() noexcept;
        void DisableAutorepeat() noexcept;
        [[nodiscard]] bool AutorepeatIsEnabled() const noexcept;

        void OnKeyPressed(KeyCode keycode) noexcept;
        void OnKeyReleased(KeyCode keycode) noexcept;
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
        bool m_keyIsPressedOnce = false;
        std::bitset<m_nKeys> m_keystates;
        std::queue<Event> m_keybuffer;
        std::queue<char> m_charbuffer;
    };
}
