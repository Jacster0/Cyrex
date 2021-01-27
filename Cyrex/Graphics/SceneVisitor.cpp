#include "SceneVisitor.h"
#include "Graphics/API/DX12/CommandList.h"
#include "Graphics/API/DX12/Mesh.h"

Cyrex::SceneVisitor::SceneVisitor(CommandList& commandList)
    :
    m_commandList(commandList)
{}

void Cyrex::SceneVisitor::Visit(Mesh& mesh) {
    mesh.Render(m_commandList);
}
