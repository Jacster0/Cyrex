#include "SceneVisitor.h"
#include "SceneNode.h"
#include "Scene.h"
#include "Camera.h"
#include "EffectPSO.h"
#include "Material.h"

#include "API/DX12/CommandList.h"
#include "Mesh.h"

using namespace Cyrex;

SceneVisitor::SceneVisitor(CommandList& commandList, const Camera& camera, EffectPSO& pso, RenderPass transparentPass)
    :
    m_commandList(commandList),
    m_camera(camera),
    m_lightingPSO(pso),
    m_renderPass(transparentPass)
{}

void SceneVisitor::Visit(Scene& scene) {
    m_lightingPSO.SetViewMatrix(m_camera.GetView());
    m_lightingPSO.SetProjectionMatrix(m_camera.GetProj());
}

void SceneVisitor::Visit(SceneNode& sceneNode) {
    auto mat = sceneNode.GetWorldTransform();
    m_lightingPSO.SetWorldMatrix(*(Math::Matrix*)(&mat));
}

void SceneVisitor::Visit(Mesh& mesh) {
    auto material = mesh.GetMaterial();

    if (m_renderPass == RenderPass::Opaque) {
        if (!material->IsTransparent()) {
            m_lightingPSO.SetMaterial(material);

            m_lightingPSO.Apply(m_commandList);
            mesh.Render(m_commandList);
        }
    }

    else if (m_renderPass == RenderPass::Transparent) {
        if (material->IsTransparent()) {
            m_lightingPSO.SetMaterial(material);

            m_lightingPSO.Apply(m_commandList);
            mesh.Render(m_commandList);
        }
    }
}
