#include "Scene.h"
#include "Graphics/API/DX12/CommandList.h"
#include "Graphics/API/DX12/Device.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/API/DX12/Texture.h"
#include "Material.h"
#include "SceneNode.h"
#include "API/DX12/VertexTypes.h"
#include "Core/Visitor.h"
#include "Managers/TextureManager.h"

#include "Core/Filesystem/FileSystem.h"
#include "Core/Utils/StringUtils.h"

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"

#include "assimp/aabb.h"
#include "assimp/Importer.hpp"
#include "assimp/Exporter.hpp"
#include "assimp/ProgressHandler.hpp"
#include "assimp/anim.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/config.h"

namespace dx = DirectX;
namespace cx = Cyrex;

using namespace Cyrex;
using namespace Cyrex::Math;

class ProgressHandler : public Assimp::ProgressHandler {
public:
    ProgressHandler(const cx::Scene& scene, const std::function<bool(float)>& progressCB)
        :
        m_scene(scene),
        m_progressCB(progressCB)
    {}
    virtual bool Update(float percentage) noexcept override  {
        if (m_progressCB) {
            return m_progressCB(percentage);
        }
        return true;
    }
private:
    const cx::Scene& m_scene;
    std::function<bool(float)> m_progressCB;
};

//Helper function to create an DirectX::BoundingBox from an aiAABB.
inline DirectX::BoundingBox CreateBoundingBox(const aiAABB& aabb) {
    auto min = dx::XMVectorSet(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z, 1.0f);
    auto max = dx::XMVectorSet(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z, 1.0f);

    DirectX::BoundingBox bb;

    DirectX::BoundingBox::CreateFromPoints(bb, min, max);

    return bb;
}

DirectX::BoundingBox Cyrex::Scene::GetAABB() const noexcept {
    DirectX::BoundingBox aabb{ { 0, 0, 0 }, { 0, 0, 0 } };

    if (m_rootNode) {
        aabb = m_rootNode->GetAABB();
    }
    return aabb;
}

void Cyrex::Scene::Accept(IVisitor& visitor) {
    visitor.Visit(*this);

    if (m_rootNode) {
        m_rootNode->Accept(visitor);
    }
}

bool Cyrex::Scene::LoadSceneFromFile(CommandList& commandList, const std::string& fileName, const std::function<bool(float)>& loadingProgress) {
    auto exportPath = FileSystem::ReplaceExtension(fileName, "assbin");

    std::string parentPath;

    if (FileSystem::HasParentPath(fileName)) {
        parentPath = FileSystem::GetParentDirectory(fileName);
    }
    else {
        parentPath = FileSystem::GetWorkingDirectory();
    }

    Assimp::Importer importer;
    const aiScene* scene;

    importer.SetProgressHandler(new ProgressHandler(*this, loadingProgress));

    //check if a preprocessed file exists
    if (FileSystem::IsFile(exportPath)) {
        scene = importer.ReadFile(exportPath, aiProcess_GenBoundingBoxes);
    }

    else {
        //File has not been preprocessed yet. Import and process the file.
        importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

        unsigned int preprocessFlag = 
            aiProcessPreset_TargetRealtime_MaxQuality | 
            aiProcess_OptimizeGraph                   | 
            aiProcess_ConvertToLeftHanded             | 
            aiProcess_GenBoundingBoxes;
        
        scene = importer.ReadFile(fileName, preprocessFlag);

        if (scene) {
            // Export the preprocessed scene file for faster loading next time.
            Assimp::Exporter exporter;
            exporter.Export(scene, "assbin", exportPath, 0);
        }
    }

    if (!scene) {
        return false;
    }

    ImportScene(commandList, *scene, parentPath);
    return true;
}

bool Cyrex::Scene::LoadSceneFromString(CommandList& commandList, const std::string& sceneString, const std::string format) {
    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

    unsigned int preprocessFlags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded | aiProcess_GenBoundingBoxes;

    scene = importer.ReadFileFromMemory(sceneString.data(), sceneString.length(), preprocessFlags, format.c_str());

    if (!scene) { 
        return false; 
    }

    ImportScene(commandList, *scene, FileSystem::GetWorkingDirectory());
    return true;
}

void Cyrex::Scene::ImportScene(CommandList& commandList, const aiScene& scene, const std::string& parentPath) {
    if (m_rootNode) {
        m_rootNode.reset();
    }

    m_materialMap.clear();
    m_materials.clear();
    m_meshes.clear();

    //Inport scene materials
    for (auto i = 0; i < scene.mNumMaterials; i++) {
        ImportMaterial(commandList, *(scene.mMaterials[i]), parentPath);
    }
    //import meshes
    for (auto i = 0; i < scene.mNumMeshes; i++) {
        ImportMesh(commandList, *(scene.mMeshes[i]));
    }

    //Import the root node
    m_rootNode = ImportSceneNode(nullptr, scene.mRootNode);
}

void Cyrex::Scene::ImportMaterial(CommandList& commandList, const aiMaterial& material, const std::string& parentPath) {
    aiString materialName;
    aiString aiTexturePath;

    aiTextureOp aiBlendOperation;
    float blendFactor;

    aiColor4D diffuseColor;
    aiColor4D specularColor;
    aiColor4D ambientColor;
    aiColor4D emissiveColor;

    float opacity;
    float indexOfRefraction;
    float reflectivity;
    float shininess;
    float bumpIntensity;

    auto pMaterial = std::make_shared<Material>();

    if (material.Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == aiReturn_SUCCESS) {
        pMaterial->SetAmbientColor({ ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a });
    }
    if (material.Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == aiReturn_SUCCESS) {
        pMaterial->SetEmissiveColor({ emissiveColor.r,emissiveColor.g,emissiveColor.b, emissiveColor.a });
    }
    if (material.Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == aiReturn_SUCCESS) {
        pMaterial->SetDiffuseColor({ diffuseColor.r,diffuseColor.g,diffuseColor.b, diffuseColor.a });
    }
    if (material.Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == aiReturn_SUCCESS) {
        pMaterial->SetSpecularColor({ specularColor.r,specularColor.g,specularColor.b, specularColor.a });
    }
    if (material.Get(AI_MATKEY_SHININESS, shininess) == aiReturn_SUCCESS) {
        pMaterial->SetSpecularPower(shininess);
    }
    if (material.Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS) {
        pMaterial->SetOpacity(opacity);
    }
    if (material.Get(AI_MATKEY_REFRACTI, indexOfRefraction) == aiReturn_SUCCESS) {
        pMaterial->SetIndexOfRefraction(indexOfRefraction);
    }
    if (material.Get(AI_MATKEY_REFLECTIVITY, reflectivity) == aiReturn_SUCCESS) {
        pMaterial->SetReflectance({ reflectivity, reflectivity, reflectivity, reflectivity });
    }
    if (material.Get(AI_MATKEY_BUMPSCALING, bumpIntensity) == aiReturn_SUCCESS) {
        pMaterial->SetBumpIntensity(bumpIntensity);
    }

    //Load ambient texture
    if (material.GetTextureCount(aiTextureType_AMBIENT) > 0 &&
        material.GetTexture(aiTextureType_AMBIENT, 0, &aiTexturePath, nullptr, nullptr,
                            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS)
    {
        auto texture = TextureManager::LoadTextureFromFile(
            commandList, 
            FileSystem::Append(parentPath, aiTexturePath.C_Str()), 
            true);
        pMaterial->SetTexture(Material::TextureType::Ambient, texture);
    }

    //Load emissive texture
    if (material.GetTextureCount(aiTextureType_EMISSIVE) > 0 &&
        material.GetTexture(aiTextureType_EMISSIVE, 0, &aiTexturePath, nullptr, nullptr,
            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS)
    {
        auto texture = TextureManager::LoadTextureFromFile(
            commandList, 
            FileSystem::Append(parentPath, aiTexturePath.C_Str()), 
            true);
        pMaterial->SetTexture(Material::TextureType::Emissive, texture);
    }

    //Load diffuse texture
    if (material.GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
        material.GetTexture(aiTextureType_DIFFUSE, 0, &aiTexturePath, nullptr, nullptr,
            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS)
    {
        auto texture = TextureManager::LoadTextureFromFile(
            commandList, 
            FileSystem::Append(parentPath, aiTexturePath.C_Str()), 
            true);
        pMaterial->SetTexture(Material::TextureType::Diffuse, texture);
    }

    //Load specular texture
    if (material.GetTextureCount(aiTextureType_SPECULAR) > 0 &&
        material.GetTexture(aiTextureType_SPECULAR, 0, &aiTexturePath, nullptr, nullptr,
            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS)
    {
        auto texture = TextureManager::LoadTextureFromFile(
            commandList, 
            FileSystem::Append(parentPath, aiTexturePath.C_Str()), 
            true);
        pMaterial->SetTexture(Material::TextureType::Specular, texture);
    }

    //Load specular power texture
    if (material.GetTextureCount(aiTextureType_SHININESS) > 0 &&
        material.GetTexture(aiTextureType_SHININESS, 0, &aiTexturePath, nullptr, nullptr,
            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS)
    {
        auto texture = TextureManager::LoadTextureFromFile(
            commandList, 
            FileSystem::Append(parentPath, aiTexturePath.C_Str()), 
            false);
        pMaterial->SetTexture(Material::TextureType::SpecularPower, texture);
    }

    //Load opacity texture
    if (material.GetTextureCount(aiTextureType_OPACITY) > 0 &&
        material.GetTexture(aiTextureType_OPACITY, 0, &aiTexturePath, nullptr, nullptr,
            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS)
    {
        auto texture = TextureManager::LoadTextureFromFile(
            commandList, 
            FileSystem::Append(parentPath, aiTexturePath.C_Str()), 
            false);
        pMaterial->SetTexture(Material::TextureType::Opacity, texture);
    }

    //Load normal map texture
    if (material.GetTextureCount(aiTextureType_NORMALS) > 0 &&
        material.GetTexture(aiTextureType_NORMALS, 0, &aiTexturePath, nullptr, nullptr,
            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS)
    {
        auto texture = TextureManager::LoadTextureFromFile(
            commandList, 
            FileSystem::Append(parentPath, aiTexturePath.C_Str()),
            false);
        pMaterial->SetTexture(Material::TextureType::Normal, texture);
    }

    //If there is no normal map, load bump map texture
    else if (material.GetTextureCount(aiTextureType_HEIGHT) > 0 &&
                                      material.GetTexture(
                                      aiTextureType_HEIGHT, 0, &aiTexturePath, 
                                      nullptr, nullptr, &blendFactor) == aiReturn_SUCCESS) 
    {
        auto texture = TextureManager::LoadTextureFromFile(
            commandList, 
            FileSystem::Append(parentPath, aiTexturePath.C_Str()), 
            false);

        //Some materials store normal maps in the bump map slot and Assimp can't tell the difference bewteen
        //those two texture type, so we are making a assumption whether the texture is a normal or bump map based
        //on the pixel depth. Bump maps are usually 8 BPP and normals are usually 24 BPP and higher.
        auto textureType = (texture->BitsPerPixel() >= 24) ? Material::TextureType::Normal : Material::TextureType::Bump;

        pMaterial->SetTexture(textureType, texture);
    }
    m_materials.push_back(pMaterial);
}

void Cyrex::Scene::ImportMesh(CommandList& commandList, const aiMesh& aiMesh) {
    auto mesh = std::make_shared<Mesh>();

    std::vector<cx::VertexPositionNormalTangentBitangentTexture> vertexData(aiMesh.mNumVertices);

    assert(aiMesh.mMaterialIndex < m_materials.size());
    mesh->SetMaterial(m_materials.at(aiMesh.mMaterialIndex));

    uint32_t index;

    //Process vertices
    if (aiMesh.HasPositions()) {
        for (index = 0; index < aiMesh.mNumVertices; index++) {
            vertexData[index].Position = *reinterpret_cast<Vector3*>(&aiMesh.mVertices[index]);
        }
    }
    if (aiMesh.HasNormals()) {
        for (index = 0; index < aiMesh.mNumVertices; index++) {
            vertexData[index].Normal = *reinterpret_cast<Vector3*>(&aiMesh.mNormals[index]);
        }
    }

    if (aiMesh.HasTangentsAndBitangents()) {
        for (index = 0; index < aiMesh.mNumVertices; index++) {
            vertexData[index].Tangent   = *reinterpret_cast<Vector3*>(&aiMesh.mTangents[index]);
            vertexData[index].Bitangent = *reinterpret_cast<Vector3*>(&aiMesh.mBitangents[index]);
        }
    }

    if (aiMesh.HasTextureCoords(0)) {
        for (index = 0; index < aiMesh.mNumVertices; index++) {
            vertexData[index].TexCoord = *reinterpret_cast<Vector2*>(&aiMesh.mTextureCoords[0][index]);
        }
    }

    const auto vertexBuffer = commandList.CopyVertexBuffer(vertexData);
    mesh->SetVertexBuffer(0, vertexBuffer);

    //process indices
    if (aiMesh.HasFaces()) {
        std::vector<uint32_t> indices;

        for (index = 0; index < aiMesh.mNumFaces; index++) {
            const aiFace& face = aiMesh.mFaces[index];

            // Only extract triangular faces
            if (face.mNumIndices == 3) {
                indices.push_back(face.mIndices[0]);
                indices.push_back(face.mIndices[1]);
                indices.push_back(face.mIndices[2]);
            }
        }

        if (indices.size() > 0) {
            const auto indexBuffer = commandList.CopyIndexBuffer(indices);
            mesh->SetIndexBuffer(indexBuffer);
        }
    }

    mesh->SetAABB(CreateBoundingBox(aiMesh.mAABB));

    m_meshes.push_back(mesh);
}

std::shared_ptr<Cyrex::SceneNode> Cyrex::Scene::ImportSceneNode(std::shared_ptr<SceneNode> parent, const aiNode* aiNode) {
    if (!aiNode) {
        return nullptr;
    }

    auto node = std::make_shared<SceneNode>(*reinterpret_cast<const Matrix*>(&aiNode->mTransformation));
    node->SetParent(parent);

    if (aiNode->mName.length > 0) {
        node->SetName(aiNode->mName.C_Str());
    }

    //Add meshes to scene node
    for (unsigned i = 0; i < aiNode->mNumMeshes; i++) {
        assert(aiNode->mMeshes[i] < m_meshes.size());

        auto pMesh = m_meshes.at(aiNode->mMeshes[i]);
        node->AddMesh(pMesh);
    }

    //Recursively Import children
    for (unsigned i = 0; i < aiNode->mNumChildren; i++) {
        const auto child = ImportSceneNode(node, aiNode->mChildren[i]);
        node->AddChild(child);
    }
    return node;
}
