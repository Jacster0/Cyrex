#include "Graphics.h"
#include "Material.h"
#include "Scene.h"
#include "SceneNode.h"
#include "SceneVisitor.h"
#include "EffectPSO.h"

#include "Managers/TextureManager.h"
#include "Managers/SceneManager.h"

#include "Editor/D3D12Layer.h"
#include "Editor/EditorContext.h"
#include "Editor/LightsEditorPanel.h"

#include "Core/Logger.h"
#include "Core/Input/Keyboard.h"
#include "Core/Input/Mouse.h"
#include "Core/Math/Math.h"
#include "Core/Math/Common.h"

#include "API/DX12/DXException.h"
#include "API/DX12/CommandQueue.h"
#include "API/DX12/CommandList.h"
#include "API/DX12/RootSignature.h"
#include "API/DX12/Device.h"
#include "API/DX12/Swapchain.h"
#include "GeometryGenerator.h"
#include "Mesh.h"
#include "API/DX12/RenderTarget.h"
#include "API/DX12/Texture.h"
#include "API/DX12/ShaderResourceView.h"
#include "API/DX12/VertexTypes.h"

#include <chrono>
#include <cstdlib>

#include <ShObjIdl.h>

import StringUtils;

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

using namespace Cyrex;
using namespace Cyrex::Math;
using namespace DirectX;

Graphics::Graphics()
    :
    m_vsync(VSync::Off)
{
    m_pointLights.resize(1);
    m_spotLights.resize(1);
    m_directionalLights.resize(1);

    Vector4 cameraPos{ 0, 5.0f, -20, 1 };
    Vector4 cameraTarget{ 0, 5, 0, 1 };
    Vector4 cameraUp{ 0, 1, 0, 0 };

    m_camera.SetLookAt(cameraPos, cameraTarget, cameraUp);
  
   
    m_cameraData.InitialCamPos = m_camera.GetTranslation();
    m_cameraData.InitialCamRot = m_camera.GetRotation();
    m_cameraData.InitialCamFov = m_camera.GetFov();

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

    m_swapChain = std::make_shared<SwapChain>(*m_device, m_hWnd, DXGI_FORMAT_R8G8B8A8_UNORM);

    m_editorLayer   = std::make_shared<D3D12Layer>(*m_device, *m_swapChain, m_hWnd);
    m_editorContext = std::make_unique<EditorContext>(*this);

    m_editorLayer->Attach();
    m_editorLayer->SetThemeColors(EditorTheme::Dark);
 
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

    m_lightBulb  = GeometryGenerator::CreateSphere(commandList, 0.1f);
    m_flashLight = GeometryGenerator::CreateCone(commandList,0.1f,0.5f);

    auto fence = commandQueue.ExecuteCommandList(commandList);

    //Create PSO's
    m_lightingPSO  = std::make_shared<EffectPSO>(*m_device, EnableLighting::True,  EnableDecal::False);
    m_decalPSO     = std::make_shared<EffectPSO>(*m_device, EnableLighting::True,  EnableDecal::True);
    m_unlitPSO     = std::make_shared<EffectPSO>(*m_device, EnableLighting::False, EnableDecal::False);

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
    colorClearValue.Color[0] = dx::XMVectorGetX(clr);
    colorClearValue.Color[1] = dx::XMVectorGetY(clr);
    colorClearValue.Color[2] = dx::XMVectorGetZ(clr);
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
    commandQueue.WaitForFenceValue(fence);
}

void Graphics::UnLoadContent() noexcept { 
    m_loadingTask.get();
    m_editorLayer->Detach();
}

bool Graphics::LoadScene(const std::string& sceneFile) {
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

        scene->GetRootNode()->SetLocalTransform(Matrix::CreateScale(scale));

        auto cameraFov        = m_camera.GetFov();
        auto distanceToObject = boudningSphere.Radius / std::tanf(Math::ToRadians(cameraFov) / 2.0f);

        Vector4 cameraPosition{ 0, 0, -distanceToObject, 1 };
        Vector4 focusPoint{ boudningSphere.Center.x * scale, boudningSphere.Center.y * scale, boudningSphere.Center.z * scale, 1.0f };

        cameraPosition = cameraPosition + focusPoint;

        m_cameraData.InitialCamPos = cameraPosition;
        m_cameraData.InitialCamFov = cameraFov;

        m_camera.SetTranslation(cameraPosition);

        m_scene = scene;
    }

    commandQueue.ExecuteCommandList(commandList);
    commandQueue.Flush();

    m_isLoading = false;

    return scene != nullptr;
}

bool Graphics::LoadingProgress(float loadingProgress) {
    m_loadingProgress = loadingProgress;

    return true;
}

void Cyrex::Graphics::SetViewPortSize(float x, float y) noexcept {
    m_viewport.Width  = x;
    m_viewport.Height = y;
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
        SceneVisitor unlitPass(*commandList, m_camera, *m_unlitPSO, RenderPass::Opaque);

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
       

        MaterialProperties lightMaterial = Material::Black;
        for (const auto& l : m_pointLights) {
            lightMaterial.Emissive = l.Color;
            auto& lightPos   = l.WorldSpacePosition;
            auto worldMatrix = Matrix::CreateTranslation(lightPos);

            m_lightBulb->GetRootNode()->SetLocalTransform(worldMatrix);
            m_lightBulb->GetRootNode()->GetMesh()->GetMaterial()->SetMaterialProperties(lightMaterial);
            m_lightBulb->Accept(unlitPass);
        }

        for (const auto& l : m_spotLights) {
            lightMaterial.Emissive = l.Color;
            auto& lightPos         = l.WorldSpacePosition;
            auto worldMatrix       = Matrix::CreateTranslation(lightPos);

            m_flashLight->GetRootNode()->SetLocalTransform(worldMatrix);
            m_flashLight->GetRootNode()->GetMesh()->GetMaterial()->SetMaterialProperties(lightMaterial);
            m_flashLight->Accept(unlitPass);
        }

        auto swapChainBuffer  = m_swapChain->GetRenderTarget().GetTexture(AttachmentPoint::Color0);
        auto msaaRenderTarget = renderTarget.GetTexture(AttachmentPoint::Color0);

        commandList->ResolveSubresource(swapChainBuffer, msaaRenderTarget);
    }

    ////Render the UI
    m_editorContext->Render(m_editorLayer, *commandList);
    //Execute the recorded commands
    commandQueue.ExecuteCommandList(commandList);
    //Present the final rendered image to the screen
    m_swapChain->Present();
}

void Graphics::Resize(uint32_t width, uint32_t height) {
    m_clientWidth  = std::max(1u, width);
    m_clientHeight = std::max(1u, height);

    m_viewport = { 0.0f, 0.0f, static_cast<float>(m_clientWidth), static_cast<float>(m_clientHeight), 0.0f, 1.0f };

    m_camera.SetProj(45.0f, m_viewport.AspectRatio(), 0.1f, 1000.0f);

    m_renderTarget.Resize(m_clientWidth, m_clientHeight);
    m_swapChain->Resize(m_clientWidth, m_clientHeight);
}

void Graphics::ToggleVsync() {
    m_swapChain->ToggleVsync();
}

VSync Graphics::GetVsync() const noexcept {
    return m_swapChain->GetVsync();
}

void Graphics::SetVsync(VSync vSync) noexcept {
    m_swapChain->SetVsync(vSync);
}

void Graphics::UpdateCamera() noexcept {
    const float speedFactor = m_cameraControls.Sneak ? 4.0f : 16.0f;
    const float dt          = static_cast<float>(m_timer.GetDeltaSeconds());

    Vector4 translate = Vector4(
        m_cameraControls.Right   - m_cameraControls.Left,
        m_cameraControls.Up      - m_cameraControls.Down,
        m_cameraControls.Forward - m_cameraControls.Backward,
        1.0f ) * speedFactor * dt;

    m_camera.Translate(translate);

    const auto& cameraRotation = Quaternion::FromPitchYawRoll(
        Math::ToRadians(m_cameraControls.Pitch),
        Math::ToRadians(m_cameraControls.Yaw), 
        0.0f); 

    m_camera.SetRotation(cameraRotation);
}

void Graphics::UpdateLights() noexcept {
    const auto viewMatrix = m_camera.GetView();

    //Load the lighproperties from the editor
    const auto& lightProperties      = m_editorContext->Get<LightProperties>();

    auto& directionalLightProperties = lightProperties.mDirectionalLightProperties;
    auto& pointLightProperties       = lightProperties.mPointLightProperties;
    auto& spotLightProperties        = lightProperties.mSpotLightProperties;

    //Pointlight...
    auto& pointLight                = m_pointLights[0];
    pointLight.Color                = pointLightProperties.Color;
    pointLight.ConstantAttenuation  = pointLightProperties.ConstantAttenuation;
    pointLight.LinearAttenuation    = pointLightProperties.LinearAttenuation;
    pointLight.QuadraticAttenuation = pointLightProperties.QuadraticAttenuation;
    pointLight.Ambient              = pointLightProperties.Ambient;
    pointLight.WorldSpacePosition   = pointLightProperties.Position;

    auto& pointLightWorldSpacePosition = pointLight.WorldSpacePosition;
    pointLight.ViewSpacePosition       = Vector4::TransformCoord(pointLightWorldSpacePosition, viewMatrix);

    //Directional light...
    auto& directionalLight                 = m_directionalLights[0];
    directionalLight.Ambient               = directionalLightProperties.Ambient;
    directionalLight.Color                 = directionalLightProperties.Color;
    directionalLight.WorldSpaceDirection.x = std::sin(Math::ToRadians(directionalLightProperties.Angle));
    directionalLight.WorldSpaceDirection.y = std::cos(Math::ToRadians(directionalLightProperties.Angle));

    auto& directionalLightWorldSpaceDirection = directionalLight.WorldSpaceDirection;
    directionalLightWorldSpaceDirection       = Vector4::Normalize(Vector4::Negate(directionalLightWorldSpaceDirection));

    directionalLight.WorldSpaceDirection = directionalLightWorldSpaceDirection;
    directionalLight.ViewSpaceDirection  =  Vector4::TransformNormal(directionalLightWorldSpaceDirection, viewMatrix);

    //Spotlight...
    auto& spotLight                 = m_spotLights[0];
    spotLight.Color                 = spotLightProperties.Color;
    spotLight.ConstantAttenuation   = spotLightProperties.ConstantAttenuation;
    spotLight.LinearAttenuation     = spotLightProperties.LinearAttenuation;
    spotLight.QuadraticAttenuation  = spotLightProperties.QuadraticAttenuation;
    spotLight.Ambient               = spotLightProperties.Ambient;
    spotLight.WorldSpacePosition    = spotLightProperties.Position;
    spotLight.SpotAngle             = Math::ToRadians(spotLightProperties.SpotAngle);
    spotLight.WorldSpaceDirection.x = std::sin(Math::ToRadians(spotLightProperties.Direction));
    spotLight.WorldSpaceDirection.y = std::cos(Math::ToRadians(spotLightProperties.Direction));

    auto spotLightWorldSpaceDirection = spotLight.WorldSpaceDirection;
    spotLightWorldSpaceDirection      = Vector4::Normalize(Vector4::Negate(spotLightWorldSpaceDirection));

    auto spotLightWorldSpacePosition  = spotLight.WorldSpacePosition;

    spotLight.WorldSpaceDirection = spotLightWorldSpaceDirection;
    spotLight.ViewSpaceDirection  = Vector4::TransformNormal(spotLightWorldSpaceDirection, viewMatrix);
    spotLight.ViewSpacePosition   = Vector4::TransformCoord(spotLightWorldSpacePosition, viewMatrix);

    //Set the lights in the PSO
    m_lightingPSO->SetSpotLights(m_spotLights);
    m_lightingPSO->SetPointLights(m_pointLights);
    m_lightingPSO->SetDirectionalLights(m_directionalLights);

    m_decalPSO->SetSpotLights(m_spotLights);
    m_decalPSO->SetPointLights(m_pointLights);
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
    m_cameraControls.Pitch  = std::clamp(m_cameraControls.Pitch, -90.0f, 90.0f);

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

void Graphics::KeyboardInput(Keyboard& kbd) noexcept {
    if (ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }

    m_cameraControls.Forward  = static_cast<float>(kbd.KeyIsPressed(KeyCode::W));
    m_cameraControls.Left     = static_cast<float>(kbd.KeyIsPressed(KeyCode::A));
    m_cameraControls.Backward = static_cast<float>(kbd.KeyIsPressed(KeyCode::S));
    m_cameraControls.Right    = static_cast<float>(kbd.KeyIsPressed(KeyCode::D));
    m_cameraControls.Down     = static_cast<float>(kbd.KeyIsPressed(KeyCode::Q));
    m_cameraControls.Up       = static_cast<float>(kbd.KeyIsPressed(KeyCode::E));
    m_cameraControls.Sneak    = kbd.KeyIsPressed(KeyCode::Shift);

    if (kbd.KeyIsPressedOnce(KeyCode::R)) {
        m_camera.SetTranslation(m_cameraData.InitialCamPos);
        m_camera.SetRotation(m_cameraData.InitialCamRot);
        m_camera.SetFov(m_cameraData.InitialCamFov);

        m_cameraControls.Pitch = 0.0f;
        m_cameraControls.Yaw   = 0.0f;
    }
}

void Graphics::OnOpenFileDialog() noexcept {
    if (m_fileDialog.Open() == DialogResult::OK) {
        const auto& filepath = m_fileDialog.GetFilePath();

        m_loadingTask = std::async(std::launch::async, [this,filepath] { return LoadScene(ToNarrow(filepath)); });
    }
}
