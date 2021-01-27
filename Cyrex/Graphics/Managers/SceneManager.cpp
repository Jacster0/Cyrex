#include "SceneManager.h"
#include "Graphics/Scene.h"
#include "Graphics/SceneNode.h"
#include "Graphics/Material.h"
#include "Graphics/API/DX12/CommandList.h"
#include "Graphics/API/DX12/Mesh.h"


std::shared_ptr<Cyrex::Scene> Cyrex::SceneManager::LoadSceneFromFile(
    CommandList& commandList,
    const std::string& fileName, 
    const std::function<bool(float)>& loadingProgress) noexcept
{
    auto scene = std::make_shared<Scene>();

    if(scene->LoadSceneFromFile(commandList, fileName, loadingProgress)) [[likely]] {
        return scene;
    }
    return nullptr;
}

std::shared_ptr<Cyrex::Scene> Cyrex::SceneManager::LoadSceneFromString(
    CommandList& commandList,
    const std::string& sceneString, 
    const std::string& format) noexcept 
{
    auto scene = std::make_shared<Scene>();

    if (scene->LoadSceneFromString(commandList, sceneString, format)) [[likely]] {
        return scene;
    }
    return nullptr;
}

std::shared_ptr<Cyrex::Scene> Cyrex::SceneManager::CreateScene(
    CommandList& commandList,
    const VertexCollection& vertices, 
    const IndexCollection& indices) noexcept
{
    if (vertices.empty()) [[unlikely]] {
        return nullptr;
    }

    auto vertexBuffer = commandList.CopyVertexBuffer(vertices);
    auto indexBuffer  = commandList.CopyIndexBuffer(indices);

    auto sceneNode = std::make_shared<SceneNode>();
    auto material  = std::make_shared<Material>(Material::White);
    auto scene     = std::make_shared<Scene>();
    auto mesh      = std::make_shared<Mesh>();
 
    mesh->SetVertexBuffer(0, vertexBuffer);
    mesh->SetIndexBuffer(indexBuffer);
    mesh->SetMaterial(material);

    sceneNode->AddMesh(mesh);
    scene->SetRootNode(sceneNode);

    return scene;
}
