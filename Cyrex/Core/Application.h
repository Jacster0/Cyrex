#pragma once
#include <optional>
#include <memory>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace Cyrex {
    class Graphics;
    class Window;

    class Application {
    public:
        Application();
        Application(const Application& rhs) = delete;
        Application& operator=(const Application& rhs) = delete;
        Application(const Application&& rhs) = delete;
        Application& operator=(const Application&& rhs) = delete;
        ~Application();
 
        int Run();
    private:
        void Initialize() noexcept;
        void HandleInput() noexcept;
        void KeyboardInput() noexcept;
        void MouseInput() noexcept;
        std::optional<int> MessagePump() noexcept;

        std::unique_ptr<Window> m_window = nullptr;
        std::unique_ptr<Graphics> m_gfx = nullptr;

        bool m_isInitialized = false;
    };
}

