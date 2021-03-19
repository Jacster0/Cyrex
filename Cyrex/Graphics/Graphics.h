#pragma once
#include "Lights.h"
#include "Camera.h"

#include "Core/Time/GameTimer.h"
#include "Core/Filesystem/OpenFileDialog.h"

#include "API/DX12/Common.h"
#include "API/DX12/RenderTarget.h"

#include <memory>
#include <array>
#include <thread>
#include <future>

namespace Cyrex {
    struct VertexPosColor {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Color;
    };

    struct LoadingData {
        float LoadingProgress;
        bool IsSceneLoading;
        std::string_view LoadingText;
    };

    enum class VSync;

    class Keyboard;
    class Mouse;
    class CommandQueue;
    class CommandList;
    class Device;
    class Mesh;
    class Scene;
    class PipelineStateObject;
    class EffectPSO;
    class RootSignature;
    class SwapChain;
    class ShaderResourceView;
    class Texture;
    class EditorLayer;
    class EditorContext;

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

        bool LoadScene(const std::string& sceneFile);
        bool LoadingProgress(float loadingProgress);

        void OnMouseWheel(float delta) noexcept;
        void OnMouseMoved(int dx, int dy) noexcept;
        void OnMouseMoved(const Mouse& mouse) noexcept;

        void KeyboardInput(Keyboard& kbd) noexcept;
        void OnOpenFileDialog() noexcept;

        void SetHwnd(HWND hWnd) noexcept { m_hWnd = hWnd; }
        bool IsInitialized() const noexcept { return m_isIntialized; }

        void ToggleVsync();
        VSync GetVsync() const noexcept;
        void SetVsync(VSync vSync) noexcept;

        bool& AnimateLights() noexcept { return m_animateLights; }

        [[nodiscard]] std::tuple<uint32_t, uint32_t> GetScreenSize() const noexcept { return { m_clientWidth, m_clientHeight }; }
        [[nodiscard]] float GetFramesPerSecond() const noexcept { return m_fps; }
        [[nodiscard]] const LoadingData GetLoadingData() const noexcept { return { m_loadingProgress, m_isLoading, m_loadingText }; }
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

        std::unique_ptr<CameraData> m_cameraData;

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
        OpenFileDialog m_fileDialog;

        uint32_t m_clientWidth{};
        uint32_t m_clientHeight{};
        HWND m_hWnd;

        D3D12_VIEWPORT m_viewport;
        D3D12_RECT m_scissorRect;

        VSync m_vsync;
        bool m_isIntialized = false;
        bool m_animateLights = true;
      
        std::shared_ptr<EditorLayer> m_editorLayer;
        std::unique_ptr<EditorContext> m_editorContext;

        std::shared_ptr<Device> m_device;
        std::shared_ptr<SwapChain> m_swapChain;

        std::shared_ptr<EffectPSO> m_lightingPSO;
        std::shared_ptr<EffectPSO> m_decalPSO;
        std::shared_ptr<EffectPSO> m_unlitPSO;

        RenderTarget m_renderTarget;
        GameTimer m_timer;

        std::shared_ptr<Scene> m_scene;
        std::shared_ptr<Scene> m_lightBulb;

        std::shared_ptr<Scene> m_cube;
        std::shared_ptr<Scene> m_sphere;
        std::shared_ptr<Scene> m_flashLight;
        std::shared_ptr<Scene> m_torus;
        std::shared_ptr<Scene> m_plane;

        std::vector<PointLight> m_pointLights;
        std::vector<SpotLight>  m_spotLights;
        std::vector<DirectionalLight> m_directionalLights;
      
        std::atomic_bool  m_isLoading;
        std::future<bool> m_loadingTask;
        float m_loadingProgress;
        std::string m_loadingText;

        float m_fps;
        static constexpr auto m_testScene = "Resources/Models/crytek-sponza/sponza_nobanner.obj";
    };
}
