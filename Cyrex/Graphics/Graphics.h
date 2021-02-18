#pragma once
#include "API/DX12/Common.h"
#include "Core/Time/GameTimer.h" 
#include "API/DX12/RenderTarget.h"
#include "Lights.h"
#include "Camera.h"
#include <memory>
#include <array>

namespace Cyrex {
    struct VertexPosColor {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Color;
    };

    class Keyboard;
    class Mouse;
    class CommandQueue;
    class Device;
    class Mesh;
    class Scene;
    class PipelineStateObject;
    class RootSignature;
    class SwapChain;
    class ShaderResourceView;
    class Texture;

    class Graphics {
    public:
        Graphics();
        Graphics(const Graphics& rhs) = delete;
        Graphics& operator=(const Graphics& rhs) = delete;
        Graphics(const Graphics&& rhs) = delete;
        Graphics& operator=(const Graphics&& rhs) = delete;
        ~Graphics();

        void Initialize(uint32_t width, uint32_t height);
        void Update() noexcept;
        void Render();
        void Resize(uint32_t width, uint32_t height);
        void LoadContent();
        void UnLoadContent() noexcept;

        void OnMouseWheel(float delta) noexcept;
        void OnMouseMoved(int dx, int dy) noexcept;
        void OnMouseMoved(const Mouse& mouse) noexcept;

        void KeyboardInput(const Keyboard& kbd) noexcept;

        void SetHwnd(HWND hWnd) noexcept { m_hWnd = hWnd; }
        bool IsInitialized() const noexcept { return m_isIntialized; }
        void ToggleVsync();
    private:
        void UpdateCamera() noexcept;
        void UpdateLights() noexcept;
        static constexpr uint8_t m_bufferCount = 3;

        Camera m_camera;

        struct alignas(16) CameraData {
            DirectX::XMVECTOR InitialCamPos;
            DirectX::XMVECTOR InitialCamRot;
           float InitialCamFov;
        };

        CameraData* m_cameraData;

        struct CameraControls {
            float Forward  = 0;
            float Backward = 0;
            float Left     = 0;
            float Right    = 0;
            float Up       = 0;
            float Down     = 0;

            float Pitch    = 0;
            float Yaw      = 0;

            bool Sneak = false;
        };

        CameraControls m_cameraControls;

        uint32_t m_clientWidth{};
        uint32_t m_clientHeight{};
        HWND m_hWnd;

        D3D12_VIEWPORT m_viewport;
        D3D12_RECT m_scissorRect;

        bool m_vsync = true;
        bool m_tearingSupported;
        bool m_isIntialized = false;
        bool m_animateLights = true;
      
        std::shared_ptr<Device> m_device;
        std::shared_ptr<RootSignature> m_rootSignature;
        std::shared_ptr<SwapChain> m_swapChain;

        std::shared_ptr<PipelineStateObject> m_pipelineState;
        std::shared_ptr<PipelineStateObject> m_unlitPipelineState;

        RenderTarget m_renderTarget;
        GameTimer m_timer;

        std::shared_ptr<Scene> m_cube;
        std::shared_ptr<Scene> m_sphere;
        std::shared_ptr<Scene> m_cone;
        std::shared_ptr<Scene> m_torus;
        std::shared_ptr<Scene> m_plane;

        std::shared_ptr<Texture> m_defaultTexture;
        std::shared_ptr<Texture> m_directXTexture;
        std::shared_ptr<Texture> m_earthTexture;
        std::shared_ptr<Texture> m_monaLisaTexture;

        std::shared_ptr<ShaderResourceView> m_defaultTextureView;
        std::shared_ptr<ShaderResourceView> m_directXTextureView;
        std::shared_ptr<ShaderResourceView> m_earthTextureView;
        std::shared_ptr<ShaderResourceView> m_monaLisaTextureView;

        std::vector<PointLight> m_pointLights;
        std::vector<SpotLight>  m_spotLights;
    };
}
