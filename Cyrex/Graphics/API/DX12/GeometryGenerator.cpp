#include "GeometryGenerator.h"
#include "CommandList.h"
#include "Core/Math/Math.h"
#include "VertexTypes.h"
#include "Graphics/Managers/SceneManager.h"
#include <array>
#include <stdexcept>
#include <algorithm>
#include <DirectXMath.h>


namespace dx = DirectX;
namespace cx = Cyrex;
using namespace Cyrex;

using VertexCollection = std::vector<VertexPositionNormalTangentBitangentTexture>;
using IndexCollection = std::vector<uint16_t>;

inline void ReverseWinding(IndexCollection& indices, VertexCollection& vertices) {
    assert((indices.size() % 3) == 0);

    for (auto it = indices.begin(); it != indices.end(); it += 3) {
        std::swap(*it, *(it + 2));
    }

    for (auto it = vertices.begin(); it != vertices.end(); ++it) {
        it->TexCoord.x = (1.f - it->TexCoord.x);
    }
}

inline void CreateCylinderCap(
    VertexCollection& vertices,
    IndexCollection& indices,
    size_t tesselation,
    float height,
    float radius,
    float isTop)
{
    using namespace DirectX;

    for (size_t i = 0; i < tesselation - 2; i++) {
        size_t i1 = (i + 1) % tesselation;
        size_t i2 = (i + 2) % tesselation;

        if (isTop) {
            std::swap(i1, i2);
        }

        size_t base = vertices.size();
        indices.push_back(static_cast<uint16_t>(base + i2));
        indices.push_back(static_cast<uint16_t>(base + i1));
        indices.push_back(static_cast<uint16_t>(base));
    }

    XMVECTOR normal = g_XMIdentityR1;
    XMVECTOR texScale = g_XMNegativeOneHalf;

    if (!isTop) {
        normal = XMVectorNegate(normal);
        texScale = XMVectorMultiply(texScale, g_XMNegateX);
    }

    // Create cap vertices
    for (size_t i = 0; i < tesselation; i++) {
        XMVECTOR circleVector = cx::Math::GetCircleVector(i, tesselation);
        XMVECTOR pos = XMVectorAdd(XMVectorScale(circleVector, radius), XMVectorScale(normal, height));
        XMVECTOR texCoord = XMVectorMultiplyAdd(XMVectorSwizzle<0, 2, 3, 3>(circleVector), texScale, g_XMOneHalf);

        vertices.emplace_back(pos, normal, texCoord);
    }
}

std::shared_ptr<Scene> GeometryGenerator::CreateCube(
    const std::shared_ptr<CommandList>& commandList, 
    float size, bool reverseWinding)
{
    using namespace DirectX;
    assert(commandList);

    float s = size * 0.5f;

    //8 edges
    XMFLOAT3 p[8] = {
        {s,s,-s}, {s,s,s}, {s,-s,s}, {s,-s,-s},
        {-s,s,s}, {-s,s,-s}, {-s,-s,-s}, {-s,-s,s}
    };

    // 6 face normals
    XMFLOAT3 n[6] = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };

    // 4 unique texcoords
    XMFLOAT3 t[4] = { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 } };

    // Indices for the vertex positions.
    uint16_t i[24] = {
        0, 1, 2, 3,  // +X
        4, 5, 6, 7,  // -X
        4, 1, 0, 5,  // +Y
        2, 7, 6, 3,  // -Y
        1, 4, 7, 2,  // +Z
        5, 0, 3, 6   // -Z
    };

    VertexCollection vertices;
    IndexCollection  indices;

    for (uint16_t f = 0; f < 6; ++f) {
        // Four vertices per face.
        vertices.emplace_back(p[i[f * 4 + 0]], n[f], t[0]);
        vertices.emplace_back(p[i[f * 4 + 1]], n[f], t[1]);
        vertices.emplace_back(p[i[f * 4 + 2]], n[f], t[2]);
        vertices.emplace_back(p[i[f * 4 + 3]], n[f], t[3]);

        // First triangle.
        indices.emplace_back(f * 4 + 0);
        indices.emplace_back(f * 4 + 1);
        indices.emplace_back(f * 4 + 2);

        // Second triangle
        indices.emplace_back(f * 4 + 2);
        indices.emplace_back(f * 4 + 3);
        indices.emplace_back(f * 4 + 0);
    }

    if (reverseWinding) {
        ReverseWinding(indices, vertices);
    }
    
    return SceneManager::CreateScene(*commandList, vertices, indices);
}

std::shared_ptr<Scene> GeometryGenerator::CreateSphere(
    const std::shared_ptr<CommandList>& commandList, 
    float radius, 
    uint32_t tessellation,
    bool reverseWinding)
{
    using namespace DirectX;
    using namespace Cyrex::Math;

    assert(commandList);

    VertexCollection vertices;
    IndexCollection indices;

    if (tessellation < 3) {
        throw std::out_of_range("Tessellation parameter out of range");
    }

    size_t verticalSegments   = tessellation;
    size_t horizontalSegments = tessellation * 2;

    for (size_t i = 0; i <= verticalSegments; i++) {
        float v        = static_cast<float>(i) / verticalSegments;
        float latidude = (i * MathConstants::pi_float / verticalSegments) - MathConstants::pi_div2;
        float dy;
        float dxz;

        XMScalarSinCos(&dy, &dxz, latidude);

        // Create a single ring of vertices at this latitude.
        for (size_t j = 0; j <= horizontalSegments; j++) {
            float u         = static_cast<float>(j) / horizontalSegments;
            float longitude = j * MathConstants::pi_mul2 / horizontalSegments;
            float dx;
            float dz;

            XMScalarSinCos(&dx, &dz, longitude);

            dx *= dxz;
            dz *= dxz;

            XMVECTOR normal   = XMVectorSet(dx, dy, dz, 0);
            XMVECTOR texCoord = XMVectorSet(u, v, 0, 0);
            auto pos = normal * radius;

            vertices.emplace_back(pos, normal, texCoord);
        }
    }

    // Fill the index buffer with triangles joining each pair of latitude rings.
    size_t stride = horizontalSegments + 1;

    for (size_t i = 0; i < verticalSegments; i++) {
        for (size_t j = 0; j <= horizontalSegments; j++) {
            const size_t nextI = i + 1;
            const size_t nextJ = (j + 1) % stride;

            indices.push_back(static_cast<uint16_t>(i * stride + nextJ));
            indices.push_back(static_cast<uint16_t>(nextI * stride + j));
            indices.push_back(static_cast<uint16_t>(i * stride + j));

            indices.push_back(static_cast<uint16_t>(nextI * stride + nextJ));
            indices.push_back(static_cast<uint16_t>(nextI * stride + j));
            indices.push_back(static_cast<uint16_t>(i * stride + nextJ));
        }
    }

    if (reverseWinding) {
        ReverseWinding(indices, vertices);
    }

    return SceneManager::CreateScene(*commandList, vertices, indices);
}

std::shared_ptr<Scene> GeometryGenerator::CreateCone(
    const std::shared_ptr<CommandList>& commandList, 
    float radius,
    float height, 
    uint32_t tessellation, 
    bool reverseWinding)
{
    using namespace DirectX;

    assert(commandList);

    VertexCollection vertices;
    IndexCollection  indices;

    if (tessellation < 3) {
        throw std::out_of_range("Tessellation parameter out of range");
    }

    height /= 2;

    XMVECTOR topOffset = XMVectorScale(g_XMIdentityR1, height);
    size_t stride = tessellation + 1;

    for (size_t i = 0; i <= tessellation; i++) {
        XMVECTOR circleVector = cx::Math::GetCircleVector(i, tessellation);
        XMVECTOR sideOffset   = XMVectorScale(circleVector, radius);

        float u = static_cast<float>(i) / static_cast<float>(tessellation);

        XMVECTOR texCoord = XMLoadFloat(&u);
        XMVECTOR pt       = XMVectorSubtract(sideOffset,topOffset);
        XMVECTOR normal   = XMVector3Cross(cx::Math::GetCircleTangent(i, tessellation), XMVectorSubtract(topOffset,pt));
        normal            = XMVector3Normalize(normal);

        vertices.emplace_back(topOffset, normal, g_XMZero);
        vertices.emplace_back(pt, normal, XMVectorAdd(texCoord, g_XMIdentityR1));

        indices.push_back((i * 2 + 1) % (stride * 2));
        indices.push_back((i * 2 + 3) % (stride * 2));
        indices.push_back(i * 2);
    }

    CreateCylinderCap(vertices, indices, tessellation, height, radius, false);

    if (reverseWinding) {
        ReverseWinding(indices, vertices);
    }

    return SceneManager::CreateScene(*commandList, vertices, indices);
}

std::shared_ptr<Scene> GeometryGenerator::CreateTorus(
    const std::shared_ptr<CommandList>& commandList, 
    float radius, 
    float thickness, 
    uint32_t tessellation,
    bool reverseWinding)
{
    using namespace DirectX;
    using namespace Cyrex::Math;

    assert(commandList);

    VertexCollection vertices;
    IndexCollection  indices;

    if (tessellation < 3)
        throw std::out_of_range("tessellation parameter out of range");

    size_t stride = tessellation + 1;

    //loop around the main ring of the torus
    for (size_t i = 0; i <= tessellation; i++) {
        float u           = static_cast<float>(i) / tessellation;
        float outerAngle = i * MathConstants::pi_mul2 / tessellation - MathConstants::pi_div2;

        XMMATRIX transform = XMMatrixTranslation(radius, 0, 0) * XMMatrixRotationY(outerAngle);

        //loop along the other axis
        for (size_t j = 0; j <= tessellation; j++) {
            float v          = 1 - static_cast<float>(j) / tessellation;
            float innerAngle = j * MathConstants::pi_mul2 / tessellation + MathConstants::pi_float;

            float dx;
            float dy;

            XMScalarSinCos(&dy, &dx, innerAngle);

            //Create a vertex
            XMVECTOR normal   = dx::XMVectorSet(dx, dy, 0, 0);
            XMVECTOR pos      = normal * thickness / 2;
            XMVECTOR texCoord = dx::XMVectorSet(u, v, 0, 0);

            pos    = XMVector3Transform(pos, transform);
            normal = XMVector3TransformNormal(normal, transform);

            vertices.emplace_back(pos, normal, texCoord);

            //create indices for two triangles
            size_t nextI = (i + 1) % stride;
            size_t nextJ = (j + 1) % stride;

            indices.push_back(nextI * stride + j);
            indices.push_back(i * stride + nextJ);
            indices.push_back(i * stride + j);

            indices.push_back(nextI * stride + j);
            indices.push_back(nextI * stride + nextJ);
            indices.push_back(i * stride + nextJ);
        }
    }

    if (reverseWinding) {
        ReverseWinding(indices, vertices);
    }

    return SceneManager::CreateScene(*commandList, vertices, indices);
}

std::shared_ptr<Scene> GeometryGenerator::CreatePlane(
    const std::shared_ptr<CommandList>& commandList, 
    float width, 
    float height, 
    bool reverseWinding)
{
    assert(commandList);

    using namespace DirectX;
    using Vertex = VertexPositionNormalTangentBitangentTexture;

    VertexCollection vertices = {
        Vertex(XMFLOAT3(-0.5f * width, 0.0f,  0.5f * height),  XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f)),
        Vertex(XMFLOAT3(0.5f *  width, 0.0f,  0.5f * height),  XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)),
        Vertex(XMFLOAT3(0.5f *  width, 0.0f, -0.5f * height),  XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f)),
        Vertex(XMFLOAT3(-0.5f * width, 0.0f, -0.5f * height),  XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f))
    };

    IndexCollection indices = { 1, 3, 0, 2, 3, 1 };

    if (reverseWinding) {
        ReverseWinding(indices, vertices);
    }

    return SceneManager::CreateScene(*commandList, vertices, indices);
}
