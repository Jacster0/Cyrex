#include "GeometryGenerator.h"
#include "API/DX12/CommandList.h"
#include "Core/Math/Math.h"
#include "Graphics/Managers/SceneManager.h"
#include <array>
#include <stdexcept>
#include <algorithm>
#include "Graphics/API/DX12/VertexTypes.h"
#include "Core/Math/Common.h"
#include <Core/Math/Matrix.h>

namespace cx = Cyrex;
using namespace Cyrex;
using namespace Cyrex::Math;

using VertexCollection = std::vector<VertexPositionNormalTangentBitangentTexture>;
using IndexCollection  = std::vector<uint16_t>;

enum class CylinderCap {Top, Bottom};

inline void ReverseWinding(IndexCollection& indices, VertexCollection& vertices) {
    assert((indices.size() % 3) == 0);

    for (auto it = indices.begin(); it != indices.end(); it += 3) {
        std::swap(*it, *(it + 2));
    }

    for (auto it = vertices.begin(); it != vertices.end(); ++it) {
        it->TexCoord.x = (1.f - it->TexCoord.x);
    }
}

void BuildCylinderCap(
    float bottomRadius,
    float topRadius,
    float height,
    CylinderCap cap,
    uint32_t numSlices,
    uint32_t numStacks,
    VertexCollection& vertices,
    IndexCollection& indices) noexcept
{
    const float one_half = (cap == CylinderCap::Top) ? 0.5f : -0.5f;
    const auto baseIndex = vertices.size();
    const float y        = height * one_half;
    const float dTheta   = 2.0f * MathConstants::PI / numSlices;

    Vector4 normal  = (cap == CylinderCap::Top) ? Vector4(0,1,0,0) : Vector4(0, -1, 0,0);
    Vector4 tangent = { 1,0,0,0 };

    for (uint32_t i = 0; i <= numSlices; i++) {
        const float x = topRadius * std::cos(i * dTheta);
        const float z = topRadius * std::sin(i * dTheta);
        const float u = x / height + 0.5f;
        const float v = z / height + 0.5f;

        vertices.emplace_back(Vector4(x, y, z,0), normal, Vector2(u, v), tangent);
    }

    vertices.emplace_back(Vector4(0, y, 0,0), normal, Vector2(0.5f, 0.5f), tangent);

    uint32_t centerIndex = vertices.size() - 1;

    for (uint32_t i = 0; i < numSlices; i++) {
        indices.push_back(centerIndex);

        if (cap == CylinderCap::Top) {
            indices.push_back(baseIndex + i + 1);
            indices.push_back(baseIndex + i);
        }
        else {
            indices.push_back(baseIndex + i);
            indices.push_back(baseIndex + i + 1);
        }
        
    }
}

std::shared_ptr<Scene> GeometryGenerator::CreateCube(
    const std::shared_ptr<CommandList>& commandList, 
    float size, bool reverseWinding)
{
    VertexCollection vertices;
    IndexCollection indices;

    // Cube is centered at 0,0,0.
    float s = size * 0.5f;

    Vector4 v[8] = { { s, s, -s,0 }, { s, s, s ,0, }, { s, -s, s,0 },   { s, -s, -s,0 },
                     { -s, s, s ,0}, { -s, s, -s ,0}, { -s, -s, -s ,0}, { -s, -s, s ,0} };
    // 6 face normals
    Vector4 n[6] = { { 1, 0, 0,0}, { -1, 0, 0,0 }, { 0, 1, 0 ,0}, { 0, -1, 0 ,0}, { 0, 0, 1 ,0}, { 0, 0, -1 ,0} };
    // 4 unique texture coordinates
    Vector2 uv[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };

    uint16_t i[24] = {
       0, 1, 2, 3,  // +X
       4, 5, 6, 7,  // -X
       4, 1, 0, 5,  // +Y
       2, 7, 6, 3,  // -Y
       1, 4, 7, 2,  // +Z
       5, 0, 3, 6   // -Z
    };

    for (uint16_t faces = 0; faces < 6; faces++)
    {
        // Four vertices per face.
        vertices.emplace_back(v[i[faces * 4 + 0]], n[faces], uv[0]);
        vertices.emplace_back(v[i[faces * 4 + 1]], n[faces], uv[1]);
        vertices.emplace_back(v[i[faces * 4 + 2]], n[faces], uv[2]);
        vertices.emplace_back(v[i[faces * 4 + 3]], n[faces], uv[3]);

        // First triangle.
        indices.emplace_back(faces * 4 + 0);
        indices.emplace_back(faces * 4 + 1);
        indices.emplace_back(faces * 4 + 2);

        // Second triangle
        indices.emplace_back(faces * 4 + 2);
        indices.emplace_back(faces * 4 + 3);
        indices.emplace_back(faces * 4 + 0);
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

    const size_t verticalSegments   = tessellation;
    const size_t horizontalSegments = tessellation * 2;

    for (size_t i = 0; i <= verticalSegments; i++) {
        float v        = static_cast<float>(i) / verticalSegments;
        float latidude = (i * MathConstants::PI / verticalSegments) - MathConstants::PI_DIV2;

        float dy  = std::sin(latidude);
        float dxz = std::cos(latidude);

        // Create a single ring of vertices at this latitude.
        for (size_t j = 0; j <= horizontalSegments; j++) {
            float u         = static_cast<float>(j) / horizontalSegments;
            float longitude = j * MathConstants::PI_MUL2 / horizontalSegments;

            float dx = std::sin(longitude);
            float dz = std::cos(longitude);

            dx *= dxz;
            dz *= dxz;

            Vector4 normal(dx, dy, dz,0);
            Vector2 texCoord(u, v);

            auto pos = normal * radius;

            vertices.emplace_back(pos, normal, texCoord);
        }
    }

    // Fill the index buffer with triangles joining each pair of latitude rings.
    const size_t stride = horizontalSegments + 1;

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

std::shared_ptr<Scene> GeometryGenerator::CreateCylinder(
    const std::shared_ptr<CommandList>& commandList, 
    float topRadius, 
    float bottomRadius,
    float height,
    bool reverseWinding,
    int numSlices, 
    int numStacks)
{
    VertexCollection vertices;
    IndexCollection indices;

    const float stackHeight = height / numStacks;
    const float radiusStep  = (topRadius - bottomRadius) / numStacks;
    const float ringCount   = static_cast<float>(numStacks + 1);

    for (uint32_t i = 0; i < ringCount; i++) {
        const float y      = -0.5f * height + i * stackHeight;
        const float r      = bottomRadius + i * radiusStep;
        const float dTheta = 2.0f * MathConstants::PI / numSlices;

        for (uint32_t j = 0; j <= numSlices; j++) {
            const float c = std::cos(j * dTheta);
            const float s = std::sin(j * dTheta);

            Vector4 pos      = { r * c, y, r * s,0 };
            Vector2 texcoord = { static_cast<float>(j) / numSlices, 1.0f - static_cast<float>(i) / numStacks};
            Vector4 tangent  = { -s, 0.0f, c,0 };

            const float dr = bottomRadius - topRadius;
            Vector3 biTangent = { dr * c, -height, dr * s };

            Vector3 normal = Vector3::Cross(tangent, biTangent).Normalized();

            vertices.emplace_back(pos, normal, texcoord, tangent);
        }
    }

    const int ringVertexCount = numSlices + 1;

    for (uint32_t i = 0; i < numStacks; i++) {
        for (uint32_t j = 0; j < numSlices; j++) {
            indices.push_back(static_cast<uint16_t>(i * ringVertexCount + j));
            indices.push_back(static_cast<uint16_t>((i + 1) * ringVertexCount + j));
            indices.push_back(static_cast<uint16_t>((i + 1) * ringVertexCount + j + 1));

            indices.push_back(static_cast<uint16_t>(i * ringVertexCount + j));
            indices.push_back(static_cast<uint16_t>((i + 1) * ringVertexCount + j + 1));
            indices.push_back(static_cast<uint16_t>(i * ringVertexCount + j + 1));
        }
    }

    BuildCylinderCap(bottomRadius, topRadius, height, CylinderCap::Top, numSlices, numStacks, vertices, indices);
    BuildCylinderCap(bottomRadius, topRadius, height, CylinderCap::Bottom, numSlices, numStacks, vertices, indices);

    if (reverseWinding) {
        ReverseWinding(indices, vertices);
    }

    return SceneManager::CreateScene(*commandList, vertices, indices);
}

std::shared_ptr<Scene> Cyrex::GeometryGenerator::CreateCone(
    const std::shared_ptr<CommandList>& commandList, 
    float radius, 
    float height,  
    bool reverseWinding)
{
    return CreateCylinder(commandList, 0.0f, radius, height, reverseWinding);
}

std::shared_ptr<Scene> GeometryGenerator::CreateTorus(
    const std::shared_ptr<CommandList>& commandList, 
    float radius, 
    float thickness, 
    uint32_t tessellation,
    bool reverseWinding)
{
    using namespace Cyrex::Math;

    assert(commandList);

    VertexCollection vertices;
    IndexCollection  indices;

    if (tessellation < 3)
        throw std::out_of_range("tessellation parameter out of range");

   const size_t stride = static_cast<size_t>(tessellation) + 1;

    //loop around the main ring of the torus
    for (size_t i = 0; i <= tessellation; i++) {
        const float u           = static_cast<float>(i) / tessellation;
        const float outerAngle = i * MathConstants::PI_MUL2 / tessellation - MathConstants::PI_DIV2;

        const Matrix transform = Matrix::CreateTranslation({ radius, 0, 0 })* Matrix::RotationY(outerAngle);

        //loop along the other axis
        for (size_t j = 0; j <= tessellation; j++) {
            const float v          = 1 - static_cast<float>(j) / tessellation;
            const float innerAngle = j * MathConstants::PI_MUL2 / tessellation + MathConstants::PI_FLOAT;

            const float dx = std::cos(innerAngle);
            const float dy = std::sin(innerAngle);

            //Create a vertex
            Vector4 normal = { dx, dy, 0, 0 };
            Vector4 pos      = normal * thickness / 2;
            Vector2 texCoord = { u, v };

            pos    = Vector4::Transform(pos, transform);
            normal = Vector4::TransformNormal(normal, transform);

            vertices.emplace_back(pos, normal, texCoord);

            //create indices for two triangles
            const size_t nextI = (i + 1) % stride;
            const size_t nextJ = (j + 1) % stride;

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
        Vertex(Vector4(-0.5f * width, 0.0f,  0.5f * height,0), Vector4(0.0f, 1.0f, 0.0f,0), Vector2(0.0f, 0.0f)),
        Vertex(Vector4(0.5f  * width, 0.0f,  0.5f * height,0), Vector4(0.0f, 1.0f, 0.0f,0), Vector2(1.0f, 0.0f)),
        Vertex(Vector4(0.5f  * width, 0.0f, -0.5f * height,0), Vector4(0.0f, 1.0f, 0.0f,0), Vector2(1.0f, 1.0f)),
        Vertex(Vector4(-0.5f * width, 0.0f, -0.5f * height,0), Vector4(0.0f, 1.0f, 0.0f,0), Vector2(0.0f, 1.0f)),
    };

    IndexCollection indices = { 1, 3, 0, 2, 3, 1 };

    if (reverseWinding) {
        ReverseWinding(indices, vertices);
    }

    return SceneManager::CreateScene(*commandList, vertices, indices);
}
