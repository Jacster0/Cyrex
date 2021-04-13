#pragma once
#include <DirectXMath.h>
#include <d3d12.h>

#include "Core/Math/Common.h"

namespace Cyrex {
    class VertexPosition {
    public:
        VertexPosition() = default;
        explicit VertexPosition(const DirectX::XMFLOAT3& position)
            :
            Position(position) 
        {}
        explicit VertexPosition(DirectX::FXMVECTOR position) {
            DirectX::XMStoreFloat3(&(this->Position), position);
        }

        DirectX::XMFLOAT3 Position;
        static const D3D12_INPUT_LAYOUT_DESC InputLayout;
    private:
        static constexpr int InputElementCount = 1;
        static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };

    class VertexPositionNormalTangentBitangentTexture {
    public:
        VertexPositionNormalTangentBitangentTexture() = default;

        explicit VertexPositionNormalTangentBitangentTexture(
            const Math::Vector3& position,
            const Math::Vector3& normal,
            const Math::Vector3& texCoord,
            const Math::Vector3& tangent = { 0, 0, 0},
            const Math::Vector3& bitangent = { 0, 0, 0})
            : 
            Position(position), 
            Normal(normal), 
            Tangent(tangent), 
            Bitangent(bitangent), 
            TexCoord(texCoord)
        {}

        Math::Vector3 Position;
        Math::Vector3 Normal;
        Math::Vector3 Tangent;
        Math::Vector3 Bitangent;
        Math::Vector3 TexCoord;

        static const D3D12_INPUT_LAYOUT_DESC InputLayout;
    private:
        static constexpr int inputElementCount = 5;
        static const D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount];
    };
}