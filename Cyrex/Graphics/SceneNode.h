#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <DirectXCollision.h>

#include "Core/Math/Matrix.h"

namespace Cyrex {
    class Mesh;
    class CommandList;
    class IVisitor;

    class SceneNode : public std::enable_shared_from_this<SceneNode> {
    public:
        explicit SceneNode(const Cyrex::Math::Matrix& localTransform = Cyrex::Math::Matrix());
        virtual ~SceneNode();

        const std::string& GetName() const noexcept;
        void SetName(const std::string name) noexcept;

        Cyrex::Math::Matrix GetLocalTransform() const noexcept;
        void SetLocalTransform(const Cyrex::Math::Matrix& localTransform);

        Cyrex::Math::Matrix GetInverseLocalTransform() const noexcept;

        Cyrex::Math::Matrix GetWorldTransform() const noexcept;
        Cyrex::Math::Matrix GetInverseWorldTransform() const noexcept;

        void AddChild(std::shared_ptr<SceneNode> childNode);
        void RemoveChild(std::shared_ptr<SceneNode> childNode);
        void SetParent(std::shared_ptr<SceneNode> parentNode);

        size_t AddMesh(std::shared_ptr<Mesh> mesh);
        void RemoveMesh(std::shared_ptr<Mesh> mesh);

        std::shared_ptr<Mesh> GetMesh(size_t index = 0) noexcept;

        const DirectX::BoundingBox& GetAABB() const noexcept;

        void Accept(IVisitor& visitor);
    protected:
        Cyrex::Math::Matrix GetParentWorldTransform() const noexcept;
    private:
        using NodePtr     = std::shared_ptr<SceneNode>;
        using NodeList    = std::vector<NodePtr>;
        using NodeNameMap = std::multimap<std::string, NodePtr>;
        using MeshList    = std::vector<std::shared_ptr<Mesh>>;

        std::string m_name{ "SceneNode" };

        struct AlignedData
        {
            Cyrex::Math::Matrix LocalTransform;
            Cyrex::Math::Matrix InverseTransform;
        }* m_alignedData;

        std::weak_ptr<SceneNode> m_parentNode;

        NodeList m_children;
        NodeNameMap m_childrenByName;
        MeshList m_meshes;

        DirectX::BoundingBox m_AABB{ { 0, 0, 0 }, {0, 0, 0} };
    };
}