#pragma once

#include "Core/Visitor.h"

namespace Cyrex {
    class CommandList;

    class SceneVisitor : public IVisitor {
    public:
        SceneVisitor(CommandList& commandList);

        void Visit(Scene& scene) override {}
        void Visit(SceneNode& sceneNode) override {}
        void Visit(Mesh& mesh) override;
    private:
        CommandList& m_commandList;
    };
}