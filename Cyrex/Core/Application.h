#pragma once
#include <optional>
#include <memory>
#include <wrl.h>
#include <d3d12.h>

namespace Cyrex {
    class Graphics;
    class Window;
    class Application {
    private:
        Application();
    public:
        Application(const Application& rhs) = delete;
        Application& operator=(const Application& rhs) = delete;
    public:
        int Run();
    public:
        static Application& Get() noexcept;
        Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const noexcept;
    private:
        void HandleInput() noexcept;
        void KeyboardInput() noexcept;
        std::optional<int> MessagePump();
    private:
        std::unique_ptr<Window> m_wnd;
        std::shared_ptr<Graphics> m_gfx = nullptr;
    };
}

