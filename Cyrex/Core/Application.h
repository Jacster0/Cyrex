#pragma once
#include <optional>
#include <memory>

namespace Cyrex {
    class Graphics;
    class Window;
    class Application {
    public:
        Application();
        Application(const Application& rhs) = delete;
        Application& operator=(const Application& rhs) = delete;
        ~Application();
    public:
        int Run();
    private:
        void HandleInput() noexcept;
        void KeyboardInput() noexcept;
        std::optional<int> MessagePump();
    private:
        std::unique_ptr<Window> m_wnd;
        std::shared_ptr<Graphics> m_gfx = nullptr;
    };
}

