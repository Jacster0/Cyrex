#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <DirectXMath.h>
#include <DirectXCollision.h>

namespace Cyrex {
    class Mesh;
    class CommandList;
    class IVisitor;

    class SceneNode : public std::enable_shared_from_this<SceneNode> {
    public:
        explicit SceneNode(const DirectX::XMMATRIX& localTransform = DirectX::XMMatrixIdentity());
        virtual ~SceneNode();

        const std::string& GetName() const noexcept;
        void SetName(const std::string name) noexcept;

        DirectX::XMMATRIX GetLocalTransform() const noexcept;
        void SetLocalTransform(const DirectX::XMMATRIX& localTransform);

        DirectX::XMMATRIX GetInverseLocalTransform() const noexcept;

        DirectX::XMMATRIX GetWorldTransform() const noexcept;
        DirectX::XMMATRIX GetInverseWorldTransform() const noexcept;

        void AddChild(std::shared_ptr<SceneNode> childNode);
        void RemoveChild(std::shared_ptr<SceneNode> childNode);
        void SetParent(std::shared_ptr<SceneNode> parentNode);

        size_t AddMesh(std::shared_ptr<Mesh> mesh);
        void RemoveMesh(std::shared_ptr<Mesh> mesh);

        std::shared_ptr<Mesh> GetMesh(size_t index) noexcept;

        const DirectX::BoundingBox& GetAABB() const noexcept;

        void Accept(IVisitor& visitor);
    protected:
        DirectX::XMMATRIX GetParentWorldTransform() const noexcept;
    private:
        using NodePtr     = std::shared_ptr<SceneNode>;
        using NodeList    = std::vector<NodePtr>;
        using NodeNameMap = std::multimap<std::string, NodePtr>;
        using MeshList    = std::vector<std::shared_ptr<Mesh>>;

        std::string m_name{ "SceneNode" };

        struct alignas(16) AlignedData
        {
            DirectX::XMMATRIX LocalTransform;
            DirectX::XMMATRIX InverseTransform;
        } *m_alignedData;

        std::weak_ptr<SceneNode> m_parentNode;

        NodeList                 m_children;
        NodeNameMap              m_childrenByName;
        MeshList                 m_meshes;

        DirectX::BoundingBox m_AABB{ { 0, 0, 0 }, {0, 0, 0} };
    };
}