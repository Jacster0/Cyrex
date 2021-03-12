#include "SceneNode.h"
#include "API/DX12/Mesh.h"
#include "Core/Visitor.h"

namespace dx = DirectX;

Cyrex::SceneNode::SceneNode(const DirectX::XMMATRIX& localTransform) {
    m_alignedData = new AlignedData();
}

Cyrex::SceneNode::~SceneNode() {
    delete m_alignedData;
}

const std::string& Cyrex::SceneNode::GetName() const noexcept {
    return m_name;
}

void Cyrex::SceneNode::SetName(const std::string name) noexcept {
    m_name = name;
}

DirectX::XMMATRIX Cyrex::SceneNode::GetLocalTransform() const noexcept {
    return m_alignedData->LocalTransform;
}

void Cyrex::SceneNode::SetLocalTransform(const DirectX::XMMATRIX& localTransform) {
    m_alignedData->LocalTransform = localTransform;
}

DirectX::XMMATRIX Cyrex::SceneNode::GetInverseLocalTransform() const noexcept {
    return m_alignedData->InverseTransform;
}

DirectX::XMMATRIX Cyrex::SceneNode::GetWorldTransform() const noexcept {
    return m_alignedData->LocalTransform * GetParentWorldTransform();
}

DirectX::XMMATRIX Cyrex::SceneNode::GetInverseWorldTransform() const noexcept {
    return dx::XMMatrixInverse(nullptr, GetWorldTransform());
}

void Cyrex::SceneNode::AddChild(std::shared_ptr<SceneNode> childNode) {
    if (childNode) {
        NodeList::const_iterator nodeListIter = std::find(m_children.begin(), m_children.end(), childNode);

        if (nodeListIter == m_children.cend()) {
            childNode->m_parentNode = shared_from_this();

            auto worldTransform = childNode->GetWorldTransform();
            auto localTransform = worldTransform * GetInverseWorldTransform();
            childNode->SetLocalTransform(localTransform);

            m_children.push_back(childNode);

            if (!childNode->GetName().empty()) {
                m_childrenByName.emplace(childNode->GetName(), childNode);
            }
        }
    }
}

void Cyrex::SceneNode::RemoveChild(std::shared_ptr<SceneNode> childNode) {
    if (childNode) {
        NodeList::const_iterator nodeListIter = std::find(m_children.begin(), m_children.end(), childNode);

        if (nodeListIter != m_children.cend()) {
            childNode->SetParent(nullptr);
            m_children.erase(nodeListIter);

            auto childNameMapIter = m_childrenByName.find(childNode->GetName());

            if (childNameMapIter != m_childrenByName.end()) {
                m_childrenByName.erase(childNameMapIter);
           }
        }
        else {
            for (auto child : m_children) {
                child->RemoveChild(childNode);
            }
        }
    }
}

void Cyrex::SceneNode::SetParent(std::shared_ptr<SceneNode> parentNode) {
    std::shared_ptr<SceneNode> me = shared_from_this();

    if (parentNode) {
        parentNode->AddChild(me);
    }
    else if (auto parent = m_parentNode.lock()) {
        auto worldTransform = GetWorldTransform();
        parent->RemoveChild(me);
        m_parentNode.reset();
        SetLocalTransform(worldTransform);
    }
}

size_t Cyrex::SceneNode::AddMesh(std::shared_ptr<Mesh> mesh) {
    size_t index = -1;

    if (mesh) {
        MeshList::const_iterator iter = std::find(m_meshes.begin(), m_meshes.end(), mesh);

        if (iter == m_meshes.end()) {
            index = m_meshes.size();

            m_meshes.push_back(mesh);

            dx::BoundingBox::CreateMerged(m_AABB, m_AABB, mesh->GetAABB());
        }
        else {
            index = iter - m_meshes.begin();
        }
    }
    return index;
}

void Cyrex::SceneNode::RemoveMesh(std::shared_ptr<Mesh> mesh) {
    if (mesh)  {
        MeshList::const_iterator iter = std::find(m_meshes.begin(), m_meshes.end(), mesh);
        if (iter != m_meshes.end()) {
            m_meshes.erase(iter);
        }
    }
}

std::shared_ptr<Cyrex::Mesh> Cyrex::SceneNode::GetMesh(size_t index) noexcept {
    std::shared_ptr<Mesh>  mesh = nullptr;

    if (index < m_meshes.size()) {
        mesh = m_meshes[index];
    }
    return mesh;
}

const DirectX::BoundingBox& Cyrex::SceneNode::GetAABB() const noexcept {
    return m_AABB;
}

void Cyrex::SceneNode::Accept(IVisitor& visitor) {
    visitor.Visit(*this);

    // Visit meshes
    for (auto& mesh : m_meshes) {
        mesh->Accept(visitor);
    }

    // Visit children
    for (auto& child : m_children) {
        child->Accept(visitor);
    }
}

DirectX::XMMATRIX Cyrex::SceneNode::GetParentWorldTransform() const noexcept {
    auto parentTransform = dx::XMMatrixIdentity();

    if (auto parentNode = m_parentNode.lock()) {
        parentTransform = parentNode->GetWorldTransform();
    }
    return parentTransform;
}
