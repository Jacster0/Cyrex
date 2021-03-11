#pragma once

#include <memory>

namespace Cyrex {
    struct DefaultUIOptions {
        int ProgressbarWindowFlags;
    };

    class CommandList;
    class EditorLayer;
    class Graphics;
   
    class EditorContext {
    public:
        static void Render(Graphics& gfx, const std::shared_ptr<EditorLayer>& editorLayer, CommandList& cmdList) noexcept;
    };
}