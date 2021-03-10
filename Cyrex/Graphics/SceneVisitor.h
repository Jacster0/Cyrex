#pragma once
#include "Core/Visitor.h"

namespace Cyrex {
    enum class RenderPass { Opaque, Transparent};

    class CommandList;
    class Camera;
    class EffectPSO;
    class SceneVisitor : public IVisitor {
    public:
        SceneVisitor(CommandList& commandList, const Camera& camera, EffectPSO& pso, RenderPass transparentPass);

        void Visit(Scene& scene) override;
        void Visit(SceneNode& sceneNode) override;
        void Visit(Mesh& mesh) override;
    private:
        CommandList& m_commandList;
        const Camera& m_camera;
        EffectPSO& m_lightingPSO;
        RenderPass m_renderPass;
    };
}