#include "SceneVisitor.h"
#include "SceneNode.h"
#include "Scene.h"
#include "Camera.h"
#include "EffectPSO.h"
#include "Material.h"

#include "API/DX12/CommandList.h"
#include "API/DX12/Mesh.h"

using namespace Cyrex;

SceneVisitor::SceneVisitor(CommandList& commandList, const Camera& camera, EffectPSO& pso, TransparentPass transparentPass)
    :
    m_commandList(commandList),
    m_camera(camera),
    m_lightingPSO(pso),
    m_transparentPass(transparentPass)
{}

void SceneVisitor::Visit(Scene& scene) {
    m_lightingPSO.SetViewMatrix(m_camera.GetView());
    m_lightingPSO.SetProjectionMatrix(m_camera.GetProj());
}

void SceneVisitor::Visit(SceneNode& sceneNode) {
    m_lightingPSO.SetWorldMatrix(sceneNode.GetWorldTransform());
}

void SceneVisitor::Visit(Mesh& mesh) {
    auto material = mesh.GetMaterial();

    if (material->IsTransparent() == static_cast<bool>(m_transparentPass)) {
        m_lightingPSO.SetMaterial(material);

        m_lightingPSO.Apply(m_commandList);
        mesh.Render(m_commandList);
    }
}
