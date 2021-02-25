#pragma once

#include "Lights.h"
#include <DirectXMath.h>
#include <memory>
#include <vector>

namespace Cyrex {
    enum class EnableLighting : bool { True = true, False = false };
    enum class EnableDecal    : bool { True = true, False = false };

    class CommandList;
    class Device;
    class Material;
    class RootSignature;
    class PipelineStateObject;
    class ShaderResourceView;
    class Texture;

    class EffectPSO {
    public:
        struct LightProperties {
            uint32_t NumPointLights;
            uint32_t NumSpotLights;
            uint32_t NumDirectionalLights;
        };

        // Transformation matrices for the vertex shader
        struct alignas(16) Matrices {
            DirectX::XMMATRIX ModelMatrix;
            DirectX::XMMATRIX ModelViewMatrix;
            DirectX::XMMATRIX InverseTransposeModelViewMatrix;
            DirectX::XMMATRIX ModelViewProjectionMatrix;
        };

        enum RootParameters {
            //Vertex shader parameter
            MatricesCB, //ConstantBuffer<Matrices> MatCB : register(b0);

            //Pixel shader parameters
            MaterialCB,         //ConstantBuffer<Material> MaterialCB               : register( b0, space1 );
            LightPropertiesCB,  //ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );

            PointLights,        // StructuredBuffer<PointLight> PointLights             : register( t0 );
            SpotLights,         // StructuredBuffer<SpotLight> SpotLights               : register( t1 );
            DirectionalLights,  // StructuredBuffer<DirectionalLight> DirectionalLights : register( t2 )

            Textures,  // Texture2D AmbientTexture       : register( t3 );
                       // Texture2D EmissiveTexture      : register( t4 );
                       // Texture2D DiffuseTexture       : register( t5 );
                       // Texture2D SpecularTexture      : register( t6 );
                       // Texture2D SpecularPowerTexture : register( t7 );
                       // Texture2D NormalTexture        : register( t8 );
                       // Texture2D BumpTexture          : register( t9 );
                       // Texture2D OpacityTexture       : register( t10 );
            NumRootParameters
        };

        EffectPSO(std::shared_ptr<Device> device, EnableLighting enableLighting, EnableDecal enableDecal);

        [[nodiscard]] const std::vector<PointLight>& GetPointLights() const noexcept { return m_pointLights; }
        void SetPointLights(const std::vector<PointLight>& pointLights) noexcept {
            m_pointLights = pointLights;
            m_dirtyFlags |= DF_PointLights;
        }

        [[nodiscard]] const std::vector<SpotLight>& GetSpotLights() const noexcept { return m_spotLights; }
        void SetSpotLights(const std::vector<SpotLight>& spotLights) noexcept {
            m_spotLights = spotLights;
            m_dirtyFlags |= DF_SpotLights;
        }

        [[nodiscard]] const std::vector<DirectionalLight>& GetDirectionalLights() const noexcept {
            return m_directionalLights;
        }
        void SetDirectionalLights(const std::vector<DirectionalLight>& directionalLights) noexcept {
            m_directionalLights = directionalLights;
            m_dirtyFlags |= DF_DirectionalLights;
        }

        [[nodiscard]] const std::shared_ptr<Material>& GetMaterial() const noexcept { return m_material; }
        void SetMaterial(const std::shared_ptr<Material>& material) noexcept {
            m_material = material;
            m_dirtyFlags |= DF_Material;
        }

        [[nodiscard]] DirectX::XMMATRIX GetWorldMatrix() const noexcept { return m_pMVP->World; }
        void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX worldMatrix)  noexcept {
            m_pMVP->World = worldMatrix;
            m_dirtyFlags |= DF_Matrices;
        }
       
        [[nodiscard]] DirectX::XMMATRIX GetViewMatrix() const noexcept { return m_pMVP->View; }
        void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX viewMatrix) noexcept {
            m_pMVP->View = viewMatrix;
            m_dirtyFlags |= DF_Matrices;
        }
       
        [[nodiscard]] DirectX::XMMATRIX GetProjectionMatrix() const noexcept  { return m_pMVP->Projection; }
        void XM_CALLCONV SetProjectionMatrix(DirectX::FXMMATRIX projectionMatrix) noexcept {
            m_pMVP->Projection = projectionMatrix;
            m_dirtyFlags |= DF_Matrices;
        }

        void Apply(CommandList& commandList);
    private:
        enum DirtyFlags {
            DF_None              = 0,
            DF_PointLights       = (1 << 0),
            DF_SpotLights        = (1 << 1),
            DF_DirectionalLights = (1 << 2),
            DF_Material          = (1 << 3),
            DF_Matrices          = (1 << 4),
            DF_All = DF_PointLights | DF_SpotLights | DF_DirectionalLights | DF_Material | DF_Matrices
        };

        struct alignas(16) MVP {
            DirectX::XMMATRIX World;
            DirectX::XMMATRIX View;
            DirectX::XMMATRIX Projection;
        };

        inline void BindTexture(CommandList& commandList, uint32_t offset, const std::shared_ptr<Texture>& texture);

        std::shared_ptr<Device> m_device;
        std::shared_ptr<RootSignature> m_rootSignature;
        std::shared_ptr<PipelineStateObject> m_pipelineStateObject;

        std::vector<PointLight> m_pointLights;
        std::vector<SpotLight> m_spotLights;
        std::vector<DirectionalLight> m_directionalLights;

        std::shared_ptr<Material> m_material;
        std::shared_ptr<ShaderResourceView> m_defaultSRV;

        std::unique_ptr<MVP> m_pMVP;
        CommandList* m_pPreviousCommandList{ nullptr };

        uint32_t m_dirtyFlags{ DF_All };

        bool m_enableLighting;
        bool m_enableDecal;
    };
}