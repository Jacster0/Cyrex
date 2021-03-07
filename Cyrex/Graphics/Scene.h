#pragma once

#include <DirectXCollision.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <map>
#include <string>

struct aiMaterial;
struct aiMesh;
struct aiNode;
struct aiScene;

namespace Cyrex {
    class CommandList;
    class Device;
    class SceneNode;
    class Mesh;
    class Material;
    class IVisitor;

    class Scene {
    public:
        Scene() = default;
        ~Scene() = default;

        std::shared_ptr<SceneNode> GetRootNode() const noexcept { return m_rootNode; }
        void SetRootNode(std::shared_ptr<SceneNode> node) noexcept { m_rootNode = node; }

        DirectX::BoundingBox GetAABB() const noexcept;
        virtual void Accept(IVisitor& visitor);

        bool LoadSceneFromFile(CommandList& commandList, const std::string& fileName, const std::function<bool(float)>& loadingProgress);
        bool LoadSceneFromString(CommandList& commandList, const std::string& sceneString, const std::string format);
    private:
        void ImportScene(CommandList& commandList, const aiScene& scene, std::filesystem::path parentPath);
        void ImportMaterial(CommandList& commandList, const aiMaterial& material, std::filesystem::path parentPath);
        void ImportMesh(CommandList& commandList, const aiMesh& aiMesh);
        std::shared_ptr<SceneNode> ImportSceneNode(std::shared_ptr<SceneNode> parent, const aiNode* aiNode);

        using MaterialMap = std::map<std::string, std::shared_ptr<Material>>;
        using MaterialList = std::vector<std::shared_ptr<Material>>;
        using MeshList = std::vector<std::shared_ptr<Mesh>>;

        MaterialMap  m_materialMap;
        MaterialList m_materials;
        MeshList     m_meshes;

        std::shared_ptr<SceneNode> m_rootNode;

        std::wstring m_sceneFile;
    };
}