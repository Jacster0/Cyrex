#pragma once
#include "Core/Visitor.h"

namespace Cyrex {
    class CommandList;
    class Camera;
    class EffectPSO;
    class SceneVisitor : public IVisitor {
    public:
        SceneVisitor(CommandList& commandList, const Camera& camera, EffectPSO& pso, bool transparent);

        void Visit(Scene& scene) override;
        void Visit(SceneNode& sceneNode) override;
        void Visit(Mesh& mesh) override;
    private:
        CommandList& m_commandList;
        const Camera& m_camera;
        EffectPSO& m_lightingPSO;
        bool m_transparentPass;
    };
}