#pragma once
namespace Cyrex {
    class Scene;
    class SceneNode;
    class Mesh;

    class IVisitor {
    public:
        IVisitor() = default;
        virtual ~IVisitor() = default;

        virtual void Visit(Scene& scene) = 0;
        virtual void Visit(SceneNode& sceneNode) = 0;
        virtual void Visit(Mesh& mesh) = 0;
    };
}