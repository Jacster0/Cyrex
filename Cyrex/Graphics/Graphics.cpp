#include "Graphics.h"
#include "Material.h"
#include "Scene.h"
#include "SceneNode.h"
#include "SceneVisitor.h"
#include "EffectPSO.h"

#include "Managers/TextureManager.h"
#include "Managers/SceneManager.h"

#include "Editor/Editor.h"

#include "Core/Logger.h"
#include "Core/Input/Keyboard.h"
#include "Core/Input/Mouse.h"
#include "Core/Math/Math.h"
#include "Core/Utils/StringUtils.h"

#include "API/DX12/DXException.h"
#include "API/DX12/CommandQueue.h"
#include "API/DX12/CommandList.h"
#include "API/DX12/RootSignature.h"
#include "API/DX12/Device.h"
#include "API/DX12/Swapchain.h"
#include "API/DX12/GeometryGenerator.h"
#include "API/DX12/Mesh.h"
#include "API/DX12/RenderTarget.h"
#include "API/DX12/Texture.h"
#include "API/DX12/ShaderResourceView.h"
#include "API/DX12/VertexTypes.h"

#include <chrono>
#include <cstdlib>

#include <ShObjIdl.h>

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

using namespace Cyrex;
using namespace DirectX;

dx::XMMATRIX XM_CALLCONV LookAtMatrix(dx::FXMVECTOR Position, dx::FXMVECTOR Direction, dx::FXMVECTOR Up) {
    assert(!dx::XMVector3Equal(Direction, dx::XMVectorZero()));
    assert(!dx::XMVector3IsInfinite(Direction));
    assert(!dx::XMVector3Equal(Up, dx::XMVectorZero()));
    assert(!dx::XMVector3IsInfinite(Up));

    dx::XMVECTOR R2 = dx::XMVector3Normalize(Direction);

    dx::XMVECTOR R0 = dx::XMVector3Cross(Up, R2);
    R0 = dx::XMVector3Normalize(R0);

    dx::XMVECTOR R1 = dx::XMVector3Cross(R2, R0);

    dx::XMMATRIX M(R0, R1, R2, Position);

    return M;
}

static constexpr auto g_longMax = std::numeric_limits<long>::max();

Graphics::Graphics()
    :
    m_scissorRect(CD3DX12_RECT(0, 0, g_longMax, g_longMax)),
    m_vsync(VSync::Off)
{
    dx::XMVECTOR cameraPos    = dx::XMVectorSet(0, 5.0f, -20, 1);
    dx::XMVECTOR cameraTarget = dx::XMVectorSet(0, 5, 0, 1);
    dx::XMVECTOR cameraUp     = dx::XMVectorSet(0, 1, 0, 0);

    m_camera.SetLookAt(cameraPos, cameraTarget, cameraUp);

    m_cameraData = std::make_unique<CameraData>();
   
    m_cameraData->InitialCamPos = m_camera.GetTranslation();
    m_cameraData->InitialCamRot = m_camera.GetRotation();
    m_cameraData->InitialCamFov = m_camera.GetFov();

    std::vector<COMDLG_FILTERSPEC> fileFilters = { {
        { L"All Files",                              L"*.*"          },
        { L"Autodesk",                               L"*.fbx"        },
        { L"Collada",                                L"*.dae"        },
        { L"glTF",                                   L"*.gltf;*.glb" },
        { L"Blender 3D",                             L"*.blend"      },
        { L"3ds Max 3DS",                            L"*.3ds"        },
        { L"3ds Max ASE",                            L"*.ase"        },
        { L"Wavefront Object",                       L"*.obj"        },
        { L"Industry Foundation Classes (IFC/Step)", L"*.ifc"        },
        { L"XGL",                                    L"*.xgl;*.zgl"  },
        { L"Stanford Polygon Library",               L"*.ply"        },
        { L"AutoCAD DXF",                            L"*.dxf"        },
        { L"LightWave",                              L"*.lws"        },
        { L"LightWave Scene",                        L"*.lws"        },
        { L"Modo",                                   L"*.lxo"        },
        { L"Stereolithography",                      L"*.stl"        },
        { L"DirectX X",                              L"*.x"          },
        { L"AC3D",                                   L"*.ac"         },
        { L"Milkshape 3D",                           L"*.ms3d"       },
        { L"TrueSpace",                              L"*.cob;*.scn"  },
        { L"Ogre XML",                               L"*.xml"        },
        { L"Irrlicht Mesh",                          L"*.irrmesh"    },
        { L"Irrlicht Scene",                         L"*.irr"        },
        { L"Quake I",                                L"*.mdl"        },
        { L"Quake II",                               L"*.md2"        },
        { L"Quake III",                              L"*.md3"        },
        { L"Quake III Map/BSP",                      L"*.pk3"        },
        { L"Return to Castle Wolfenstein",           L"*.mdc"        },
        { L"Doom 3",                                 L"*.md5*"       },
        { L"Valve Model",                            L"*.smd;*.vta"  },
        { L"Open Game Engine Exchange",              L"*.ogx"        },
        { L"Unreal",                                 L"*.3d"         },
        { L"BlitzBasic 3D",                          L"*.b3d"        },
        { L"Quick3D",                                L"*.q3d;*.q3s"  },
        { L"Neutral File Format",                    L"*.nff"        },
        { L"Sense8 WorldToolKit",                    L"*.nff"        },
        { L"Object File Format",                     L"*.off"        },
        { L"PovRAY Raw",                             L"*.raw"        },
        { L"Terragen Terrain",                       L"*.ter"        },
        { L"Izware Nendo",                           L"*.ndo"        }, }
    };

    m_fileDialog.SetFilter(std::move(fileFilters));
    m_fileDialog.SetFilterIndex(0);
}

Graphics::~Graphics() { }

void Graphics::Initialize(uint32_t width, uint32_t height) {
    //Check for DirectX Math suport
    if (!DirectX::XMVerifyCPUSupport()) {
        crxlog::err("No support for DirectX Math found");
        m_isIntialized = false;
        return;
    }

    m_device = Device::Create();

    //Log the adapter 
    crxlog::wlog(L"Adapter: ", m_device->GetDescription(), Logger::WNewLine());

    m_swapChain = m_device->CreateSwapChain(m_hWnd, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_tearingSupported = m_swapChain->IsTearingSupported();

    m_swapChain->SetVsync(m_vsync);

    m_editor = std::make_unique<Editor>(*m_device, m_hWnd, m_swapChain->GetRenderTarget());
    Editor::SetDarkThemeColors();
   
    m_clientWidth  = width;
    m_clientHeight = height;

    LoadContent();
    Resize(width, height);

    m_isIntialized = true;
}

void Graphics::Update() noexcept {
    using namespace DirectX;

    static uint64_t frameCount = 0;

    m_timer.Tick();
    frameCount++;  

    if (m_timer.GetElapsedSeconds() > 1.0) {
        m_fps = frameCount / m_timer.GetElapsedSeconds();

        frameCount = 0;
        m_timer.ResetElapsedTime();
    }
    if (m_showOpenFileDialog) {
        m_showOpenFileDialog = false;
        OnOpenFileDialog();
    }
     //Wait for the swapchain before updating the camera in order to reduce input lag
    m_swapChain->WaitForSwapChain();

    UpdateCamera();
    UpdateLights();
}

void Graphics::LoadContent() {
    //Load the scene asynchronously
    m_loadingTask = std::async(std::launch::async, [&]() -> bool { return LoadScene(m_testScene); });

    auto& commandQueue = m_device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto  commandList  = commandQueue.GetCommandList();

    auto fence = commandQueue.ExecuteCommandList(commandList);

    //Create PSO's
    m_lightingPSO = std::make_shared<EffectPSO>(m_device, EnableLighting::True,  EnableDecal::False);
    m_decalPSO    = std::make_shared<EffectPSO>(m_device, EnableLighting::True,  EnableDecal::True);
    m_unlitPSO    = std::make_shared<EffectPSO>(m_device, EnableLighting::False, EnableDecal::False);

    // Create a color buffer with sRGB for gamma correction.
    const auto backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    const auto depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

    const auto sampleDesc = m_device->GetMultisampleQualityLevels(backBufferFormat);

    // Create an off-screen render target with a single color buffer and a depth buffer.
    const auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        backBufferFormat, 
        m_clientWidth, 
        m_clientHeight, 
        1u, 
        1u, 
        sampleDesc.Count,
        sampleDesc.Quality, 
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    auto clr = Colors::LightSteelBlue;
    D3D12_CLEAR_VALUE colorClearValue;
    colorClearValue.Format   = colorDesc.Format;
    colorClearValue.Color[0] = XMVectorGetX(clr);
    colorClearValue.Color[1] = XMVectorGetY(clr);
    colorClearValue.Color[2] = XMVectorGetZ(clr);
    colorClearValue.Color[3] = 1.0f;

    auto colorTexture = m_device->CreateTexture(colorDesc, &colorClearValue);
    colorTexture->SetName(L"Color Render Target");

    auto depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        depthBufferFormat, 
        m_clientWidth, 
        m_clientHeight, 
        1u, 
        1u, 
        sampleDesc.Count,
        sampleDesc.Quality, 
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    D3D12_CLEAR_VALUE depthClearValue;
    depthClearValue.Format       = depthDesc.Format;
    depthClearValue.DepthStencil = { 1.0f, 0 };

    auto depthTexture = m_device->CreateTexture(depthDesc, &depthClearValue);
    depthTexture->SetName(L"Depth Render Target");

    // Attach the textures to the render target.
    m_renderTarget.AttachTexture(AttachmentPoint::Color0, colorTexture);
    m_renderTarget.AttachTexture(AttachmentPoint::DepthStencil, depthTexture);

    // Make sure the copy command queue is finished before leaving this function.
   /* commandQueue.WaitForFenceValue(fence);*/
}

void Graphics::UnLoadContent() noexcept { 
    m_loadingTask.get();
}

bool Cyrex::Graphics::LoadScene(const std::string& sceneFile) {
    m_isLoading     = true;

    auto& commandQueue = m_device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList   = commandQueue.GetCommandList();

    m_loadingText = std::string("Loading ") + sceneFile + "...";

    auto scene = SceneManager::LoadSceneFromFile(*commandList, sceneFile, 
        [&](float loadingProgress) -> bool { 
            return LoadingProgress(loadingProgress); 
        });

    if (scene) [[likely]] {
        BoundingSphere boudningSphere;
        BoundingSphere::CreateFromBoundingBox(boudningSphere, scene->GetAABB());

        auto scale = 50.0f / (boudningSphere.Radius * 2.0f);
        boudningSphere.Radius *= scale;

        scene->GetRootNode()->SetLocalTransform(dx::XMMatrixScaling(scale, scale, scale));

        auto cameraFov        = m_camera.GetFov();
        auto distanceToObject = boudningSphere.Radius / std::tanf(dx::XMConvertToRadians(cameraFov) / 2.0f);

        auto cameraPosition = dx::XMVectorSet(0, 0, -distanceToObject, 1);
        auto focusPoint     = dx::XMVectorSet(boudningSphere.Center.x * scale, boudningSphere.Center.y * scale, boudningSphere.Center.z * scale, 1.0f);

        cameraPosition = cameraPosition + focusPoint;

       
        m_camera.SetTranslation(cameraPosition);

        m_scene = scene;
    }

    commandQueue.ExecuteCommandList(commandList);
    commandQueue.Flush();

    m_isLoading = false;

    return scene != nullptr;
}

bool Cyrex::Graphics::LoadingProgress(float loadingProgress) {
    m_loadingProgress = loadingProgress;

    return true;
}

void Graphics::Render() {
    using namespace DirectX;

    auto& commandQueue = m_device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList   = commandQueue.GetCommandList();

    const auto& renderTarget = m_isLoading ? m_swapChain->GetRenderTarget() : m_renderTarget;

    //If we are still loading the scene, just clear the screen with a blue color.
    if (m_isLoading) {
        TextureManager::ClearTexture(*commandList, renderTarget.GetTexture(AttachmentPoint::Color0), Colors::LightSteelBlue);
    }
    else {
        //Create the scene visitors
        SceneVisitor opaquePass(*commandList, m_camera, *m_lightingPSO, RenderPass::Opaque);
        SceneVisitor transparentPass(*commandList, m_camera, *m_decalPSO, RenderPass::Transparent);

        // Clear the render targets.
        TextureManager::ClearTexture(
            *commandList,
            renderTarget.GetTexture(AttachmentPoint::Color0),
            DirectX::Colors::LightSteelBlue);

        TextureManager::ClearDepthStencilTexture(
            *commandList,
            renderTarget.GetTexture(AttachmentPoint::DepthStencil),
            D3D12_CLEAR_FLAG_DEPTH);

        commandList->SetViewport(m_viewport);
        commandList->SetScissorRect(m_scissorRect);
        commandList->SetRenderTarget(m_renderTarget);

        //render the scene
        if (m_scene) {
            m_scene->Accept(opaquePass);
            m_scene->Accept(transparentPass);
        }
       
        auto swapChainBuffer  = m_swapChain->GetRenderTarget().GetTexture(AttachmentPoint::Color0);
        auto msaaRenderTarget = renderTarget.GetTexture(AttachmentPoint::Color0);

        commandList->ResolveSubresource(swapChainBuffer, msaaRenderTarget);
    }

    OnGUI(commandList, m_swapChain->GetRenderTarget());
    commandQueue.ExecuteCommandList(commandList);
    m_swapChain->Present();
}

void Graphics::Resize(uint32_t width, uint32_t height) {
    m_clientWidth  = std::max(1u, width);
    m_clientHeight = std::max(1u, height);

    float aspectRatio = m_clientWidth / static_cast<float>(m_clientHeight);
    m_camera.SetProj(45.0f, aspectRatio, 0.1f, 100.0f);

    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_clientWidth), static_cast<float>(m_clientHeight));

    m_renderTarget.Resize(m_clientWidth, m_clientHeight);
    m_swapChain->Resize(m_clientWidth, m_clientHeight);
}

void Graphics::ToggleVsync() {
    m_swapChain->ToggleVsync();
}

void Graphics::UpdateCamera() noexcept {
    using namespace DirectX;

    const float speedFactor = m_cameraControls.Sneak ? 4.0f : 16.0f;
    const float dt = static_cast<float>(m_timer.GetDeltaSeconds());

    auto translate = XMVectorSet(
        m_cameraControls.Right - m_cameraControls.Left,
        0.0f,
        m_cameraControls.Forward - m_cameraControls.Backward,
        1.0f) * speedFactor * dt;

    auto pan = XMVectorSet(0.0f, m_cameraControls.Up - m_cameraControls.Down, 0.0f, 1.0f) * speedFactor * dt;

    m_camera.Translate(translate);
    m_camera.Translate(pan);

    auto cameraRotation = XMQuaternionRotationRollPitchYaw(
        XMConvertToRadians(m_cameraControls.Pitch),
        XMConvertToRadians(m_cameraControls.Yaw),
        0.0f);

    m_camera.SetRotation(cameraRotation);
}

void Graphics::UpdateLights() noexcept {
    XMMATRIX viewMatrix = m_camera.GetView();

    static constexpr auto pi = Math::MathConstants::pi_float;
    const int numDirectionalLights = 3;

    static const std::array lightColors = { Colors::White, Colors::White, Colors::White };

    static float lightAnimTime = 0.0f;

    if (m_animateLights) {
        lightAnimTime += static_cast<float>(m_timer.GetDeltaSeconds()) * 0.5f * pi;
    }

    constexpr float radius = 1.0f;

    float directionalLightOffset = numDirectionalLights > 0 ? 2.0f * pi / numDirectionalLights : 0;

    m_directionalLights.resize(numDirectionalLights);

    for (int i = 0; i < numDirectionalLights; i++) {
        DirectionalLight& light = m_directionalLights[i];

        float angle = lightAnimTime + directionalLightOffset * i;

        XMVECTORF32 worldSpacePosition = { static_cast<float>(std::cos(angle)) * radius,
                                           static_cast<float>(std::sin(angle)) * radius, radius, 1.0f };

        XMVECTOR worldSpaceDirection = XMVector3Normalize(XMVectorNegate(worldSpacePosition));
        XMVECTOR viewSpaceDirection  = XMVector3TransformNormal(worldSpaceDirection, viewMatrix);

        XMStoreFloat4(&light.WorldSpaceDirection, worldSpaceDirection);
        XMStoreFloat4(&light.ViewSpaceDirection, viewSpaceDirection);

        light.Color = XMFLOAT4(lightColors[i]);
    }

    m_lightingPSO->SetDirectionalLights(m_directionalLights);

    m_decalPSO->SetDirectionalLights(m_directionalLights);
}

void Graphics::OnMouseWheel(float delta) noexcept {
    float fov = m_camera.GetFov();

    fov -= delta;
    fov = std::clamp(fov, 12.0f, 90.0f);

    m_camera.SetFov(fov);

    crxlog::info("Fov: ", fov);
}

void Graphics::OnMouseMoved(int dx, int dy) noexcept {
    //Represent a fps camera
    constexpr auto mouseSpeed = 0.1f;

    m_cameraControls.Pitch += dy * mouseSpeed;
    m_cameraControls.Pitch = std::clamp(m_cameraControls.Pitch, -90.0f, 90.0f);

    m_cameraControls.Yaw += dx * mouseSpeed;
}

void Graphics::OnMouseMoved(const Mouse& mouse) noexcept {
    //What kind of camera is this? Inverted fps camera? Whatever.
    //When we are not reading raw mouse movements and our 
    //cursor is present, we are using this camera.
    constexpr auto mouseSpeed = 0.1f;

    if (mouse.LeftIsPressed()) {
        m_cameraControls.Pitch -= mouse.GetDeltaY() * mouseSpeed;
        m_cameraControls.Pitch  = std::clamp(m_cameraControls.Pitch, -90.0f, 90.0f);

        m_cameraControls.Yaw -= mouse.GetDeltaX() * mouseSpeed;
    }
}

void Cyrex::Graphics::OnGUI(const std::shared_ptr<CommandList>& commandList, const RenderTarget& renderTarget)
{
    m_editor->NewFrame();

    if (m_isLoading) {
        // Show a progress bar.
        ImGui::SetNextWindowPos(
            ImVec2(m_clientWidth / 2.0f, m_clientHeight / 2.0f), 
            0,
            ImVec2(0.5, 0.5));
        ImGui::SetNextWindowSize(ImVec2(m_clientWidth / 2.0f, 0));

        ImGui::Begin("Loading ", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

        ImGui::ProgressBar(m_loadingProgress);
        ImGui::Text(m_loadingText.c_str());

        ImGui::End();
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open file...", "Ctrl+O", nullptr, !m_isLoading)) {
                m_showOpenFileDialog = true;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Options")) {
            auto vSync = static_cast<bool>(m_swapChain->GetVsync());
            if (ImGui::MenuItem("V-Sync", "V", &vSync)) {
                m_swapChain->SetVsync(static_cast<VSync>(vSync));

                const auto val = (vSync) ? "On" : "Off";
                crxlog::info("Toggled Vsync: ", val);
            }

            ImGui::MenuItem("Animate Lights", "Space", &m_animateLights);
            ImGui::EndMenu();
        }
        
        static constexpr auto BUFFER_SIZE = 256;
        char buffer[BUFFER_SIZE];
        {
            sprintf_s(buffer, BUFFER_SIZE, "FPS: %.2f (%.2f ms)  ", m_fps, 1.0 / m_fps * 1000.0);

            const auto fpsTextSize = ImGui::CalcTextSize(buffer);

            ImGui::SameLine((float)m_clientWidth  - fpsTextSize.x);
            ImGui::Text(buffer);
        }
      
        ImGui::EndMainMenuBar();
    }
    m_editor->Render(commandList, renderTarget);
}

void Graphics::KeyboardInput(Keyboard& kbd) noexcept {
    if (ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }

    m_cameraControls.Forward  = static_cast<float>(kbd.KeyIsPressed('W'));
    m_cameraControls.Left     = static_cast<float>(kbd.KeyIsPressed('A'));
    m_cameraControls.Backward = static_cast<float>(kbd.KeyIsPressed('S'));
    m_cameraControls.Right    = static_cast<float>(kbd.KeyIsPressed('D'));
    m_cameraControls.Down     = static_cast<float>(kbd.KeyIsPressed('Q'));
    m_cameraControls.Up       = static_cast<float>(kbd.KeyIsPressed('E'));
    m_cameraControls.Sneak    = (kbd.KeyIsPressed(VK_SHIFT));

    if (kbd.KeyIsPressed('R')) {
        m_camera.SetTranslation(m_cameraData->InitialCamPos);
        m_camera.SetRotation(m_cameraData->InitialCamRot);
        m_camera.SetFov(m_cameraData->InitialCamFov);

        m_cameraControls.Pitch = 0.0f;
        m_cameraControls.Yaw   = 0.0f;
    }
}

void Cyrex::Graphics::OnOpenFileDialog() noexcept {
    if (m_fileDialog.Open() == DialogResult::OK) {
        const auto& filePath = ToNarrow(m_fileDialog.GetDisplayName(SIGDN_FILESYSPATH));

        m_loadingTask = std::async(std::launch::async, [this, filePath]() -> bool { return LoadScene(filePath); });
    }
}
