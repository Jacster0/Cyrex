#pragma once
#include <vector>
#include <memory>
#include <string>
#include <functional>

namespace Cyrex {
  
    class Scene;
    class VertexPositionNormalTangentBitangentTexture;
    class CommandList;

    class SceneManager {
        using VertexCollection = std::vector<VertexPositionNormalTangentBitangentTexture>;
        using IndexCollection = std::vector<uint16_t>;
    public:
        [[nodiscard("Loading a scene without using it is a waste of time and memory.")]] 
        static std::shared_ptr<Scene> LoadSceneFromFile(
            CommandList& commandList, 
            const std::string& fileName, 
            const std::function<bool(float)>& loadingProgress) noexcept;

        [[nodiscard("Loading a scene without using it is a waste of time and memory.")]] 
        static std::shared_ptr<Scene> LoadSceneFromString(
            CommandList& commandList, 
            const std::string& sceneString, 
            const std::string& format) noexcept;

        [[nodiscard("Creating a scene without using it is a waste of time and memory.")]]
        static std::shared_ptr<Scene> CreateScene(
            CommandList& commandList, 
            const VertexCollection& vertices, 
            const IndexCollection& indices) noexcept;
    };
}
