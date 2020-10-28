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
    class CommandQueue;
    class Window;
    class Application {
    private:
        Application();
    public:
        Application(const Application& rhs) = delete;
        Application& operator=(const Application& rhs) = delete;
    public:
        static void Create();
        int Run();
        void Flush() const;
    public:
        static Application& Get() noexcept;
        Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const noexcept;
        std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const noexcept;
        bool IsTearingSupported() const noexcept { return m_tearingSupported; }
        static uint64_t& FrameCount() noexcept { return ms_frameCount; }
    private:
        void Initialize();
        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
        Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);
        void CreateCommandQueue();
        bool CheckTearingSupport();
        void EnableDebugLayer() const;
    private:
        void HandleInput() noexcept;
        void KeyboardInput() noexcept;
        std::optional<int> MessagePump();
    private:
        Microsoft::WRL::ComPtr<ID3D12Device2> m_device;

        std::shared_ptr<CommandQueue> m_directCommandQueue;
        std::shared_ptr<CommandQueue> m_computeCommandQueue;
        std::shared_ptr<CommandQueue> m_copyCommandQueue;

        std::unique_ptr<Window> m_wnd = nullptr;
        std::shared_ptr<Graphics> m_gfx = nullptr;
        static uint64_t ms_frameCount;
        bool m_tearingSupported = false;
        bool m_isInitialized = false;
    };
}

