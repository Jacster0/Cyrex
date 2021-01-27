#pragma once
#include "Mesh.h"
#include <memory>

namespace Cyrex {
    class CommandList;
    class Scene;
    class GeometryGenerator {
    public:
        static std::shared_ptr<Scene> CreateCube(
            const std::shared_ptr<CommandList>& commandList,
            float size = 1.0f, bool reverseWinding = false);

        static std::shared_ptr<Scene> CreateSphere(
            const std::shared_ptr<CommandList>& commandList,
            float radius = 0.5f, 
            uint32_t tessellation = 16,
            bool reverseWinding = false);

        static std::shared_ptr<Scene> CreateCone(
            const std::shared_ptr<CommandList>& commandList,
            float radius = 0.5, 
            float height = 1.0f, 
            uint32_t tessellationbool = 32,
            bool reverseWinding = false);

        static std::shared_ptr<Scene> CreateTorus(
            const std::shared_ptr<CommandList>& commandList,
            float radius = 0.5f, 
            float thickness = 0.333f, 
            uint32_t tessellation = 32,
            bool reverseWinding = false);

        static std::shared_ptr<Scene> CreatePlane(
            const std::shared_ptr<CommandList>& commandList,
            float width = 1.0f, 
            float height = 1.0f,
            bool reverseWinding = false);
    };
}
